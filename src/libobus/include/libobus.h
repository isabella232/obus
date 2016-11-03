/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file libobus.h
 *
 * @brief object bus library api
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

#ifndef _LIBOBUS_H_
#define _LIBOBUS_H_

/* needed for variadic log */
#include <stdarg.h>

/* needed for uint*_t definitions */
#include <stdint.h>

/* for printf format specifiers */
#include <inttypes.h>

/* needed for size_t */
#include <stdlib.h>

/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef	__cplusplus
#define OBUS_BEGIN_DECLS extern "C" {
#define OBUS_END_DECLS }
#else
#define OBUS_BEGIN_DECLS
#define OBUS_END_DECLS
#endif

OBUS_BEGIN_DECLS

/* forward declaration for posix api */
struct pollfd;

/**
 * obus bus description structure
 */
struct obus_bus_desc;

/**
 * bus event description structure
 */
struct obus_bus_event_desc;

/**
 * obus bus event
 */
struct obus_bus_event;

/**
 * obus peer
 */
struct obus_peer;

/**
 * obus invalid object handle
 **/
#define OBUS_INVALID_HANDLE 0

/**
 * obus invalid uid
 **/
#define OBUS_INVALID_UID 0


/* represent an handle */
typedef uint16_t obus_handle_t;

/* obus handle printing format specifier */
#define PRIobhdl PRIu16

/* represent a boolean */
typedef uint8_t obus_bool_t;

/**
 * obus log level
 **/
enum obus_log_level {
	/* A critical condition. */
	OBUS_LOG_CRITICAL = 0,
	/* An error message. */
	OBUS_LOG_ERROR,
	/* A warning message. */
	OBUS_LOG_WARNING,
	/* A condition requiring special handling. */
	OBUS_LOG_NOTICE,
	/* A general information message. */
	OBUS_LOG_INFO,
	/* A message useful for debugging programs. */
	OBUS_LOG_DEBUG,
};

/**
 * obus log callback definition
 *
 * @param level log message level (see @enum obus_log_level)
 * @param fmt log message printf format
 * @param args log message arguments list
 */
typedef void (*obus_log_cb_t) (enum obus_log_level level, const char *fmt,
			       va_list args);

/**
 * Set obus log callback
 *
 * Because callback is shared along with obus clients and servers
 * running in the current process, it is strongly recommended to
 * set callback before created obus clients and servers.
 *
 * A default callback is logging to stderr
 *
 * @param cb obus log callback
 * @return 0 on success
 */
int obus_log_set_cb(obus_log_cb_t cb);

/**
 * Log a message using obus logging system.
 *
 * @param level log message level (see @enum obus_log_level)
 * @param fmt log message printf format & arguments
 */
void obus_log(enum obus_log_level level, const char *fmt, ...)
	      __attribute__ ((format (printf, 2, 3)));

/**
 * obus method state
 **/
enum obus_method_state {
	/* method is not supported */
	OBUS_METHOD_NOT_SUPPORTED = 0,
	/* method is enabled */
	OBUS_METHOD_ENABLED,
	/* method is disabled */
	OBUS_METHOD_DISABLED,
};

/**
 * Get method state string.
 *
 * @param state method state.
 * @return constant method state string.
 */
const char *obus_method_state_str(enum obus_method_state state);

/* obus call acknowledge status */
enum obus_call_status {
	OBUS_CALL_INVALID = 0,
	/* call acknowledged (ie server has parsed arguments and
	 * acknowledge the call)*/
	OBUS_CALL_ACKED,
	/* call aborted (object removed or client disconnected ...) */
	OBUS_CALL_ABORTED,
	/* method is disabled */
	OBUS_CALL_METHOD_DISABLED,
	/* method is not supported */
	OBUS_CALL_METHOD_NOT_SUPPORTED,
	/* invalid call arguments */
	OBUS_CALL_INVALID_ARGUMENTS,
	/* call refused by server */
	OBUS_CALL_REFUSED,
};

/**
 * get call ack string (for logging purpose).
 *
 * @param status ack status.
 * @return constant ack status string.
 */
const char *obus_call_status_str(enum obus_call_status status);

/**
 * obus client structure.
 */
struct obus_client;

/**
 * obus client bus event callback definition.
 *
 * @param event obus bus event.
 * @param user_data client callback user_data.
 */
typedef void (*obus_bus_event_cb_t) (struct obus_bus_event *event,
				     void *user_data);


/**
 * instantiate an obus client.
 *
 * @param name client name.
 * @param bus bus description.
 * @param cb client bus event callback.
 * @param user_data client bus event callback user data.
 * @return obus_client or NULL on error.
 */
struct obus_client *obus_client_new(const char *name,
				    const struct obus_bus_desc *desc,
				    obus_bus_event_cb_t cb,
				    void *user_data);

/**
 * start client.
 *
 * @param client obus client.
 * @param addr server address (inet, inet6 or unix).
 * @return 0 on success.
 *
 * @note address string format is '<family>:<address>' where
 * - <family> is one of 'inet', 'inet6', 'unix'
 * - <address> depends on family
 *	inet:	'<ipv4>:<port>' where
 *		<ipv4> is in dotted decimal format (x.x.x.x)
 *		<port> is in decimal format
 *		no host resolution is done !
 *
 *		 example: 'inet:127.0.0.1:5050'
 *
 *	inet6:	'<ipv6>:<port>' where
 *		<ipv6> is any standard representation format
 *		<port> is in decimal format
 *		no host resolution is done
 *
 *		 example: 'inet6:::1:5050'
 *
 *	unix:	'<sun_path>' is unix socket path
 *		abstract socket are supported (<sun_path> must start by '@')
 *
 *		example: 'unix:/tmp/obus/mybus'
 *		example: 'unix:@/obus/mybus'
 *
 */
int obus_client_start(struct obus_client *client, const char *addr);

/**
 * destroy obus client.
 *
 * @param obus client previously created with obus_client_new.
 * @return 0 on success.
 */
int obus_client_destroy(struct obus_client *client);

/**
 * check if obus client is connected.
 *
 * @param obus client.
 * @return 1 if connected 0 else.
 */
int obus_client_is_connected(struct obus_client *client);

/**
 * get obus client associated file descriptor.
 *
 * @param obus client.
 * @return obus client file descriptor.
 */
int obus_client_fd(struct obus_client *client);

/**
 * process obus client associated file descriptor events.
 *
 * @param obus client.
 * @return 0 on success.
 */
int obus_client_process_fd(struct obus_client *client);

/**
 * Get number of internal fds that need to be watched by the application.
 *
 * @param obus client.
 * @return number of fds.
 */
int obus_client_posix_get_fd_count(struct obus_client *client);

/**
 * Fill an array of pollfd structures that can be used in a 'poll' call in
 * application.
 *
 * @param obus client.
 * @param pfds array of pollfd structures to fill.
 * @param size size of array (use obus_client_posix_get_fd_count to get count).
 * @return number of fds put in array
 */
int obus_client_posix_get_fds(struct obus_client *client, struct pollfd *pfds,
			      int size);

/**
 * Process a single fd that is ready.
 *
 * @param obus client.
 * @param pfd single pollfd structure to process.
 * @return 0 on success.
 */
int obus_client_posix_process_fd(struct obus_client *client,
				 const struct pollfd *pfd);

/**
 * commit bus event
 * @param client
 * @param event
 * @return
 */
int obus_client_commit_bus_event(struct obus_client *client,
				 struct obus_bus_event *event);

/**
 * obus server structure.
 */
struct obus_server;

/**
 * instantiate an obus server.
 *
 * @param bus server bus description.
 * @return obus server or NULL on error.
 */
struct obus_server *obus_server_new(const struct obus_bus_desc *bus);

/**
 * obus peer connection event
 */
enum obus_peer_event {
	/* peer connecting */
	OBUS_PEER_EVENT_CONNECTING,
	/* peer connected */
	OBUS_PEER_EVENT_CONNECTED,
	/* peer disconnected */
	OBUS_PEER_EVENT_DISCONNECTED,
};

/**
 * Get peer connection event string.
 *
 * @param event peer connection event.
 * @return constant peer connection event string.
 */
const char *obus_peer_event_str(enum obus_peer_event event);

/**
 * obus peer connection notification cb
 * @param peer
 * @param user_data
 */
typedef void (*obus_peer_connection_cb_t) (enum obus_peer_event event,
				struct obus_peer *peer, void *user_data);

/**
 * set peer event callback
 * @param srv obus server
 * @param cb user peer event callback
 * @param user_data user data given back in callback
 * @return 0 on success
 */
int obus_server_set_peer_connection_cb(struct obus_server *srv,
				       obus_peer_connection_cb_t cb,
				       void *user_data);

/**
 * refuse a peer to be connected to bus.
 *
 * this function can only be called when OBUS_PEER_EVENT_CONNECTING event
 * is raised in user peer connection callback.
 *
 * @param peer peer
 * @return 0 on success
 */
int obus_peer_refuse_connection(struct obus_peer *peer);

/**
 * get peer name (ie obus client name given in obus_client_new)
 * @param peer peer
 * @return peer name or NULL
 */
const char *obus_peer_get_name(struct obus_peer *peer);

/**
 * get peer socket address
 * @param peer peer
 * @return peer socket address in string format (see @obus_client_new) or NULL
 */
const char *obus_peer_get_address(struct obus_peer *peer);

/**
 * associate a user data
 * @param peer
 * @param user_data
 * @return 0 on success
 */
int obus_peer_set_user_data(struct obus_peer *peer, void *user_data);

/**
 * get peer user data
 * @param peer peer
 * @return peer user_data pointer
 */
void *obus_peer_get_user_data(const struct obus_peer *peer);


/**
 * start server.
 *
 * @param srv obus server.
 * @param addrs server listening socket address arrays
 *		(see @obus_client_new for supported address format)
 * @param n_addrs number of listening socket address.
 * @return 0 on success.
 */
int obus_server_start(struct obus_server *srv, const char *const addrs[],
		      size_t n_addrs);

/**
 * destroy an obus server.
 *
 * @param srv obus server to be destroyed.
 * @return 0 on success.
 */
int obus_server_destroy(struct obus_server *srv);

/**
 * get obus server associated file descriptor.
 *
 * @param srv obus server.
 * @return obus server file descriptor.
 */
int obus_server_fd(struct obus_server *srv);

/**
 * process obus server associated file descriptor events.
 *
 * @param srv obus server.
 * @return 0 on success.
 */
int obus_server_process_fd(struct obus_server *srv);

/**
 * Get number of internal fds that need to be watched by the application.
 *
 * @param srv obus server.
 * @return number of fds.
 */
int obus_server_posix_get_fd_count(struct obus_server *srv);

/**
 * Fill an array of pollfd structures that can be used in a 'poll' call in
 * application.
 *
 * @param srv obus server.
 * @param pfds array of pollfd structures to fill.
 * @param size size of array (use obus_client_posix_get_fd_count to get count).
 * @return number of fds put in array
 */
int obus_server_posix_get_fds(struct obus_server *srv, struct pollfd *pfds,
			      int size);

/**
 * Process a single fd that is ready.
 *
 * @param srv obus server.
 * @param pfd single pollfd structure to process.
 * @return 0 on success.
 */
int obus_server_posix_process_fd(struct obus_server *srv,
				 const struct pollfd *pfd);

/**
 * send call method ack status
 * @param srv obus server
 * @param handle call handle
 * @param status call ack status
 * @return 0 on success
 */
int obus_server_send_ack(struct obus_server *srv, obus_handle_t handle,
			 enum obus_call_status status);

/**
 * get the peer of an obus call
 *
 * The peer is only valid during a method call :
 * the returned peer can be used safely in a method handler, but no reference
 * should be retained for a later use.
 *
 * @param srv obus server
 * @param handle call handle
 * @return the peer that sent the call
 */
struct obus_peer *obus_server_get_call_peer(struct obus_server *srv,
					    obus_handle_t handle);

/**
 * macro used by server to set object properties and methods arguments
 */
#define OBUS_SET(st, name, value)	\
	do {				\
		(st)->name = value;	\
		(st)->fields.name = 1;	\
	} while (0)

/**
 * macro used by server to set object properties and methods arguments
 */
#define OBUS_ARRAY_SET(st, name, values, size)	\
	do {					\
		(st)->name = values;		\
		(st)->n_##name = size;		\
		(st)->fields.name = 1;		\
	} while (0)

/**
 * macro used by server to clear object properties and methods arguments
 */
#define OBUS_CLEAR(st, name)	\
	(st)->fields.name = 0

/**
 * macro used by server to disable a method
 */
#define OBUS_DISABLE_METHOD(st, name)	\
	OBUS_SET(st, method_##name, OBUS_METHOD_DISABLED)

/**
 * macro used by server to enable a method
 */
#define OBUS_ENABLE_METHOD(st, name)	\
	OBUS_SET(st, method_##name, OBUS_METHOD_ENABLED)

/**
 * macro used by server to set method unsupported
 */
#define OBUS_UNSUPPORT_METHOD(st, name) \
	OBUS_SET(st, method_##name, OBUS_METHOD_NOT_SUPPORTED)

OBUS_END_DECLS

#endif /* _LIBOBUS_H_ */
