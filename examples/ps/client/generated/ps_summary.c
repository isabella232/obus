/**
 * @file ps_summary.c
 *
 * @brief obus ps_summary object client api
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
ps_summary_from_handle(struct obus_client *client, obus_handle_t handle)
{
	struct obus_object *obj;

	obj = obus_client_get_object(client, handle);
	return ps_summary_from_object(obj);
}

struct ps_summary *
ps_summary_next(struct obus_client *client, struct ps_summary *previous)
{
	struct obus_object *next, *prev;

	prev = (struct obus_object *)previous;
	next = obus_client_object_next(client, prev, ps_summary_desc.uid);
	return ps_summary_from_object(next);
}

static inline struct obus_event *ps_summary_obus_event(struct ps_summary_event
						       *event)
{
	return event
	    && (obus_event_get_object_desc((struct obus_event *)event) ==
		&ps_summary_desc) ? (struct obus_event *)event : NULL;
}

static inline const struct obus_event *ps_summary_const_obus_event(const struct
								   ps_summary_event
								   *event)
{
	return event
	    && (obus_event_get_object_desc((const struct obus_event *)event) ==
		&ps_summary_desc) ? (const struct obus_event *)event : NULL;
}


enum ps_summary_event_type
ps_summary_event_get_type(const struct ps_summary_event *event)
{
	const struct obus_event_desc *desc;
	desc = obus_event_get_desc(ps_summary_const_obus_event(event));
	return desc ? (enum ps_summary_event_type)(desc -
						   ps_summary_events_desc) :
	    PS_SUMMARY_EVENT_COUNT;
}

void ps_summary_event_log(const struct ps_summary_event *event,
			  enum obus_log_level level)
{
	obus_event_log(ps_summary_const_obus_event(event), level);
}

int ps_summary_event_is_empty(const struct ps_summary_event *event)
{
	return obus_event_is_empty(ps_summary_const_obus_event(event));
}

int ps_summary_event_commit(struct ps_summary_event *event)
{
	return obus_event_commit(ps_summary_obus_event(event));
}

const struct ps_summary_info *

ps_summary_event_get_info(const struct ps_summary_event *event)
{
	return (const struct ps_summary_info *)
	    obus_event_get_info(ps_summary_const_obus_event(event));
}

void ps_summary_set_refresh_rate_args_init(struct
					   ps_summary_set_refresh_rate_args
					   *args)
{
	if (args)
		memset(args, 0, sizeof(*args));
}

int ps_summary_set_refresh_rate_args_is_empty(const struct
					      ps_summary_set_refresh_rate_args
					      *args)
{
	return (args && !args->fields.refresh_rate);
}

int ps_summary_call_set_refresh_rate(struct obus_client *client,
				     struct ps_summary *object,
				     const struct
				     ps_summary_set_refresh_rate_args *args,
				     ps_summary_method_status_cb_t cb,
				     uint16_t *handle)
{
	const struct obus_method_desc *desc =
	    &ps_summary_methods_desc[PS_SUMMARY_METHOD_SET_REFRESH_RATE];
	struct obus_struct st = {.u.const_addr = args,.desc = desc->args_desc };
	return obus_client_call(client, ps_summary_object(object), desc, &st,
				(obus_method_call_status_handler_cb_t) cb,
				handle);
}

void ps_summary_set_mode_args_init(struct ps_summary_set_mode_args *args)
{
	if (args)
		memset(args, 0, sizeof(*args));
}

int ps_summary_set_mode_args_is_empty(const struct ps_summary_set_mode_args
				      *args)
{
	return (args && !args->fields.mode);
}

int ps_summary_call_set_mode(struct obus_client *client,
			     struct ps_summary *object,
			     const struct ps_summary_set_mode_args *args,
			     ps_summary_method_status_cb_t cb,
			     uint16_t *handle)
{
	const struct obus_method_desc *desc =
	    &ps_summary_methods_desc[PS_SUMMARY_METHOD_SET_MODE];
	struct obus_struct st = {.u.const_addr = args,.desc = desc->args_desc };
	return obus_client_call(client, ps_summary_object(object), desc, &st,
				(obus_method_call_status_handler_cb_t) cb,
				handle);
}

/**
 * @brief subscribe to events concerning ps_summary objects.
 *
 * @param[in] client bus client.
 * @param[in] provider callback set for reacting on ps_summary events.
 * @param[in] user_data data passed to callbacks on events.
 *
 * @retval 0 success.
 **/
int ps_summary_subscribe(struct obus_client *client,
			 struct ps_summary_provider *provider, void *user_data)
{
	struct obus_provider *p;
	int ret;
	if (!client || !provider || !provider->add || !provider->remove
	    || !provider->event)
		return -EINVAL;

	p = calloc(1, sizeof(*p));
	if (!p)
		return -ENOMEM;

	p->add = (obus_provider_add_cb_t) provider->add;
	p->remove = (obus_provider_remove_cb_t) provider->remove;
	p->event = (obus_provider_event_cb_t) provider->event;
	p->desc = &ps_summary_desc;
	p->user_data = user_data;

	ret = obus_client_register_provider(client, p);
	if (ret < 0) {
		free(p);
		return ret;
	}

	provider->priv = p;
	return 0;
}

/**
 * @brief unsubscribe to events concerning ps_summary objects.
 *
 * @param[in] client bus client.
 * @param[in] provider passed to ps_summary_subscribe.
 *
 * @retval 0 success.
 **/ int ps_summary_unsubscribe(struct obus_client *client,
				struct ps_summary_provider *provider)
{
	int ret;
	if (!client || !provider)
		return -EINVAL;

	ret = obus_client_unregister_provider(client, provider->priv);
	if (ret < 0)
		return ret;

	free(provider->priv);
	provider->priv = NULL;
	return 0;
}
