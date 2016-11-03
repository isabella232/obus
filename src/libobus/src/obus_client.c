/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_client.c
 *
 * @brief object bus client
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

#define OBUS_DEFAULT_BUFFER_SIZE 1024

enum obus_client_state {
	STATE_IDLE,
	STATE_CONNECTING,
	STATE_CONNECTED,
	STATE_DISCONNECTING,
	STATE_DISCONNECTED,
	STATE_REFUSED,
	STATE_COUNT,
};

static const char *const states[STATE_COUNT] = {
	[STATE_IDLE] = "IDLE",
	[STATE_CONNECTING] = "CONNECTING",
	[STATE_CONNECTED] = "CONNECTED",
	[STATE_DISCONNECTING] = "DISCONNECTING",
	[STATE_DISCONNECTED] = "DISCONNECTED",
	[STATE_REFUSED] = "REFUSED",
};

/* internal obus bus event uid */
#define OBUS_BUS_EVENT_CONNECTED_UID 1
#define OBUS_BUS_EVENT_DISCONNECTED_UID 2
#define OBUS_BUS_EVENT_CONNECTION_REFUSED_UID 3

static const char *obus_client_state_str(enum obus_client_state state)
{
	return (state >= STATE_COUNT) ? "<INVALID>" : states[state];
}

/* obus client */
struct obus_client {
	/* client name */
	char *name;
	/* client bus */
	struct obus_bus bus;
	/* client bus event callback */
	obus_bus_event_cb_t cb;
	/* client bus event callback user_data*/
	void *user_data;
	/* client loop context */
	struct obus_loop *loop;
	/* client socket */
	struct obus_socket_client *sk;
	/* client connection status */
	enum obus_client_state state;
	/* client socket io */
	struct obus_io *io;
	/* client buffer pool */
	struct obus_buffer_pool pool;
	/* client packet decoder */
	struct obus_packet_decoder decoder;
	/* connected event desc */
	const struct obus_bus_event_desc *connected_desc;
	/* disconnected event desc */
	const struct obus_bus_event_desc *disconnected_desc;
	/* connection refused event desc */
	const struct obus_bus_event_desc *connection_refused_desc;
	/* log flags */
	uint32_t log_flags;
};

static void obus_client_handle_bus_event(struct obus_client *client,
					 struct obus_bus_event *event);

static void obus_client_bus_event_notify(struct obus_client *client,
					 struct obus_bus_event *event)
{
	(*client->cb) (event, client->user_data);
}

static void obus_client_disconnect_bus(struct obus_client *client)
{
	struct obus_object *obj, *t;
	struct obus_bus_event event;

	obus_bus_event_init(&event, client->disconnected_desc);

	/* add objects in bus event unregistered objects list */
	obus_list_walk_entry_forward_safe(&client->bus.objects, obj, t, node) {
		obus_list_add_before(&event.remove_objs,
				     &obj->event_node);
	}

	/* handle event */
	obus_client_handle_bus_event(client, &event);
}

static void obus_client_disconnect(struct obus_client *client, int reconnect)
{
	enum obus_client_state state;

	state = client->state;
	client->state = STATE_DISCONNECTING;

	if (client->log_flags & OBUS_LOG_CONNECTION)
		obus_info("disconnect socket client %s", client->name);

	/* destroy io */
	if (client->io) {
		/* destroy decoder */
		obus_packet_decoder_destroy(&client->decoder);

		/* destroy io */
		obus_io_destroy(client->io);
		client->io = NULL;
	}

	/* reconnect socket on demand */
	if (reconnect) {
		if (client->log_flags & OBUS_LOG_CONNECTION)
			obus_info("reconnect socket client %s", client->name);
		obus_socket_client_reconnect(client->sk);
	} else {
		obus_socket_client_destroy(client->sk);
		client->sk = NULL;
	}

	/* disconnect bus if needed  */
	if (state == STATE_CONNECTED)
		obus_client_disconnect_bus(client);

	/* update client state to connecting */
	client->state = reconnect ? STATE_CONNECTING :
				    STATE_DISCONNECTED;
}

static void obus_client_io_write_done(enum obus_io_status status,
				      struct obus_buffer *buf,
				      void *user_data)
{
	struct obus_client *client = user_data;

	/* unref buffer */
	obus_buffer_unref(buf);

	if (status != OBUS_IO_OK) {
		/* write failed => disconnect client */
		obus_info("obus client write failed: status=%d", status);
		obus_client_disconnect(client, 1);
		return;
	}
}

static void obus_client_sanitize_event(struct obus_event *event)
{
	(void)obus_event_sanitize(event, 0);
}

static void obus_client_event(struct obus_client *client,
			      struct obus_event *event,
			      const struct obus_bus_event *bus_event)
{
	struct obus_provider *prov;
	struct obus_object *obj = event->obj;

	/* log object event if requested */
	if (client->log_flags & OBUS_LOG_BUS) {
		obus_info("event received:");
		obus_event_log(event, OBUS_LOG_INFO);
	}

	obus_client_sanitize_event(event);

	/* call provider event */
	prov = obus_bus_provider(&client->bus, obj->desc->uid);
	if (prov && prov->event)
		(*prov->event) (obj, event, bus_event, prov->user_data);

	/* commit event if not done by user */
	obus_event_commit(event);

	/* destroy event */
	obus_event_destroy(event);
}

static void obus_client_add_object_notify(struct obus_client *client,
		   struct obus_object *obj,
		   const struct obus_bus_event *bus_event)
{
	struct obus_provider *prov;

	/* call provider add */
	prov = obus_bus_provider(&client->bus, obj->desc->uid);
	if (prov && prov->add)
		(*prov->add) (obj, bus_event, prov->user_data);

}

static void obus_client_add_object(struct obus_client *client,
				   struct obus_object *obj,
				   const struct obus_bus_event *bus_event,
				   int notify)
{
	int ret;

	/* log object if requested */
	if (client->log_flags & OBUS_LOG_BUS) {
		obus_info("object registered:");
		obus_object_log(obj, OBUS_LOG_INFO);
	}

	/* add object in bus */
	ret = obus_bus_add_object(&client->bus, obj);
	if (ret < 0) {
		/* destroy object on error */
		obus_object_destroy(obj);
		return;
	}

	/* register object in bus */
	ret = obus_bus_register_object(&client->bus, obj);
	if (ret < 0) {
		/* remove object from bus */
		obus_bus_remove_object(&client->bus, obj);
		/* destroy object on error */
		obus_object_destroy(obj);
		return;
	}

	if (notify)
		obus_client_add_object_notify(client, obj, bus_event);
}

static void obus_client_connection_response(struct obus_client *client,
					    struct obus_packet_conresp *pkt)
{
	struct obus_object *obj, *tmp;
	struct obus_bus_event event;

	if (client->log_flags & OBUS_LOG_BUS)
		obus_info("server connection response: %s (%zu objects)",
			  obus_conresp_status_str(pkt->status),
			  obus_list_length(&pkt->objects));

	/* check connection state */
	if (client->state != STATE_CONNECTING) {
		obus_warn("ignoring connection response in %s state",
			  obus_client_state_str(client->state));
		return;
	}

	/* handle connection refused */
	if (pkt->status != OBUS_CONRESP_ACCEPTED) {
		client->state = STATE_REFUSED;

		/* init connection refused bus event */
		obus_bus_event_init(&event, client->connection_refused_desc);

		/* notify client */
		obus_client_bus_event_notify(client, &event);
		return;
	}

	/* update client state to connected */
	client->state = STATE_CONNECTED;

	if (client->log_flags & OBUS_LOG_CONNECTION)
		obus_info("client connected to '%s' bus",
			  client->bus.api.desc->name);

	/* init connected bus event */
	obus_bus_event_init(&event, client->connected_desc);

	/* process bus event */
	/* add objects in bus event registered objects list */
	obus_list_walk_entry_forward_safe(&pkt->objects, obj, tmp, node) {
		obus_list_del(&obj->node);
		obus_list_add_before(&event.add_objs, &obj->event_node);
	}

	/* handle bus event */
	obus_client_handle_bus_event(client, &event);
}

static void obus_client_remove_object(struct obus_client *client,
				      struct obus_object *obj,
				      const struct obus_bus_event *event)
{
	struct obus_provider *prov;

	/* log object if requested */
	if (client->log_flags & OBUS_LOG_BUS) {
		obus_info("object unregistered:");
		obus_object_log(obj, OBUS_LOG_INFO);
	}

	/* abort all object pending calls */
	obus_bus_abort_call(&client->bus, obj);

	/* call provider remove */
	prov = obus_bus_provider(&client->bus, obj->desc->uid);
	if (prov && prov->remove)
		(*prov->remove) (obj, event, prov->user_data);

	/* unregister object from bus */
	obus_bus_unregister_object(&client->bus, obj);

	/* remove object from bus */
	obus_bus_remove_object(&client->bus, obj);

	/* destroy object on error */
	obus_object_destroy(obj);
}

static void obus_client_ack(struct obus_client *client, struct obus_ack *ack)
{
	struct obus_call *call;

	/* get call ack */
	call = obus_bus_call(&client->bus, ack->handle);

	/* log ack if requested */
	if (client->log_flags & OBUS_LOG_BUS) {
		obus_info("call ack received:");
		/* here call may be null */
		obus_ack_log(ack, call, OBUS_LOG_INFO);
	}

	/* check call is valid */
	if (!call)
		return;

	/* unregister call */
	obus_bus_unregister_call(&client->bus, call);

	/* notify call ack handler */
	obus_call_ack_notify(call, ack->status);

	/* destroy call */
	obus_call_destroy(call);
}

OBUS_API int obus_client_commit_bus_event(struct obus_client *client,
					  struct obus_bus_event *event)
{
	struct obus_object *obj, *to;
	struct obus_event *evt, *te;

	if (!client || !event)
		return -EINVAL;

	if (event->is_committed)
		return 0;

	/* turn on commit flag */
	event->is_committed = 1;

	/* register new objects */
	obus_list_walk_entry_forward_safe(&event->add_objs, obj, to,
					  event_node) {
		/* add object */
		obus_client_add_object(client, obj, event, 0);
	}
	/* notify for all the added objects */
	obus_list_walk_entry_forward_safe(&event->add_objs, obj, to,
					  event_node) {
		/* remove object from bus event list */
		obus_list_del(&obj->event_node);
		/* notify object */
		obus_client_add_object_notify(client, obj, event);
	}

	/* commit object event */
	obus_list_walk_entry_forward_safe(&event->obj_events, evt, te,
					  event_node) {
		/* remove event from bus event list */
		obus_list_del(&evt->event_node);
		/* commit event */
		obus_client_event(client, evt, event);
	}

	/*  unregister object */
	obus_list_walk_entry_forward_safe(&event->remove_objs, obj, to,
					  event_node) {
		/* remove object from bus event list */
		obus_list_del(&obj->event_node);
		/* remove object from bus */
		obus_client_remove_object(client, obj, event);
	}

	return 0;
}

static void obus_client_handle_bus_event(struct obus_client *client,
					 struct obus_bus_event *event)
{
	/* log object if requested */
	if (client->log_flags & OBUS_LOG_BUS)
		obus_bus_event_log(event, OBUS_LOG_INFO, 1);

	/* call client bus event callback */
	obus_client_bus_event_notify(client, event);

	/* commit bus event if not already done */
	obus_client_commit_bus_event(client, event);

	/* destroy bus event */
	obus_bus_event_destroy(event);
}

static void obus_client_io_read_event(int events, void *user_data)
{
	struct obus_client *client = user_data;
	struct obus_packet_info info;
	int ret;

	if (obus_fd_event_error(events)) {
		/* disconnect client on error */
		obus_info("read events=%d", events);
		obus_client_disconnect(client, 1);
		return;
	}

	/* do not treat event other than read available */
	if (!obus_fd_event_read(events))
		return;

	/* read packet */
	do {
		/* read packet */
		ret = obus_packet_decoder_read(&client->decoder, &info);
		/* if no more data available wait ... */
		if (ret == -EAGAIN)
			return;

		if (ret < 0) {
			/* read error occurs: disconnect client */
			obus_info("obus client read failed");
			obus_client_disconnect(client, 1);
			return;
		}

		/* packet read ok, now decode it */
		switch (info.type) {
		case OBUS_PKT_CONRESP:
			obus_client_connection_response(client, &info.conresp);
		break;

		case OBUS_PKT_ADD:
			obus_client_add_object(client, info.object, NULL, 1);
		break;

		case OBUS_PKT_REMOVE:
			obus_client_remove_object(client, info.object, NULL);
		break;

		case OBUS_PKT_EVENT:
			obus_client_event(client, info.event, NULL);
		break;

		case OBUS_PKT_BUS_EVENT:
			obus_client_handle_bus_event(client, info.bus_event);
		break;

		case OBUS_PKT_ACK:
			obus_client_ack(client, &info.ack);
		break;

		/* other packet should not be received by clients */
		case OBUS_PKT_CONREQ:
		case OBUS_PKT_CALL:
		case OBUS_PKT_COUNT:
		default:
		break;
		}

	} while (ret == 0);
}

static int obus_client_send_connection_request(struct obus_client *client)
{
	struct obus_buffer *buf;
	int ret;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&client->pool);
	if (!buf)
		return -ENOMEM;

	/* encode packet */
	ret = obus_packet_conreq_encode(buf, client->name,
					client->bus.api.desc->name,
					client->bus.api.desc->crc);
	if (ret < 0) {
		obus_error("can't encode connection request packet error=%d",
			   ret);
		obus_buffer_unref(buf);
		return ret;
	}

	/* write packet */
	ret = obus_io_write(client->io, buf);
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

static void obus_client_socket_connected(struct obus_socket_client *sk,
					  void *user_data)
{
	struct obus_client *client = user_data;
	struct obus_buffer *buf;
	int log_io = 0;
	int ret;

	/* create io from socket fd */
	client->io = obus_io_new(client->loop,
				 obus_socket_client_name(client->sk),
				 obus_socket_client_fd(client->sk),
				 &obus_client_io_write_done,
				 &obus_client_io_read_event, client);
	if (!client->io) {
		obus_error("can't create client io");
		obus_client_disconnect(client, 1);
		return;
	}

	/* get rx buffer for decoder */
	buf = obus_buffer_pool_peek(&client->pool);
	if (!buf) {
		obus_client_disconnect(client, 1);
		return;
	}

	/* enable io log traffic on demand */
	if (client->log_flags & OBUS_LOG_IO)
		log_io = 1;

	obus_io_log_traffic(client->io, log_io);

	/* init decoder */
	obus_packet_decoder_init(&client->decoder, buf, &client->bus,
				 client->io, log_io);
	obus_buffer_unref(buf);

	/* send bus connection request */
	ret = obus_client_send_connection_request(client);
	if (ret < 0) {
		obus_error("can't send connection request");
		obus_client_disconnect(client, 1);
	}
}

OBUS_API
struct obus_client *obus_client_new(const char *name,
				    const struct obus_bus_desc *desc,
				    obus_bus_event_cb_t cb, void *user_data)
{
	int ret;
	struct obus_client *client;

	/* client must have a non empty name */
	if (!name || name[0] == '\0' || !desc || !cb)
		return NULL;

	/* allocate client structure */
	client = calloc(1, sizeof(struct obus_client));
	if (!client)
		return NULL;

	/* get client log flags */
	client->log_flags = obus_get_log_flags_from_env(desc->name);

	/* init client buffer pool */
	obus_buffer_pool_init(&client->pool, OBUS_DEFAULT_BUFFER_SIZE);

	/* copy client callbacks */
	client->cb = cb;
	client->user_data = user_data;
	client->name = strdup(name);
	if (!client->name)
		goto free_client;

	/* create bus from bus description */
	ret = obus_bus_init(&client->bus, desc);
	if (ret < 0)
		goto free_name;

	client->connected_desc = obus_bus_api_bus_event(&client->bus.api,
					OBUS_BUS_EVENT_CONNECTED_UID);

	client->disconnected_desc = obus_bus_api_bus_event(&client->bus.api,
					OBUS_BUS_EVENT_DISCONNECTED_UID);

	client->connection_refused_desc = obus_bus_api_bus_event(
		&client->bus.api, OBUS_BUS_EVENT_CONNECTION_REFUSED_UID);

	if (!client->connected_desc ||
	    !client->disconnected_desc ||
	    !client->connection_refused_desc) {
		obus_critical("can't find connection bus events description");
		obus_critical("please update generated files with obusgen");
		goto free_name;
	}

	/* create poll fd set */
	client->loop = obus_loop_new();
	if (!client->loop)
		goto destroy_bus;

	/* connection in progress */
	client->state = STATE_IDLE;
	return client;

destroy_bus:
	obus_bus_destroy(&client->bus);
free_name:
	free(client->name);
free_client:
	free(client);
	return NULL;
}

OBUS_API
int obus_client_start(struct obus_client *client, const char *addr)
{
	int ret;

	if (!client || !addr)
		return -EINVAL;

	if (client->state != STATE_IDLE)
		return -EPERM;

	/* create socket client */
	ret = obus_socket_client_new(client->loop, addr,
				     &obus_client_socket_connected, client,
				     (client->log_flags & OBUS_LOG_SOCKET) ?
				     1 : 0, &client->sk);
	if (ret < 0)
		return ret;

	/* connection in progress */
	client->state = STATE_CONNECTING;
	return ret;
}

OBUS_API int obus_client_destroy(struct obus_client *client)
{
	if (!client)
		return -EINVAL;

	obus_client_disconnect(client, 0);
	obus_bus_destroy(&client->bus);
	obus_loop_unref(client->loop);
	obus_buffer_pool_destroy(&client->pool);
	free(client->name);
	free(client);
	return 0;
}

OBUS_API int obus_client_is_connected(struct obus_client *client)
{
	return (client && client->state == STATE_CONNECTED) ? 1 : 0;
}

OBUS_API int obus_client_fd(struct obus_client *client)
{
	return client ? obus_loop_fd(client->loop) : -1;
}

OBUS_API int obus_client_process_fd(struct obus_client *client)
{
	return client ? obus_loop_process(client->loop) : -EINVAL;
}

OBUS_API int obus_client_posix_get_fd_count(struct obus_client *client)
{
	return client ? obus_loop_posix_get_fd_count(client->loop) : -EINVAL;
}

OBUS_API int obus_client_posix_get_fds(struct obus_client *client,
				       struct pollfd *pfds, int size)
{
	return client ? obus_loop_posix_get_fds(client->loop, pfds, size) :
			-EINVAL;
}

OBUS_API int obus_client_posix_process_fd(struct obus_client *client,
					  const struct pollfd *pfd)
{
	return client ? obus_loop_posix_process_fd(client->loop, pfd) :
			-EINVAL;
}

OBUS_API struct obus_object *obus_client_get_object(struct obus_client *client,
						    obus_handle_t handle)
{
	return client ? obus_bus_object(&client->bus, handle) : NULL;
}

OBUS_API
struct obus_object *obus_client_object_next(struct obus_client *client,
					    struct obus_object *prev,
					    uint16_t uid)
{
	return client ? obus_bus_object_next(&client->bus, prev, uid) : NULL;
}

OBUS_API int obus_client_register_provider(struct obus_client *client,
					   struct obus_provider *prov)
{
	if (!client || !prov)
		return -EINVAL;

	return obus_bus_register_provider(&client->bus, prov);
}

OBUS_API int obus_client_unregister_provider(struct obus_client *client,
					     struct obus_provider *prov)
{
	if (!client || !prov)
		return -EINVAL;

	return obus_bus_unregister_provider(&client->bus, prov);
}

OBUS_API int obus_client_call(struct obus_client *client,
			      struct obus_object *obj,
			      const struct obus_method_desc *desc,
			      const struct obus_struct *args,
			      obus_method_call_status_handler_cb_t cb,
			      uint16_t *handle)
{
	struct obus_call *call;
	struct obus_buffer *buf;
	enum obus_method_state state;
	int ret;

	if (!client || !obj || !desc)
		return -EINVAL;

	/* can't send call when client is not connected */
	if (!obus_client_is_connected(client)) {
		obus_warn("can't call method '%s': client is not connected !",
			  desc->name);
		return -EPERM;
	}

	/* can't send call event on non registered object ! */
	if (!obus_object_is_registered(obj)) {
		obus_warn("can't call method '%s': object is not registered !",
			  desc->name);
		return -EPERM;
	}

	/* check method is enabled */
	state = obus_object_get_method_state(obj, desc->uid);
	if (state != OBUS_METHOD_ENABLED) {
		obus_warn("can't call method '%s': method is not enabled !",
			  desc->name);
		return -EPERM;
	}

	/* create call object */
	call = obus_call_new(obj, desc, cb, args);
	if (!call)
		return -ENOMEM;

	/* register call */
	ret = obus_bus_register_call(&client->bus, call);
	if (ret < 0)
		goto destroy_call;

	/* peek buffer */
	buf = obus_buffer_pool_peek(&client->pool);
	if (!buf) {
		ret = -ENOMEM;
		goto unregister_call;
	}

	/* encode object call packet */
	ret = obus_packet_call_encode(buf, call);
	if (ret < 0) {
		obus_error("can't encode call packet");
		goto unref_buffer;
	}

	/* write buffer */
	ret = obus_io_write(client->io, buf);
	if (ret == 0) {
		/* buffer written, unref it */
		obus_buffer_unref(buf);
	} else if (ret == -EAGAIN) {
		/* buffer put in write queue don't unref it */
		ret = 0;
	} else {
		/* buffer write failure */
		goto unref_buffer;
	}

	if (client->log_flags & OBUS_LOG_BUS)
		obus_call_log(call, OBUS_LOG_INFO);

	/* give handle back for client */
	if (handle)
		*handle = call->handle;

	/* detach call args from call */
	call->args.u.addr = NULL;
	return 0;

unref_buffer:
	obus_buffer_unref(buf);
unregister_call:
	obus_bus_unregister_call(&client->bus, call);
	call->handle = OBUS_INVALID_HANDLE;
destroy_call:
	/* detach call args from call */
	call->args.u.addr = NULL;
	obus_call_destroy(call);
	return ret;
}
