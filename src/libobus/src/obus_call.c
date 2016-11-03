/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_call.c
 *
 * @brief obus object call implementation
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

struct obus_call *obus_call_new(struct obus_object *obj,
				const struct obus_method_desc *desc,
				obus_method_call_status_handler_cb_t cb,
				const struct obus_struct *args)
{
	struct obus_call *call;
	off_t args_offset;
	size_t size;

	if (!obj || !desc)
		return NULL;

	size = sizeof(*call);
	if (!args) {
		args_offset = (off_t)size;
		/* method may have no args !*/
		if (desc->args_desc)
			size += desc->args_desc->size;
	}

	call = calloc(size, sizeof(*call));
	if (!call)
		return NULL;

	/* init call */
	call->desc = desc;
	call->handle = OBUS_INVALID_HANDLE;
	call->obj = obj;
	call->status = OBUS_CALL_INVALID;
	call->cb = cb;
	call->args.desc = desc->args_desc;

	if (call->args.desc) {
		if (args)
			call->args.u.addr = args->u.addr;
		else
			call->args.u.addr = ((uint8_t *)call + args_offset);
	}

	return call;
}

int obus_call_destroy(struct obus_call *call)
{
	if (!call)
		return -EINVAL;

	if (call->args.desc && call->args.u.addr)
		obus_struct_destroy(&call->args);

	free(call);
	return 0;
}

struct obus_object *obus_call_get_object(struct obus_call *call)
{
	return call ? call->obj : NULL;
}

const struct obus_object_desc *
obus_call_get_object_desc(const struct obus_call *call)
{
	return call ? call->obj->desc : NULL;
}

void obus_call_log(struct obus_call *call, enum obus_log_level level)
{
	obus_log(level, "CALL:%-19.19s = %d", call->desc->name, call->handle);
	obus_log(level, "|-O:%-20.20s = %d", call->obj->desc->name,
		 call->obj->handle);
	if (call->args.desc && call->args.u.addr)
		obus_struct_log(&call->args, level);
}

OBUS_API
const char *obus_call_status_str(enum obus_call_status status)
{
	switch (status) {
	case OBUS_CALL_INVALID: return "INVALID";
	case OBUS_CALL_ACKED: return "ACKED";
	case OBUS_CALL_REFUSED: return "REFUSED";
	case OBUS_CALL_ABORTED: return "ABORTED";
	case OBUS_CALL_METHOD_DISABLED: return "METHOD_DISABLED";
	case OBUS_CALL_METHOD_NOT_SUPPORTED: return "METHOD_NOT_SUPPORTED";
	case OBUS_CALL_INVALID_ARGUMENTS: return "INVALID_ARGUMENTS";
	default: return "INVALID";
	}
	return NULL;
}

void obus_ack_log(struct obus_ack *ack, struct obus_call *call,
		  enum obus_log_level level)
{
	if (call) {
		obus_log(level, "ACK:%s", obus_call_status_str(ack->status));
		obus_log(level, "|-C:%-20.20s = %d", call->desc->name,
			 ack->handle);
		obus_log(level, "|-O:%-20.20s = %d", call->obj->desc->name,
			 call->obj->handle);
		if (call->args.u.addr)
			obus_struct_log(&call->args, level);
	} else {
		obus_log(level, "ACK:%s", obus_call_status_str(ack->status));
		obus_log(level, "|-C:%-20.20s = %d", "???", ack->handle);
	}
}

int obus_call_encode(struct obus_call *call, struct obus_buffer *buf)
{
	int ret;
	size_t offset, length;

	/* add object uid */
	ret = obus_buffer_append_u16(buf, call->obj->desc->uid);
	if (ret < 0)
		return ret;

	/* add object handle */
	ret = obus_buffer_append_u16(buf, call->obj->handle);
	if (ret < 0)
		return ret;

	/* add call method uid */
	ret = obus_buffer_append_u16(buf, call->desc->uid);
	if (ret < 0)
		return ret;

	/* add call handle */
	ret = obus_buffer_append_u16(buf, call->handle);
	if (ret < 0)
		return ret;

	/* get event data size offset in buffer */
	offset = obus_buffer_write_offset(buf);

	/* reserve 4 bytes in buffer for event data size */
	ret = obus_buffer_reserve(buf, sizeof(uint32_t));
	if (ret < 0)
		return ret;

	/* add call args if any */
	if (call->args.desc) {
		ret = obus_struct_encode(&call->args, buf);
		if (ret < 0)
			return ret;
	}

	/* compute object data size */
	length = obus_buffer_length(buf) - (offset + sizeof(uint32_t));

	/* write object data size */
	obus_buffer_write_u32(buf, (uint32_t)length, offset);
	return 0;
}

struct obus_call *obus_call_decode(struct obus_bus *bus,
				   struct obus_buffer *buf)
{
	int ret;
	const struct obus_method_desc *desc;
	struct obus_object *obj;
	struct obus_call *call = NULL;
	size_t offset, end_pos;
	obus_handle_t handle, uid, mtd_uid, call_handle;
	uint32_t size;

	/* read object uid */
	ret = obus_buffer_read_u16(buf, &uid);
	if (ret < 0)
		goto error;

	/* read object handle */
	ret = obus_buffer_read_u16(buf, &handle);
	if (ret < 0)
		goto error;

	/* read call method uid */
	ret = obus_buffer_read_u16(buf, &mtd_uid);
	if (ret < 0)
		goto error;

	/* read call handle */
	ret = obus_buffer_read_u16(buf, &call_handle);
	if (ret < 0)
		goto error;

	/* read object data size */
	ret = obus_buffer_read_u32(buf, &size);
	if (ret < 0)
		goto error;

	/* get buffer read position */
	offset = obus_buffer_get_read_position(buf);
	end_pos = offset + size;

	/* find object given its handle & uid ? */
	obj = obus_bus_object(bus, handle);
	if (!obj || obj->desc->uid != uid) {
		obus_error("can't decode call: object {uid=%d hande=%d} not "
			   "found", uid, handle);
		goto eat_bytes;
	}

	/* find method descriptor */
	desc = obus_bus_api_method(&bus->api, obj->desc->uid, mtd_uid);
	if (!desc) {
		obus_error("can't decode call uid=%d, "
			   "method descriptor not found", mtd_uid);
		goto eat_bytes;
	}

	/* create call */
	call = obus_call_new(obj, desc, NULL, NULL);
	if (!call)
		goto eat_bytes;

	/* set call handle */
	call->handle = call_handle;

	/* decode call struct content */
	if (call->args.desc) {
		ret = obus_struct_decode(&call->args, buf);
		if (ret < 0)
			goto eat_bytes;
	}

	/* check read position is ok */
	if (obus_buffer_get_read_position(buf) != end_pos) {
		obus_warn("%s: object method {uid=%d, name='%s'} call data "
			  "size mismatch", __func__, desc->uid, desc->name);

		/* set read position (should be already done ...)*/
		obus_buffer_set_read_position(buf, end_pos);
	}

	return call;

eat_bytes:
	obus_buffer_set_read_position(buf, end_pos);

error:
	obus_call_destroy(call);
	return NULL;
}

void obus_call_ack_notify(struct obus_call *call, enum obus_call_status status)
{
	if (call->cb)
		(*call->cb) (call->obj, call->handle, status);
}
