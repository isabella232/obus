/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_socket.c
 *
 * @brief obus socket
 *
 * @author jean-baptiste.dubois@parrot.com
 *
 * Copyright (c) 2013 Parrot S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PARROT COMPANY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#include "obus_header.h"

/* connection attempt interval in ms used by socket clients */
#define OBUS_SOCKET_CONN_INTVL 500

/* socket role */
enum obus_socket_role {
	OBUS_SOCKET_CLIENT,
	OBUS_SOCKET_SERVER,
	OBUS_SOCKET_PEER,
};

/* socket base object */
struct obus_socket_base {
	/* socket loop */
	struct obus_loop *loop;
	/* socket obus fd */
	struct obus_fd ofd;
	/* socket fd */
	int fd;
	/* socket type */
	sa_family_t type;
	/* socket role */
	enum obus_socket_role role;
};

/* socket client object object */
struct obus_socket_client {
	/* socket base */
	struct obus_socket_base sock;
	/* local address */
	struct obus_socket_addr local_addr;
	/* peer address */
	struct obus_socket_addr peer_addr;
	/* socket connection timer */
	struct obus_timer *timer;
	/* socket connected user callback */
	obus_socket_client_connected_cb_t cb;
	/* socket connected user data */
	void *user_data;
	/* client log flag */
	int log;
};

/* socket peer object */
struct obus_socket_peer {
	/* socket peer base */
	struct obus_socket_base sock;
	/* local address */
	struct obus_socket_addr local_addr;
	/* peer address */
	struct obus_socket_addr peer_addr;
	/* peer pid (unix socket) */
	pid_t peer_pid;
	/* peer node in server peers list */
	struct obus_node node;
	/* peer server socket */
	struct obus_socket_server *srv;
};

/* socket server object */
struct obus_socket_server {
	/* server socket base */
	struct obus_socket_base sock;
	/* socket bind timer */
	struct obus_timer *timer;
	/* server address */
	struct obus_socket_addr addr;
	/* server socket peers list */
	struct obus_node peers;
	/* server socket accept user callback */
	obus_socket_server_accept_cb_t cb;
	/* server socket user data*/
	void *user_data;
	/* server log flag */
	int log;
};

static socklen_t obus_socket_addr_length(const struct obus_socket_addr *addr)
{
	socklen_t len;

	switch (addr->base->sa_family) {
	case AF_INET:
		len = sizeof(struct sockaddr_in);
	break;
	case AF_INET6:
		len = sizeof(struct sockaddr_in6);
	break;
	case AF_UNIX:
		len = sizeof(struct sockaddr_un);
	break;
	default:
		len = 0;
	break;
	}

	return len;
}
static void obus_socket_addr_init(struct obus_socket_addr *addr,
				  sa_family_t family)
{
	if (!addr)
		return;

	memset(addr, 0 , sizeof(*addr));
	addr->base = (struct sockaddr *)&addr->st;
	addr->base->sa_family = family;

	switch (family) {
	case AF_INET:
		addr->in = (struct sockaddr_in *)addr->base;
	break;
	case AF_INET6:
		addr->in6 = (struct sockaddr_in6 *)addr->base;
	break;
	case AF_UNIX:
		addr->un = (struct sockaddr_un *)addr->base;
	break;
	default:
	break;
	}
}

static int obus_socket_addr_init_from_string(struct obus_socket_addr *addr,
					     const char *name)
{
	int ret;
	size_t i;
	char *str_addr;
	const char *remain;
	char *port;

	static const struct {
		const char *prefix;
		size_t len;
		sa_family_t type;
	} families[] = {
		{"inet:", 5, AF_INET},
		{"inet6:", 6 , AF_INET6},
		{"unix:", 5, AF_UNIX},
	};


	if (!addr || !name)
		return -EINVAL;

	/* find socket family prefix */
	remain = NULL;
	for (i = 0; i < OBUS_SIZEOF_ARRAY(families); i++) {
		if (strncmp(name, families[i].prefix, families[i].len) != 0)
			continue;

		/* init address */
		obus_socket_addr_init(addr, families[i].type);

		/* handle address */
		remain = name + families[i].len;
		break;
	}

	if (!remain)
		return -EINVAL;

	switch (addr->base->sa_family) {
	case AF_INET:
	case AF_INET6:
		str_addr = strdup(remain);
		if (!str_addr)
			return -EINVAL;

		/* find port separator */
		port = strrchr(str_addr, ':');
		if (!port) {
			free(str_addr);
			return -EINVAL;
		}

		port[0] = '\0';
		port++;
		if (addr->base->sa_family == AF_INET) {
			addr->in->sin_port = htons((uint16_t)atoi(port));
			if (addr->in->sin_port == 0) {
				obus_error("invalid inet port: %s", name);
				ret = -EINVAL;
			} else {
				ret = inet_pton(AF_INET, str_addr,
						&addr->in->sin_addr);
				if (ret <= 0) {
					obus_error("inet address: %s not in "
						   "presentation format",
						   str_addr);
					ret = -EINVAL;
				} else {
					ret = 0;
				}
			}

		} else {
			addr->in6->sin6_port = htons((uint16_t)atoi(port));
			if (addr->in6->sin6_port == 0) {
				obus_error("invalid inet6 port: %s", name);
				ret = -EINVAL;
			} else {
				ret = inet_pton(AF_INET6, str_addr,
						&addr->in6->sin6_addr);
				if (ret <= 0) {
					obus_error("inet6 address: %s not in "
						   "presentation format",
						   str_addr);
					ret = -EINVAL;
				} else {
					ret = 0;
				}
			}
		}

		free(str_addr);
	break;
	case AF_UNIX:
		/* copy sun path */
		snprintf(addr->un->sun_path, sizeof(addr->un->sun_path), "%s",
			 remain);

		/*replace @ to '\0' for abstract socket */
		if (addr->un->sun_path[0] == '@')
			addr->un->sun_path[0] = '\0';
	break;
	default:
		obus_error("invalid socket family %d", addr->base->sa_family);
		return -EINVAL;
		break;
	}

	addr->name = strdup(name);
	return 0;
}

static char *obus_socket_addr_to_string(const struct obus_socket_addr *addr)
{
	const char *tmp;
	char address[INET6_ADDRSTRLEN + 1];
	char name[255];

	name[0] = '\0';

	switch (addr->base->sa_family) {
	case AF_INET:
		/* format address */
		tmp = inet_ntop(AF_INET, &addr->in->sin_addr, address,
				sizeof(address));

		snprintf(name, sizeof(name),"inet:%s:%d", tmp ? tmp : "???",
				ntohs(addr->in->sin_port));
	break;
	case AF_INET6:
		/* format address */
		tmp = inet_ntop(AF_INET6, &addr->in6->sin6_addr, address,
				sizeof(address));

		snprintf(name, sizeof(name), "inet6:%s:%d", tmp ? tmp : "???",
				ntohs(addr->in->sin_port));
	break;
	case AF_UNIX:
		if (addr->un->sun_path[0] == '\0') {
			snprintf(name, sizeof(name), "unix:@%s", &addr->un->sun_path[1]);
		} else {
			snprintf(name, sizeof(name), "unix:%s", addr->un->sun_path);
		}
	break;
	default:
		obus_error("invalid socket family %d", addr->base->sa_family);
	break;
	}

	return strdup(name);
}

static pid_t obus_socket_peer_get_pid(int fd)
{
#ifndef SO_PEERCRED
	return -1;
#else /* SO_PEERCRED */
	struct ucred credits;
	socklen_t len = sizeof(credits);

	/* fill in the user data structure */
	if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &credits, &len) < 0) {
		obus_log_fd_errno("getsockopt(SO_PEERCRED)", fd);
		return 0;
	}

	/* the process ID of the process on the other side of the socket */
	return credits.pid;
#endif /* SO_PEERCRED */
}

static int obus_socket_activate_keepalive(int fd, int keepidle, int keepintvl,
					  int keepcnt)
{
	int ret;
	int keepalive = 1;

	/* activate keep alive on socket */
	ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive,
			sizeof(int));
	if (ret < 0) {
		obus_log_fd_errno("setsockopt(SO_KEEPALIVE)", fd);
		goto out;
	}

#if defined(TCP_KEEPIDLE) && defined(TCP_KEEPINTVL) && defined(TCP_KEEPCNT)
	/*
	 * TCP_KEEPIDLE:
	 * The time (in seconds) the connection needs to remain
	 * idle before TCP starts sending keepalive probes,
	 * if the socket
	 * option SO_KEEPALIVE has been set on this socket. */
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int));
	if (ret < 0) {
		obus_log_fd_errno("setsockopt(TCP_KEEPIDLE)", fd);
		goto out;
	}

	/*
	 * TCP_KEEPINTVL:
	 * The time (in seconds) between individual keepalive
	 * probes. */
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int));
	if (ret < 0) {
		obus_log_fd_errno("setsockopt(TCP_KEEPINTVL)", fd);
		goto out;
	}

	/*
	 * TCP_KEEPCNT:
	 * The maximum number of keepalive probes TCP
	 * should send before dropping the connection */
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int));
	if (ret < 0) {
		obus_log_fd_errno("setsockopt(TCP_KEEPCNT)", fd);
		goto out;
	}
#endif

out:
	return ret;
}

static int obus_socket_base_init(struct obus_socket_base *sock,
				 enum obus_socket_role role,
				 sa_family_t type,
				 struct obus_loop *loop)
{
	if (!sock || !loop)
		return -EINVAL;

	memset(sock, 0, sizeof(struct obus_socket_base));
	sock->type = type;
	sock->role = role;
	sock->loop = obus_loop_ref(loop);
	sock->fd = -1;
	obus_fd_reset(&sock->ofd);
	return 0;
}

static void obus_socket_base_close(struct obus_socket_base *sock)
{
	int ret;

	/* remove fd from loop */
	if (obus_fd_is_used(&sock->ofd))
		obus_loop_remove(sock->loop, &sock->ofd);

	if (sock->fd != -1) {
		/* shutdown socket (do not warn if not connected) */
		ret = shutdown(sock->fd, SHUT_RDWR);
		if (ret < 0 && errno != ENOTCONN)
			obus_log_fd_errno("shutdown", sock->fd);

		/* close socket */
		ret = close(sock->fd);
		if (ret < 0)
			obus_log_fd_errno("close", sock->fd);

		sock->fd = -1;
	}

	obus_fd_reset(&sock->ofd);
}

static int obus_socket_base_destroy(struct obus_socket_base *sock)
{
	obus_socket_base_close(sock);
	obus_loop_unref(sock->loop);
	return 0;
}

static int obus_socket_base_open(struct obus_socket_base *sock)
{
	int ret, sockfd, opt = 1;

	/* create socket stream */
	sockfd = socket(sock->type, SOCK_STREAM, 0);
	if (sockfd < 0) {
		obus_log_errno("socket");
		ret = -errno;
		goto error;
	}

	/* set socket fd close on exec flag */
	ret = obus_fd_set_close_on_exec(sockfd);
	if (ret < 0)
		goto close_socket;

	/* set socket fd non blocking */
	ret = obus_fd_add_flags(sockfd, O_NONBLOCK);
	if (ret < 0)
		goto close_socket;

	/**
	 * allow reuse of socket
	 * tells the kernel that even if this port is busy
	 * go ahead and reuse it anyway (useful if process crashed)
	 */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if (ret < 0) {
		obus_log_fd_errno("setsockopt", sockfd);
		ret = -errno;
		goto close_socket;
	}

	sock->fd = sockfd;
	return 0;

close_socket:
	close(sockfd);
error:
	obus_log_func_error(ret);
	return ret;
}


static void obus_socket_server_events(struct obus_fd *ofd, int events,
				      void *data)
{
	struct obus_socket_server *srv = data;
	struct obus_socket_peer *peer;
	struct obus_socket_addr peer_addr, local_addr;
	pid_t peer_pid = 0;
	socklen_t len;
	int sockfd, ret;

	if (obus_fd_event_error(events)) {
		obus_loop_remove(srv->sock.loop, &srv->sock.ofd);
		return;
	}

	if (!obus_fd_event_read(events))
		return;

	obus_socket_addr_init(&local_addr, srv->addr.base->sa_family);
	obus_socket_addr_init(&peer_addr, srv->addr.base->sa_family);
	len = obus_socket_addr_length(&peer_addr);

	if (srv->log)
		obus_debug("socket server '%s' connection request",
			   srv->addr.name);

	/* accept socket client connection */
	sockfd = accept(srv->sock.fd, peer_addr.base, &len);
	if (sockfd < 0) {
		obus_log_fd_errno("accept", srv->sock.fd);
		return;
	}

	/* set socket fd close on exec flag */
	ret = obus_fd_set_close_on_exec(sockfd);
	if (ret < 0)
		goto close_peer;

	/* set socket non blocking  */
	ret = obus_fd_add_flags(sockfd, O_NONBLOCK);
	if (ret < 0)
		goto close_peer;

	if (peer_addr.base->sa_family == AF_UNIX) {
		/* for unix, recopy sun path (same as server's one) */
		memcpy(peer_addr.un->sun_path, srv->addr.un->sun_path,
		       sizeof(peer_addr.un->sun_path));

		/* get peer pid for unix socket */
		peer_pid = obus_socket_peer_get_pid(sockfd);
		/* allocate peer name memory */
		peer_addr.name = malloc(strlen(srv->addr.name) + 20);
		if (peer_addr.name)
				snprintf(peer_addr.name, len,"%s (pid=%d)", srv->addr.name,
						 peer_pid);
	} else {
		/* get peer socket name */
		peer_addr.name = obus_socket_addr_to_string(&peer_addr);
	}

	/* activate peer keep alive for ip socket */
	if (srv->addr.base->sa_family == AF_INET ||
	    srv->addr.base->sa_family == AF_INET6)
		obus_socket_activate_keepalive(sockfd, 5, 1, 2);

	/* get address of the local socket */
	len = obus_socket_addr_length(&local_addr);

	/* get socket name*/
	ret = getsockname(sockfd, local_addr.base, &len);
	if (ret < 0) {
		obus_log_fd_errno("getsockname", sockfd);
		goto close_peer;
	}

	/* get local socket name */
	local_addr.name = obus_socket_addr_to_string(&local_addr);

	/* create peer socket */
	peer = calloc(1, sizeof(struct obus_socket_peer));
	if (!peer)
		goto close_peer;

	/* init peer socket */
	obus_socket_base_init(&peer->sock, OBUS_SOCKET_PEER,
			      srv->addr.base->sa_family, srv->sock.loop);

	/* set peer socket fd & address */
	peer->sock.fd = sockfd;
	peer->srv = srv;
	peer->local_addr = local_addr;
	peer->peer_addr = peer_addr;
	peer->peer_pid = peer_pid;

	/* add peer socket in server peer list */
	obus_list_add_before(&srv->peers, &peer->node);

	/* log peer connection */
	if (srv->log)
		obus_debug("socket server '%s' has new peer '%s'",
			   srv->addr.name, peer->peer_addr.name);

	/* notify server callback, of peer connected */
	(*srv->cb) (srv, peer, srv->user_data);
	return;

close_peer:
	close(sockfd);
	free(peer_addr.name);
	free(local_addr.name);
	return;
}

static int create_unix_socket_dir(const struct sockaddr_un *un)
{
	int ret;
	char tmp[256];
	char *p;

	if (!un || un->sun_path[0] != '/')
		return 0;

	snprintf(tmp, sizeof(tmp), "%s", un->sun_path);
	p = strrchr(tmp, '/');
	if (p)
		*p = '\0';

	if (tmp[0] =='\0')
		return 0;

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			ret = mkdir(tmp, S_IRWXU);
			if (ret < 0 && errno != EEXIST) {
				ret = -errno;
				obus_error("mkdir(%s) error(%d):%s", tmp,
					   errno, strerror(errno));
				return ret;
			}
			*p = '/';
		}
	}

	ret = mkdir(tmp, S_IRWXU);
	if (ret < 0 && errno != EEXIST) {
		ret = -errno;
		obus_error("mkdir(%s) error(%d):%s", tmp,
			   errno, strerror(errno));
	}

	return ret;
}

static int obus_socket_server_bind(struct obus_socket_server *srv)
{
	int ret;
	socklen_t len;

	ret = obus_socket_base_open(&srv->sock);
	if (ret < 0)
		goto error;

	/* for non abstract unix socket, unlink file before bind */
	if (srv->addr.un->sun_family == AF_UNIX &&
	    srv->addr.un->sun_path[0] != '\0') {
		/* unlink previous file if exists */
		unlink(srv->addr.un->sun_path);
		/* create socket directory */
		(void)create_unix_socket_dir(srv->addr.un);
	}

	len = obus_socket_addr_length(&srv->addr);

	/* bind socket to server address */
	ret = bind(srv->sock.fd, srv->addr.base, len);

	/* handle case where ip address do not match a existent interface
	 * retry bind later */
	if ((srv->addr.un->sun_family == AF_INET ||
	     srv->addr.un->sun_family == AF_INET6) &&
	     ret < 0 && errno == EADDRNOTAVAIL) {
		obus_debug("bind address not available");
		goto retry;

	/* for other errors do not retry later (ie: fatal bind error) */
	} else if (ret < 0) {
		ret = -errno;
		obus_error("can't bind socket server '%s' error(%d):%s",
			   srv->addr.name, errno, strerror(errno));
		goto close_socket;
	}

	/* listen with a limit up to 10 connection requests */
	ret = listen(srv->sock.fd, 10);
	if (ret < 0) {
		ret = -errno;
		obus_error("can't listen socket server '%s' error:%s",
			   srv->addr.name, strerror(errno));
		goto close_socket;
	}

	/* create obus fd */
	obus_fd_init(&srv->sock.ofd, srv->sock.fd, OBUS_FD_IN,
		     &obus_socket_server_events, srv);

	/* add obus fd in loop */
	ret = obus_loop_add(srv->sock.loop, &srv->sock.ofd);
	if (ret < 0)
		goto close_socket;

	if (srv->log)
		obus_info("socket server listening on '%s'", srv->addr.name);

	return 0;

retry:
	/* close socket and retry later */
	close(srv->sock.fd);
	srv->sock.fd = -1;
	obus_timer_set(srv->timer, OBUS_SOCKET_CONN_INTVL);
	return 0;

close_socket:
	close(srv->sock.fd);
	srv->sock.fd = -1;
error:
	obus_log_func_error(ret);
	return ret;
}

static void obus_socket_bind_timer_cb(struct obus_timer *timer,
				      uint64_t *nbexpired, void *data)
{
	struct obus_socket_server *srv = data;
	obus_timer_clear(srv->timer);
	obus_socket_server_bind(srv);
}

int obus_socket_server_new(struct obus_loop *loop,
			   const char *addr,
			   obus_socket_server_accept_cb_t cb,
			   void *user_data,
			   int log,
			   struct obus_socket_server **server)
{
	struct obus_socket_server *srv;
	int ret;

	if (!loop || !addr || !cb || !server)
		return -EINVAL;

	srv = calloc(1, sizeof(struct obus_socket_server));
	if (!srv)
		return -ENOMEM;

	ret = obus_socket_addr_init_from_string(&srv->addr, addr);
	if (ret < 0) {
		free(srv);
		return ret;
	}

	obus_list_init(&srv->peers);
	srv->cb = cb;
	srv->user_data = user_data;
	srv->log = log ? 1 : 0;

	ret = obus_socket_base_init(&srv->sock, OBUS_SOCKET_SERVER,
				    srv->addr.base->sa_family, loop);
	if (ret < 0)
		goto free_srv;

	/* create socket server bind timer */
	srv->timer = obus_timer_new(loop, &obus_socket_bind_timer_cb, srv);
	if (!srv->timer)
		goto destroy_socket;

	/* open server */
	ret = obus_socket_server_bind(srv);
	if (ret < 0)
		goto destroy_timer;

	*server = srv;
	return 0;

destroy_timer:
	obus_timer_destroy(srv->timer);
destroy_socket:
	obus_socket_base_destroy(&srv->sock);
	free(srv->addr.name);
free_srv:
	free(srv);
	return ret;
}

int obus_socket_server_destroy(struct obus_socket_server *srv)
{
	if (!srv)
		return -EINVAL;

	/* check no peer connected */
	if (!obus_list_is_empty(&srv->peers)) {
		obus_error("can't destroy socket server '%s': "
			  "%zu remaing peers connected", srv->addr.name,
			  obus_list_length(&srv->peers));
		return -EPERM;
	}

	/* destroy timer */
	obus_timer_destroy(srv->timer);

	/* destroy socket */
	obus_socket_base_destroy(&srv->sock);

	/* for non abstract unix socket, unlink file also */
	if (srv->addr.un->sun_family == AF_UNIX &&
	    srv->addr.un->sun_path[0] != '\0')
		unlink(srv->addr.un->sun_path);

	free(srv->addr.name);
	free(srv);
	return 0;
}

int obus_socket_peer_disconnect(struct obus_socket_peer *peer)
{
	if (!peer || !peer->srv)
		return -EINVAL;

	/* remove peer from list */
	obus_list_del(&peer->node);

	/* close and destroy socket base */
	obus_socket_base_destroy(&peer->sock);

	if (peer->srv->log)
		obus_debug("socket peer '%s' disconnected",
			   peer->peer_addr.name);

	/* destroy peer */
	free(peer->peer_addr.name);
	free(peer->local_addr.name);
	free(peer);
	return 0;
}

const char *obus_socket_peer_name(struct obus_socket_peer *peer)
{
	return peer ? peer->peer_addr.name : NULL;
}

int obus_socket_peer_fd(struct obus_socket_peer *peer)
{
	return peer ? peer->sock.fd : -EINVAL;
}

pid_t obus_socket_peer_pid(struct obus_socket_peer *peer)
{
	return peer ? peer->peer_pid : 0;
}

static void obus_socket_client_connected(struct obus_socket_client *client)
{
	socklen_t len;
	int ret;

	/* get local socket address */
	obus_socket_addr_init(&client->local_addr,
			      client->peer_addr.base->sa_family);
	len = obus_socket_addr_length(&client->local_addr);

	ret = getsockname(client->sock.fd, client->local_addr.base, &len);
	if (ret < 0)
		obus_log_fd_errno("getsockname", client->sock.fd);

	if (client->local_addr.base->sa_family == AF_UNIX) {
		/* for unix, recopy sun path (same as peer's one) */
		memcpy(client->local_addr.un->sun_path,
		       client->peer_addr.un->sun_path,
			sizeof(client->local_addr.un->sun_path));
	}

	/* get local socket name */
	client->local_addr.name =
			obus_socket_addr_to_string(&client->local_addr);

	if (client->log)
		obus_debug("socket '%s' connected to '%s'",
			   client->local_addr.name, client->peer_addr.name);

	/* activate peer keep alive for ip socket */
	if (client->local_addr.base->sa_family == AF_INET ||
	    client->local_addr.base->sa_family == AF_INET6)
		obus_socket_activate_keepalive(client->sock.fd, 5, 1, 2);

	/* notify sock user */
	(*client->cb) (client, client->user_data);
}

static void obus_socket_client_events(struct obus_fd *ofd, int events,
				      void *data)
{
	struct obus_socket_client *client = data;
	int ret;
	int err = -1;
	socklen_t len = sizeof(int);

	obus_loop_remove(client->sock.loop, &client->sock.ofd);

	/* if fd error POLLERR | POLLHUP connection failed */
	if (obus_fd_event_error(events))
		goto out;

	/* check socket connection error */
	ret = getsockopt(client->sock.fd, SOL_SOCKET, SO_ERROR,
			 (char *)&err, &len);
	if (ret < 0) {
		obus_log_fd_errno("getsockopt", client->sock.fd);
	} else if (err != 0 &&
		   err != ECONNREFUSED &&
		   err != EHOSTUNREACH &&
		   err != EHOSTDOWN &&
		   err != ENOENT) {
		/* only log error other that connection refused */
		obus_error("getsockopt(SO_ERROR) error=%d(%s)", err,
			   strerror(err));
	}

out:
	if (err == 0) {
		/* connection succeed */
		obus_socket_client_connected(client);
	} else {
		/* connection failed, close socket and set retry timer */
		close(client->sock.fd);
		client->sock.fd = -1;
		obus_timer_set(client->timer, OBUS_SOCKET_CONN_INTVL);
	}
}

static void obus_socket_client_do_connect(struct obus_socket_client *client)
{
	int ret;
	socklen_t len;

	ret = obus_socket_base_open(&client->sock);
	if (ret < 0)
		return;

	/* create socket obus fd */
	obus_fd_init(&client->sock.ofd, client->sock.fd, OBUS_FD_OUT,
		     &obus_socket_client_events, client);

	/* get socket length */
	len = obus_socket_addr_length(&client->peer_addr);

	/* connect in non blocking mode */
	ret = connect(client->sock.fd, client->peer_addr.base, len);
	if (ret == 0) {
		/* connected */
		obus_socket_client_connected(client);
	} else if (errno == EINPROGRESS) {
		/* connection in progress */;
		obus_loop_add(client->sock.loop, &client->sock.ofd);
	} else {
		/* only log non common error */
		if (errno != ECONNREFUSED &&
		    errno != EHOSTUNREACH &&
		    errno != EHOSTDOWN &&
		    errno != ENETUNREACH &&
		    errno != ENETDOWN &&
		    errno != ENOENT &&
		    errno != ETIMEDOUT) {
			obus_error("connect(fd=%d, %s) error=%d(%s)",
				   client->sock.fd, client->peer_addr.name,
				   errno, strerror(errno));
		}

		/*  close socket and set connect retry timer */
		close(client->sock.fd);
		client->sock.fd = -1;
		obus_timer_set(client->timer, OBUS_SOCKET_CONN_INTVL);
	}
}

static void obus_socket_connect_timer_cb(struct obus_timer *timer,
					 uint64_t *nbexpired, void *data)
{
	struct obus_socket_client *client = data;
	obus_timer_clear(client->timer);
	obus_socket_client_do_connect(client);
}

int obus_socket_client_new(struct obus_loop *loop,
			   const char *addr,
			   obus_socket_client_connected_cb_t cb,
			   void *user_data, int log,
			   struct obus_socket_client **sk)
{
	struct obus_socket_client *client;
	int ret;

	if (!loop || !addr || !cb || !sk)
		return -EINVAL;

	client = calloc(1, sizeof(struct obus_socket_client));
	if (!client)
		return -ENOMEM;

	client->cb = cb;
	client->user_data = user_data;
	client->log = log ? 1 : 0;
	ret = obus_socket_addr_init_from_string(&client->peer_addr, addr);
	if (ret < 0) {
		obus_error("bad socket address format '%s'", addr);
		free(client);
		return ret;
	}

	ret = obus_socket_base_init(&client->sock, OBUS_SOCKET_CLIENT,
				    client->peer_addr.base->sa_family, loop);
	if (ret < 0)
		goto free_client;

	/* create socket client timer */
	client->timer = obus_timer_new(loop, &obus_socket_connect_timer_cb,
				       client);
	if (!client->timer)
		goto destroy_socket;

	/* first connect in 10 ms */
	ret = obus_timer_set(client->timer, 10);
	if (ret < 0)
		goto destroy_timer;

	if (client->log)
		obus_debug("connecting to '%s'",  client->peer_addr.name);

	*sk = client;
	return 0;

destroy_timer:
	obus_timer_destroy(client->timer);
destroy_socket:
	obus_socket_base_destroy(&client->sock);
free_client:
	free(client->peer_addr.name);
	free(client->local_addr.name);
	free(client);
	return ret;
}

int obus_socket_client_destroy(struct obus_socket_client *client)
{
	if (!client)
		return -EINVAL;

	obus_timer_destroy(client->timer);
	obus_socket_base_destroy(&client->sock);
	free(client->peer_addr.name);
	free(client->local_addr.name);
	free(client);
	return 0;
}

int obus_socket_client_reconnect(struct obus_socket_client *client)
{
	int ret;
	if (!client)
		return -EINVAL;

	/* clear socket timer */
	obus_timer_clear(client->timer);

	/* close socket */
	obus_socket_base_close(&client->sock);

	/* open socket */
	ret = obus_socket_base_open(&client->sock);
	if (ret < 0)
		return ret;

	/* create socket obus fd */
	obus_fd_init(&client->sock.ofd, client->sock.fd, OBUS_FD_OUT,
		     &obus_socket_client_events, client);

	/* set connect retry timer */
	obus_timer_set(client->timer, OBUS_SOCKET_CONN_INTVL);
	return 0;
}

int obus_socket_client_fd(struct obus_socket_client *client)
{
	return client ? client->sock.fd : -EINVAL;
}

const char *obus_socket_client_name(struct obus_socket_client *client)
{
	return client ? client->peer_addr.name : NULL;
}
