/**
 * @file ps_process.c
 *
 * @brief obus ps_process object client api
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
ps_process_from_handle(struct obus_client *client, obus_handle_t handle)
{
	struct obus_object *obj;

	obj = obus_client_get_object(client, handle);
	return ps_process_from_object(obj);
}

struct ps_process *
ps_process_next(struct obus_client *client, struct ps_process *previous)
{
	struct obus_object *next, *prev;

	prev = (struct obus_object *)previous;
	next = obus_client_object_next(client, prev, ps_process_desc.uid);
	return ps_process_from_object(next);
}

static inline struct obus_event *ps_process_obus_event(struct ps_process_event
						       *event)
{
	return event
	    && (obus_event_get_object_desc((struct obus_event *)event) ==
		&ps_process_desc) ? (struct obus_event *)event : NULL;
}

static inline const struct obus_event *ps_process_const_obus_event(const struct
								   ps_process_event
								   *event)
{
	return event
	    && (obus_event_get_object_desc((const struct obus_event *)event) ==
		&ps_process_desc) ? (const struct obus_event *)event : NULL;
}


enum ps_process_event_type
ps_process_event_get_type(const struct ps_process_event *event)
{
	const struct obus_event_desc *desc;
	desc = obus_event_get_desc(ps_process_const_obus_event(event));
	return desc ? (enum ps_process_event_type)(desc -
						   ps_process_events_desc) :
	    PS_PROCESS_EVENT_COUNT;
}

void ps_process_event_log(const struct ps_process_event *event,
			  enum obus_log_level level)
{
	obus_event_log(ps_process_const_obus_event(event), level);
}

int ps_process_event_is_empty(const struct ps_process_event *event)
{
	return obus_event_is_empty(ps_process_const_obus_event(event));
}

int ps_process_event_commit(struct ps_process_event *event)
{
	return obus_event_commit(ps_process_obus_event(event));
}

const struct ps_process_info *

ps_process_event_get_info(const struct ps_process_event *event)
{
	return (const struct ps_process_info *)
	    obus_event_get_info(ps_process_const_obus_event(event));
}

/**
 * @brief subscribe to events concerning ps_process objects.
 *
 * @param[in] client bus client.
 * @param[in] provider callback set for reacting on ps_process events.
 * @param[in] user_data data passed to callbacks on events.
 *
 * @retval 0 success.
 **/
int ps_process_subscribe(struct obus_client *client,
			 struct ps_process_provider *provider, void *user_data)
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
	p->desc = &ps_process_desc;
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
 * @brief unsubscribe to events concerning ps_process objects.
 *
 * @param[in] client bus client.
 * @param[in] provider passed to ps_process_subscribe.
 *
 * @retval 0 success.
 **/ int ps_process_unsubscribe(struct obus_client *client,
				struct ps_process_provider *provider)
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
