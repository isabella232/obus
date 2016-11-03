/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_bus_event.h
 *
 * @brief object bus event
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

#ifndef _OBUS_BUS_EVENT_H_
#define _OBUS_BUS_EVENT_H_

struct obus_bus_event {
	/* obus bus event description */
	const struct obus_bus_event_desc *desc;
	/* obus bus event add objects list */
	struct obus_node add_objs;
	/* obus bus event remove objects list */
	struct obus_node remove_objs;
	/* obus bus object event objects list */
	struct obus_node obj_events;
	/* is event committed */
	unsigned int is_committed:1;
	/* is event allocated */
	unsigned int is_allocated:1;
};

const struct obus_bus_event_desc *
obus_bus_event_get_desc(const struct obus_bus_event *event);

void obus_bus_event_init(struct obus_bus_event *event,
			 const struct obus_bus_event_desc *desc);

int obus_bus_event_register_object(struct obus_bus_event *bus_event,
				   struct obus_object *object);

int obus_bus_event_unregister_object(struct obus_bus_event *bus_event,
				     struct obus_object *object);

int obus_bus_event_add_event(struct obus_bus_event *bus_event,
			     struct obus_event *event);

int obus_bus_event_encode(struct obus_bus_event *event,
			  struct obus_buffer *buf);

struct obus_bus_event *
obus_bus_event_decode(struct obus_bus *bus, struct obus_buffer *buf);

void obus_bus_event_log(struct obus_bus_event *event,
			enum obus_log_level level,
			int only_header);

#endif /* _OBUS_BUS_EVENT_H_ */
