/******************************************************************************
* @file net_interface.c
*
* @brief obus net_interface object server api
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#define OBUS_USE_PRIVATE
#include "libobus.h"
#include "libobus_private.h"
#include "net_interface.h"

int net_interface_state_is_valid(int32_t value)
{
	return (value == NET_INTERFACE_STATE_UP ||
		value == NET_INTERFACE_STATE_DOWN);
}

const char *net_interface_state_str(enum net_interface_state value)
{
	const char *str;

	switch (value) {
	case NET_INTERFACE_STATE_UP:
		str = "UP";
		break;
	case NET_INTERFACE_STATE_DOWN:
		str = "DOWN";
		break;
	default:
		str = "???";
		break;
	}

	return str;
}

static void net_interface_state_set_value(void *addr, int32_t value)
{
	enum net_interface_state *v = addr;
	*v = (enum net_interface_state)value;
}

static int32_t net_interface_state_get_value(const void *addr)
{
	const enum net_interface_state *v = addr;
	return (int32_t) (*v);
}

static void net_interface_state_format(const void *addr, char *buf, size_t size)
{
	const enum net_interface_state *v = addr;

	if (net_interface_state_is_valid((int32_t) (*v)))
		snprintf(buf, size, "%s", net_interface_state_str(*v));
	else
		snprintf(buf, size, "??? (%d)", (int32_t) (*v));
}

static const struct obus_enum_driver net_interface_state_driver = {
	.name = "net_interface_state",
	.size = sizeof(enum net_interface_state),
	.default_value = NET_INTERFACE_STATE_DOWN,
	.is_valid = net_interface_state_is_valid,
	.set_value = net_interface_state_set_value,
	.get_value = net_interface_state_get_value,
	.format = net_interface_state_format
};

enum net_interface_field_type {
	NET_INTERFACE_FIELD_NAME = 0,
	NET_INTERFACE_FIELD_STATE,
	NET_INTERFACE_FIELD_HW_ADDR,
	NET_INTERFACE_FIELD_IP_ADDR,
	NET_INTERFACE_FIELD_BROADCAST,
	NET_INTERFACE_FIELD_NETMASK,
	NET_INTERFACE_FIELD_BYTES,
	NET_INTERFACE_FIELD_METHOD_UP,
	NET_INTERFACE_FIELD_METHOD_DOWN,
};

static const struct obus_field_desc net_interface_info_fields[] = {
	[NET_INTERFACE_FIELD_NAME] = {
				      .uid = 1,
				      .name = "name",
				      .offset =
				      obus_offsetof(struct net_interface_info,
						    name),
				      .role = OBUS_PROPERTY,
				      .type = OBUS_FIELD_STRING,
				      },
	[NET_INTERFACE_FIELD_STATE] = {
				       .uid = 2,
				       .name = "state",
				       .offset =
				       obus_offsetof(struct net_interface_info,
						     state),
				       .role = OBUS_PROPERTY,
				       .type = OBUS_FIELD_ENUM,
				       .enum_drv = &net_interface_state_driver,
				       },
	[NET_INTERFACE_FIELD_HW_ADDR] = {
					 .uid = 3,
					 .name = "hw_addr",
					 .offset =
					 obus_offsetof(struct
						       net_interface_info,
						       hw_addr),
					 .role = OBUS_PROPERTY,
					 .type = OBUS_FIELD_STRING,
					 },
	[NET_INTERFACE_FIELD_IP_ADDR] = {
					 .uid = 4,
					 .name = "ip_addr",
					 .offset =
					 obus_offsetof(struct
						       net_interface_info,
						       ip_addr),
					 .role = OBUS_PROPERTY,
					 .type = OBUS_FIELD_STRING,
					 },
	[NET_INTERFACE_FIELD_BROADCAST] = {
					   .uid = 5,
					   .name = "broadcast",
					   .offset =
					   obus_offsetof(struct
							 net_interface_info,
							 broadcast),
					   .role = OBUS_PROPERTY,
					   .type = OBUS_FIELD_STRING,
					   },
	[NET_INTERFACE_FIELD_NETMASK] = {
					 .uid = 6,
					 .name = "netmask",
					 .offset =
					 obus_offsetof(struct
						       net_interface_info,
						       netmask),
					 .role = OBUS_PROPERTY,
					 .type = OBUS_FIELD_STRING,
					 },
	[NET_INTERFACE_FIELD_BYTES] = {
				       .uid = 7,
				       .name = "bytes",
				       .offset =
				       obus_offsetof(struct net_interface_info,
						     bytes),
				       .role = OBUS_PROPERTY,
				       .type =
				       OBUS_FIELD_U64 | OBUS_FIELD_ARRAY,
				       .nb_offset =
				       obus_offsetof(struct net_interface_info,
						     n_bytes),
				       },
	[NET_INTERFACE_FIELD_METHOD_UP] = {
					   .uid = 8,
					   .name = "up",
					   .offset =
					   obus_offsetof(struct
							 net_interface_info,
							 method_up),
					   .role = OBUS_METHOD,
					   .type = OBUS_FIELD_ENUM,
					   .enum_drv =
					   &obus_method_state_driver,
					   },
	[NET_INTERFACE_FIELD_METHOD_DOWN] = {
					     .uid = 9,
					     .name = "down",
					     .offset =
					     obus_offsetof(struct
							   net_interface_info,
							   method_down),
					     .role = OBUS_METHOD,
					     .type = OBUS_FIELD_ENUM,
					     .enum_drv =
					     &obus_method_state_driver,
					     },

};

static const struct obus_struct_desc net_interface_info_desc = {
	.size = sizeof(struct net_interface_info),
	.fields_offset = obus_offsetof(struct net_interface_info, fields),
	.n_fields = OBUS_SIZEOF_ARRAY(net_interface_info_fields),
	.fields = net_interface_info_fields,
};

static const struct obus_event_update_desc event_up_updates[] = {
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_STATE],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_HW_ADDR],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_IP_ADDR],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_BROADCAST],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_NETMASK],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_METHOD_UP],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_METHOD_DOWN],
	 .flags = 0,
	 }

};

static const struct obus_event_update_desc event_down_updates[] = {
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_STATE],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_HW_ADDR],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_IP_ADDR],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_BROADCAST],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_NETMASK],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_METHOD_UP],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_METHOD_DOWN],
	 .flags = 0,
	 }

};

static const struct obus_event_update_desc event_configured_updates[] = {
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_HW_ADDR],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_IP_ADDR],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_BROADCAST],
	 .flags = 0,
	 }

	,
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_NETMASK],
	 .flags = 0,
	 }

};

static const struct obus_event_update_desc event_traffic_updates[] = {
	{
	 .field = &net_interface_info_fields[NET_INTERFACE_FIELD_BYTES],
	 .flags = 0,
	 }

};

static const struct obus_event_desc net_interface_events_desc[] = {
	{
	 .uid = 1,
	 .name = "up",
	 .updates = event_up_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_up_updates),
	 }

	,
	{
	 .uid = 2,
	 .name = "down",
	 .updates = event_down_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_down_updates),
	 }

	,
	{
	 .uid = 3,
	 .name = "configured",
	 .updates = event_configured_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_configured_updates),
	 }

	,
	{
	 .uid = 4,
	 .name = "traffic",
	 .updates = event_traffic_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_traffic_updates),
	 }

	,
	{
	 .uid = 5,
	 .name = "up_failed",
	 .updates = NULL,
	 .n_updates = 0,
	 }

	,
	{
	 .uid = 6,
	 .name = "down_failed",
	 .updates = NULL,
	 .n_updates = 0,
	 }

};

const char *net_interface_event_type_str(enum net_interface_event_type type)
{
	if (type >= OBUS_SIZEOF_ARRAY(net_interface_events_desc))
		return "???";

	return net_interface_events_desc[type].name;
}

enum net_interface_method_type {
	NET_INTERFACE_METHOD_UP = 0,
	NET_INTERFACE_METHOD_DOWN,
	NET_INTERFACE_METHOD_COUNT,
};

static const struct obus_field_desc net_interface_up_args_fields[] = {
	{.uid = 1,
	 .name = "ip_addr",
	 .offset = obus_offsetof(struct net_interface_up_args, ip_addr),
	 .role = OBUS_ARGUMENT,
	 .type = OBUS_FIELD_STRING,
	 }
	,
	{.uid = 2,
	 .name = "netmask",
	 .offset = obus_offsetof(struct net_interface_up_args, netmask),
	 .role = OBUS_ARGUMENT,
	 .type = OBUS_FIELD_STRING,
	 }
};

static const struct obus_struct_desc net_interface_up_args_desc = {
	.size = sizeof(struct net_interface_up_args),
	.fields_offset = obus_offsetof(struct net_interface_up_args, fields),
	.n_fields = OBUS_SIZEOF_ARRAY(net_interface_up_args_fields),
	.fields = net_interface_up_args_fields,
};

static const struct obus_struct_desc net_interface_down_args_desc = {
	.size = 0,
	.fields_offset = 0,
	.n_fields = 0,
	.fields = NULL,
};

static const struct obus_method_desc net_interface_methods_desc[] = {
	{.uid = 8,
	 .name = "up",
	 .args_desc = &net_interface_up_args_desc,
	 }
	,
	{.uid = 9,
	 .name = "down",
	 .args_desc = &net_interface_down_args_desc,
	 }
};

const struct obus_object_desc net_interface_desc = {
	.uid = NET_INTERFACE_UID,
	.name = "interface",
	.info_desc = &net_interface_info_desc,
	.n_events = OBUS_SIZEOF_ARRAY(net_interface_events_desc),
	.events = net_interface_events_desc,
	.n_methods = OBUS_SIZEOF_ARRAY(net_interface_methods_desc),
	.methods = net_interface_methods_desc,
};

int net_interface_method_handlers_is_valid(const struct
					   net_interface_method_handlers
					   *handlers)
{
	return handlers && handlers->method_up && handlers->method_down;
}

int net_interface_up_args_is_complete(const struct net_interface_up_args *args)
{
	return args && args->fields.ip_addr && args->fields.netmask;
}

void net_interface_info_init(struct net_interface_info *info)
{
	if (info)
		memset(info, 0, sizeof(*info));
}

int net_interface_info_is_empty(const struct net_interface_info *info)
{
	return info &&
	    !info->fields.name &&
	    !info->fields.state &&
	    !info->fields.hw_addr &&
	    !info->fields.ip_addr &&
	    !info->fields.broadcast &&
	    !info->fields.netmask &&
	    !info->fields.bytes &&
	    !info->fields.method_up && !info->fields.method_down;
}

void net_interface_info_set_methods_state(struct net_interface_info *info,
					  enum obus_method_state state)
{
	OBUS_SET(info, method_up, state);
	OBUS_SET(info, method_down, state);
}


static inline struct net_interface *
net_interface_from_object(struct obus_object *object)
{
	const struct obus_object_desc *desc;

	if (!object)
		return NULL;

	desc = obus_object_get_desc(object);
	if (desc != &net_interface_desc)
		return NULL;

	return (struct net_interface *)object;
}


static inline struct obus_object *
net_interface_object(struct net_interface *object)
{
	struct obus_object *obj;

	obj = (struct obus_object *)object;
	if (net_interface_from_object(obj) != object)
		return NULL;

	return (struct obus_object *)object;
}


static inline const struct obus_object *
net_interface_const_object(const struct net_interface *object)
{
	const struct obus_object *obj;

	obj = (const struct obus_object *)object;
	if (obus_object_get_desc(obj) != &net_interface_desc)
		return NULL;

	return obj;
}


struct net_interface *
net_interface_new(struct obus_server *srv,
		  const struct net_interface_info *info,
		  const struct net_interface_method_handlers *handlers)
{
	obus_method_handler_cb_t cbs[NET_INTERFACE_METHOD_COUNT];
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = net_interface_desc.info_desc
	};

	cbs[NET_INTERFACE_METHOD_UP] =
	    (obus_method_handler_cb_t) handlers->method_up;
	cbs[NET_INTERFACE_METHOD_DOWN] =
	    (obus_method_handler_cb_t) handlers->method_down;

	return (struct net_interface *)obus_server_new_object(srv,
							      &net_interface_desc,
							      cbs,
							      info ? &st :
							      NULL);
}

int net_interface_destroy(struct net_interface *object)
{
	return obus_object_destroy(net_interface_object(object));
}

int net_interface_register(struct obus_server *server,
			   struct net_interface *object)
{
	struct obus_object *obj = (struct obus_object *)object;
	return obus_server_register_object(server, obj);
}

int net_interface_unregister(struct obus_server *server,
			     struct net_interface *object)
{
	struct obus_object *obj = (struct obus_object *)object;
	return obus_server_unregister_object(server, obj);
}

int net_interface_is_registered(const struct net_interface *object)
{
	const struct obus_object *obj = (const struct obus_object *)object;
	return obus_object_is_registered(obj);
}


const struct net_interface_info *
net_interface_get_info(const struct net_interface *object)
{
	return (const struct net_interface_info *)
	    obus_object_get_info(net_interface_const_object(object));
}

void net_interface_log(const struct net_interface *object,
		       enum obus_log_level level)
{
	obus_object_log(net_interface_const_object(object), level);
}

int net_interface_set_user_data(struct net_interface *object, void *user_data)
{
	return obus_object_set_user_data(net_interface_object(object),
					 user_data);
}

void *net_interface_get_user_data(const struct net_interface *object)
{
	return obus_object_get_user_data(net_interface_const_object(object));
}

obus_handle_t net_interface_get_handle(const struct net_interface *object)
{
	return obus_object_get_handle(net_interface_const_object(object));
}

struct net_interface *
net_interface_from_handle(struct obus_server *server, obus_handle_t handle)
{
	struct obus_object *obj;

	obj = obus_server_get_object(server, handle);
	return net_interface_from_object(obj);
}

struct net_interface *
net_interface_next(struct obus_server *server, struct net_interface *previous)
{
	struct obus_object *next, *prev;

	prev = (struct obus_object *)previous;
	next = obus_server_object_next(server, prev, net_interface_desc.uid);
	return net_interface_from_object(next);
}

int net_interface_send_event(struct obus_server *server,
			     struct net_interface *object,
			     enum net_interface_event_type type,
			     const struct net_interface_info *info)
{
	int ret;
	struct obus_event event;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = net_interface_desc.info_desc
	};

	if (!object || !server || type >= NET_INTERFACE_EVENT_COUNT)
		return -EINVAL;

	ret =
	    obus_event_init(&event, net_interface_object(object),
			    &net_interface_events_desc[type], &st);
	if (ret < 0)
		return ret;

	return obus_server_send_event(server, &event);
}

int net_bus_event_add_interface_event(struct net_bus_event *bus_event,
				      struct net_interface *object,
				      enum net_interface_event_type type,
				      const struct net_interface_info *info)
{
	int ret;
	struct obus_event *event;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = net_interface_desc.info_desc
	};

	if (!object || !bus_event || type >= NET_INTERFACE_EVENT_COUNT)
		return -EINVAL;

	event =
	    obus_event_new(net_interface_object(object),
			   &net_interface_events_desc[type], &st);
	if (!event)
		return -ENOMEM;

	ret =
	    obus_bus_event_add_event((struct obus_bus_event *)bus_event, event);
	if (ret < 0)
		obus_event_destroy(event);

	return ret;
}

int net_bus_event_register_interface(struct net_bus_event *bus_event,
				     struct net_interface *object)
{
	return obus_bus_event_register_object((struct obus_bus_event *)
					      bus_event,
					      net_interface_object(object));
}

int net_bus_event_unregister_interface(struct net_bus_event *bus_event,
				       struct net_interface *object)
{
	return obus_bus_event_unregister_object((struct obus_bus_event *)
						bus_event,
						net_interface_object(object));
}
