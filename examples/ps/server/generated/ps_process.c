/**
 * @file ps_process.c
 *
 * @brief obus ps_process object server api
 *
 * @author obusgen 1.0.3 generated file, do not modify it.
 */
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
#include "ps_process.h"

int ps_process_state_is_valid(int32_t value)
{
	return (value == PS_PROCESS_STATE_UNKNOWN ||
		value == PS_PROCESS_STATE_RUNNING ||
		value == PS_PROCESS_STATE_SLEEPING ||
		value == PS_PROCESS_STATE_STOPPED ||
		value == PS_PROCESS_STATE_ZOMBIE);
}

const char *ps_process_state_str(enum ps_process_state value)
{
	const char *str;

	switch (value) {
	case PS_PROCESS_STATE_UNKNOWN:
		str = "UNKNOWN";
		break;
	case PS_PROCESS_STATE_RUNNING:
		str = "RUNNING";
		break;
	case PS_PROCESS_STATE_SLEEPING:
		str = "SLEEPING";
		break;
	case PS_PROCESS_STATE_STOPPED:
		str = "STOPPED";
		break;
	case PS_PROCESS_STATE_ZOMBIE:
		str = "ZOMBIE";
		break;
	default:
		str = "???";
		break;
	}

	return str;
}

static void ps_process_state_set_value(void *addr, int32_t value)
{
	enum ps_process_state *v = addr;
	*v = (enum ps_process_state)value;
}

static int32_t ps_process_state_get_value(const void *addr)
{
	const enum ps_process_state *v = addr;
	return (int32_t) (*v);
}

static void ps_process_state_format(const void *addr, char *buf, size_t size)
{
	const enum ps_process_state *v = addr;

	if (ps_process_state_is_valid((int32_t) (*v)))
		snprintf(buf, size, "%s", ps_process_state_str(*v));
	else
		snprintf(buf, size, "??? (%d)", (int32_t) (*v));
}

static const struct obus_enum_driver ps_process_state_driver = {
	.name = "ps_process_state",
	.size = sizeof(enum ps_process_state),
	.default_value = PS_PROCESS_STATE_UNKNOWN,
	.is_valid = ps_process_state_is_valid,
	.set_value = ps_process_state_set_value,
	.get_value = ps_process_state_get_value,
	.format = ps_process_state_format
};

enum ps_process_field_type {
	PS_PROCESS_FIELD_PID = 0,
	PS_PROCESS_FIELD_PPID,
	PS_PROCESS_FIELD_NAME,
	PS_PROCESS_FIELD_EXE,
	PS_PROCESS_FIELD_PCPU,
	PS_PROCESS_FIELD_STATE,
};

static const struct obus_field_desc ps_process_info_fields[] = {
	[PS_PROCESS_FIELD_PID] = {
				  .uid = 1,
				  .name = "pid",
				  .offset =
				  obus_offsetof(struct ps_process_info, pid),
				  .role = OBUS_PROPERTY,
				  .type = OBUS_FIELD_U32,
				  },
	[PS_PROCESS_FIELD_PPID] = {
				   .uid = 2,
				   .name = "ppid",
				   .offset =
				   obus_offsetof(struct ps_process_info, ppid),
				   .role = OBUS_PROPERTY,
				   .type = OBUS_FIELD_U32,
				   },
	[PS_PROCESS_FIELD_NAME] = {
				   .uid = 3,
				   .name = "name",
				   .offset =
				   obus_offsetof(struct ps_process_info, name),
				   .role = OBUS_PROPERTY,
				   .type = OBUS_FIELD_STRING,
				   },
	[PS_PROCESS_FIELD_EXE] = {
				  .uid = 4,
				  .name = "exe",
				  .offset =
				  obus_offsetof(struct ps_process_info, exe),
				  .role = OBUS_PROPERTY,
				  .type = OBUS_FIELD_STRING,
				  },
	[PS_PROCESS_FIELD_PCPU] = {
				   .uid = 5,
				   .name = "pcpu",
				   .offset =
				   obus_offsetof(struct ps_process_info, pcpu),
				   .role = OBUS_PROPERTY,
				   .type = OBUS_FIELD_F32,
				   },
	[PS_PROCESS_FIELD_STATE] = {
				    .uid = 6,
				    .name = "state",
				    .offset =
				    obus_offsetof(struct ps_process_info,
						  state),
				    .role = OBUS_PROPERTY,
				    .type = OBUS_FIELD_ENUM,
				    .enum_drv = &ps_process_state_driver,
				    },

};

static const struct obus_struct_desc ps_process_info_desc = {
	.size = sizeof(struct ps_process_info),
	.fields_offset = obus_offsetof(struct ps_process_info, fields),
	.n_fields = OBUS_SIZEOF_ARRAY(ps_process_info_fields),
	.fields = ps_process_info_fields,
};

static const struct obus_event_update_desc event_updated_updates[] = {
	{
	 .field = &ps_process_info_fields[PS_PROCESS_FIELD_PPID],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_process_info_fields[PS_PROCESS_FIELD_NAME],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_process_info_fields[PS_PROCESS_FIELD_EXE],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_process_info_fields[PS_PROCESS_FIELD_PCPU],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_process_info_fields[PS_PROCESS_FIELD_STATE],
	 .flags = 0,
	 }

};

static const struct obus_event_desc ps_process_events_desc[] = {
	{
	 .uid = 1,
	 .name = "updated",
	 .updates = event_updated_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_updated_updates),
	 }

};

const char *ps_process_event_type_str(enum ps_process_event_type type)
{
	if (type >= OBUS_SIZEOF_ARRAY(ps_process_events_desc))
		return "???";

	return ps_process_events_desc[type].name;
}

const struct obus_object_desc ps_process_desc = {
	.uid = PS_PROCESS_UID,
	.name = "process",
	.info_desc = &ps_process_info_desc,
	.n_events = OBUS_SIZEOF_ARRAY(ps_process_events_desc),
	.events = ps_process_events_desc,
	.n_methods = 0,
	.methods = NULL,
};

void ps_process_info_init(struct ps_process_info *info)
{
	if (info)
		memset(info, 0, sizeof(*info));
}

int ps_process_info_is_empty(const struct ps_process_info *info)
{
	return info &&
	    !info->fields.pid &&
	    !info->fields.ppid &&
	    !info->fields.name &&
	    !info->fields.exe && !info->fields.pcpu && !info->fields.state;
}


static inline struct ps_process *
ps_process_from_object(struct obus_object *object)
{
	const struct obus_object_desc *desc;

	if (!object)
		return NULL;

	desc = obus_object_get_desc(object);
	if (desc != &ps_process_desc)
		return NULL;

	return (struct ps_process *)object;
}


static inline struct obus_object *
ps_process_object(struct ps_process *object)
{
	struct obus_object *obj;

	obj = (struct obus_object *)object;
	if (ps_process_from_object(obj) != object)
		return NULL;

	return (struct obus_object *)object;
}


static inline const struct obus_object *
ps_process_const_object(const struct ps_process *object)
{
	const struct obus_object *obj;

	obj = (const struct obus_object *)object;
	if (obus_object_get_desc(obj) != &ps_process_desc)
		return NULL;

	return obj;
}


struct ps_process *
ps_process_new(struct obus_server *srv, const struct ps_process_info *info)
{
	obus_method_handler_cb_t *cbs = NULL;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = ps_process_desc.info_desc
	};

	return (struct ps_process *)obus_server_new_object(srv,
							   &ps_process_desc,
							   cbs,
							   info ? &st : NULL);
}

int ps_process_destroy(struct ps_process *object)
{
	return obus_object_destroy(ps_process_object(object));
}

int ps_process_register(struct obus_server *server, struct ps_process *object)
{
	struct obus_object *obj = (struct obus_object *)object;
	return obus_server_register_object(server, obj);
}

int ps_process_unregister(struct obus_server *server, struct ps_process *object)
{
	struct obus_object *obj = (struct obus_object *)object;
	return obus_server_unregister_object(server, obj);
}

int ps_process_is_registered(const struct ps_process *object)
{
	const struct obus_object *obj = (const struct obus_object *)object;
	return obus_object_is_registered(obj);
}


const struct ps_process_info *
ps_process_get_info(const struct ps_process *object)
{
	return (const struct ps_process_info *)
	    obus_object_get_info(ps_process_const_object(object));
}

void ps_process_log(const struct ps_process *object, enum obus_log_level level)
{
	obus_object_log(ps_process_const_object(object), level);
}

int ps_process_set_user_data(struct ps_process *object, void *user_data)
{
	return obus_object_set_user_data(ps_process_object(object), user_data);
}

void *ps_process_get_user_data(const struct ps_process *object)
{
	return obus_object_get_user_data(ps_process_const_object(object));
}

obus_handle_t ps_process_get_handle(const struct ps_process *object)
{
	return obus_object_get_handle(ps_process_const_object(object));
}

struct ps_process *
ps_process_from_handle(struct obus_server *server, obus_handle_t handle)
{
	struct obus_object *obj;

	obj = obus_server_get_object(server, handle);
	return ps_process_from_object(obj);
}

struct ps_process *
ps_process_next(struct obus_server *server, struct ps_process *previous)
{
	struct obus_object *next, *prev;

	prev = (struct obus_object *)previous;
	next = obus_server_object_next(server, prev, ps_process_desc.uid);
	return ps_process_from_object(next);
}

int ps_process_send_event(struct obus_server *server, struct ps_process *object,
			  enum ps_process_event_type type,
			  const struct ps_process_info *info)
{
	int ret;
	struct obus_event event;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = ps_process_desc.info_desc
	};

	if (!object || !server || type >= PS_PROCESS_EVENT_COUNT)
		return -EINVAL;

	ret =
	    obus_event_init(&event, ps_process_object(object),
			    &ps_process_events_desc[type], &st);
	if (ret < 0)
		return ret;

	return obus_server_send_event(server, &event);
}

int ps_bus_event_add_process_event(struct ps_bus_event *bus_event,
				   struct ps_process *object,
				   enum ps_process_event_type type,
				   const struct ps_process_info *info)
{
	int ret;
	struct obus_event *event;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = ps_process_desc.info_desc
	};

	if (!object || !bus_event || type >= PS_PROCESS_EVENT_COUNT)
		return -EINVAL;

	event =
	    obus_event_new(ps_process_object(object),
			   &ps_process_events_desc[type], &st);
	if (!event)
		return -ENOMEM;

	ret =
	    obus_bus_event_add_event((struct obus_bus_event *)bus_event, event);
	if (ret < 0)
		obus_event_destroy(event);

	return ret;
}

int ps_bus_event_register_process(struct ps_bus_event *bus_event,
				  struct ps_process *object)
{
	return obus_bus_event_register_object((struct obus_bus_event *)
					      bus_event,
					      ps_process_object(object));
}

int ps_bus_event_unregister_process(struct ps_bus_event *bus_event,
				    struct ps_process *object)
{
	return obus_bus_event_unregister_object((struct obus_bus_event *)
						bus_event,
						ps_process_object(object));
}
