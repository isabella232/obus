/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_event.h
 *
 * @brief object event
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

#ifndef _OBUS_EVENT_H_
#define _OBUS_EVENT_H_

int obus_event_init(struct obus_event *event, struct obus_object *obj,
		    const struct obus_event_desc *desc,
		    const struct obus_struct *info);

struct obus_event *obus_event_new(struct obus_object *obj,
				  const struct obus_event_desc *desc,
				  const struct obus_struct *info);

int obus_event_destroy(struct obus_event *event);

int obus_event_sanitize(struct obus_event *event, int is_server);

const struct obus_object_desc *
obus_event_get_object_desc(const struct obus_event *event);

int obus_event_is_committed(struct obus_event *event);

int obus_event_is_empty(const struct obus_event *event);

int obus_event_commit(struct obus_event *event);

struct obus_object *obus_event_get_object(struct obus_event *event);

int obus_event_encode(struct obus_event *event, struct obus_buffer *buf);

struct obus_event *obus_event_decode(struct obus_bus *bus,
				     struct obus_buffer *buf);

void obus_event_log(const struct obus_event *event, enum obus_log_level level);

#endif /* _OBUS_EVENT_H_ */
