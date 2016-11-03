/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_event.c
 *
 * @brief obus object event implementation
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

OBUS_API int obus_event_init(struct obus_event *event,
			     struct obus_object *obj,
			     const struct obus_event_desc *desc,
			     const struct obus_struct *info)
{
	if (!event || !obj || !desc || !info || !info->u.addr)
		return -EINVAL;

	if (info->desc != obj->desc->info_desc) {
		obus_error("invalid event struct info");
		return -EINVAL;
	}

	memset(event, 0 , sizeof(*event));
	/* set addr to null (for non dynamic allocated event) */
	obus_node_unref(&event->node);
	obus_node_unref(&event->event_node);
	event->desc = desc;
	event->is_allocated = 0;
	event->is_committed = 0;
	event->obj = obj;
	event->info.desc = obj->desc->info_desc;
	event->info.u.addr = info->u.addr;
	return 0;
}

OBUS_API
struct obus_event *obus_event_new(struct obus_object *obj,
				  const struct obus_event_desc *desc,
				  const struct obus_struct *info)
{
	struct obus_event *event;
	off_t info_offset;
	size_t size;
	int ret;

	if (!obj || !desc)
		return NULL;

	if (info && info->desc != obj->desc->info_desc) {
		obus_error("invalid event struct info");
		return NULL;
	}

	/* compute event size allocation */
	size = sizeof(*event);
	info_offset = (off_t)size;
	size += obj->desc->info_desc->size;

	/* allocate object */
	event = calloc(1, size);
	if (!event)
		return NULL;

	/* init event */
	obus_node_unref(&event->node);
	obus_node_unref(&event->event_node);
	event->desc = desc;
	event->obj = obj;
	event->is_committed = 0;
	event->is_allocated = 1;
	event->info.desc = obj->desc->info_desc;
	event->info.u.addr = ((uint8_t *)event + info_offset);

	/* init event info struct */
	ret = obus_struct_init(&event->info);
	if (ret < 0)
		goto error;

	/* merge info struct if given */
	if (info) {
		ret = obus_struct_merge(&event->info, info);
		if (ret < 0)
			goto error;
	}

	return event;

error:
	free(event);
	return NULL;
}

OBUS_API int obus_event_destroy(struct obus_event *event)
{
	if (!event)
		return -EINVAL;

	if (!event->is_allocated) {
		obus_error("refusing to destroy non mallocated event");
		return -EPERM;
	}

	/**
	 * if event has been added in an bus event,
	 * remove it from bus event object event list
	 **/
	if (obus_node_is_ref(&event->event_node))
		obus_list_del(&event->event_node);

	obus_struct_destroy(&event->info);
	free(event);
	return 0;
}

OBUS_API
struct obus_object *obus_event_get_object(struct obus_event *event)
{
	return event ? event->obj : NULL;
}

OBUS_API
const struct obus_object_desc *
obus_event_get_object_desc(const struct obus_event *event)
{
	return event ? event->obj->desc : NULL;
}

OBUS_API
const struct obus_event_desc *
obus_event_get_desc(const struct obus_event *event)
{
	return event ? event->desc : NULL;
}

OBUS_API
int obus_event_is_committed(struct obus_event *event)
{
	return event ? event->is_committed : 0;
}

OBUS_API
const void *obus_event_get_info(const struct obus_event *event)
{
	return event ? event->info.u.addr : NULL;
}

OBUS_API
void obus_event_log(const struct obus_event *event, enum obus_log_level level)
{
	if (!event)
		return;

	obus_log(level, "EVENT:%-17.17s", event->desc->name);
	obus_log(level, "|-O:%-20.20s = %d", event->obj->desc->name,
		 event->obj->handle);
	obus_struct_log(&event->info, level);
}

int obus_event_sanitize(struct obus_event *event, int is_server)
{
	const struct obus_event_desc *desc;
	struct obus_struct *st;
	int found;
	int ret;
	size_t i, j;

	st = &event->info;
	desc = event->desc;

	/* iterate on each fields and remove those not described in events */
	ret = 0;
	for (i = 0; i < st->desc->n_fields; i++) {
		if (!obus_struct_has_field(st, &st->desc->fields[i]))
			continue;

		/* find field in update list */
		found = 0;
		for (j = 0; j < desc->n_updates; j++) {
			if (st->desc->fields[i].uid ==
			    desc->updates[j].field->uid) {
				found = 1;
				break;
			}
		}

		if (found)
			continue;

		if (is_server) {
			obus_error("object '%s' (handle=%d) event '%s' "
				  "can't update %s '%s'. "
				  "Server must fix this error",
				  event->obj->desc->name, event->obj->handle,
				  event->desc->name,
				  (st->desc->fields[i].role == OBUS_PROPERTY) ?
				  "property" : "method",
				  st->desc->fields[i].name);
		} else {
			obus_warn("object '%s' (handle=%d) event '%s' "
				  "updates undeclared %s '%s'",
				  event->obj->desc->name, event->obj->handle,
				  event->desc->name,
				  (st->desc->fields[i].role == OBUS_PROPERTY) ?
				  "property" : "method",
				  st->desc->fields[i].name);
		}

		/* for server only clear this field,
		 * update field will not be committed,
		 * for client update field */
		if (is_server)
			obus_struct_clear_has_field(st, &st->desc->fields[i]);

		ret++;
	}
	return ret;
}

int obus_event_encode(struct obus_event *event, struct obus_buffer *buf)
{
	int ret;
	size_t offset, length;

	/* add object uid */
	ret = obus_buffer_append_u16(buf, event->obj->desc->uid);
	if (ret < 0)
		return ret;

	/* add object handle */
	ret = obus_buffer_append_u16(buf, event->obj->handle);
	if (ret < 0)
		return ret;

	/* add event uid */
	ret = obus_buffer_append_u16(buf, event->desc->uid);
	if (ret < 0)
		return ret;

	/* get event data size offset in buffer */
	offset = obus_buffer_write_offset(buf);

	/* reserve 4 bytes in buffer for event data size */
	ret = obus_buffer_reserve(buf, sizeof(uint32_t));
	if (ret < 0)
		return ret;

	/* add event struct content */
	ret = obus_struct_encode(&event->info, buf);
	if (ret < 0)
		return ret;

	/* compute event data size */
	length = obus_buffer_length(buf) - (offset + sizeof(uint32_t));

	/* write event data size */
	obus_buffer_write_u32(buf, (uint32_t)length, offset);
	return 0;
}

struct obus_event *obus_event_decode(struct obus_bus *bus,
				     struct obus_buffer *buf)
{
	int ret;
	const struct obus_event_desc *desc;
	struct obus_object *obj;
	struct obus_event *event = NULL;
	size_t offset, end_pos;
	obus_handle_t handle, uid, event_uid;
	uint32_t size;

	/* read object uid */
	ret = obus_buffer_read_u16(buf, &uid);
	if (ret < 0)
		goto error;

	/* read object handle */
	ret = obus_buffer_read_u16(buf, &handle);
	if (ret < 0)
		goto error;

	/* read event uid */
	ret = obus_buffer_read_u16(buf, &event_uid);
	if (ret < 0)
		goto error;

	/* read event data size */
	ret = obus_buffer_read_u32(buf, &size);
	if (ret < 0)
		goto error;

	/* get buffer read position */
	offset = obus_buffer_get_read_position(buf);
	end_pos = offset + size;

	/* check object uid */
	if (uid == OBUS_INVALID_UID) {
		obus_warn("can't decode object event, invalid object uid");
		goto eat_bytes;
	}

	/* check object handle value */
	if (handle == OBUS_INVALID_HANDLE) {
		obus_warn("can't decode object event, invalid object handle");
		goto eat_bytes;
	}

	/* check event uid */
	if (event_uid == OBUS_INVALID_UID) {
		obus_warn("can't decode object event, invalid event uid");
		goto eat_bytes;
	}

	/* find object given its handle & uid ? */
	obj = obus_bus_object(bus, handle);
	if (!obj || obj->desc->uid != uid) {
		obus_warn("can't decode object event: object "
			  "{uid=%d hande=%d} not found", uid, handle);
		goto eat_bytes;
	}

	/* find event descriptor */
	desc = obus_bus_api_event(&bus->api, obj->desc->uid, event_uid);
	if (!desc) {
		obus_warn("can't decode object {uid=%d hande=%d} event uid=%d:"
			  " descriptor not found", uid, handle, event_uid);
		goto eat_bytes;
	}

	/* create event */
	event = obus_event_new(obj, desc, NULL);
	if (!event)
		goto eat_bytes;

	/* decode event struct content */
	ret = obus_struct_decode(&event->info, buf);
	if (ret < 0) {
		obus_warn("can't decode object {uid=%d, name='%s'} event "
			  "{uid=%d, name='%s'} data", obj->desc->uid,
			  obj->desc->name, desc->uid, desc->name);
		goto eat_bytes;
	}

	/* check read position is ok */
	if (obus_buffer_get_read_position(buf) != end_pos) {
		obus_warn("object {uid=%d, name='%s'}  event "
			  "{uid=%d, name='%s'} data size mismatch",
			  obj->desc->uid, obj->desc->name, desc->uid,
			  desc->name);

		/* set read position (should be already done ...)*/
		obus_buffer_set_read_position(buf, end_pos);
	}
	return event;

eat_bytes:
	obus_buffer_set_read_position(buf, end_pos);
error:
	obus_event_destroy(event);
	return NULL;
}

OBUS_API
int obus_event_commit(struct obus_event *event)
{
	int ret;

	if (!event)
		return -EINVAL;

	if (event->is_committed)
		return 0;

	/* merge struct */
	ret = obus_struct_merge(&event->obj->info, &event->info);
	if (ret < 0)
		return ret;

	event->is_committed = 1;
	return 0;
}

OBUS_API int obus_event_is_empty(const struct obus_event *event)
{
	return event ? obus_struct_is_empty(&event->info) : 1;
}
