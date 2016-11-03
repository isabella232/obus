/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_packet.c
 *
 * @brief obus packet
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

/* obus magic */
#define OBUS_MAGIC_0 'o'
#define OBUS_MAGIC_1 'b'
#define OBUS_MAGIC_2 'u'
#define OBUS_MAGIC_3 's'

static const uint32_t obus_magic = (OBUS_MAGIC_0 << 24) +
				   (OBUS_MAGIC_1 << 16) +
				   (OBUS_MAGIC_2 << 8) +
				    OBUS_MAGIC_3;

static const char *const obus_pkt_types[OBUS_PKT_COUNT] = {
	[OBUS_PKT_CONREQ]	= "CONREQ",
	[OBUS_PKT_CONRESP]	= "CONRESP",
	[OBUS_PKT_ADD]		= "ADD",
	[OBUS_PKT_REMOVE]	= "REMOVE",
	[OBUS_PKT_EVENT]	= "EVENT",
	[OBUS_PKT_CALL]		= "CALL",
	[OBUS_PKT_ACK]		= "ACK",
	[OBUS_PKT_BUS_EVENT]	= "BUS_EVENT",
};

/* log header */
static void obus_packet_log_header(const struct obus_packet_header *hdr)
{
	obus_debug("PACKET[%s]: %d bytes", obus_pkt_types[hdr->type],
		   hdr->size);
}

static const char *const conresp_status[OBUS_CONRESP_STATUS_COUNT] = {
	[OBUS_CONRESP_ACCEPTED]		= "ACCEPTED",
	[OBUS_CONRESP_REFUSED]		= "REFUSED",
};

const char *obus_conresp_status_str(enum obus_conresp_status status)
{
	return (status >= OBUS_CONRESP_STATUS_COUNT) ? "<INVALID>" :
			conresp_status[status];
}

/******** packet header format ***
 ******************************
 *  magic | size | type |
 *   4B      4B     1B
 ******************************/
#define OBUS_PKT_HDR_SIZE 9
#define OBUS_PKT_HDR_MAGIC_OFFSET 0
#define OBUS_PKT_HDR_SIZE_OFFSET 4
#define OBUS_PKT_HDR_TYPE_OFFSET 8

static int obus_packet_encode_header(struct obus_buffer *buf, uint8_t type)
{
	int ret;

	/* write packet magic */
	ret = obus_buffer_write_u32(buf, obus_magic, OBUS_PKT_HDR_MAGIC_OFFSET);
	if (ret < 0)
		return ret;

	/* write packet size */
	ret = obus_buffer_write_u32(buf, (uint32_t)obus_buffer_length(buf),
				    OBUS_PKT_HDR_SIZE_OFFSET);
	if (ret < 0)
		return ret;

	/* write packet type */
	ret = obus_buffer_write_u8(buf, type, OBUS_PKT_HDR_TYPE_OFFSET);
	if (ret < 0)
		return ret;

	return 0;
}

static int obus_packet_conreq_decode(struct obus_packet_decoder *d,
				     struct obus_packet_conreq *req)
{
	int ret;

	if (!d || !req)
		return -EINVAL;

	/* read protocol version */
	ret = obus_buffer_read_u8(d->buf, &req->version);
	if (ret < 0)
		return ret;

	/* read bus name */
	ret = obus_buffer_read_string(d->buf, &req->bus);
	if (ret < 0)
		return ret;

	/* read crc client api */
	ret = obus_buffer_read_u32(d->buf, &req->crc);
	if (ret < 0)
		return ret;

	/* read client name */
	ret = obus_buffer_read_string(d->buf, &req->client);
	if (ret < 0)
		return ret;

	return 0;
}

/* encode connection request info from read packet */
int obus_packet_conreq_encode(struct obus_buffer *buf, const char *client,
			      const char *bus, uint32_t crc)
{
	int ret;

	if (!buf)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* add protocol version */
	ret = obus_buffer_append_u8(buf, OBUS_PROTOCOL_VERSION);
	if (ret < 0)
		return ret;

	/* add bus name */
	ret = obus_buffer_append_string(buf, bus);
	if (ret < 0)
		return ret;

	/* add bus crc api */
	ret = obus_buffer_append_u32(buf, crc);
	if (ret < 0)
		return ret;

	/* add client name */
	ret = obus_buffer_append_string(buf, client);
	if (ret < 0)
		return ret;

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_CONREQ);
}

static int obus_packet_conresp_decode(struct obus_packet_decoder *d,
				      struct obus_packet_conresp *resp)
{
	int ret;
	struct obus_object *obj;
	uint32_t i, n_objects;
	uint8_t status;

	if (!d || !resp)
		return -EINVAL;

	obus_list_init(&resp->objects);

	/* read connection status */
	ret = obus_buffer_read_u8(d->buf, &status);
	if (ret < 0)
		return ret;

	/* check status */
	if (status > OBUS_CONRESP_STATUS_COUNT) {
		obus_error("invalid connection response status %d", status);
		return -EINVAL;
	}
	/* set status */
	resp->status = (enum obus_conresp_status)status;

	/* read number of objects in list */
	ret = obus_buffer_read_u32(d->buf, &n_objects);
	if (ret < 0)
		return ret;

	/* create objects */
	for (i = 0; i < n_objects; i++) {
		/* decode object */
		obj = obus_object_add_decode(&d->bus->api, d->buf);
		/* add object in list */
		if (obj)
			obus_list_add_before(&resp->objects, &obj->node);
	}

	return 0;
}

/* encode connection response info to packet buffer */
int obus_packet_conresp_encode(struct obus_buffer *buf,
			       enum obus_conresp_status status,
			       struct obus_node *objects)
{
	int ret;
	struct obus_object *obj;
	uint32_t n_objects;

	if (!buf)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* add connection response status */
	ret = obus_buffer_append_u8(buf, (uint8_t)status);
	if (ret < 0)
		return ret;

	/* add number of objects in list */
	n_objects = objects ? (uint32_t)obus_list_length(objects) : 0;
	ret = obus_buffer_append_u32(buf, n_objects);
	if (ret < 0)
		return ret;

	/* encode each objects */
	if (n_objects > 0) {
		obus_list_walk_entry_forward(objects, obj, node) {
			/* encode object */
			ret = obus_object_add_encode(obj, buf);
			if (ret < 0)
				return ret;
		}
	}

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_CONRESP);
}

static int obus_packet_add_decode(struct obus_packet_decoder *d,
				  struct obus_object **obj)
{
	if (!obj || !d)
		return -EINVAL;

	/* decode object */
	*obj = obus_object_add_decode(&d->bus->api, d->buf);
	if (!*obj) {
		obus_error("can't decode object from packet");
		return -ENOENT;
	}

	return 0;
}

int obus_packet_add_encode(struct obus_buffer *buf, struct obus_object *obj)
{
	int ret;

	if (!buf || !obj)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* encode object */
	ret = obus_object_add_encode(obj, buf);
	if (ret < 0) {
		obus_error("can't encode object (uid=%d) handle=%d",
			   obj->desc->uid, obj->handle);
		return ret;
	}

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_ADD);
}

static int
obus_packet_remove_decode(struct obus_packet_decoder *d,
			  struct obus_object **obj)
{
	if (!obj || !d)
		return -EINVAL;

	*obj = obus_object_remove_decode(d->bus, d->buf);
	return 0;
}

int obus_packet_remove_encode(struct obus_buffer *buf, struct obus_object *obj)
{
	int ret;

	if (!buf || !obj)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* encode unregister object */
	ret = obus_object_remove_encode(obj, buf);
	if (ret < 0)
		return ret;

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_REMOVE);
}

static int
obus_packet_event_decode(struct obus_packet_decoder *d,
			 struct obus_event **event)
{
	if (!event || !d)
		return -EINVAL;

	/* decode object event */
	*event = obus_event_decode(d->bus, d->buf);
	if (!*event) {
		obus_error("can't decode object event from packet");
		return -ENOENT;
	}

	return 0;
}

int obus_packet_event_encode(struct obus_buffer *buf,
				    struct obus_event *event)
{
	int ret;

	if (!buf || !event)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* encode object event */
	ret = obus_event_encode(event, buf);
	if (ret < 0) {
		obus_error("can't encode object %s event %s",
			   event->obj->desc->name, event->desc->name);
		return ret;
	}

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_EVENT);
}

/* encode bus event */
int obus_packet_bus_event_encode(struct obus_buffer *buf,
				 struct obus_bus_event *event)
{
	int ret;

	if (!buf || !event)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* encode bus event */
	ret = obus_bus_event_encode(event, buf);
	if (ret < 0) {
		obus_error("can't encode bus event %s", event->desc->name);
		return ret;
	}

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_BUS_EVENT);
}

static int
obus_packet_bus_event_decode(struct obus_packet_decoder *d,
			     struct obus_bus_event **event)
{
	if (!event || !d)
		return -EINVAL;

	/* decode object event */
	*event = obus_bus_event_decode(d->bus, d->buf);
	if (!*event) {
		obus_error("can't decode bus event from packet");
		return -ENOENT;
	}

	return 0;
}

static int
obus_packet_call_decode(struct obus_packet_decoder *d, struct obus_call **call)
{
	if (!call || !d)
		return -EINVAL;

	/* decode object event */
	*call = obus_call_decode(d->bus, d->buf);
	if (!*call) {
		obus_error("can't decode object call from packet");
		return -ENOENT;
	}

	return 0;
}


int obus_packet_call_encode(struct obus_buffer *buf, struct obus_call *call)
{
	int ret;

	if (!buf || !call)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* encode object call */
	ret = obus_call_encode(call, buf);
	if (ret < 0) {
		obus_error("can't encode object %s method %s call",
			   call->obj->desc->name, call->desc->name);
		return ret;
	}

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_CALL);
}

/* encode ack */
int obus_packet_ack_encode(struct obus_buffer *buf, struct obus_ack *ack)
{
	int ret;

	if (!buf)
		return -EINVAL;

	/* clear buffer */
	obus_buffer_clear(buf);

	/* reserve extra space for header */
	ret = obus_buffer_reserve(buf, OBUS_PKT_HDR_SIZE);
	if (ret < 0)
		return ret;

	/* add call handle */
	ret = obus_buffer_append_u16(buf, ack->handle);
	if (ret < 0)
		return ret;

	/* add call ack */
	ret = obus_buffer_append_u8(buf, (uint8_t)ack->status);
	if (ret < 0)
		return ret;

	/* encode header */
	return obus_packet_encode_header(buf, OBUS_PKT_ACK);
}


static int
obus_packet_ack_decode(struct obus_packet_decoder *d, struct obus_ack *ack)
{
	uint8_t status;
	int ret;

	if (!d || !ack)
		return -EINVAL;

	/* read ack handle */
	ret = obus_buffer_read_u16(d->buf, &ack->handle);
	if (ret < 0)
		return ret;

	/* read ack status */
	ret = obus_buffer_read_u8(d->buf, &status);
	if (ret < 0)
		return ret;

	ack->status = (enum obus_call_status)status;
	return 0;
}

static int obus_packet_decode_header(struct obus_packet_decoder *d)
{
	size_t pos;

	while (obus_buffer_read_length(d->buf) >= OBUS_PKT_HDR_SIZE) {
		memset(&d->hdr, 0, sizeof(d->hdr));

		/* get read position in buffer */
		pos = obus_buffer_get_read_position(d->buf);

		/* read magic 32 bits from buffer
		 * this can't failed because buffer size is checked in
		 * while condition */
		(void)obus_buffer_read_u32(d->buf, &d->hdr.magic);

		/* check magic value */
		if (d->hdr.magic != obus_magic) {
			/* bad magic, try again ... */
			obus_buffer_set_read_position(d->buf, pos + 1);
			continue;
		}

		/* read message size */
		(void)obus_buffer_read_u32(d->buf, &d->hdr.size);

		/* read packet type */
		(void)obus_buffer_read_u8(d->buf, &d->hdr.type);

		/* check packet type */
		if (d->hdr.type >= OBUS_PKT_COUNT) {
			/* invalid packet type */
			obus_buffer_set_read_position(d->buf, pos + 1);
			continue;
		}

		/* header found:
		 * 1. remove all unused bytes before header
		 * 2. set header pointer to start of buffer */
		obus_buffer_remove_first(d->buf, pos);
		d->hdr_valid = 1;
		return 0;
	}

	/* header not found, clear buffer, keep last bytes
	 * maybe start of new packet header */
	if (obus_buffer_length(d->buf) >= OBUS_PKT_HDR_SIZE) {
		obus_buffer_remove_first(d->buf,
					 obus_buffer_length(d->buf) -
					 (OBUS_PKT_HDR_SIZE - 1));
	}

	return -EAGAIN;
}

/* init decoder */
int obus_packet_decoder_init(struct obus_packet_decoder *d,
			     struct obus_buffer *buf,
			     struct obus_bus *bus,
			     struct obus_io *io,
			     int log_hdr)
{
	d->hdr_valid = 0;
	d->bus = bus;
	d->io = io;
	d->buf = obus_buffer_ref(buf);
	d->log_hdr = log_hdr ? 1 : 0;
	obus_buffer_clear(d->buf);
	return 0;
}

/* init decoder */
int obus_packet_decoder_destroy(struct obus_packet_decoder *d)
{
	obus_buffer_clear(d->buf);
	obus_buffer_unref(d->buf);
	d->buf = NULL;
	return 0;
}

/* reset decoder */
int obus_packet_decoder_reset(struct obus_packet_decoder *d)
{
	d->hdr_valid = 0;
	obus_buffer_clear(d->buf);
	return 0;
}


/* read from decoder */
int obus_packet_decoder_read(struct obus_packet_decoder *d,
			     struct obus_packet_info *info)
{
	int ret, read_more;
	size_t len;
	ssize_t nbytes;

	if (!d || !info)
		return -EINVAL;

	read_more = 0;
	while (1) {
		ret = -1;
		if (read_more) {
			/* check remaining write space in buffer */
			len = obus_buffer_write_space(d->buf);

			/* if no more space available,
			 * double buffer capacity */
			if (len == 0)
				len = obus_buffer_size(d->buf);

			/* read io */
			nbytes = obus_io_read(d->io, d->buf, len);
			if (nbytes < 0)
				return (int)nbytes;
			else if (nbytes == 0)
				return -EIO;
		}

		/* check packet header is already decoded */
		if (!d->hdr_valid) {
			/* decode packet header */
			ret = obus_packet_decode_header(d);
			if (ret < 0) {
				read_more = 1;
				continue;
			}
		}

		/* header parsed & valid, check whole packet read */
		if (obus_buffer_length(d->buf) < d->hdr.size) {
			read_more = 1;
			continue;
		}

		/* log packet header */
		if (d->log_hdr)
			obus_packet_log_header(&d->hdr);

		/* decode packet */
		info->type = (enum obus_packet_type)d->hdr.type;
		switch (info->type) {
		case OBUS_PKT_CONREQ:
			ret = obus_packet_conreq_decode(d, &info->conreq);
			break;
		case OBUS_PKT_CONRESP:
			ret = obus_packet_conresp_decode(d, &info->conresp);
			break;
		case OBUS_PKT_ADD:
			ret = obus_packet_add_decode(d, &info->object);
			break;
		case OBUS_PKT_REMOVE:
			ret = obus_packet_remove_decode(d, &info->object);
			break;
		case OBUS_PKT_BUS_EVENT:
			ret = obus_packet_bus_event_decode(d, &info->bus_event);
			break;
		case OBUS_PKT_EVENT:
			ret = obus_packet_event_decode(d, &info->event);
			break;
		case OBUS_PKT_CALL:
			ret = obus_packet_call_decode(d, &info->call);
			break;
		case OBUS_PKT_ACK:
			ret = obus_packet_ack_decode(d, &info->ack);
			break;
		case OBUS_PKT_COUNT:
		default:
			ret = -ENOENT;
			break;
		}

		/* remove decoded packet data from buffer */
		obus_buffer_remove_first(d->buf, d->hdr.size);
		obus_buffer_set_read_position(d->buf, 0);
		d->hdr_valid = 0;
		if (ret == 0)
			return 0;
	}

	return ret;
}
