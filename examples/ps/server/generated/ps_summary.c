/**
 * @file ps_summary.c
 *
 * @brief obus ps_summary object server api
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
#include "ps_summary.h"

int ps_summary_mode_is_valid(int32_t value)
{
	return (value == PS_SUMMARY_MODE_SOLARIS ||
		value == PS_SUMMARY_MODE_IRIX);
}

const char *ps_summary_mode_str(enum ps_summary_mode value)
{
	const char *str;

	switch (value) {
	case PS_SUMMARY_MODE_SOLARIS:
		str = "SOLARIS";
		break;
	case PS_SUMMARY_MODE_IRIX:
		str = "IRIX";
		break;
	default:
		str = "???";
		break;
	}

	return str;
}

static void ps_summary_mode_set_value(void *addr, int32_t value)
{
	enum ps_summary_mode *v = addr;
	*v = (enum ps_summary_mode)value;
}

static int32_t ps_summary_mode_get_value(const void *addr)
{
	const enum ps_summary_mode *v = addr;
	return (int32_t) (*v);
}

static void ps_summary_mode_format(const void *addr, char *buf, size_t size)
{
	const enum ps_summary_mode *v = addr;

	if (ps_summary_mode_is_valid((int32_t) (*v)))
		snprintf(buf, size, "%s", ps_summary_mode_str(*v));
	else
		snprintf(buf, size, "??? (%d)", (int32_t) (*v));
}

static const struct obus_enum_driver ps_summary_mode_driver = {
	.name = "ps_summary_mode",
	.size = sizeof(enum ps_summary_mode),
	.default_value = PS_SUMMARY_MODE_IRIX,
	.is_valid = ps_summary_mode_is_valid,
	.set_value = ps_summary_mode_set_value,
	.get_value = ps_summary_mode_get_value,
	.format = ps_summary_mode_format
};

enum ps_summary_field_type {
	PS_SUMMARY_FIELD_PCPUS = 0,
	PS_SUMMARY_FIELD_TASK_TOTAL,
	PS_SUMMARY_FIELD_TASK_RUNNING,
	PS_SUMMARY_FIELD_TASK_SLEEPING,
	PS_SUMMARY_FIELD_TASK_STOPPED,
	PS_SUMMARY_FIELD_TASK_ZOMBIE,
	PS_SUMMARY_FIELD_REFRESH_RATE,
	PS_SUMMARY_FIELD_MODE,
	PS_SUMMARY_FIELD_METHOD_SET_REFRESH_RATE,
	PS_SUMMARY_FIELD_METHOD_SET_MODE,
};

static const struct obus_field_desc ps_summary_info_fields[] = {
	[PS_SUMMARY_FIELD_PCPUS] = {
				    .uid = 1,
				    .name = "pcpus",
				    .offset =
				    obus_offsetof(struct ps_summary_info,
						  pcpus),
				    .role = OBUS_PROPERTY,
				    .type = OBUS_FIELD_F32 | OBUS_FIELD_ARRAY,
				    .nb_offset =
				    obus_offsetof(struct ps_summary_info,
						  n_pcpus),
				    },
	[PS_SUMMARY_FIELD_TASK_TOTAL] = {
					 .uid = 2,
					 .name = "task_total",
					 .offset =
					 obus_offsetof(struct ps_summary_info,
						       task_total),
					 .role = OBUS_PROPERTY,
					 .type = OBUS_FIELD_U32,
					 },
	[PS_SUMMARY_FIELD_TASK_RUNNING] = {
					   .uid = 3,
					   .name = "task_running",
					   .offset =
					   obus_offsetof(struct ps_summary_info,
							 task_running),
					   .role = OBUS_PROPERTY,
					   .type = OBUS_FIELD_U32,
					   },
	[PS_SUMMARY_FIELD_TASK_SLEEPING] = {
					    .uid = 4,
					    .name = "task_sleeping",
					    .offset =
					    obus_offsetof(struct
							  ps_summary_info,
							  task_sleeping),
					    .role = OBUS_PROPERTY,
					    .type = OBUS_FIELD_U32,
					    },
	[PS_SUMMARY_FIELD_TASK_STOPPED] = {
					   .uid = 5,
					   .name = "task_stopped",
					   .offset =
					   obus_offsetof(struct ps_summary_info,
							 task_stopped),
					   .role = OBUS_PROPERTY,
					   .type = OBUS_FIELD_U32,
					   },
	[PS_SUMMARY_FIELD_TASK_ZOMBIE] = {
					  .uid = 6,
					  .name = "task_zombie",
					  .offset =
					  obus_offsetof(struct ps_summary_info,
							task_zombie),
					  .role = OBUS_PROPERTY,
					  .type = OBUS_FIELD_U32,
					  },
	[PS_SUMMARY_FIELD_REFRESH_RATE] = {
					   .uid = 7,
					   .name = "refresh_rate",
					   .offset =
					   obus_offsetof(struct ps_summary_info,
							 refresh_rate),
					   .role = OBUS_PROPERTY,
					   .type = OBUS_FIELD_U32,
					   },
	[PS_SUMMARY_FIELD_MODE] = {
				   .uid = 8,
				   .name = "mode",
				   .offset =
				   obus_offsetof(struct ps_summary_info, mode),
				   .role = OBUS_PROPERTY,
				   .type = OBUS_FIELD_ENUM,
				   .enum_drv = &ps_summary_mode_driver,
				   },
	[PS_SUMMARY_FIELD_METHOD_SET_REFRESH_RATE] = {
						      .uid = 101,
						      .name =
						      "set_refresh_rate",
						      .offset =
						      obus_offsetof(struct
								    ps_summary_info,
								    method_set_refresh_rate),
						      .role = OBUS_METHOD,
						      .type = OBUS_FIELD_ENUM,
						      .enum_drv =
						      &obus_method_state_driver,
						      },
	[PS_SUMMARY_FIELD_METHOD_SET_MODE] = {
					      .uid = 102,
					      .name = "set_mode",
					      .offset =
					      obus_offsetof(struct
							    ps_summary_info,
							    method_set_mode),
					      .role = OBUS_METHOD,
					      .type = OBUS_FIELD_ENUM,
					      .enum_drv =
					      &obus_method_state_driver,
					      },

};

static const struct obus_struct_desc ps_summary_info_desc = {
	.size = sizeof(struct ps_summary_info),
	.fields_offset = obus_offsetof(struct ps_summary_info, fields),
	.n_fields = OBUS_SIZEOF_ARRAY(ps_summary_info_fields),
	.fields = ps_summary_info_fields,
};

static const struct obus_event_update_desc event_updated_updates[] = {
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_PCPUS],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_TASK_TOTAL],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_TASK_RUNNING],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_TASK_SLEEPING],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_TASK_STOPPED],
	 .flags = 0,
	 }

	,
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_TASK_ZOMBIE],
	 .flags = 0,
	 }

};

static const struct obus_event_update_desc event_refresh_rate_changed_updates[]
    = {
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_REFRESH_RATE],
	 .flags = 0,
	 }

};

static const struct obus_event_update_desc event_mode_changed_updates[] = {
	{
	 .field = &ps_summary_info_fields[PS_SUMMARY_FIELD_MODE],
	 .flags = 0,
	 }

};

static const struct obus_event_desc ps_summary_events_desc[] = {
	{
	 .uid = 1,
	 .name = "updated",
	 .updates = event_updated_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_updated_updates),
	 }

	,
	{
	 .uid = 2,
	 .name = "refresh_rate_changed",
	 .updates = event_refresh_rate_changed_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_refresh_rate_changed_updates),
	 }

	,
	{
	 .uid = 3,
	 .name = "mode_changed",
	 .updates = event_mode_changed_updates,
	 .n_updates = OBUS_SIZEOF_ARRAY(event_mode_changed_updates),
	 }

};

const char *ps_summary_event_type_str(enum ps_summary_event_type type)
{
	if (type >= OBUS_SIZEOF_ARRAY(ps_summary_events_desc))
		return "???";

	return ps_summary_events_desc[type].name;
}

enum ps_summary_method_type {
	PS_SUMMARY_METHOD_SET_REFRESH_RATE = 0,
	PS_SUMMARY_METHOD_SET_MODE,
	PS_SUMMARY_METHOD_COUNT,
};

static const struct obus_field_desc ps_summary_set_refresh_rate_args_fields[] = {
	{.uid = 1,
	 .name = "refresh_rate",
	 .offset =
	 obus_offsetof(struct ps_summary_set_refresh_rate_args, refresh_rate),
	 .role = OBUS_ARGUMENT,
	 .type = OBUS_FIELD_U32,
	 }
};

static const struct obus_struct_desc ps_summary_set_refresh_rate_args_desc = {
	.size = sizeof(struct ps_summary_set_refresh_rate_args),
	.fields_offset =
	    obus_offsetof(struct ps_summary_set_refresh_rate_args, fields),
	.n_fields = OBUS_SIZEOF_ARRAY(ps_summary_set_refresh_rate_args_fields),
	.fields = ps_summary_set_refresh_rate_args_fields,
};

static const struct obus_field_desc ps_summary_set_mode_args_fields[] = {
	{.uid = 1,
	 .name = "mode",
	 .offset = obus_offsetof(struct ps_summary_set_mode_args, mode),
	 .role = OBUS_ARGUMENT,
	 .type = OBUS_FIELD_ENUM,
	 .enum_drv = &ps_summary_mode_driver,
	 }
};

static const struct obus_struct_desc ps_summary_set_mode_args_desc = {
	.size = sizeof(struct ps_summary_set_mode_args),
	.fields_offset = obus_offsetof(struct ps_summary_set_mode_args, fields),
	.n_fields = OBUS_SIZEOF_ARRAY(ps_summary_set_mode_args_fields),
	.fields = ps_summary_set_mode_args_fields,
};

static const struct obus_method_desc ps_summary_methods_desc[] = {
	{.uid = 101,
	 .name = "set_refresh_rate",
	 .args_desc = &ps_summary_set_refresh_rate_args_desc,
	 }
	,
	{.uid = 102,
	 .name = "set_mode",
	 .args_desc = &ps_summary_set_mode_args_desc,
	 }
};

const struct obus_object_desc ps_summary_desc = {
	.uid = PS_SUMMARY_UID,
	.name = "summary",
	.info_desc = &ps_summary_info_desc,
	.n_events = OBUS_SIZEOF_ARRAY(ps_summary_events_desc),
	.events = ps_summary_events_desc,
	.n_methods = OBUS_SIZEOF_ARRAY(ps_summary_methods_desc),
	.methods = ps_summary_methods_desc,
};

int ps_summary_method_handlers_is_valid(const struct ps_summary_method_handlers
					*handlers)
{
	return handlers &&
	    handlers->method_set_refresh_rate && handlers->method_set_mode;
}

int ps_summary_set_refresh_rate_args_is_complete(const struct
						 ps_summary_set_refresh_rate_args
						 *args)
{
	return args && args->fields.refresh_rate;
}

int ps_summary_set_mode_args_is_complete(const struct ps_summary_set_mode_args
					 *args)
{
	return args && args->fields.mode;
}

void ps_summary_info_init(struct ps_summary_info *info)
{
	if (info)
		memset(info, 0, sizeof(*info));
}

int ps_summary_info_is_empty(const struct ps_summary_info *info)
{
	return info &&
	    !info->fields.pcpus &&
	    !info->fields.task_total &&
	    !info->fields.task_running &&
	    !info->fields.task_sleeping &&
	    !info->fields.task_stopped &&
	    !info->fields.task_zombie &&
	    !info->fields.refresh_rate &&
	    !info->fields.mode &&
	    !info->fields.method_set_refresh_rate &&
	    !info->fields.method_set_mode;
}

void ps_summary_info_set_methods_state(struct ps_summary_info *info,
				       enum obus_method_state state)
{
	OBUS_SET(info, method_set_refresh_rate, state);
	OBUS_SET(info, method_set_mode, state);
}


static inline struct ps_summary *
ps_summary_from_object(struct obus_object *object)
{
	const struct obus_object_desc *desc;

	if (!object)
		return NULL;

	desc = obus_object_get_desc(object);
	if (desc != &ps_summary_desc)
		return NULL;

	return (struct ps_summary *)object;
}


static inline struct obus_object *
ps_summary_object(struct ps_summary *object)
{
	struct obus_object *obj;

	obj = (struct obus_object *)object;
	if (ps_summary_from_object(obj) != object)
		return NULL;

	return (struct obus_object *)object;
}


static inline const struct obus_object *
ps_summary_const_object(const struct ps_summary *object)
{
	const struct obus_object *obj;

	obj = (const struct obus_object *)object;
	if (obus_object_get_desc(obj) != &ps_summary_desc)
		return NULL;

	return obj;
}


struct ps_summary *
ps_summary_new(struct obus_server *srv, const struct ps_summary_info *info,
	       const struct ps_summary_method_handlers *handlers)
{
	obus_method_handler_cb_t cbs[PS_SUMMARY_METHOD_COUNT];
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = ps_summary_desc.info_desc
	};

	cbs[PS_SUMMARY_METHOD_SET_REFRESH_RATE] =
	    (obus_method_handler_cb_t) handlers->method_set_refresh_rate;
	cbs[PS_SUMMARY_METHOD_SET_MODE] =
	    (obus_method_handler_cb_t) handlers->method_set_mode;

	return (struct ps_summary *)obus_server_new_object(srv,
							   &ps_summary_desc,
							   cbs,
							   info ? &st : NULL);
}

int ps_summary_destroy(struct ps_summary *object)
{
	return obus_object_destroy(ps_summary_object(object));
}

int ps_summary_register(struct obus_server *server, struct ps_summary *object)
{
	struct obus_object *obj = (struct obus_object *)object;
	return obus_server_register_object(server, obj);
}

int ps_summary_unregister(struct obus_server *server, struct ps_summary *object)
{
	struct obus_object *obj = (struct obus_object *)object;
	return obus_server_unregister_object(server, obj);
}

int ps_summary_is_registered(const struct ps_summary *object)
{
	const struct obus_object *obj = (const struct obus_object *)object;
	return obus_object_is_registered(obj);
}


const struct ps_summary_info *
ps_summary_get_info(const struct ps_summary *object)
{
	return (const struct ps_summary_info *)
	    obus_object_get_info(ps_summary_const_object(object));
}

void ps_summary_log(const struct ps_summary *object, enum obus_log_level level)
{
	obus_object_log(ps_summary_const_object(object), level);
}

int ps_summary_set_user_data(struct ps_summary *object, void *user_data)
{
	return obus_object_set_user_data(ps_summary_object(object), user_data);
}

void *ps_summary_get_user_data(const struct ps_summary *object)
{
	return obus_object_get_user_data(ps_summary_const_object(object));
}

obus_handle_t ps_summary_get_handle(const struct ps_summary *object)
{
	return obus_object_get_handle(ps_summary_const_object(object));
}

struct ps_summary *
ps_summary_from_handle(struct obus_server *server, obus_handle_t handle)
{
	struct obus_object *obj;

	obj = obus_server_get_object(server, handle);
	return ps_summary_from_object(obj);
}

struct ps_summary *
ps_summary_next(struct obus_server *server, struct ps_summary *previous)
{
	struct obus_object *next, *prev;

	prev = (struct obus_object *)previous;
	next = obus_server_object_next(server, prev, ps_summary_desc.uid);
	return ps_summary_from_object(next);
}

int ps_summary_send_event(struct obus_server *server, struct ps_summary *object,
			  enum ps_summary_event_type type,
			  const struct ps_summary_info *info)
{
	int ret;
	struct obus_event event;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = ps_summary_desc.info_desc
	};

	if (!object || !server || type >= PS_SUMMARY_EVENT_COUNT)
		return -EINVAL;

	ret =
	    obus_event_init(&event, ps_summary_object(object),
			    &ps_summary_events_desc[type], &st);
	if (ret < 0)
		return ret;

	return obus_server_send_event(server, &event);
}

int ps_bus_event_add_summary_event(struct ps_bus_event *bus_event,
				   struct ps_summary *object,
				   enum ps_summary_event_type type,
				   const struct ps_summary_info *info)
{
	int ret;
	struct obus_event *event;
	struct obus_struct st = {
		.u.const_addr = info,
		.desc = ps_summary_desc.info_desc
	};

	if (!object || !bus_event || type >= PS_SUMMARY_EVENT_COUNT)
		return -EINVAL;

	event =
	    obus_event_new(ps_summary_object(object),
			   &ps_summary_events_desc[type], &st);
	if (!event)
		return -ENOMEM;

	ret =
	    obus_bus_event_add_event((struct obus_bus_event *)bus_event, event);
	if (ret < 0)
		obus_event_destroy(event);

	return ret;
}

int ps_bus_event_register_summary(struct ps_bus_event *bus_event,
				  struct ps_summary *object)
{
	return obus_bus_event_register_object((struct obus_bus_event *)
					      bus_event,
					      ps_summary_object(object));
}

int ps_bus_event_unregister_summary(struct ps_bus_event *bus_event,
				    struct ps_summary *object)
{
	return obus_bus_event_unregister_object((struct obus_bus_event *)
						bus_event,
						ps_summary_object(object));
}
