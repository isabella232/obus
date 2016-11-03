/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_struct.c
 *
 * @brief obus internal struct api
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
#include "obus_struct.h"

static char obus_field_role_char(const struct obus_field_desc *desc)
{
	switch (desc->role) {
	case OBUS_PROPERTY:
		return 'P';
	break;
	case OBUS_METHOD:
		return 'M';
	break;
	case OBUS_ARGUMENT:
		return 'A';
	break;
	default:
		return '?';
	break;
	}

	return '?';
}

void obus_struct_log(const struct obus_struct *st, enum obus_log_level level)
{
	uint16_t i;

	const struct obus_field_desc *desc;
	char buf[256];

	for (i = 0; i < st->desc->n_fields; i++) {
		desc = &st->desc->fields[i];
		if (!obus_struct_has_field(st, desc))
			continue;

		buf[0] = '\0';
		obus_field_format(st, desc, buf, sizeof(buf));
		obus_log(level, "|-%c:%-20.20s = %s",
			 obus_field_role_char(desc), desc->name, buf);
	}
}

int obus_struct_init(const struct obus_struct *st)
{
	uint32_t i;
	int ret;

	if (!st || !st->u.addr || !st->desc)
		return -EINVAL;

	/* init each field of struct */
	for (i = 0; i < st->desc->n_fields; i++) {
		ret = obus_field_init(st, &st->desc->fields[i]);
		if (ret < 0)
			return ret;
	}

	obus_struct_clear_has_fields(st);
	return 0;
}

void obus_struct_destroy(const struct obus_struct *st)
{
	uint32_t i;

	if (!st || !st->desc || !st->u.addr)
		return;

	/* destroy each field of struct */
	for (i = 0; i < st->desc->n_fields; i++)
		obus_field_destroy(st, &st->desc->fields[i]);
}

int obus_struct_has_field(const struct obus_struct *st,
			  const struct obus_field_desc *desc)
{
	uint32_t *fields;
	long int idx;

	fields = (uint32_t *)((uint8_t *)st->u.addr + st->desc->fields_offset);

	/* get field description index in struct description array */
	idx = desc - st->desc->fields;
	if (idx < 0 || (size_t)idx >= st->desc->n_fields)
		return 0;

	/* check field bit in array */
	return ((fields[idx / 32] & (uint32_t)(1 << (idx % 32))) != 0);
}


int obus_struct_set_has_field(const struct obus_struct *st,
			      const struct obus_field_desc *desc)
{
	uint32_t *fields;
	long int idx;

	fields = (uint32_t *)((uint8_t *)st->u.addr + st->desc->fields_offset);

	/* get field description index in struct description array */
	idx = desc - st->desc->fields;
	if (idx < 0 || (size_t)idx >= st->desc->n_fields)
		return -EINVAL;

	/* set field bit in array */
	fields[idx / 32] |= (uint32_t)(1 << (idx % 32));
	return 0;
}

int obus_struct_clear_has_field(const struct obus_struct *st,
				const struct obus_field_desc *desc)
{
	uint32_t *fields;
	long int idx;

	fields = (uint32_t *)((uint8_t *)st->u.addr + st->desc->fields_offset);

	/* get field description index in struct description array */
	idx = desc - st->desc->fields;
	if (idx < 0 || (size_t)idx >= st->desc->n_fields)
		return -EINVAL;

	/* clear field bit in array */
	fields[idx / 32] &= ~(uint32_t)(1 << (idx % 32));
	return 0;
}


int obus_struct_set_has_fields(const struct obus_struct *st)
{
	int ret = 0;
	uint32_t i;

	for (i = 0; i < st->desc->n_fields; i++)
		ret |= obus_struct_set_has_field(st, &st->desc->fields[i]);

	return ret;
}

int obus_struct_clear_has_fields(const struct obus_struct *st)
{
	int ret = 0;
	uint32_t i;

	for (i = 0; i < st->desc->n_fields; i++)
		ret |= obus_struct_clear_has_field(st, &st->desc->fields[i]);

	return ret;
}

int obus_struct_encode(const struct obus_struct *st, struct obus_buffer *buf)
{
	int ret;
	uint16_t i, n_fields;

	/* count number of field set */
	n_fields = 0;
	for (i = 0; i < st->desc->n_fields; i++) {
		if (obus_struct_has_field(st, &st->desc->fields[i]))
			n_fields++;
	}

	/* encode field numbers */
	ret = obus_buffer_append_u16(buf, n_fields);
	if (ret < 0)
		goto error;

	/* encode fields */
	for (i = 0; i < st->desc->n_fields; i++) {
		if (obus_struct_has_field(st, &st->desc->fields[i])) {
			ret = obus_field_encode(st, &st->desc->fields[i], buf);
			if (ret < 0)
				goto error;
		}
	}

	return 0;

error:
	obus_error("can't encode struct: error=%d", ret);
	return ret;
}

int obus_struct_decode(const struct obus_struct *st, struct obus_buffer *buf)
{
	const struct obus_field_desc *desc;
	uint16_t i, n_fields;
	int ret;

	/* clear fields bitset */
	ret = obus_struct_clear_has_fields(st);
	if (ret < 0)
		goto error;

	/* read struct number of fields */
	ret = obus_buffer_read_u16(buf, &n_fields);
	if (ret < 0)
		goto error;

	/* decode fields */
	for (i = 0; i < n_fields; i++) {
		/* decode field */
		desc = obus_field_decode(st, buf);
		/* mark field has decoded */
		if (desc)
			obus_struct_set_has_field(st, desc);
	}

	return 0;

error:
	return ret;
}

int obus_struct_copy(const struct obus_struct *dst,
		     const struct obus_struct *src)
{
	uint16_t i;
	int ret;

	if (dst->desc != src->desc) {
		obus_error("can't copy struct with different descriptor");
		return -EINVAL;
	}

	/* merge src fields to dest one */
	for (i = 0; i < src->desc->n_fields; i++) {
		ret = obus_field_copy(dst, src, &src->desc->fields[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int obus_struct_merge(const struct obus_struct *dst,
		      const struct obus_struct *src)
{
	uint16_t i;
	int ret;

	if (dst->desc != src->desc) {
		obus_error("can't merge struct with different descriptor");
		return -EINVAL;
	}

	/* merge src fields to dest one */
	for (i = 0; i < src->desc->n_fields; i++) {
		if (obus_struct_has_field(src, &src->desc->fields[i])) {
			ret = obus_field_copy(dst, src, &src->desc->fields[i]);
			if (ret < 0)
				return ret;
			/* ensure dest has field */
			obus_struct_set_has_field(dst, &src->desc->fields[i]);
		}
	}

	return 0;
}

int obus_struct_is_empty(const struct obus_struct *st)
{
	uint16_t i;
	for (i = 0; i < st->desc->n_fields; i++) {
		if (obus_struct_has_field(st, &st->desc->fields[i]))
			return 0;
	}

	return 1;
}

const struct obus_field_desc *
obus_struct_get_field_desc(const struct obus_struct *st, uint16_t uid)
{
	uint16_t i;
	for (i = 0; i < st->desc->n_fields; i++) {
		if (st->desc->fields[i].uid == uid)
			return &st->desc->fields[i];
	}

	return NULL;
}
