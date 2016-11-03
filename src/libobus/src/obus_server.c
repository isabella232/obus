/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_server.c
 *
 * @brief object bus server
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

/* sizeof buffer */
#define OBUS_DEFAULT_BUFFER_SIZE 1024

/* obus server state */
enum obus_server_state {
	SERVER_STATE_IDLE = 0,
	SERVER_STATE_STARTED,
};

/* obus peer state */
enum obus_peer_state {
	PEER_STATE_IDLE = 0,
	PEER_STATE_CONNECTING,
	PEER_STATE_CONNECTED,
	PEER_STATE_DISCONNECTED,
	PEER_STATE_REFUSED,
};

/* obus peer */
struct obus_peer {
	char *name;
	struct obus_node node;
	enum obus_peer_state state;
	struct obus_server *srv;
	struct obus_io *io;
	struct obus_socket_peer *sk;
	struct obus_packet_decoder decoder;
	void *user_data;
};

/* obus server */
struct obus_server {
	struct obus_node peers;
	struct obus_bus bus;
	struct obus_loop *loop;
	struct obus_socket_server **sks;
	size_t n_sks;
	struct obus_buffer_pool pool;
	struct obus_call *call;
	enum obus_server_state state;
	size_t n_peers_connected;
	uint32_t log_flags;
	obus_peer_connection_cb_t peer_connection_cb;
	void *user_data;
};

static int obus_peer_is_connected(struct obus_peer *peer)
{
	return peer && (peer->state == PEER_STATE_CONNECTED);
}

OBUS_API
const char *obus_peer_event_str(enum obus_peer_event event)
{
	switch (event) {
	case OBUS_PEER_EVENT_CONNECTING: return "CONNECTING";
	case OBUS_PEER_EVENT_CONNECTED: return "CONNECTED";
	case OBUS_PEER_EVENT_DISCONNECTED: return "DISCONNECTED";
	default: return "INVALID";
	}
	return NULL;
}

OBUS_API
int obus_server_set_peer_connection_cb(struct obus_server *srv,
				       obus_peer_connection_cb_t cb,
				       void *user_data)
{
	if (!srv)
		return -EINVAL;

	srv->peer_connection_cb = cb;
	srv->user_data = user_data;
	return 0;
}

OBUS_API
int obus_peer_set_user_data(struct obus_peer *peer, void *user_data)
{
	if (!peer)
		return -EINVAL;

	peer->user_data = user_data;
	return 0;
}

OBUS_API
void *obus_peer_get_user_data(const struct obus_peer *peer)
{
	return peer ? peer->user_data : NULL;
}

OBUS_API
const char *obus_peer_get_name(struct obus_peer *peer)
{
	return peer ? peer->name : NULL;
}

OBUS_API
const char *obus_peer_get_address(struct obus_peer *peer)
{
	return peer ? obus_socket_peer_name(peer->sk) : NULL;
}

OBUS_API
int obus_peer_refuse_connection(struct obus_peer *peer)
{
	if (!peer)
		return -EINVAL;

	/* user can only refuse peer in connecting state */
	if (peer->state != PEER_STATE_CONNECTING)
		return -EPERM;

	peer->state = PEER_STATE_REFUSED;
	return 0;
}

static void obus_peer_notify_user(struct obus_peer *peer,
				  enum obus_peer_event event)
{
	if (peer->srv->peer_connection_cb)
		(*peer->srv->peer_connection_cb) (event, peer,
						  peer->srv->user_data);
}

static int obus_peer_destroy(struct obus_peer *peer)
{
	struct obus_server *srv = peer->srv;

	if (peer->state == PEER_STATE_CONNECTED) {
		peer->state = PEER_STATE_DISCONNECTED;
		srv->n_peers_connected--;
		obus_peer_notify_user(peer, OBUS_PEER_EVENT_DISCONNECTED);
	}

	if (peer->srv->log_flags & OBUS_LOG_CONNECTION)
		obus_info("peer {addr='%s', name='%s'} disconnected from "
			  "'%s' bus", obus_socket_peer_name(peer->sk),
			  peer->name,  srv->bus.api.desc->name);

	/* remove peer from list */
	obus_list_del(&peer->node);
	obus_io_destroy(peer->io);
	obus_packet_decoder_destroy(&peer->decoder);
	obus_socket_peer_disconnect(peer->sk);
	free(peer->name);
	free(peer);
	return 0;
}

static void obus_server_send_peers(struct obus_server *srv,
				   struct obus_buffer *buf)
{
	struct obus_peer *peer, *tmp;
	int ret;

	/* notify peers of un registered object */
	obus_list_walk_entry_forward_safe(&srv->peers, peer, tmp, node) {
		/* only notify connected peers */
		if (!obus_peer_is_connected(peer))
			continue;

		/* write packet to peer */
		ret = obus_io_write(peer->io, buf);
		if (ret == -EAGAIN) {
			/* buffer write async get a ref on it */
			obus_buffer_ref(buf);
		} else if (ret != 0) {
			/* peer write error => disconnect peer */
			obus_peer_destroy(peer);
		}
	}
}

static void obus_peer_io_write_done(enum obus_io_status status,
				    struct obus_buffer *buf, void *user_data)
{
	struct obus_peer *peer = user_data;

	/* unref buffer */
	obus_buffer_unref(buf);

	/* destroy peer on error */
	if (status != OBUS_IO_OK)
		obus_peer_destroy(peer);
}

static int obus_peer_send_connection_response(struct obus_peer *peer,
					      enum obus_conresp_status status)
{
	struct obus_buffer *buf;
	struct obus_node *objects;
	int ret;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&peer->srv->pool);
	if (!buf)
		return -ENOMEM;

	/* only add objects if connection accepted */
	objects = (status == OBUS_CONRESP_ACCEPTED) ?
		   &peer->srv->bus.objects : NULL;

	ret = obus_packet_conresp_encode(buf, status, objects);
	if (ret < 0) {
		obus_error("can't encode connection response packet");
		obus_buffer_unref(buf);
		return ret;
	}

	/* write packet */
	ret = obus_io_write(peer->io, buf);
	if (ret == 0) {
		/* buffer written, unref it */
		obus_buffer_unref(buf);
	} else if (ret == -EAGAIN) {
		/* buffer put in write queue don't unref it */
		ret = 0;
	} else {
		/* buffer write failure */
		obus_buffer_unref(buf);
	}

	return ret;
}

static int obus_peer_connection_request(struct obus_peer *peer,
					struct obus_packet_conreq *pkt)
{
	int ret;
	struct obus_server *srv;
	struct obus_bus *bus;
	enum obus_conresp_status status;

	/* ignore connection request if not in idle */
	if (peer->state != PEER_STATE_IDLE)
		return 0;

	/* get bus object */
	srv = peer->srv;
	bus = &srv->bus;

	/* set status to accept */
	status = OBUS_CONRESP_ACCEPTED;

	/* get client name */
	peer->name = strdup(pkt->client);

	/* check client protocol version against server's one */
	if (pkt->version != OBUS_PROTOCOL_VERSION) {
		obus_warn("protocol version mismatch");
		obus_warn("peer  :%d", pkt->version);
		obus_warn("server:%d", OBUS_PROTOCOL_VERSION);
		status = OBUS_CONRESP_REFUSED;
	}

	/* check client bus name against server's */
	if (status == OBUS_CONRESP_ACCEPTED &&
	    (strcmp(pkt->bus, bus->api.desc->name) != 0)) {
		obus_warn("bus name mismatch: peer:'%s', srv:'%s'", pkt->bus,
			  bus->api.desc->name);
		status = OBUS_CONRESP_REFUSED;
	}

	/* check client bus crc against server's */
	if (status == OBUS_CONRESP_ACCEPTED &&
	    (pkt->crc != bus->api.desc->crc)) {
		obus_warn("'%s' bus crc mismatch: peer:'%d', srv:'%d'",
			  bus->api.desc->name, pkt->crc,
			  bus->api.desc->crc);
		status = OBUS_CONRESP_REFUSED;
	}

	/* refuse empty clear name */
	if (status == OBUS_CONRESP_ACCEPTED &&
	    (pkt->client == NULL || pkt->client[0] == '\0')) {
		obus_warn("peer with empty name !");
		status = OBUS_CONRESP_REFUSED;
	}


	/* let user accept or refuse peer connection */
	if (status == OBUS_CONRESP_ACCEPTED) {
		peer->state = PEER_STATE_CONNECTING;
		/* notify user of connection request and let it
		 * refuse connection */
		obus_peer_notify_user(peer, OBUS_PEER_EVENT_CONNECTING);
		if (peer->state == PEER_STATE_REFUSED)
			status = OBUS_CONRESP_REFUSED;
	}

	/* send connection response */
	ret = obus_peer_send_connection_response(peer, status);
	if (ret < 0)
		goto destroy_peer;

	/* goto connected state */
	if (status == OBUS_CONRESP_ACCEPTED) {
		/* accept connection */
		peer->state = PEER_STATE_CONNECTED;
		peer->srv->n_peers_connected++;

		if (peer->srv->log_flags & OBUS_LOG_CONNECTION)
			obus_info("peer {addr='%s', name='%s'} connected to "
				  "'%s' bus", obus_socket_peer_name(peer->sk),
				  peer->name,  pkt->bus);

		obus_peer_notify_user(peer, OBUS_PEER_EVENT_CONNECTED);
	} else {
		/* refused connection */
		peer->state = PEER_STATE_REFUSED;

		if (peer->srv->log_flags & OBUS_LOG_CONNECTION)
			obus_info("peer {addr='%s', name='%s'} "
				  "connection to bus '%s' refused",
				  obus_socket_peer_name(peer->sk), pkt->client,
				  pkt->bus);
	}

	return 0;

destroy_peer:
	obus_peer_destroy(peer);
	return -1;
}


static int obus_peer_call_request(struct obus_peer *peer,
				   struct obus_call *call)
{
	obus_method_handler_cb_t handler;
	enum obus_method_state state;
	enum obus_call_status status;

	/* log object event if requested */
	if (peer->srv->log_flags & OBUS_LOG_BUS)
		obus_call_log(call, OBUS_LOG_INFO);

	/* set current call */
	peer->srv->call = call;

	/* if object exist but is not registered, abort call */
	if (!obus_object_is_registered(call->obj)) {
		obus_warn("reject call on unregistered object '%s' "
			  "(handle=%d)",  call->obj->desc->name,
			  call->obj->handle);
		status = OBUS_CALL_ABORTED;
		goto send_ack;
	}

	/* get method state & handler */
	state = obus_object_get_method_state(call->obj, call->desc->uid);
	handler = obus_object_get_method_handler(call->obj, call->desc);
	switch (state) {
	case OBUS_METHOD_ENABLED:
		/* process call */
		if (handler)
			(*handler) (call->obj, call->handle, call->args.u.addr);

		status = OBUS_CALL_REFUSED;
	break;

	case OBUS_METHOD_DISABLED:
		status = OBUS_CALL_METHOD_DISABLED;
	break;
	case OBUS_METHOD_NOT_SUPPORTED:
		status = OBUS_CALL_METHOD_NOT_SUPPORTED;
	break;
	default:
		status = OBUS_CALL_REFUSED;
	break;
	}

send_ack:
	/* acknowledge call if not already done done */
	if (call->status == OBUS_CALL_INVALID)
		obus_server_send_ack(peer->srv, call->handle, status);

	return 0;
}

static void obus_peer_io_read_event(int events, void *user_data)
{
	struct obus_peer *peer = user_data;
	struct obus_packet_info info;
	int ret;

	if (obus_fd_event_error(events)) {
		/* destroy peer on error */
		obus_peer_destroy(peer);
		return;
	}

	/* do not treat event other than read available */
	if (!obus_fd_event_read(events))
		return;

	/* read packet */
	do {
		/* read packet */
		ret = obus_packet_decoder_read(&peer->decoder, &info);

		/* if no more data available wait ... */
		if (ret == -EAGAIN)
			return;

		if (ret < 0) {
			/* read error occurs: destroy peer */
			obus_peer_destroy(peer);
			return;
		}

		/* packet read ok, now decode it */
		switch (info.type) {
		case OBUS_PKT_CONREQ:
			ret = obus_peer_connection_request(peer, &info.conreq);
			/* free info */
			free(info.conreq.bus);
			free(info.conreq.client);
		break;

		case OBUS_PKT_CALL:
			/* set call peer */
			info.call->peer = peer;
			ret = obus_peer_call_request(peer, info.call);

			/* destroy call */
			obus_call_destroy(info.call);
			info.call = NULL;
		break;

		/* other packet should not be received by servers */
		case OBUS_PKT_BUS_EVENT:
		case OBUS_PKT_EVENT:
		case OBUS_PKT_CONRESP:
		case OBUS_PKT_ADD:
		case OBUS_PKT_REMOVE:
		case OBUS_PKT_ACK:
		case OBUS_PKT_COUNT:
		default:
		break;
		}

	} while (ret == 0);
}

static void obus_server_accept(struct obus_socket_server *sk_srv,
			       struct obus_socket_peer *sk_peer,
			       void *user_data)
{
	struct obus_server *srv = user_data;
	struct obus_buffer *buf;
	struct obus_peer *peer;
	int log_io = 0;

	/* allocate peer */
	peer = calloc(1, sizeof(*peer));
	if (!peer)
		goto disconnect;

	/* set peer to idle (ie socket connected but not obus connected) */
	peer->state = PEER_STATE_IDLE;

	/* get socket peer ref */
	peer->sk = sk_peer;

	/* get server ref */
	peer->srv = srv;

	/* create new io from socket fd */
	peer->io = obus_io_new(srv->loop, obus_socket_peer_name(peer->sk),
			       obus_socket_peer_fd(peer->sk),
			       obus_peer_io_write_done,
			       obus_peer_io_read_event, peer);
	if (!peer->io) {
		obus_error("can't create peer io");
		goto destroy_peer;
	}

	/* enable io log traffic on demand */
	if (peer->srv->log_flags & OBUS_LOG_IO)
		log_io = 1;

	obus_io_log_traffic(peer->io, log_io);

	/* get buffer */
	buf = obus_buffer_pool_peek(&srv->pool);
	if (!buf)
		goto destroy_io;

	/* init decoder */
	obus_packet_decoder_init(&peer->decoder, buf, &peer->srv->bus,
				 peer->io, log_io);
	obus_buffer_unref(buf);

	/* add peer in list */
	obus_list_add_before(&srv->peers, &peer->node);

	if (peer->srv->log_flags & OBUS_LOG_CONNECTION)
		obus_info("obus peer socket '%s' connected",
			  obus_socket_peer_name(peer->sk));
	return;

destroy_io:
	obus_io_destroy(peer->io);
destroy_peer:
	free(peer);
disconnect:
	/* disconnect peer from socket server */
	obus_socket_peer_disconnect(sk_peer);
}

OBUS_API
struct obus_server *obus_server_new(const struct obus_bus_desc *desc)
{
	int ret;
	struct obus_server *srv;

	if (!desc)
		return NULL;

	/* allocate struct */
	srv = calloc(1, sizeof(struct obus_server));
	if (!srv)
		return NULL;

	/* get server log flags */
	srv->log_flags = obus_get_log_flags_from_env(desc->name);

	/* init peers list */
	obus_list_init(&srv->peers);

	/* init buffer pool */
	obus_buffer_pool_init(&srv->pool, OBUS_DEFAULT_BUFFER_SIZE);

	/* create bus from bus description */
	ret = obus_bus_init(&srv->bus, desc);
	if (ret < 0)
		goto free_srv;

	/* create poll fd set */
	srv->loop = obus_loop_new();
	if (!srv->loop)
		goto destroy_bus;

	srv->state = SERVER_STATE_IDLE;
	srv->n_peers_connected = 0;
	return srv;

destroy_bus:
	obus_bus_destroy(&srv->bus);
free_srv:
	free(srv);
	return NULL;
}

OBUS_API int obus_server_start(struct obus_server *srv,
			       const char *const addrs[], size_t n_addrs)
{
	int ret, log;
	size_t i;
	struct obus_socket_server **sks;

	if (!srv || !addrs || n_addrs == 0)
		return -EINVAL;

	if (srv->state == SERVER_STATE_STARTED)
		return -EPERM;

	log = (srv->log_flags & OBUS_LOG_SOCKET) ? 1 : 0;

	/* allocate servers array */
	sks = calloc(n_addrs, sizeof(struct obus_socket_server *));
	if (!sks)
		return -ENOMEM;

	/* create socket server */
	for (i = 0; i < n_addrs; i++) {
		ret = obus_socket_server_new(srv->loop, addrs[i],
					     &obus_server_accept, srv, log,
					     &sks[i]);
		if (ret < 0)
			goto destroy_server;
	}

	srv->state = SERVER_STATE_STARTED;
	srv->sks = sks;
	srv->n_sks = n_addrs;
	return 0;

destroy_server:
	for (i = 0; i < n_addrs; i++)
		obus_socket_server_destroy(sks[i]);

	free(sks);
	return ret;
}

OBUS_API int obus_server_destroy(struct obus_server *srv)
{
	struct obus_peer *current, *tmp;
	size_t i;

	if (!srv)
		return -EINVAL;

	/* destroy all connected peers */
	obus_list_walk_entry_forward_safe(&srv->peers, current, tmp, node) {
		obus_peer_destroy(current);
	}

	/* destroy socket server's */
	for (i = 0; i < srv->n_sks; i++)
		obus_socket_server_destroy(srv->sks[i]);

	free(srv->sks);

	/* destroy bus  */
	obus_bus_destroy(&srv->bus);

	/* destroy loop */
	obus_loop_unref(srv->loop);

	/* destroy pool */
	obus_buffer_pool_destroy(&srv->pool);

	/* free server struct */
	free(srv);
	return 0;
}

OBUS_API int obus_server_fd(struct obus_server *srv)
{
	return srv ? obus_loop_fd(srv->loop) : -1;
}

OBUS_API int obus_server_process_fd(struct obus_server *srv)
{
	return srv ? obus_loop_process(srv->loop) : -EINVAL;
}

OBUS_API int obus_server_posix_get_fd_count(struct obus_server *srv)
{
	return srv ? obus_loop_posix_get_fd_count(srv->loop) : -EINVAL;
}

OBUS_API int obus_server_posix_get_fds(struct obus_server *srv,
				       struct pollfd *pfds, int size)
{
	return srv ? obus_loop_posix_get_fds(srv->loop, pfds, size) : -EINVAL;
}

OBUS_API int obus_server_posix_process_fd(struct obus_server *srv,
					  const struct pollfd *pfd)
{
	return srv ? obus_loop_posix_process_fd(srv->loop, pfd) : -EINVAL;
}

static void obus_server_sanitize_event(struct obus_event *event)
{
	(void)obus_event_sanitize(event, 1);
}

OBUS_API struct obus_object *
obus_server_new_object(struct obus_server *srv,
		       const struct obus_object_desc *desc,
		       const obus_method_handler_cb_t *cbs,
		       const struct obus_struct *info)
{
	struct obus_object *obj;
	int ret;

	if (!srv)
		return NULL;

	/* allocate object */
	obj = obus_object_new(desc, cbs, info);
	if (!obj)
		return NULL;

	/* add object in bus without registering it */
	ret = obus_bus_add_object(&srv->bus, obj);
	if (ret < 0) {
		obus_object_destroy(obj);
		return NULL;
	}

	return obj;
}

OBUS_API int
obus_server_register_object(struct obus_server *srv, struct obus_object *obj)
{
	struct obus_buffer *buf;
	int ret;

	if (!srv || !obj)
		return -EINVAL;

	/* can't register already registered object ! */
	if (obus_object_is_registered(obj))
		return -EPERM;

	/* register object */
	ret = obus_bus_register_object(&srv->bus, obj);
	if (ret < 0)
		return ret;

	/* skip packet encoding if no connected peers */
	if (srv->n_peers_connected == 0)
		goto out;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&srv->pool);
	if (!buf)
		return -ENOMEM;

	/* encode add packet */
	ret = obus_packet_add_encode(buf, obj);
	if (ret < 0) {
		obus_error("can't encode objec add packet");
		obus_buffer_unref(buf);
		return ret;
	}

	/* send packet to connected peers */
	obus_server_send_peers(srv, buf);

	/* unref packet */
	obus_buffer_unref(buf);

out:
	/* log object if requested */
	if (srv->log_flags & OBUS_LOG_BUS) {
		obus_info("object registered:");
		obus_object_log(obj, OBUS_LOG_INFO);
	}

	return 0;
}

OBUS_API int
obus_server_unregister_object(struct obus_server *srv, struct obus_object *obj)
{
	struct obus_buffer *buf;
	int ret;

	if (!srv || !obj)
		return -EINVAL;

	/* can't unregistered not registered object ! */
	if (!obus_object_is_registered(obj))
		return -EPERM;

	/* unregister object */
	ret = obus_bus_unregister_object(&srv->bus, obj);
	if (ret < 0)
		return ret;

	/* skip packet encoding if no connected peers */
	if (srv->n_peers_connected == 0)
		goto out;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&srv->pool);
	if (!buf)
		return -ENOMEM;

	/* encode remove packet */
	ret = obus_packet_remove_encode(buf, obj);
	if (ret < 0) {
		obus_error("can't encode object add packet");
		obus_buffer_unref(buf);
		return ret;
	}

	/* send packet to connected peers */
	obus_server_send_peers(srv, buf);

	/* unref packet */
	obus_buffer_unref(buf);

out:
	/* log object if requested */
	if (srv->log_flags & OBUS_LOG_BUS) {
		obus_info("object unregistered:");
		obus_object_log(obj, OBUS_LOG_INFO);
	}

	return 0;
}

OBUS_API struct obus_object *
obus_server_object_next(struct obus_server *srv, struct obus_object *prev,
			uint16_t uid)
{
	return srv ? obus_bus_object_next(&srv->bus, prev, uid) : NULL;
}

OBUS_API struct obus_object *obus_server_get_object(struct obus_server *srv,
						    obus_handle_t handle)
{
	return srv ? obus_bus_object(&srv->bus, handle) : NULL;
}

OBUS_API int obus_server_send_event(struct obus_server *srv,
				    struct obus_event *event)
{
	struct obus_buffer *buf;
	int ret;

	if (!srv || !event)
		return -EINVAL;

	/* can't send event on non registered object ! */
	if (!obus_object_is_registered(event->obj)) {
		obus_warn("can't to send object event: object not registered");
		return -EPERM;
	}

	/* sanitize event */
	obus_server_sanitize_event(event);

	/* skip packet encoding if no connected peers */
	if (srv->n_peers_connected == 0)
		goto out;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&srv->pool);
	if (!buf)
		return -ENOMEM;

	/* encode object event packet */
	ret = obus_packet_event_encode(buf, event);
	if (ret < 0) {
		obus_error("can't encode object event packet");
		obus_buffer_unref(buf);
		return ret;
	}

	/* send packet to connected peers */
	obus_server_send_peers(srv, buf);

	/* unref packet */
	obus_buffer_unref(buf);

out:
	/* log object event if requested */
	if (srv->log_flags & OBUS_LOG_BUS) {
		obus_info("event sent:");
		obus_event_log(event, OBUS_LOG_INFO);
	}

	/* commit event */
	obus_event_commit(event);
	return 0;
}

static int obus_server_check_bus_event(const struct obus_bus_event *event)
{
	struct obus_object *obj;
	struct obus_event *evt;

	/* check objects added are not registered  */
	obus_list_walk_entry_forward(&event->add_objs, obj, event_node) {
		if (obus_object_is_registered(obj)) {
			obus_error("can't register object %s, object is "
				   "already registered !", obj->desc->name);
			return -EPERM;
		}
	}


	/* check objects to be removed are registered  */
	obus_list_walk_entry_forward(&event->remove_objs, obj, event_node) {
		if (!obus_object_is_registered(obj)) {
			obus_error("can't unregister object %s, object is "
				   "not registered !", obj->desc->name);
			return -EPERM;
		}
	}

	/* check events objects exists  */
	obus_list_walk_entry_forward(&event->obj_events, evt, event_node) {
		if (!obus_object_is_registered(evt->obj)) {
			obus_error("can't send object event %s, related "
				   "object '%s' is  not registered !",
				   evt->desc->name, obj->desc->name);
			return -EPERM;
		}

		/* sanitize event */
		obus_server_sanitize_event(evt);
	}

	return 0;
}

static void obus_server_undo_register_objects(struct obus_server *srv,
					      struct obus_bus_event *event)
{
	struct obus_object *obj;

	obus_list_walk_entry_forward(&event->add_objs, obj, event_node) {
		obus_bus_unregister_object(&srv->bus, obj);
	}
}

static int obus_server_register_objects(struct obus_server *srv,
					struct obus_bus_event *event)
{
	struct obus_object *obj;
	int ret = 0;

	/* register new object */
	obus_list_walk_entry_forward(&event->add_objs, obj, event_node) {
		/* register object */
		ret = obus_bus_register_object(&srv->bus, obj);
		if (ret < 0)
			break;
	}

	if (ret < 0)
		obus_warn("%s: bus event '%s' register error", __func__,
			  event->desc->name);

	return ret;
}

static int obus_server_commit_bus_event(struct obus_server *srv,
					struct obus_bus_event *event)
{
	struct obus_object *obj;
	struct obus_event *evt;
	int ret = 0;

	/* register new objects are already done !*/

	/* commit object event */
	obus_list_walk_entry_forward(&event->obj_events, evt, event_node) {
		/* commit event */
		ret |= obus_event_commit(evt);
	}

	/* unregister object */
	obus_list_walk_entry_forward(&event->remove_objs, obj, event_node) {
		/* remove object from bus */
		ret |= obus_bus_unregister_object(&srv->bus, obj);
	}

	if (ret < 0)
		obus_warn("bus event '%s' commit error", event->desc->name);

	return ret;
}

OBUS_API
int obus_server_send_bus_event(struct obus_server *srv,
			       struct obus_bus_event *event)
{
	struct obus_buffer *buf;
	int ret;

	if (!srv || !event)
		return -EINVAL;

	/* check bus event integrity */
	ret = obus_server_check_bus_event(event);
	if (ret < 0)
		return ret;

	/* register new objects */
	ret = obus_server_register_objects(srv, event);
	if (ret < 0)
		goto undo_register_objects;

	/* skip packet encoding if no connected peers */
	if (srv->n_peers_connected == 0)
		goto out;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&srv->pool);
	if (!buf) {
		ret = -ENOMEM;
		goto undo_register_objects;
	}

	/* encode object event packet */
	ret = obus_packet_bus_event_encode(buf, event);
	if (ret < 0) {
		obus_error("can't encode bus event packet");
		obus_buffer_unref(buf);
		goto undo_register_objects;
	}

	/* send packet to connected peers */
	obus_server_send_peers(srv, buf);

	/* unref packet */
	obus_buffer_unref(buf);

out:
	/* log object event if requested */
	if (srv->log_flags & OBUS_LOG_BUS) {
		obus_info("bus event sent:");
		obus_bus_event_log(event, OBUS_LOG_INFO, 0);
	}

	/* commit bus event */
	obus_server_commit_bus_event(srv, event);
	return 0;

undo_register_objects:
	obus_server_undo_register_objects(srv, event);
	return ret;
}

OBUS_API int obus_server_send_ack(struct obus_server *srv,
				  obus_handle_t handle,
				  enum obus_call_status status)
{
	struct obus_buffer *buf;
	struct obus_peer *peer;
	struct obus_call *call;
	struct obus_ack ack;
	int ret;

	if (!srv)
		return -EINVAL;

	call = srv->call;
	if (!call || call->handle != handle)
		return -ENOENT;

	peer = call->peer;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&srv->pool);
	if (!buf)
		return -ENOMEM;

	/* encode object event packet */
	ack.handle = call->handle;
	ack.status = status;
	ret = obus_packet_ack_encode(buf, &ack);
	if (ret < 0) {
		obus_error("can't encode ack packet");
		obus_buffer_unref(buf);
		return ret;
	}

	/* write packet */
	ret = obus_io_write(peer->io, buf);
	if (ret == 0) {
		/* buffer written, unref it */
		obus_buffer_unref(buf);
	} else if (ret == -EAGAIN) {
		/* buffer put in write queue don't unref it */
		ret = 0;
	} else {
		/* buffer write failure */
		obus_buffer_unref(buf);
	}

	/* update call ack status */
	call->status = status;

	/* log ack if requested */
	if (srv->log_flags & OBUS_LOG_BUS) {
		obus_info("call ack sent:");
		obus_ack_log(&ack, call, OBUS_LOG_INFO);
	}

	srv->call = NULL;
	return 0;
}

OBUS_API struct obus_peer *obus_server_get_call_peer(struct obus_server *srv,
						     obus_handle_t handle)
{
	struct obus_call *call;

	if (!srv)
		return NULL;

	call = srv->call;
	if (!call || call->handle != handle)
		return NULL;

	return srv->call->peer;
}
