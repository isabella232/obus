/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_buffer.h
 *
 * @brief obus buffer
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

#ifndef _OBUS_BUFFER_H_
#define _OBUS_BUFFER_H_

struct obus_buffer_pool {
	struct obus_node free_bufs;	/* free buffer list */
	size_t buffsize;		/* default buffer size */
};

struct obus_buffer {
	struct obus_buffer_pool *pool;	/* buffer pool */
	struct obus_node node;	/* buffer node */
	size_t size;		/* size of memory buffer */
	size_t length;		/* buffer length */
	size_t pos;		/* current read position */
	uint8_t *data;		/* data address of memory buffer */
	int refcnt;		/* buffer reference counter */
};

static inline
int obus_buffer_pool_put(struct obus_buffer_pool *pool,
			 struct obus_buffer *buf)
{
	if (!pool || !buf)
		return -EINVAL;

	obus_list_add_before(&pool->free_bufs, &buf->node);
	buf->refcnt = 0;
	return 0;
}


/* create buffer */
static inline
struct obus_buffer *obus_buffer_new(size_t size, struct obus_buffer_pool *pool)
{
	struct obus_buffer *buf;

	buf = calloc(1, sizeof(*buf));
	if (!buf)
		return NULL;

	buf->data = (uint8_t *)malloc(size);
	if (!buf->data) {
		free(buf);
		return NULL;
	}

	buf->size = size;
	buf->length = 0;
	buf->pos = 0;
	buf->refcnt = 1;
	buf->pool = pool;
	return buf;
}

static inline
struct obus_buffer *obus_buffer_ref(struct obus_buffer *buf)
{
	buf->refcnt++;
	return buf;
}

static inline
int obus_buffer_unref(struct obus_buffer *buf)
{
	buf->refcnt--;
	if ((buf->refcnt == 0) && buf->pool)
		obus_buffer_pool_put(buf->pool, buf);

	return buf->refcnt;
}

static inline
uint8_t *obus_buffer_ptr(struct obus_buffer *buf)
{
	return buf->data;
}

/* destroy buffer */
static inline
void obus_buffer_destroy(struct obus_buffer *buf)
{
	if (buf) {
		free(buf->data);
		free(buf);
	}
}

/* ensure buffer size is enough */
static inline
int obus_buffer_ensure_realloc(struct obus_buffer *buf, size_t size)
{
	uint8_t *data;
	if (size > buf->size) {
		data = (uint8_t *)realloc(buf->data, size);
		if (!data)
			return -ENOMEM;

		buf->data = data;
		buf->size = size;
	}
	return 0;
}

/* ensure buffer size is enough */
static inline
int obus_buffer_ensure_write_space(struct obus_buffer *buf, size_t length)
{
	return obus_buffer_ensure_realloc(buf, buf->length + length);
}

static inline
size_t obus_buffer_size(struct obus_buffer *buf)
{
	return buf->size;
}

static inline
void obus_buffer_clear(struct obus_buffer *buf)
{
	buf->length = 0;
}

/* get write offset */
static inline
size_t obus_buffer_write_offset(struct obus_buffer *buf)
{
	return buf->length;
}

/* get write pointer */
static inline
void *obus_buffer_write_ptr(struct obus_buffer *buf)
{
	return (void *)&buf->data[buf->length];
}

/* get write space */
static inline
size_t obus_buffer_write_space(struct obus_buffer *buf)
{
	return buf->size - buf->length;
}

/* get write pointer */
static inline
void obus_buffer_inc_write_ptr(struct obus_buffer *buf, size_t length)
{
	buf->length += length;
}

/* get buffer length */
static inline
size_t obus_buffer_length(struct obus_buffer *buf)
{
	return buf->length;
}

/* get buffer read length */
static inline
size_t obus_buffer_read_length(struct obus_buffer *buf)
{
	return (buf->length > buf->pos) ? buf->length - buf->pos : 0;
}

/* reserve space */
static inline
int obus_buffer_reserve(struct obus_buffer *buf, size_t length)
{
	int ret;

	ret = obus_buffer_ensure_realloc(buf, buf->length + length);
	if (ret < 0)
		return ret;

	buf->length += length;
	return 0;
}

/* write size bytes in buffer at given offset */
static inline
int obus_buffer_write(struct obus_buffer *buf, size_t offset, const void *ptr,
		      size_t size)
{
	int ret;

	/* reallocate buffer if needed */
	ret = obus_buffer_ensure_realloc(buf, offset + size);
	if (ret < 0)
		return ret;

	memcpy(buf->data + offset, ptr, size);
	if (offset + size > buf->length)
		buf->length += (offset - buf->length) + size;

	return 0;
}

/* write size bytes in buffer at given offset */
static inline
int obus_buffer_append(struct obus_buffer *buf, const void *ptr, size_t size)
{
	return obus_buffer_write(buf, buf->length, ptr, size);
}

static inline
int obus_buffer_append_u8(struct obus_buffer *buf, uint8_t value)
{
	int ret;
	ret = obus_buffer_ensure_write_space(buf, 1);
	if (ret < 0)
		return ret;

	buf->data[buf->length++] = value;
	return 0;
}

static inline
int obus_buffer_write_u8(struct obus_buffer *buf, uint8_t value,
			  size_t offset)
{
	int ret;
	ret = obus_buffer_ensure_realloc(buf, offset + 1);
	if (ret < 0)
		return ret;

	buf->data[offset] = value;
	return 0;
}

static inline
size_t obus_buffer_get_read_position(const struct obus_buffer *buf)
{
	return buf->pos;
}

static inline
int obus_buffer_set_read_position(struct obus_buffer *buf, size_t pos)
{
	if (pos > buf->length)
		return -EINVAL;

	buf->pos = pos;
	return 0;
}

static inline
int obus_buffer_inc_read_position(struct obus_buffer *buf, size_t value)
{
	if (value + buf->pos > buf->length)
		return -EINVAL;

	buf->pos += value;
	return 0;
}

static inline
int obus_buffer_read_u8(struct obus_buffer *buf, uint8_t *val)
{
	if (!buf || !val)
		return -EINVAL;

	if (buf->pos >= buf->length)
		return -EAGAIN;

	*val = buf->data[buf->pos];
	buf->pos++;
	return 0;
}

static inline
int obus_buffer_append_u16(struct obus_buffer *buf, uint16_t value)
{
	int ret;
	ret = obus_buffer_ensure_write_space(buf, 2);
	if (ret < 0)
		return ret;

	buf->data[buf->length++] = (uint8_t)((value >> 8) & 0xff);
	buf->data[buf->length++] = (uint8_t)(value & 0xff);
	return 0;
}

static inline
int obus_buffer_write_u16(struct obus_buffer *buf, uint16_t value,
			  size_t offset)
{
	int ret;
	ret = obus_buffer_ensure_realloc(buf, offset + 2);
	if (ret < 0)
		return ret;

	buf->data[offset] = (uint8_t)((value >> 8) & 0xff);
	buf->data[offset + 1] = (uint8_t)(value & 0xff);
	return 0;
}

static inline
int obus_buffer_read_u16(struct obus_buffer *buf, uint16_t *val)
{
	if (!val || ((buf->pos + 1) >= buf->length))
		return -EINVAL;

	*val = (uint16_t)((buf->data[buf->pos] << 8) +
			   buf->data[buf->pos + 1]);
	buf->pos += 2;
	return 0;
}

static inline
int obus_buffer_append_u32(struct obus_buffer *buf, uint32_t value)
{
	int ret;
	ret = obus_buffer_ensure_write_space(buf, 4);
	if (ret < 0)
		return ret;

	buf->data[buf->length++] = (uint8_t)((value >> 24) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 16) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 8) & 0xff);
	buf->data[buf->length++] = (uint8_t)(value & 0xff);
	return 0;
}

static inline
int obus_buffer_write_u32(struct obus_buffer *buf, uint32_t value,
			  size_t offset)
{
	int ret;
	ret = obus_buffer_ensure_realloc(buf, offset + 4);
	if (ret < 0)
		return ret;

	buf->data[offset] = (uint8_t)((value >> 24) & 0xff);
	buf->data[offset + 1] = (uint8_t)((value >> 16) & 0xff);
	buf->data[offset + 2] = (uint8_t)((value >> 8) & 0xff);
	buf->data[offset + 3] = (uint8_t)(value & 0xff);
	return 0;
}

static inline
int obus_buffer_read_u32(struct obus_buffer *buf, uint32_t *val)
{
	if (!buf || !val)
		return -EINVAL;

	if ((buf->pos + 3) >= buf->length)
		return -EAGAIN;

	*val = ((uint32_t)buf->data[buf->pos]) << 24;
	*val |= ((uint32_t)buf->data[buf->pos + 1]) << 16;
	*val |= ((uint32_t)buf->data[buf->pos + 2]) << 8;
	*val |= ((uint32_t)buf->data[buf->pos + 3]);
	buf->pos += 4;
	return 0;
}

static inline
int obus_buffer_append_u64(struct obus_buffer *buf, uint64_t value)
{
	int ret;
	ret = obus_buffer_ensure_write_space(buf, 8);
	if (ret < 0)
		return ret;

	buf->data[buf->length++] = (uint8_t)((value >> 56) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 48) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 40) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 32) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 24) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 16) & 0xff);
	buf->data[buf->length++] = (uint8_t)((value >> 8) & 0xff);
	buf->data[buf->length++] = (uint8_t)(value & 0xff);
	return 0;
}

static inline
int obus_buffer_write_u64(struct obus_buffer *buf, uint64_t value,
			  size_t offset)
{
	int ret;
	ret = obus_buffer_ensure_realloc(buf, offset + 8);
	if (ret < 0)
		return ret;

	buf->data[offset] = (uint8_t)((value >> 56) & 0xff);
	buf->data[offset + 1] = (uint8_t)((value >> 48) & 0xff);
	buf->data[offset + 2] = (uint8_t)((value >> 40) & 0xff);
	buf->data[offset + 3] = (uint8_t)((value >> 32) & 0xff);
	buf->data[offset + 4] = (uint8_t)((value >> 24) & 0xff);
	buf->data[offset + 5] = (uint8_t)((value >> 16) & 0xff);
	buf->data[offset + 6] = (uint8_t)((value >> 8) & 0xff);
	buf->data[offset + 7] = (uint8_t)(value & 0xff);
	return 0;
}

static inline
int obus_buffer_read_u64(struct obus_buffer *buf, uint64_t *val)
{
	if (!val || ((buf->pos + 7) >= buf->size))
		return -EINVAL;

	*val = ((uint64_t)buf->data[buf->pos]) << 56;
	*val |= ((uint64_t)buf->data[buf->pos + 1]) << 48;
	*val |= ((uint64_t)buf->data[buf->pos + 2]) << 40;
	*val |= ((uint64_t)buf->data[buf->pos + 3]) << 32;
	*val |= ((uint64_t)buf->data[buf->pos + 4]) << 24;
	*val |= ((uint64_t)buf->data[buf->pos + 5]) << 16;
	*val |= ((uint64_t)buf->data[buf->pos + 6]) << 8;
	*val |= ((uint64_t)buf->data[buf->pos + 7]);
	buf->pos += 8;
	return 0;
}

static inline
int obus_buffer_append_string(struct obus_buffer *buf, const char *str)
{
	int ret;
	uint32_t size;

	size = str ? (uint32_t)(strlen(str) + 1) : 0;
	ret = obus_buffer_ensure_write_space(buf, size + 4);
	if (ret < 0)
		return ret;

	ret |= obus_buffer_append_u32(buf, size);
	if (size > 0)
		ret |= obus_buffer_append(buf, str, size);

	return ret;
}


static inline
int obus_buffer_dup(struct obus_buffer *buf, size_t size, void **data)
{
	void *ptr;

	if (!buf || !data)
		return -EINVAL;

	*data = NULL;
	if (size == 0)
		return 0;

	if ((buf->pos + size) > buf->length)
		return -EAGAIN;

	/* add extra allocated bytes usefull for string */
	ptr = malloc(size);
	if (!ptr)
		return -ENOMEM;

	memcpy(ptr, &buf->data[buf->pos], size);
	buf->pos += size;
	*data = ptr;
	return 0;
}

static inline
int obus_buffer_read_string(struct obus_buffer *buf, char **str)
{
	int ret;
	uint32_t size;

	if (!buf || !str)
		return -EINVAL;

	/* read next 4 bytes */
	ret = obus_buffer_read_u32(buf, &size);
	if (ret < 0)
		return ret;

	/* NULL string */
	if (size == 0) {
		*str = NULL;
		return 0;
	}

	ret = obus_buffer_dup(buf, size, (void **)str);
	if (ret < 0)
		return ret;

	/* ensure NULL terminated string */
	(*str)[size - 1] = '\0';
	return 0;
}

static inline
int obus_buffer_append_f32(struct obus_buffer *buf, float value)
{
	union {
		float f;
		uint32_t u32;
	} val = { .f = value };

	return obus_buffer_append_u32(buf, val.u32);
}

static inline
int obus_buffer_write_f32(struct obus_buffer *buf, float value,
			  size_t offset)
{
	union {
		float f;
		uint32_t u32;
	} val = { .f = value };

	return obus_buffer_write_u32(buf, val.u32, offset);
}

static inline
int obus_buffer_read_f32(struct obus_buffer *buf, float *value)
{
	int ret;
	union {
		float f;
		uint32_t u32;
	} val;

	ret = obus_buffer_read_u32(buf, &val.u32);
	if (ret == 0)
		*value = val.f;

	return ret;
}

static inline
int obus_buffer_append_f64(struct obus_buffer *buf, double value)
{
	union {
		double d;
		uint64_t u64;
	} val = { .d = value };

	return obus_buffer_append_u64(buf, val.u64);
}

static inline
int obus_buffer_write_f64(struct obus_buffer *buf, double value,
			  size_t offset)
{
	union {
		double d;
		uint64_t u64;
	} val = { .d = value };

	return obus_buffer_write_u64(buf, val.u64, offset);
}

static inline
int obus_buffer_read_f64(struct obus_buffer *buf, double *value)
{
	int ret;
	union {
		double d;
		uint64_t u64;
	} val;

	ret = obus_buffer_read_u64(buf, &val.u64);
	if (ret == 0)
		*value = val.d;

	return ret;
}

/* find first byte in buffer */
static inline
ssize_t obus_buffer_find(struct obus_buffer *buf, size_t offset, uint8_t byte)
{
	size_t i;
	for (i = offset; i < buf->length; i++) {
		if (buf->data[i] == byte)
			return (ssize_t)i;
	}

	return -1;
}

static inline
void obus_buffer_remove_first(struct obus_buffer *buf, size_t size)
{
	if (size == 0 || buf->length == 0)
		return;

	if (size >= buf->length) {
		buf->length = 0;
		return;
	}

	memmove(buf->data, buf->data + size, buf->length - size);
	buf->length -= size;
	buf->pos = (buf->pos > size) ? (buf->pos - size) : 0;
}

static inline
void obus_buffer_remove_read(struct obus_buffer *buf)
{
	obus_buffer_remove_first(buf, buf->pos);
	buf->pos = 0;
}

static inline
void obus_buffer_pool_init(struct obus_buffer_pool *pool, size_t buffsize)
{
	if (!pool)
		return;

	pool->buffsize = buffsize;
	obus_list_init(&pool->free_bufs);
}

static inline
void obus_buffer_pool_destroy(struct obus_buffer_pool *pool)
{
	struct obus_buffer *buf, *tmp;
	if (!pool)
		return;

	obus_list_walk_entry_forward_safe(&pool->free_bufs, buf, tmp, node) {
		obus_list_del(&buf->node);
		obus_buffer_destroy(buf);
	}
}

static inline
struct obus_buffer *obus_buffer_pool_peek(struct obus_buffer_pool *pool)
{
	struct obus_buffer *buf;
	struct obus_node *first;

	if (!pool)
		return NULL;

	if (obus_list_is_empty(&pool->free_bufs)) {
		/* create a new buffer */
		buf = obus_buffer_new(pool->buffsize, pool);
	} else {
		/* get buffer and detach from free list */
		first = obus_list_first(&pool->free_bufs);
		buf = obus_list_entry(first, struct obus_buffer, node);
		if (buf) {
			obus_list_del(&buf->node);
			buf->refcnt = 1;
		}
	}

	return buf;
}



#endif /* _OBUS_BUFFER_H_*/
