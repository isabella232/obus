/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_packet.h
 *
 * @brief obus packet header
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

#ifndef _OBUS_PACKET_H_
#define _OBUS_PACKET_H_

/* current obus protocol version */
#define OBUS_PROTOCOL_VERSION 0x02

/* packet type */
enum obus_packet_type {
	/**
	 * connection request, from client to server
	 */
	OBUS_PKT_CONREQ = 0,
	/**
	 * connection response, from server to client
	 */
	OBUS_PKT_CONRESP,
	/**
	 * add/register object, from server to client
	 */
	OBUS_PKT_ADD,
	/**
	 * remove/unregister object, from server to client
	 */
	OBUS_PKT_REMOVE,
	/**
	 * bus event, from server to client
	 */
	OBUS_PKT_BUS_EVENT,
	/**
	 * object event, from server to client
	 */
	OBUS_PKT_EVENT,
	/**
	 * method call, from client to server
	 */
	OBUS_PKT_CALL,
	/**
	 * method call acknowledgment, from server to client
	 */
	OBUS_PKT_ACK,
	OBUS_PKT_COUNT,
};

struct obus_packet_conreq {
	/* protocol version */
	uint8_t version;
	/* bus name */
	char *bus;
	/* crc bus */
	uint32_t crc;
	/* bus client name */
	char *client;
};

enum obus_conresp_status {
	OBUS_CONRESP_ACCEPTED = 0,
	OBUS_CONRESP_REFUSED,
	OBUS_CONRESP_STATUS_COUNT,
};

const char *obus_conresp_status_str(enum obus_conresp_status status);

struct obus_ack {
	/* call handle */
	obus_handle_t handle;
	/* call status */
	enum obus_call_status status;
};

struct obus_packet_conresp {
	/* connection status */
	enum obus_conresp_status status;
	/* object list sync */
	struct obus_node objects;
};

struct obus_packet_info {
	enum obus_packet_type type;
	union {
		struct obus_packet_conreq conreq;
		struct obus_packet_conresp conresp;
		struct obus_bus_event *bus_event;
		struct obus_event *event;
		struct obus_call *call;
		struct obus_object *object;
		struct obus_ack ack;
	};
};

struct obus_packet_header {
	uint32_t magic;
	uint32_t size;
	uint8_t type;
};

/* packet reader */
struct obus_packet_decoder {
	/* packet header */
	struct obus_packet_header hdr;
	/* has valid header */
	int hdr_valid;
	/* associated bus */
	struct obus_bus *bus;
	/* associated buffer */
	struct obus_buffer *buf;
	/* associated io */
	struct obus_io *io;
	/* packet log header flag */
	int log_hdr;
};

/* init decoder */
int obus_packet_decoder_init(struct obus_packet_decoder *dec,
			     struct obus_buffer *buf,
			     struct obus_bus *bus,
			     struct obus_io *io,
			     int log_hdr);

/* init decoder */
int obus_packet_decoder_destroy(struct obus_packet_decoder *dec);

/* reset decoder */
int obus_packet_decoder_reset(struct obus_packet_decoder *d);

/* read from decoder */
int obus_packet_decoder_read(struct obus_packet_decoder *d,
			     struct obus_packet_info *info);

/* encode connection request info to packet buffer */
int obus_packet_conreq_encode(struct obus_buffer *buf, const char *client,
			      const char *bus, uint32_t crc);

/* encode connection response info to packet buffer */
int obus_packet_conresp_encode(struct obus_buffer *buf,
			       enum obus_conresp_status status,
			       struct obus_node *objects);

/* encode add object */
int obus_packet_add_encode(struct obus_buffer *buf,
			   struct obus_object *obj);

/* encode remove object */
int obus_packet_remove_encode(struct obus_buffer *buf,
			      struct obus_object *obj);

/* encode object event */
int obus_packet_event_encode(struct obus_buffer *buf,
			     struct obus_event *event);

/* encode bus event */
int obus_packet_bus_event_encode(struct obus_buffer *buf,
				 struct obus_bus_event *event);

/* encode object call */
int obus_packet_call_encode(struct obus_buffer *buf,
			    struct obus_call *call);

/* encode ack */
int obus_packet_ack_encode(struct obus_buffer *buf, struct obus_ack *ack);

#endif /* _OBUS_PACKET_H_ */
