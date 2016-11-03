/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_object.h
 *
 * @brief brief obus object header
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

#ifndef _OBUS_OBJECT_H_
#define _OBUS_OBJECT_H_

struct obus_object {
	/* object node */
	struct obus_node node;
	/* bus */
	struct obus_bus *bus;
	/* object event bus event node */
	struct obus_node event_node;
	/* object description */
	const struct obus_object_desc *desc;
	/* object handle */
	obus_handle_t handle;
	/* is object registered */
	unsigned int is_registered:1;
	/* user data */
	void *user_data;
	/* object info struct */
	struct obus_struct info;
	/* call handlers array */
	obus_method_handler_cb_t handlers[0];
};

struct obus_object *obus_object_new(const struct obus_object_desc *desc,
				    const obus_method_handler_cb_t *cbs,
				    const struct obus_struct *info);

int obus_object_destroy(struct obus_object *obj);

int obus_object_is_registered(const struct obus_object *obj);

obus_handle_t obus_object_get_handle(const struct obus_object *obj);

const struct obus_object_desc *
obus_object_get_desc(const struct obus_object *obj);

obus_method_handler_cb_t
obus_object_get_method_handler(struct obus_object *obj,
			       const struct obus_method_desc *desc);

enum obus_method_state
obus_object_get_method_state(struct obus_object *obj, uint16_t uid);

int obus_object_remove_encode(struct obus_object *obj,
			      struct obus_buffer *buf);

struct obus_object *obus_object_remove_decode(struct obus_bus *bus,
					     struct obus_buffer *buf);

int obus_object_add_encode(struct obus_object *obj, struct obus_buffer *buf);

struct obus_object *obus_object_add_decode(struct obus_bus_api *api,
					   struct obus_buffer *buf);

void obus_object_log(const struct obus_object *obj, enum obus_log_level level);

int obus_object_set_user_data(struct obus_object *object, void *user_data);
void *obus_object_get_user_data(const struct obus_object *object);

const void *obus_object_get_info(const struct obus_object *object);


#endif /* _OBUS_OBJECT_H_ */
