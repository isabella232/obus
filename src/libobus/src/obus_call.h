/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_call.h
 *
 * @brief obus object call
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

#ifndef _OBUS_CALL_H_
#define _OBUS_CALL_H_

struct obus_call {
	/* call node */
	struct obus_node node;
	/* call method description */
	const struct obus_method_desc *desc;
	/* call related object */
	struct obus_object *obj;
	/* call handle */
	obus_handle_t handle;
	/* peer which do the call */
	struct obus_peer *peer;
	/* call status */
	enum obus_call_status status;
	/* method call arguments */
	struct obus_struct args;
	/* call ack handler */
	obus_method_call_status_handler_cb_t cb;
};

struct obus_call *obus_call_new(struct obus_object *obj,
				const struct obus_method_desc *desc,
				obus_method_call_status_handler_cb_t cb,
				const struct obus_struct *args);

int obus_call_destroy(struct obus_call *call);

struct obus_object *obus_call_get_object(struct obus_call *call);

const struct obus_object_desc *
obus_call_get_object_desc(const struct obus_call *call);

int obus_call_encode(struct obus_call *call, struct obus_buffer *buf);

struct obus_call *obus_call_decode(struct obus_bus *bus,
				  struct obus_buffer *buf);

void obus_call_log(struct obus_call *call, enum obus_log_level level);

void obus_ack_log(struct obus_ack *ack, struct obus_call *call,
		  enum obus_log_level level);

void obus_call_ack_notify(struct obus_call *call, enum obus_call_status status);


#endif /* _OBUS_CALL_H_ */
