/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file libobus_private.h
 *
 * @brief libobus private header used by generated code
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

#ifndef _LIBOBUS_PRIVATE_H_
#define _LIBOBUS_PRIVATE_H_

#ifndef OBUS_USE_PRIVATE
#error Only generated c files must include this header !
#endif

/**
 * obus_offsetof
 * evaluates to the offset (in bytes) of a given member within a
 * struct or union type, an expression of type size_t
 *
 * @param type structure name
 * @param member name of a member within the structure
 */
#define obus_offsetof(type, member) __builtin_offsetof(type, member)

/**
 * obus_container_of
 * cast a member of a structure out to the containing structure
 *
 * @param ptr the pointer to the member.
 * @param type the type of the container struct this is embedded in.
 * @param member the name of the member within the struct.
 * @return base address of member containing structure
 **/
#define obus_container_of(ptr, type, member)				\
({									\
	const typeof(((type *)0)->member) * __mptr = (ptr);		\
	(type *)((uintptr_t)__mptr - obus_offsetof(type, member));	\
})

/**
 * list node declaration
 */
struct obus_node {
	struct obus_node *next, *prev;
};

/**
 * obus sizeof array macro give number of elements in array
 */
#define OBUS_SIZEOF_ARRAY(x) (sizeof((x)) / sizeof((x)[0]))

/**
 * enum driver
 */
struct obus_enum_driver {
	/* enum name */
	const char *name;
	/* size of enum type */
	size_t size;
	/* enum default value */
	int32_t default_value;
	/* check if enum value is valid */
	int (*is_valid) (int32_t value);
	/* init enum to default value */
	void (*set_value) (void *addr, int32_t value);
	/* init enum to default value */
	int32_t (*get_value) (const void *addr);
	/* format enum value */
	void (*format) (const void *addr, char *buf, size_t size);
};

/**
 * method state enum driver
 */
extern const struct obus_enum_driver obus_method_state_driver;

/**
 * obus supported field type
 */
enum obus_field_type {
	OBUS_FIELD_U8,
	OBUS_FIELD_I8,
	OBUS_FIELD_U16,
	OBUS_FIELD_I16,
	OBUS_FIELD_U32,
	OBUS_FIELD_I32,
	OBUS_FIELD_U64,
	OBUS_FIELD_I64,
	OBUS_FIELD_ENUM,
	OBUS_FIELD_STRING,
	OBUS_FIELD_BOOL,
	OBUS_FIELD_F32,
	OBUS_FIELD_F64,
	OBUS_FIELD_ARRAY = (1 << 7),
	OBUS_FIELD_MASK = (0x7F)
};

/**
 * field role
 **/
enum obus_field_role {
	/* field role */
	OBUS_PROPERTY = 0,
	OBUS_METHOD,
	OBUS_ARGUMENT,
};

/**
 * field description
 */
struct obus_field_desc {
	/* field uid */
	uint16_t uid;
	/* field name */
	const char *name;
	/* field offset in struct */
	off_t offset;
	/* field role */
	enum obus_field_role role;
	/* field type */
	uint8_t type;
	/* field enum driver (if type is OBUS_FIELD_ENUM) */
	const struct obus_enum_driver *enum_drv;
	/* field array uint32 offset number for array */
	off_t nb_offset;
};

/**
 * struct field description
 */
struct obus_struct_desc {
	/* struct allocated size */
	size_t size;
	/* fields bits offset in struct */
	off_t fields_offset;
	/* number of fields in struct */
	uint32_t n_fields;
	/* fields array */
	const struct obus_field_desc *fields;
};

/**
 * represent a struct and its allocated content
 **/
struct obus_struct {
	/* struct description */
	const struct obus_struct_desc *desc;
	/* struct allocated address */
	union {
		void *addr;
		const void *const_addr;
	} u;
};

/**
 * obus object method description
 */
struct obus_method_desc {
	/* call uid */
	uint16_t uid;
	/* call name */
	const char *name;
	/* method args description */
	const struct obus_struct_desc *args_desc;
};

/**
 * obus bus event description
 */
struct obus_bus_event_desc {
	/* event uid */
	uint16_t uid;
	/* event name */
	const char *name;
};

struct obus_event_update_desc {
	/* update field */
	const struct obus_field_desc *field;
	/* update fag */
	uint32_t flags;
};

/**
 * obus object event description
 */
struct obus_event_desc {
	/* event uid */
	uint16_t uid;
	/* event name */
	const char *name;
	/* event updates descriptions */
	const struct obus_event_update_desc *updates;
	/* number of event updates */
	size_t n_updates;
};

/* obus method handler callback */
struct obus_object;

typedef void (*obus_method_handler_cb_t) (struct obus_object *obj,
			obus_handle_t handle, const void *args);

/* obus method status handler cb */
typedef void (*obus_method_call_status_handler_cb_t) (struct obus_object *obj,
			obus_handle_t handle, enum obus_call_status status);

/**
 * obus object description
 */
struct obus_object_desc {
	/* object uid */
	uint16_t uid;
	/* object name */
	const char *name;
	/* object & event info struct description */
	const struct obus_struct_desc *info_desc;
	/* number of events */
	uint16_t n_events;
	/* events array */
	const struct obus_event_desc *events;
	/* number of methods */
	uint16_t n_methods;
	/* methods descriptions struct array */
	const struct obus_method_desc *methods;
};

/**
 * obus bus description
 */
struct obus_bus_desc {
	/* bus name */
	const char *name;
	/* bus objects type description */
	uint16_t n_objects;
	const struct obus_object_desc * const *objects;
	/* bus events type description */
	uint16_t n_events;
	const struct obus_bus_event_desc *events;
	/* bus crc api */
	uint32_t crc;
};

struct obus_event {
	/* event node */
	struct obus_node node;
	/* event bus event node */
	struct obus_node event_node;
	/* event description */
	const struct obus_event_desc *desc;
	/* event related object */
	struct obus_object *obj;
	/* is event dynamically allocated */
	unsigned int is_allocated:1;
	/* is event committed */
	unsigned int is_committed:1;
	/* event info struct (same type as object one) */
	struct obus_struct info;
};

typedef void (*obus_provider_add_cb_t) (void *priv_object,
		const struct obus_bus_event *bus_event, void *user_data);

typedef void (*obus_provider_remove_cb_t) (void *priv_object,
		const struct obus_bus_event *bus_event, void *user_data);

typedef void (*obus_provider_event_cb_t) (void *priv_object, void *priv_event,
		const struct obus_bus_event *bus_event, void *user_data);

struct obus_provider {
	/* provider node */
	struct obus_node node;
	/* object description */
	const struct obus_object_desc *desc;
	/* object registered callback */
	obus_provider_add_cb_t add;
	/* object unregistered callback */
	obus_provider_remove_cb_t remove;
	/* object event callback */
	obus_provider_event_cb_t event;
	/* provider user data */
	void *user_data;
};

struct obus_call;

int obus_client_register_provider(struct obus_client *client,
				  struct obus_provider *prov);

int obus_client_unregister_provider(struct obus_client *client,
				    struct obus_provider *prov);

struct obus_object *obus_client_object_next(struct obus_client *client,
					    struct obus_object *prev,
					    uint16_t uid);

int obus_client_call(struct obus_client *client,
		     struct obus_object *object,
		     const struct obus_method_desc *desc,
		     const struct obus_struct *args,
		     obus_method_call_status_handler_cb_t cb,
		     uint16_t *handle);

struct obus_object *obus_server_object_next(struct obus_server *srv,
					    struct obus_object *prev,
					    uint16_t uid);

int obus_server_register_object(struct obus_server *srv,
				struct obus_object *obj);

int obus_server_unregister_object(struct obus_server *srv,
				  struct obus_object *obj);

int obus_server_send_event(struct obus_server *srv, struct obus_event *event);

int obus_server_send_bus_event(struct obus_server *srv,
			       struct obus_bus_event *event);

struct obus_object *obus_server_new_object(struct obus_server *srv,
					   const struct obus_object_desc *desc,
					   const obus_method_handler_cb_t *cbs,
					   const struct obus_struct *info);

void obus_object_log(const struct obus_object *obj, enum obus_log_level level);

int obus_object_destroy(struct obus_object *obj);

int obus_object_is_registered(const struct obus_object *obj);

struct obus_object *
obus_client_get_object(struct obus_client *client, obus_handle_t handle);

struct obus_object *
obus_server_get_object(struct obus_server *srv, obus_handle_t handle);

const struct obus_object_desc *
obus_object_get_desc(const struct obus_object *obj);

const void *obus_object_get_info(const struct obus_object *obj);

uint16_t obus_object_get_handle(const struct obus_object *object);

int obus_object_set_user_data(struct obus_object *object, void *user_data);

void *obus_object_get_user_data(const struct obus_object *object);

int obus_event_init(struct obus_event *event, struct obus_object *obj,
		    const struct obus_event_desc *desc,
		    const struct obus_struct *info);

struct obus_event *obus_event_new(struct obus_object *obj,
				  const struct obus_event_desc *desc,
				  const struct obus_struct *info);

int obus_event_destroy(struct obus_event *event);

const struct obus_object_desc *
obus_event_get_object_desc(const struct obus_event *event);

const struct obus_event_desc *
obus_event_get_desc(const struct obus_event *event);

int obus_event_is_empty(const struct obus_event *event);

int obus_event_commit(struct obus_event *event);

const void *obus_event_get_info(const struct obus_event *event);

void obus_event_log(const struct obus_event *event, enum obus_log_level level);


int obus_bus_event_register_object(struct obus_bus_event *bus_event,
				   struct obus_object *object);

int obus_bus_event_unregister_object(struct obus_bus_event *bus_event,
				     struct obus_object *object);

int obus_bus_event_add_event(struct obus_bus_event *bus_event,
			     struct obus_event *event);

struct obus_bus_event *
obus_bus_event_new(const struct obus_bus_event_desc *desc);

int obus_bus_event_destroy(struct obus_bus_event *event);

const struct obus_bus_event_desc *
obus_bus_event_get_desc(const struct obus_bus_event *event);

#endif /* _LIBOBUS_PRIVATE_H_ */
