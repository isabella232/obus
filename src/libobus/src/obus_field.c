/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_field.c
 *
 * @brief obus field
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

/* static empty string */
static const union empty_string {
	const char *const_v;
	char *v;
} s_empty_string = {.const_v = ""};

uint32_t *obus_field_array_nb_address(const struct obus_struct *st,
				      const struct obus_field_desc *desc)
{
	return (uint32_t *)((uint8_t *)st->u.addr + desc->nb_offset);
}

void *obus_field_address(const struct obus_struct *st,
			 const struct obus_field_desc *desc)
{
	return (uint8_t *)st->u.addr + desc->offset;
}


static void *obus_field_array_item(const struct obus_field_desc *desc,
				   void *base, uint32_t idx)
{
	void *addr;

	/* get item address */
	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
	case OBUS_FIELD_BOOL:
		addr = (uint8_t *)base + idx*sizeof(uint8_t);
	break;
	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
		addr = (uint8_t *)base + idx*sizeof(uint16_t);
	break;
	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
		addr = (uint8_t *)base + idx*sizeof(uint32_t);
	break;
	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
		addr = (uint8_t *)base + idx*sizeof(uint64_t);
	break;
	case OBUS_FIELD_ENUM:
		addr = (uint8_t *)base + idx*desc->enum_drv->size;
	break;
	case OBUS_FIELD_STRING:
		addr = (uint8_t *)base + idx*sizeof(char *);
	break;
	case OBUS_FIELD_F32:
		addr = (uint8_t *)base + idx*sizeof(float);
	break;
	case OBUS_FIELD_F64:
		addr = (uint8_t *)base + idx*sizeof(double);
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
		addr = NULL;
	break;
	}

	return addr;
}

static void obus_format_value(const struct obus_field_desc *desc, void *addr,
			      char *buf, size_t size)
{
	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
		snprintf(buf, size, "%"PRIu8, *(uint8_t *)addr);
	break;
	case OBUS_FIELD_I8:
		snprintf(buf, size, "%"PRIi8, *(int8_t *)addr);
	break;
	case OBUS_FIELD_U16:
		snprintf(buf, size, "%"PRIu16, *(uint16_t *)addr);
	break;
	case OBUS_FIELD_I16:
		snprintf(buf, size, "%"PRIi16, *(int16_t *)addr);
	break;
	case OBUS_FIELD_U32:
		snprintf(buf, size, "%"PRIu32, *(uint32_t *)addr);
	break;
	case OBUS_FIELD_I32:
		snprintf(buf, size, "%"PRIi32, *(int32_t *)addr);
	break;
	case OBUS_FIELD_U64:
		snprintf(buf, size, "%"PRIu64, *(uint64_t *)addr);
	break;
	case OBUS_FIELD_I64:
		snprintf(buf, size, "%"PRIi64, *(int64_t *)addr);
	break;
	case OBUS_FIELD_ENUM:
		(*desc->enum_drv->format) (addr, buf, size);
	break;
	case OBUS_FIELD_BOOL:
		snprintf(buf, size, "%s", (*(uint8_t *)addr) ?
			 "TRUE" : "FALSE");
	break;
	case OBUS_FIELD_STRING:
	{
		const char **str = addr;
		snprintf(buf, size, "'%s'", (*str) ? (*str) : "");
	}
	break;
	case OBUS_FIELD_F32:
		snprintf(buf, size, "%f", *(float *)addr);
	break;
	case OBUS_FIELD_F64:
		snprintf(buf, size, "%f", *(double *)addr);
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
	break;
	}
}

static int obus_copy_value(const struct obus_field_desc *desc, void *dst,
			   const void *src)
{
	int ret = 0;

	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
		*(uint8_t *)dst = *(const uint8_t *)src;
	break;

	case OBUS_FIELD_BOOL:
		*(uint8_t *)dst = (*(const uint8_t *)src) ? 1 : 0;
	break;

	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
		*(uint16_t *)dst = *(const uint16_t *)src;
	break;

	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
		*(uint32_t *)dst = *(const uint32_t *)src;
	break;

	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
		*(uint64_t *)dst = *(const uint64_t *)src;
	break;

	case OBUS_FIELD_ENUM:
	{
		int32_t value;
		value = (*desc->enum_drv->get_value) (src);
		(*desc->enum_drv->set_value) (dst, value);
	}
	break;
	case OBUS_FIELD_STRING:
	{
		const char * const *v_src = src;
		char **v_dest = dst;
		char *v_dup;

		v_dup = (*v_src && *v_src != s_empty_string.const_v) ?
			strdup(*v_src) : s_empty_string.v;

		if ((*v_src && *v_src != s_empty_string.const_v) && !v_dup)
			ret = -ENOMEM;

		if (ret == 0) {
			if (*v_dest != s_empty_string.const_v)
				free(*v_dest);

			*v_dest = v_dup;
		}
	}
	break;
	case OBUS_FIELD_F32:
		*(float *)dst = *(const float *)src;
	break;
	case OBUS_FIELD_F64:
		*(double *)dst = *(const double *)src;
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
	break;
	}

	return ret;
}

static void obus_format_array_value(const struct obus_struct *st,
				    const struct obus_field_desc *desc,
				    char *buf, size_t size)
{
	uint32_t *n_items, i;
	void *addr;
	uint8_t **array;
	size_t length;

	/* get field array base address */
	array = (uint8_t **)obus_field_address(st, desc);

	/* get field array items count u32 address */
	n_items = obus_field_array_nb_address(st, desc);

	/* ex: [5]:{5, 6, 7, 9 , ...} */
	snprintf(buf, size, "[%d]:{", *n_items);
	length = strlen(buf);
	/* encode each items */
	for (i = 0; i < *n_items; i++) {
		/* get item address */
		addr = obus_field_array_item(desc, *array, i);

		/* format item value */
		obus_format_value(desc, addr, buf + length, size - length);
		length = strlen(buf);

		/* truncate log if buffer is too short */
		if (length + 6 >= size) {
			length = size - 6;
			buf[length++] = '.';
			buf[length++] = '.';
			buf[length++] = '.';
			break;
		}

		/* prepare next item */
		if (i + 1 < *n_items) {
			buf[length++] = ',';
			buf[length++] = ' ';
		}
	}

	buf[length++] = '}';
	buf[length++] = '\0';
}


static int obus_encode_value(const struct obus_field_desc *desc, void *addr,
			     struct obus_buffer *buf)
{
	int ret;

	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_BOOL:
		/* fixup boolean value in u8 (1 or 0) */
		ret = obus_buffer_append_u8(buf, (*(uint8_t *)addr) ? 1 : 0);
	break;
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
		ret = obus_buffer_append_u8(buf, *(uint8_t *)addr);
	break;
	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
		ret = obus_buffer_append_u16(buf, *(uint16_t *)addr);
	break;
	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
		ret = obus_buffer_append_u32(buf, *(uint32_t *)addr);
	break;
	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
		ret = obus_buffer_append_u64(buf, *(uint64_t *)addr);
	break;
	case OBUS_FIELD_ENUM:
	{
		uint32_t value;
		value = (uint32_t)(*desc->enum_drv->get_value) (addr);
		ret = obus_buffer_append_u32(buf, value);
	}
	break;
	case OBUS_FIELD_STRING:
		ret = obus_buffer_append_string(buf, *(char **)addr);
	break;
	case OBUS_FIELD_F32:
		ret = obus_buffer_append_f32(buf, *(float *)addr);
	break;
	case OBUS_FIELD_F64:
		ret = obus_buffer_append_f64(buf, *(double *)addr);
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
		ret = -EINVAL;
	break;
	}

	return ret;
}

static int obus_decode_value(const struct obus_field_desc *desc, void *addr,
			     struct obus_buffer *buf)
{
	int ret;

	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
		ret = obus_buffer_read_u8(buf, (uint8_t *)addr);
	break;
	case OBUS_FIELD_BOOL:
		ret = obus_buffer_read_u8(buf, (uint8_t *)addr);
		/* fixup boolean value in u8 (1 or 0) */
		if (ret == 0)
			*(uint8_t *)addr = (*(uint8_t *)addr) ? 1 : 0;
	break;

	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
		ret = obus_buffer_read_u16(buf, (uint16_t *)addr);
	break;
	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
		ret = obus_buffer_read_u32(buf, (uint32_t *)addr);
	break;
	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
		ret = obus_buffer_read_u64(buf, (uint64_t *)addr);
	break;
	case OBUS_FIELD_ENUM:
	{
		uint32_t value;
		ret = obus_buffer_read_u32(buf, &value);
		if (ret == 0)
			(*desc->enum_drv->set_value) (addr, (int32_t)value);
	}
	break;
	case OBUS_FIELD_STRING:
		ret = obus_buffer_read_string(buf, (char **)addr);
	break;
	case OBUS_FIELD_F32:
		ret = obus_buffer_read_f32(buf, (float *)addr);
	break;
	case OBUS_FIELD_F64:
		ret = obus_buffer_read_f64(buf, (double *)addr);
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
		ret = -EINVAL;
	break;
	}

	return ret;
}


static void obus_destroy_value(const struct obus_field_desc *desc, void *addr)
{
	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
	case OBUS_FIELD_ENUM:
	case OBUS_FIELD_BOOL:
	case OBUS_FIELD_F32:
	case OBUS_FIELD_F64:
		/* nothing to freed */
	break;

	case OBUS_FIELD_STRING:
	{
		char **str = addr;
		if (str && (*str != s_empty_string.const_v))
			free(*str);
	}
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
	break;
	}
}

static void obus_field_skip_value(uint16_t type, struct obus_buffer *buf)
{
	uint32_t size;
	int ret;

	switch (type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
	case OBUS_FIELD_BOOL:
		obus_buffer_inc_read_position(buf, sizeof(uint8_t));
	break;
	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
		obus_buffer_inc_read_position(buf, sizeof(uint16_t));
	break;
	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
	case OBUS_FIELD_ENUM:
		obus_buffer_inc_read_position(buf, sizeof(uint32_t));
	break;
	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
		obus_buffer_inc_read_position(buf, sizeof(uint64_t));
	break;
	case OBUS_FIELD_STRING:
		/* read next 4 bytes */
		size = 0;
		ret = obus_buffer_read_u32(buf, &size);
		if (ret == 0)
			obus_buffer_inc_read_position(buf, size);
	break;
	case OBUS_FIELD_F32:
		obus_buffer_inc_read_position(buf, sizeof(float));
	break;
	case OBUS_FIELD_F64:
		obus_buffer_inc_read_position(buf, sizeof(double));
	break;
	default:
		obus_warn("unknown field type %d", type & OBUS_FIELD_MASK);
	break;
	}
}

static void *obus_field_array_alloc(const struct obus_field_desc *desc,
				    uint32_t n_items)
{
	void *addr;

	/* get item address */
	switch (desc->type & OBUS_FIELD_MASK) {
	case OBUS_FIELD_U8:
	case OBUS_FIELD_I8:
	case OBUS_FIELD_BOOL:
		addr = calloc(n_items, sizeof(uint8_t));
	break;
	case OBUS_FIELD_U16:
	case OBUS_FIELD_I16:
		addr = calloc(n_items, sizeof(uint16_t));
	break;
	case OBUS_FIELD_U32:
	case OBUS_FIELD_I32:
		addr = calloc(n_items, sizeof(uint32_t));
	break;
	case OBUS_FIELD_U64:
	case OBUS_FIELD_I64:
		addr = calloc(n_items, sizeof(uint64_t));
	break;
	case OBUS_FIELD_ENUM:
		addr = calloc(n_items, sizeof(desc->enum_drv->size));
	break;
	case OBUS_FIELD_STRING:
	{
		uint32_t i;
		char **array;
		array = calloc(n_items, sizeof(char *));

		/* by default set string to default empty string */
		for (i = 0; array && i < n_items; i++)
			array[i] = s_empty_string.v;

		addr = array;
	}
	break;
	case OBUS_FIELD_F32:
		addr = calloc(n_items, sizeof(float));
	break;
	case OBUS_FIELD_F64:
		addr = calloc(n_items, sizeof(double));
	break;
	default:
		obus_warn("unknown field type %d",
			  desc->type & OBUS_FIELD_MASK);
		addr = NULL;
	break;
	}

	return addr;
}



static void obus_field_array_skip_value(uint16_t type, struct obus_buffer *buf)
{
	uint32_t i, n_items;
	int ret;

	/* read field array number of items */
	ret = obus_buffer_read_u32(buf, &n_items);
	if (ret < 0)
		return;

	/* skip array items values */
	for (i = 0; i < n_items; i++)
		obus_field_skip_value(type, buf);
}

static int obus_field_array_encode(const struct obus_struct *st,
				   const struct obus_field_desc *desc,
				   struct obus_buffer *buf)
{
	void *addr;
	uint8_t **array;
	uint32_t i, *n_items;
	int ret;

	/* get field array base address */
	array = (uint8_t **)obus_field_address(st, desc);

	/* get field array items count u32 address */
	n_items = obus_field_array_nb_address(st, desc);

	/* encode field array number of items */
	ret = obus_buffer_append_u32(buf, *n_items);
	if (ret < 0)
		return ret;

	/* encode array items */
	for (i = 0; i < *n_items; i++) {
		/* get array item address */
		addr = obus_field_array_item(desc, *array, i);
		if (addr == NULL) {
			ret = -EINVAL;
			break;
		}

		/* encode item value */
		ret = obus_encode_value(desc, addr, buf);
		if (ret < 0)
			break;
	}

	return ret;
}

static int obus_field_array_decode(const struct obus_struct *st,
				   const struct obus_field_desc *desc,
				   struct obus_buffer *buf)
{
	void *addr;
	uint8_t **array;
	uint32_t i, *n_items;
	int ret;

	/* get field array base address */
	array = (uint8_t **)obus_field_address(st, desc);

	/* get field array items count u32 address */
	n_items = obus_field_array_nb_address(st, desc);

	/* read field array number of items */
	ret = obus_buffer_read_u32(buf, n_items);
	if (ret < 0)
		return ret;

	/* allocate item array */
	if (*n_items > 0) {
		*array = obus_field_array_alloc(desc, *n_items);
		if (!*array)
			return -ENOMEM;
	} else {
		*array = NULL;
	}

	/* decode array items */
	for (i = 0; i < *n_items; i++) {
		/* get array item address */
		addr = obus_field_array_item(desc, *array, i);
		if (addr == NULL) {
			ret = -EINVAL;
			break;
		}

		/* decode item value */
		ret = obus_decode_value(desc, addr, buf);
		if (ret < 0)
			break;
	}

	/* free allocated array on error */
	if (ret < 0 && (*array)) {
		free(*array);
		*array = NULL;
	}

	return ret;
}

int obus_field_encode(const struct obus_struct *st,
		      const struct obus_field_desc *desc,
		      struct obus_buffer *buf)
{
	void *addr;
	int ret;

	/* encode field uid */
	ret = obus_buffer_append_u16(buf, desc->uid);
	if (ret < 0)
		return ret;

	/* encode field type */
	ret = obus_buffer_append_u8(buf, desc->type);
	if (ret < 0)
		return ret;

	/* encode field array */
	if (desc->type & OBUS_FIELD_ARRAY) {
		/* encode array */
		ret = obus_field_array_encode(st, desc, buf);
	} else {
		/* get field address */
		addr = obus_field_address(st, desc);

		/* encode field value */
		ret = obus_encode_value(desc, addr, buf);
	}

	return ret;
}

const struct obus_field_desc *
obus_field_decode(const struct obus_struct *st, struct obus_buffer *buf)
{
	const struct obus_field_desc *desc;
	void *addr;
	size_t pos;
	uint16_t uid;
	uint8_t type;
	int ret;

	/* read field uid */
	ret = obus_buffer_read_u16(buf, &uid);
	if (ret < 0)
		return NULL;

	/* read field type */
	ret = obus_buffer_read_u8(buf, &type);
	if (ret < 0)
		return NULL;

	/* get field position */
	pos = obus_buffer_get_read_position(buf);

	/* get field description from uid */
	desc = obus_struct_get_field_desc(st, uid);
	if (!desc) {
		obus_warn("can't decode field uid=%d: no descriptor found",
			  uid);
		goto skip_field;
	}

	/* check field descriptor type & type match*/
	if (desc->type != type) {
		obus_warn("can't decode field uid=%d: descriptor type=%d and "
			  "decoded type=%d mismatch", uid, desc->type, type);
		goto skip_field;
	}

	/* decode field array */
	if (type & OBUS_FIELD_ARRAY) {
		ret = obus_field_array_decode(st, desc, buf);
	} else {
		/* get field address */
		addr = obus_field_address(st, desc);

		/* decode field value */
		ret = obus_decode_value(desc, addr, buf);
	}

	/* on decode failure, restore buffer read position and
	 * skip field in buffer  */
	if (ret < 0) {
		obus_buffer_set_read_position(buf, pos);
		goto skip_field;
	}

	return desc;

skip_field:
	if (type & OBUS_FIELD_ARRAY)
		obus_field_array_skip_value(type, buf);
	else
		obus_field_skip_value(type, buf);

	return NULL;
}

int obus_field_init(const struct obus_struct *st,
		    const struct obus_field_desc *desc)
{
	uint32_t *n_items;
	void *addr;
	int ret = 0;

	/* get field pointer */
	addr = obus_field_address(st, desc);

	if (desc->type & OBUS_FIELD_ARRAY) {
		n_items = obus_field_array_nb_address(st, desc);
		*n_items = 0;
		*(uint8_t **)addr = NULL;
	} else {
		switch (desc->type & OBUS_FIELD_MASK) {
		case OBUS_FIELD_U8:
		case OBUS_FIELD_I8:
		case OBUS_FIELD_BOOL:
			*(uint8_t *)addr = 0;
		break;
		case OBUS_FIELD_U16:
		case OBUS_FIELD_I16:
			*(uint16_t *)addr = 0;
		break;
		case OBUS_FIELD_U32:
		case OBUS_FIELD_I32:
			*(uint32_t *)addr = 0;
		break;
		case OBUS_FIELD_U64:
		case OBUS_FIELD_I64:
			*(uint64_t *)addr = 0;
		break;
		case OBUS_FIELD_ENUM:
			/* set enum to default value */
			(*desc->enum_drv->set_value) (addr,
					desc->enum_drv->default_value);
		break;
		case OBUS_FIELD_STRING:
			*(char **)addr = s_empty_string.v;
		break;
		case OBUS_FIELD_F32:
			*(float *)addr = 0.0f;
		break;
		case OBUS_FIELD_F64:
			*(double *)addr = 0.0;
		break;
		default:
			obus_warn("unknown field type %d",
				  desc->type & OBUS_FIELD_MASK);
		break;
		}
	}

	return ret;
}


static void obus_field_array_destroy(const struct obus_struct *st,
				     const struct obus_field_desc *desc)
{
	void *addr;
	uint8_t **array;
	uint32_t i, *n_items;


	/* get field array base address */
	array = (uint8_t **)obus_field_address(st, desc);

	/* get field array items count u32 address */
	n_items = obus_field_array_nb_address(st, desc);

	/* encode array items */
	for (i = 0; i < *n_items; i++) {
		/* get array item address */
		addr = obus_field_array_item(desc, *array, i);
		if (!addr)
			continue;

		/* destroy value */
		obus_destroy_value(desc, addr);
	}

	/* destroy array */
	free(*array);
}

void obus_field_destroy(const struct obus_struct *st,
			const struct obus_field_desc *desc)
{
	void *addr;

	/* check if field is an array */
	if (desc->type & OBUS_FIELD_ARRAY) {
		obus_field_array_destroy(st, desc);
	} else {
		/* get field address pointer */
		addr = obus_field_address(st, desc);

		/* destroy value */
		obus_destroy_value(desc, addr);
	}
}


void obus_field_format(const struct obus_struct *st,
		       const struct obus_field_desc *desc,
		       char *buf, size_t size)
{
	void *addr;

	/* check if field is an array */
	if (desc->type & OBUS_FIELD_ARRAY) {
		obus_format_array_value(st, desc, buf, size);
	} else {
		/* get field address pointer */
		addr = obus_field_address(st, desc);

		/* destroy value */
		obus_format_value(desc, addr, buf, size);
	}
}

static int obus_field_array_ref_value(const struct obus_field_desc *desc,
				      uint32_t n_dst_items,
				      void *src_addr, void *new_dst_addr,
				      void *dst_array)
{
	uint32_t i;
	char **src_str_addr, **dst_str_addr;

	if ((desc->type & OBUS_FIELD_MASK) != OBUS_FIELD_STRING)
		return -ENOSYS;

	src_str_addr = (char **)src_addr;
	for (i = 0; i < n_dst_items; i++) {
		dst_str_addr = (char **)obus_field_array_item(desc, dst_array,
							      i);

		if (*dst_str_addr == *src_str_addr) {
			/* unref current dest array
			 * item string */
			*dst_str_addr = NULL;
			/* ref src array addr in new dest array
			 * item string */
			*(char **)new_dst_addr = *src_str_addr;
			return 0;
		}
	}

	return -ENOENT;
}

static int obus_field_array_copy(const struct obus_struct *dst,
				 const struct obus_struct *src,
				 const struct obus_field_desc *desc)
{
	uint8_t **src_array, **dst_array, *new_dst_array;
	uint32_t i, *n_src_items, *n_dst_items;
	void *src_addr, *new_dst_addr;
	int ret;

	/* get field array base address */
	src_array = obus_field_address(src, desc);
	dst_array = obus_field_address(dst, desc);

	/* get field array items count u32 address */
	n_src_items = obus_field_array_nb_address(src, desc);
	n_dst_items = obus_field_array_nb_address(dst, desc);

	/* allocate item array */
	if (*n_src_items > 0) {
		new_dst_array = obus_field_array_alloc(desc, *n_src_items);
		if (!new_dst_array)
			return -ENOMEM;
	} else {
		new_dst_array = NULL;
	}

	/* copy array items */
	for (i = 0; i < *n_src_items; i++) {
		src_addr = obus_field_array_item(desc, *src_array, i);
		new_dst_addr = obus_field_array_item(desc, new_dst_array, i);

		/* for string array check if src_addr ptr is used
		 * on dest_array. if true do not copy string but only copy ptr
		 * and unref string in dst_array */
		ret = obus_field_array_ref_value(desc, *n_dst_items, src_addr,
						 new_dst_addr, *dst_array);

		/* if reference has been found do not call copy_value */
		if (ret == 0)
			continue;

		ret = obus_copy_value(desc, new_dst_addr, src_addr);
		if (ret < 0)
			return ret;
	}

	/* destroy old dest array */
	obus_field_array_destroy(dst, desc);

	/* replace dest array pointers */
	*dst_array = new_dst_array;

	/* update dst items numbers */
	*n_dst_items = *n_src_items;
	return 0;
}

int obus_field_copy(const struct obus_struct *dst,
		    const struct obus_struct *src,
		    const struct obus_field_desc *desc)
{
	int ret;
	void *src_addr, *dst_addr;

	/* check if field is an array */
	if (desc->type & OBUS_FIELD_ARRAY) {
		/* copy src array to dest one */
		ret = obus_field_array_copy(dst, src, desc);
	} else {
		/* copy src field value to dest one */
		src_addr = obus_field_address(src, desc);
		dst_addr = obus_field_address(dst, desc);
		ret = obus_copy_value(desc, dst_addr, src_addr);
	}

	return ret;
}
