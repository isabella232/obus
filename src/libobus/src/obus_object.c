/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_object.c
 *
 * @brief obus object implementation
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

OBUS_API
const char *obus_method_state_str(enum obus_method_state state)
{
	switch (state) {
	case OBUS_METHOD_NOT_SUPPORTED: return "NOT_SUPPORTED";
	case OBUS_METHOD_ENABLED: return "ENABLED";
	case OBUS_METHOD_DISABLED: return "DISABLED";
	default: return "???";
	}
	return NULL;
}

static int obus_method_state_is_valid(int32_t value)
{
	return (value == OBUS_METHOD_ENABLED ||
		value == OBUS_METHOD_NOT_SUPPORTED ||
		value == OBUS_METHOD_DISABLED);
}

static void obus_method_state_set_value(void *addr, int32_t value)
{
	enum obus_method_state *state = addr;
	*state = (enum obus_method_state)value;
}

static int32_t obus_method_state_get_value(const void *addr)
{
	const enum obus_method_state *state = addr;
	return (int32_t)(*state);
}

static void obus_method_state_format(const void *addr,
				    char *buf, size_t size)
{
	const enum obus_method_state *state = addr;
	snprintf(buf, size, "%s", (obus_method_state_str(*state)));
}

OBUS_API const struct obus_enum_driver obus_method_state_driver = {
	"obus_method_state",
	sizeof(enum obus_method_state),
	OBUS_METHOD_DISABLED,
	obus_method_state_is_valid,
	obus_method_state_set_value,
	obus_method_state_get_value,
	obus_method_state_format,
};

struct obus_object *obus_object_new(const struct obus_object_desc *desc,
				    const obus_method_handler_cb_t *cbs,
				    const struct obus_struct *info)
{
	struct obus_object *obj;
	off_t info_offset;
	uint32_t i;
	size_t size;
	int ret;

	if (!desc)
		return NULL;

	if (info && info->desc != desc->info_desc) {
		obus_error("invalid object struct info");
		return NULL;
	}

	/* compute object size allocation */
	size = sizeof(*obj);
	size += (desc->n_methods * sizeof(obus_method_handler_cb_t));
	info_offset = (off_t)size;
	size += desc->info_desc->size;

	/* allocate object */
	obj = calloc(1, size);
	if (!obj)
		return NULL;

	/* init object */
	obus_node_unref(&obj->node);
	obus_node_unref(&obj->event_node);
	obj->desc = desc;
	obj->handle = OBUS_INVALID_HANDLE;
	obj->info.desc = desc->info_desc;
	obj->info.u.addr = ((uint8_t *)obj + info_offset);

	/* set object method handler callback */
	if (cbs) {
		for (i = 0; i < desc->n_methods; i++)
			obj->handlers[i] = cbs[i];
	}

	/* init object info struct */
	ret = obus_struct_init(&obj->info);
	if (ret < 0)
		goto error;

	/* set has fields */
	obus_struct_set_has_fields(&obj->info);

	/* merge info struct if given */
	if (info) {
		ret = obus_struct_merge(&obj->info, info);
		if (ret < 0)
			goto error;
	}

	return obj;

error:
	free(obj);
	return NULL;
}

OBUS_API int obus_object_destroy(struct obus_object *obj)
{
	if (!obj)
		return -EINVAL;

	if (obus_object_is_registered(obj)) {
		obus_error("can't destroy registered object '%s'",
			   obj->desc->name);
		return -EPERM;
	}

	/* remove object from bus */
	obus_bus_remove_object(obj->bus, obj);

	/**
	 * if object has been added in an bus event,
	 * remove it from bus event object list
	 **/
	if (obus_node_is_ref(&obj->event_node))
		obus_list_del(&obj->event_node);

	/* release object memory */
	obus_struct_destroy(&obj->info);
	free(obj);
	return 0;
}

OBUS_API
const struct obus_object_desc *
obus_object_get_desc(const struct obus_object *obj)
{
	return obj ? obj->desc : NULL;
}

OBUS_API
int obus_object_is_registered(const struct obus_object *obj)
{
	return obj && obj->is_registered;
}

OBUS_API
obus_handle_t obus_object_get_handle(const struct obus_object *obj)
{
	return (obus_handle_t) (obj ? obj->handle : OBUS_INVALID_HANDLE);
}

OBUS_API
const void *obus_object_get_info(const struct obus_object *obj)
{
	return obj ? obj->info.u.addr : NULL;
}

OBUS_API
int obus_object_set_user_data(struct obus_object *object, void *user_data)
{
	if (!object)
		return -EINVAL;
	object->user_data = user_data;
	return 0;
}

OBUS_API
void *obus_object_get_user_data(const struct obus_object *object)
{
	return object ? object->user_data : NULL;
}

OBUS_API
void obus_object_log(const struct obus_object *obj, enum obus_log_level level)
{
	if (!obj)
		return;

	obus_log(level, "OBJECT:%-17.17s = %d", obj->desc->name, obj->handle);
	obus_struct_log(&obj->info, level);
}

obus_method_handler_cb_t
obus_object_get_method_handler(struct obus_object *obj,
			       const struct obus_method_desc *desc)
{
	long int idx;

	idx = desc - obj->desc->methods;
	if (idx < 0 || idx >= obj->desc->n_methods)
		return NULL;

	return obj->handlers[idx];
}

enum obus_method_state
obus_object_get_method_state(struct obus_object *obj, uint16_t uid)
{
	enum obus_method_state *state;
	const struct obus_field_desc *fdesc;

	fdesc = obus_struct_get_field_desc(&obj->info, uid);
	if (!fdesc) {
		obus_error("no object field uid=%d found !", uid);
		return OBUS_METHOD_DISABLED;
	}

	if (fdesc->role != OBUS_METHOD) {
		obus_error("object field uid=%d is not a method !", uid);
		return OBUS_METHOD_DISABLED;
	}

	if (fdesc->enum_drv != &obus_method_state_driver) {
		obus_error("object field uid=%d is not a method state!", uid);
		return OBUS_METHOD_DISABLED;
	}

	/* get field value */
	state = (enum obus_method_state *)obus_field_address(&obj->info, fdesc);
	return *state;
}

int obus_object_remove_encode(struct obus_object *obj, struct obus_buffer *buf)
{
	int ret;

	/* add object uid */
	ret = obus_buffer_append_u16(buf, obj->desc->uid);
	if (ret < 0)
		return ret;

	/* add object handle */
	ret = obus_buffer_append_u16(buf, obj->handle);
	if (ret < 0)
		return ret;

	return 0;
}

struct obus_object *obus_object_remove_decode(struct obus_bus *bus,
					      struct obus_buffer *buf)
{
	uint16_t uid, handle;
	struct obus_object *object;
	int ret;

	/* read object uid */
	ret = obus_buffer_read_u16(buf, &uid);
	if (ret < 0)
		goto error;

	/* read object handle */
	ret = obus_buffer_read_u16(buf, &handle);
	if (ret < 0)
		goto error;

	/* find object in bus */
	object = obus_bus_object(bus, handle);
	if (!object)
		obus_warn("%s: no object with handle=%d and uid=%d", __func__,
			  handle, uid);

	return object;

error:
	return NULL;
}

int obus_object_add_encode(struct obus_object *obj, struct obus_buffer *buf)
{
	int ret;
	size_t offset, length;

	/* add object uid */
	ret = obus_buffer_append_u16(buf, obj->desc->uid);
	if (ret < 0)
		return ret;

	/* add object handle */
	ret = obus_buffer_append_u16(buf, obj->handle);
	if (ret < 0)
		return ret;

	/* get object data size offset in buffer */
	offset = obus_buffer_write_offset(buf);

	/* reserve 4 bytes in buffer for object data size */
	ret = obus_buffer_reserve(buf, sizeof(uint32_t));
	if (ret < 0)
		return ret;

	/* add object struct content */
	ret = obus_struct_encode(&obj->info, buf);
	if (ret < 0)
		return ret;

	/* compute object data size */
	length = obus_buffer_length(buf) - (offset + sizeof(uint32_t));

	/* write object data size */
	obus_buffer_write_u32(buf, (uint32_t)length, offset);
	return 0;
}

struct obus_object *obus_object_add_decode(struct obus_bus_api *api,
					   struct obus_buffer *buf)
{
	int ret;
	const struct obus_object_desc *desc;
	struct obus_object *obj = NULL;
	size_t offset, end_pos;
	uint16_t uid, handle;
	uint32_t size;

	/* read object uid */
	ret = obus_buffer_read_u16(buf, &uid);
	if (ret < 0)
		goto error;

	/* read object handle */
	ret = obus_buffer_read_u16(buf, &handle);
	if (ret < 0)
		goto error;

	/* read object data size */
	ret = obus_buffer_read_u32(buf, &size);
	if (ret < 0)
		goto error;

	/* get buffer read position */
	offset = obus_buffer_get_read_position(buf);
	end_pos = offset + size;

	/* check object uid */
	if (uid == OBUS_INVALID_UID) {
		obus_warn("can't decode object, invalid uid");
		goto eat_bytes;
	}

	/* check object handle value */
	if (handle == OBUS_INVALID_HANDLE) {
		obus_warn("can't decode object uid=%d, invalid handle", uid);
		goto eat_bytes;
	}

	/* find object descriptor */
	desc = obus_bus_api_object(api, uid);
	if (!desc) {
		obus_warn("can't decode object uid=%d, descriptor not found",
			  uid);
		goto eat_bytes;
	}

	/* create object */
	obj = obus_object_new(desc, NULL, NULL);
	if (!obj)
		goto eat_bytes;

	/* add object handle */
	obj->handle = handle;

	/* decode object struct content */
	ret = obus_struct_decode(&obj->info, buf);
	if (ret < 0) {
		obus_warn("can't decode object {uid=%d, name='%s'} data",
			  desc->uid, desc->name);
		goto eat_bytes;
	}

	/* check read position is ok */
	if (obus_buffer_get_read_position(buf) != end_pos) {
		obus_warn("object {uid=%d, name='%s'} data size mismatch",
			  desc->uid, desc->name);

		/* set read position (should be already done ...)*/
		obus_buffer_set_read_position(buf, end_pos);
	}

	return obj;

eat_bytes:
	obus_buffer_set_read_position(buf, end_pos);

error:
	obus_object_destroy(obj);
	return NULL;
}
