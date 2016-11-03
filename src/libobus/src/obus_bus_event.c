/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_event.c
 *
 * @brief obus bus event
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

void obus_bus_event_init(struct obus_bus_event *event,
			const struct obus_bus_event_desc *desc)
{
	event->desc = desc;
	obus_list_init(&event->add_objs);
	obus_list_init(&event->remove_objs);
	obus_list_init(&event->obj_events);
	event->is_committed = 0;
	event->is_allocated = 0;
}

OBUS_API
const struct obus_bus_event_desc *
obus_bus_event_get_desc(const struct obus_bus_event *event)
{
	return event ? event->desc : NULL;
}

OBUS_API
int obus_bus_event_register_object(struct obus_bus_event *bus_event,
				struct obus_object *object)
{
	if (!bus_event || !object)
		return -EINVAL;

	/* object can only be added once in a bus event */
	if (obus_node_is_ref(&object->event_node)) {
		obus_error("%s: object '%s' already attached to a bus event",
			   __func__, object->desc->name);
		return -EPERM;
	}

	/* add object in bus event */
	obus_list_add_before(&bus_event->add_objs, &object->event_node);
	return 0;
}

OBUS_API
int obus_bus_event_unregister_object(struct obus_bus_event *bus_event,
				     struct obus_object *object)
{
	if (!bus_event || !object)
		return -EINVAL;

	/* object can only be added once in a bus event */
	if (obus_node_is_ref(&object->event_node)) {
		obus_error("%s: object '%s' already attached to a bus event",
			   __func__, object->desc->name);
		return -EPERM;
	}

	/* add object in bus event */
	obus_list_add_before(&bus_event->remove_objs, &object->event_node);
	return 0;
}

OBUS_API
int obus_bus_event_add_event(struct obus_bus_event *bus_event,
			     struct obus_event *event)
{
	if (!bus_event || !event)
		return -EINVAL;

	/* object event can only be added once in a bus event */
	if (obus_node_is_ref(&event->event_node)) {
		obus_error("%s: event '%s' is already attached to a bus event",
			   __func__, event->desc->name);
		return -EPERM;
	}

	/* add object event in bus event */
	obus_list_add_before(&bus_event->obj_events, &event->event_node);
	return 0;
}

OBUS_API struct obus_bus_event *
obus_bus_event_new(const struct obus_bus_event_desc *desc)
{
	struct obus_bus_event *event;

	if (!desc)
		return NULL;

	event = calloc(1, sizeof(*event));
	if (!event)
		return NULL;

	obus_bus_event_init(event, desc);
	event->is_allocated = 1;
	return event;
}

OBUS_API int obus_bus_event_destroy(struct obus_bus_event *event)
{
	struct obus_event *evt, *tmp;
	struct obus_object *obj, *otmp;

	if (!event)
		return -EINVAL;

	/**
	 * destroy object events
	 * */
	obus_list_walk_entry_forward_safe(&event->obj_events, evt, tmp,
					  event_node) {
		obus_list_del(&evt->event_node);
		obus_event_destroy(evt);
	}

	/**
	 * only unref object from bus event.
	 * object added or removed in bus event are owned by server.
	 * */
	obus_list_walk_entry_forward_safe(&event->add_objs, obj, otmp,
					  event_node) {
		obus_list_del(&obj->event_node);
	}

	obus_list_walk_entry_forward_safe(&event->remove_objs, obj, otmp,
					  event_node) {
		obus_list_del(&obj->event_node);
		/* destroy object */
		obus_object_destroy(obj);
	}

	if (event->is_allocated)
		free(event);

	return 0;
}

OBUS_API
void obus_bus_event_log(struct obus_bus_event *event,
			enum obus_log_level level,
			int only_header)
{
	struct obus_event *evt;
	struct obus_object *obj;


	if (!event)
		return;

	obus_log(level, "BUS EVENT:%-15.15s", event->desc->name);
	obus_log(level, "|-%zu objects registered",
		 obus_list_length(&event->add_objs));
	obus_log(level, "|-%zu objects unregistered",
		 obus_list_length(&event->remove_objs));
	obus_log(level, "|-%zu objects events",
		 obus_list_length(&event->obj_events));

	if (only_header)
		return;

	obus_list_walk_entry_forward(&event->add_objs, obj, event_node) {
		obus_log(level, "object registered:");
		obus_object_log(obj, level);
	}

	obus_list_walk_entry_forward(&event->remove_objs, obj, event_node) {
		obus_log(level, "object unregistered:");
		obus_object_log(obj, level);
	}

	obus_list_walk_entry_forward(&event->obj_events, evt, event_node) {
		obus_log(level, "event sent:");
		obus_event_log(evt, level);
	}
}

int obus_bus_event_encode(struct obus_bus_event *event,
			  struct obus_buffer *buf)
{
	int ret;
	uint32_t n_items;
	struct obus_event *evt;
	struct obus_object *obj;

	/* add bus event uid */
	ret = obus_buffer_append_u16(buf, event->desc->uid);
	if (ret < 0)
		return ret;

	/* add number of registered objects */
	n_items = (uint32_t)obus_list_length(&event->add_objs);
	ret = obus_buffer_append_u32(buf, n_items);
	if (ret < 0)
		return ret;

	/* add number of unregistered objects */
	n_items = (uint32_t)obus_list_length(&event->remove_objs);
	ret = obus_buffer_append_u32(buf, n_items);
	if (ret < 0)
		return ret;

	/* add number of objects events */
	n_items = (uint32_t)obus_list_length(&event->obj_events);
	ret = obus_buffer_append_u32(buf, n_items);
	if (ret < 0)
		return ret;

	/* encode object */
	obus_list_walk_entry_forward(&event->add_objs, obj, event_node) {
		/* encode object */
		ret = obus_object_add_encode(obj, buf);
		if (ret < 0)
			return ret;
	}

	/* encode number of unregistered object */
	obus_list_walk_entry_forward(&event->remove_objs, obj, event_node) {
		/* encode unregister object */
		ret = obus_object_remove_encode(obj, buf);
		if (ret < 0)
			return ret;
	}

	/* encode object event */
	obus_list_walk_entry_forward(&event->obj_events, evt, event_node) {
		/* encode object */
		ret = obus_event_encode(evt, buf);
		if (ret < 0)
			return ret;
	}

	return 0;
}

struct obus_bus_event *obus_bus_event_decode(struct obus_bus *bus,
					     struct obus_buffer *buf)
{
	int ret;
	uint16_t uid;
	uint32_t i, n_add_objs, n_remove_objs, n_obj_events;
	const struct obus_bus_event_desc *desc;
	struct obus_bus_event *event;
	struct obus_object *obj;
	struct obus_event *evt;

	/* read object uid */
	ret = obus_buffer_read_u16(buf, &uid);
	if (ret < 0)
		goto error;

	/* read event data size */
	ret = obus_buffer_read_u32(buf, &n_add_objs);
	if (ret < 0)
		goto error;

	/* read event data size */
	ret = obus_buffer_read_u32(buf, &n_remove_objs);
	if (ret < 0)
		goto error;

	/* read event data size */
	ret = obus_buffer_read_u32(buf, &n_obj_events);
	if (ret < 0)
		goto error;

	/* get bus event desc */
	desc = obus_bus_api_bus_event(&bus->api, uid);
	if (!desc)
		goto error;

	/* create event */
	event = obus_bus_event_new(desc);
	if (!event)
		goto error;

	/* parse add objects */
	for (i = 0; i < n_add_objs; i++) {
		obj = obus_object_add_decode(&bus->api, buf);
		if (!obj)
			continue;

		/* add object in event list */
		obus_list_add_before(&event->add_objs, &obj->event_node);
	}

	/* parse remove objects */
	for (i = 0; i < n_remove_objs; i++) {
		obj = obus_object_remove_decode(bus, buf);
		if (!obj)
			continue;

		/* add object in event list */
		obus_list_add_before(&event->remove_objs, &obj->event_node);
	}

	/* parse event objects */
	for (i = 0; i < n_obj_events; i++) {
		evt = obus_event_decode(bus, buf);
		if (!evt)
			continue;

		/* add event in object event list */
		obus_list_add_before(&event->obj_events, &evt->event_node);
	}

	return event;

error:
	return NULL;
}
