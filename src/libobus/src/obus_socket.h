/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_socket.h
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

#ifndef _OBUS_SOCKET_H_
#define _OBUS_SOCKET_H_

/* socket address */
struct obus_socket_addr {
	char *name;
	struct sockaddr_storage st;
	struct sockaddr *base;
	union {
		struct sockaddr_in6 *in6;/* inet6 address */
		struct sockaddr_in *in;	/* inet address */
		struct sockaddr_un *un;	/* unix address */
	};
};

/* obus socket server object */
struct obus_socket_server;

/* obus socket peer object (a socket connected to a server) */
struct obus_socket_peer;

/* obus socket client object */
struct obus_socket_client;

/* socket client connection callback */
typedef void (*obus_socket_client_connected_cb_t) (struct obus_socket_client
						   *client, void *user_data);

/* server peer connection callback */
typedef void (*obus_socket_server_accept_cb_t) (struct obus_socket_server *srv,
						struct obus_socket_peer *peer,
						void *user_data);
/**
 * create a new socket server listening for connections
 *
 * @param loop file descriptor events loop
 * @param addr socket server listening address
 * @param cb socket server peer connection user callback
 * @param data socket server peer connection user data
 * @param log log socket peer connection & disconnection
 * @return obus server object or NULL on error
 */
int obus_socket_server_new(struct obus_loop *loop,
			   const char *addr,
			   obus_socket_server_accept_cb_t cb,
			   void *user_data,
			   int log,
			   struct obus_socket_server **srv);

/**
 * destroy socket server
 *
 * @param srv socket server
 * @return 0 on success
 */
int obus_socket_server_destroy(struct obus_socket_server *srv);

/**
 * disconnect and remove a peer socket from server
 *
 * @param peer socket peer object
 * @return 0 on success
 */
int obus_socket_peer_disconnect(struct obus_socket_peer *peer);

/**
 * get socket peer name
 *
 * @param peer
 * @return peer name
 */
const char *obus_socket_peer_name(struct obus_socket_peer *peer);

/**
 * get peer socket fd
 *
 * @param peer socket peer object
 * @return peer fd on success, negative value on error
 */
int obus_socket_peer_fd(struct obus_socket_peer *peer);

/**
 * get peer pid
 *
 * @param peer socket process id
 * @return peer process id or 0 if not supported
 */
pid_t obus_socket_peer_pid(struct obus_socket_peer *peer);

/**
 * create a new socket client make a server connection attempt
 * once socket is connected, obus_socket_client_connected_cb_t is called
 * user must monitor socket fd to monitor socket connection lost ...
 *
 * @param loop file descriptor events loop
 * @param addr socket server address to connect
 * @param cb socket client callback called on connection succeed
 * @param data socket client callback user data
 * @param log log socket client connection & disconnection
 * @return obus client object or NULL on error
 */

int obus_socket_client_new(struct obus_loop *loop,
			   const char *addr,
			   obus_socket_client_connected_cb_t cb,
			   void *user_data, int log,
			   struct obus_socket_client **sk);

/**
 * destroy (and disconnect) socket client
 * @param client socket client object
 * @return0 on success
 */
int obus_socket_client_destroy(struct obus_socket_client *client);

/**
 * disconnect & try to reconnect socket client to server
 * @param client socket client object
 * @return 0 on success
 */
int obus_socket_client_reconnect(struct obus_socket_client *client);

/**
 * get socket client name
 * @param client
 * @return
 */
const char *obus_socket_client_name(struct obus_socket_client *client);

/**
 * get client socket fd
 *
 * @param client socket peer object
 * @return client fd on success, negative value on error
 */
int obus_socket_client_fd(struct obus_socket_client *client);

#endif /* _OBUS_SOCKET_H_ */
