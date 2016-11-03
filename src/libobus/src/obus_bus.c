/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_client.c
 *
 * @brief object client
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

#define OBUS_BUS_HASH_SIZE 4093

int obus_bus_init(struct obus_bus *bus, const struct obus_bus_desc *desc)
{
	int ret;
	if (!bus || !desc)
		return -EINVAL;

	memset(bus, 0, sizeof(*bus));

	/* init bus api */
	ret = obus_bus_api_init(&bus->api, desc);
	if (ret < 0)
		return ret;

	/* init bus objects list */
	obus_list_init(&bus->objects);

	/* init objects hash */
	ret = obus_hash_init(&bus->objects_hash, OBUS_BUS_HASH_SIZE);
	if (ret < 0)
		goto destroy_api;

	/* init bus call list */
	obus_list_init(&bus->calls);

	/* init bus call hash */
	ret = obus_hash_init(&bus->calls_hash, OBUS_BUS_HASH_SIZE);
	if (ret < 0)
		goto destroy_objects_hash;

	/* init bus provider list */
	obus_list_init(&bus->providers);

	/* init bus provider hash */
	ret = obus_hash_init(&bus->providers_hash, bus->api.desc->n_objects);
	if (ret < 0)
		goto destroy_calls_hash;

	return 0;

destroy_calls_hash:
	obus_hash_destroy(&bus->providers_hash);
destroy_objects_hash:
	obus_hash_destroy(&bus->objects_hash);
destroy_api:
	obus_bus_api_destroy(&bus->api);
	return ret;
}

int obus_bus_destroy(struct obus_bus *bus)
{
	if (!bus)
		return -EINVAL;

	obus_bus_clear(bus);
	obus_bus_api_destroy(&bus->api);
	obus_hash_destroy(&bus->objects_hash);
	obus_hash_destroy(&bus->calls_hash);
	obus_hash_destroy(&bus->providers_hash);
	memset(bus, 0, sizeof(*bus));
	return 0;
}

/**
 * abort bus pending call
 * @param bus bus
 * @return 0 on success
 */
void obus_bus_abort_call(struct obus_bus *bus, struct obus_object *obj)
{
	struct obus_call *call, *ctmp;
	/* destroy call */
	obus_list_walk_entry_forward_safe(&bus->calls, call, ctmp, node) {
		if ((obj == NULL) || (obj && call->obj == obj)) {
			/* unregister call */
			obus_bus_unregister_call(bus, call);
			/* abort call */
			obus_call_ack_notify(call, OBUS_CALL_ABORTED);
			/* destroy call */
			obus_call_destroy(call);
		}
	}
}

int obus_bus_clear(struct obus_bus *bus)
{
	struct obus_hash_entry *entry, *tmp;
	struct obus_object *obj;
	struct obus_call *call, *ctmp;

	if (!bus)
		return -EINVAL;

	/* destroy call */
	obus_list_walk_entry_forward_safe(&bus->calls, call, ctmp, node) {
		/* unregister call */
		obus_bus_unregister_call(bus, call);
		/* abort call */
		obus_call_ack_notify(call, OBUS_CALL_ABORTED);
		/* destroy call */
		obus_call_destroy(call);
	}

	/* destroy objects */
	obus_list_walk_entry_forward_safe(&bus->objects_hash.entries, entry,
					  tmp, node) {
		obj = (struct obus_object *)entry->data;
		/* unregister object */
		obus_bus_unregister_object(bus, obj);
		/* destroy object */
		obus_object_destroy(obj);
	}

	return 0;
}

int obus_bus_register_call(struct obus_bus *bus, struct obus_call *call)
{
	int ret;
	int has_handle = 0;

	if (!bus || !call || !call->obj)
		return -EINVAL;

	/* object with valid handle come's from server sync or add */
	has_handle = (call->handle == OBUS_INVALID_HANDLE) ? 0 : 1;

	/* generate handle not already used ! */
	do {
		if (!has_handle) {
			call->handle = obus_rand_handle();
			if (call->handle == OBUS_INVALID_HANDLE)
				return -ENODEV;
		}

		/* check if handle already used */
		ret = obus_hash_lookup(&bus->calls_hash, call->handle, NULL);
	} while (!has_handle && ret == 0);

	/* add object in hash */
	ret = obus_hash_insert(&bus->calls_hash, call->handle, call);
	if (ret < 0) {
		obus_error("call '%s' handle=%d insert error=%d(%s)",
			   call->desc->name, call->handle, -ret,
			   strerror(-ret));

		if (!has_handle)
			call->handle = OBUS_INVALID_HANDLE;

		return ret;
	}

	/* add call in list */
	obus_list_add_before(&bus->calls, &call->node);
	return 0;
}

int obus_bus_unregister_call(struct obus_bus *bus, struct obus_call *call)
{
	int ret;
	if (!bus || !call)
		return -EINVAL;

	/* remove call from hash */
	ret = obus_hash_remove(&bus->calls_hash, call->handle);
	if (ret < 0) {
		obus_error("can't remove call object %s, error=%d(%s)",
			   call->obj->desc->name, -ret, strerror(-ret));
		return ret;
	}

	/* remove calls from list */
	obus_list_del(&call->node);
	return 0;
}

struct obus_call *
obus_bus_call(struct obus_bus *bus, obus_handle_t handle)
{
	struct obus_call *call = NULL;
	int ret;

	if (!bus || handle == OBUS_INVALID_HANDLE)
		return NULL;

	ret = obus_hash_lookup(&bus->calls_hash, handle, (void **)&call);
	return (ret == 0) ? call : NULL;
}

int obus_bus_register_provider(struct obus_bus *bus, struct obus_provider *prov)
{
	int ret;

	/* add provider in hash */
	ret = obus_hash_insert(&bus->providers_hash, prov->desc->uid, prov);
	if (ret < 0) {
		obus_error("can't insert object '%s' provider error=%d(%s)",
			   prov->desc->name, -ret, strerror(-ret));
		return ret;
	}

	obus_list_add_before(&bus->providers, &prov->node);
	return 0;
}

int obus_bus_unregister_provider(struct obus_bus *bus,
				 struct obus_provider *prov)
{
	int ret;

	/* remove object from hash */
	ret = obus_hash_remove(&bus->providers_hash, prov->desc->uid);
	if (ret < 0) {
		obus_error("can't remove object '%s' provider error=%d(%s)",
			   prov->desc->name, -ret, strerror(-ret));
		return ret;
	}

	obus_list_del(&prov->node);
	return 0;
}

struct obus_provider *obus_bus_provider(struct obus_bus *bus, uint16_t uid)
{
	struct obus_provider *prov = NULL;
	int ret;

	if (!bus || uid == OBUS_INVALID_UID)
		return NULL;

	ret = obus_hash_lookup(&bus->providers_hash, uid, (void **)&prov);
	return (ret == 0) ? prov : NULL;
}

int obus_bus_add_object(struct obus_bus *bus, struct obus_object *obj)
{
	int ret;
	int has_handle = 0;

	if (!bus || !obj || !obj->desc || obj->desc->uid == OBUS_INVALID_UID)
		return -EINVAL;

	/* object with valid handle come's from server sync or add */
	has_handle = (obj->handle == OBUS_INVALID_HANDLE) ? 0 : 1;

	/* generate handle not already used if needed ! */
	do {
		if (!has_handle) {
			obj->handle = obus_rand_handle();
			if (obj->handle == OBUS_INVALID_HANDLE)
				return -ENODEV;
		}

		/* check if handle already used */
		ret = obus_hash_lookup(&bus->objects_hash, obj->handle, NULL);
	} while (!has_handle && ret == 0);

	/* add object in hash */
	ret = obus_hash_insert(&bus->objects_hash, obj->handle, obj);
	if (ret < 0) {
		obus_error("object '%s' handle=%d insert error=%d(%s)",
			   obj->desc->name, obj->handle, -ret, strerror(-ret));

		if (!has_handle)
			obj->handle = OBUS_INVALID_HANDLE;

		return ret;
	}

	obj->bus = bus;
	return 0;
}

int obus_bus_remove_object(struct obus_bus *bus, struct obus_object *obj)
{
	int ret;

	if (!bus || !obj || !obj->desc || obj->desc->uid == OBUS_INVALID_UID)
		return -EINVAL;

	/* remove object from hash */
	ret = obus_hash_remove(&bus->objects_hash, obj->handle);
	if (ret < 0) {
		obus_error("object uid=%d handle=%d remove error=%d(%s)",
			   obj->desc->uid, obj->handle, -ret, strerror(-ret));
		return ret;
	}

	obj->bus = NULL;
	return 0;
}

int obus_bus_register_object(struct obus_bus *bus, struct obus_object *obj)
{
	obj->is_registered = 1;
	obus_list_add_before(&bus->objects, &obj->node);
	return 0;
}

int obus_bus_unregister_object(struct obus_bus *bus, struct obus_object *obj)
{
	obj->is_registered = 0;
	obus_list_del(&obj->node);
	return 0;
}

struct obus_object *obus_bus_object(struct obus_bus *bus, obus_handle_t handle)
{
	struct obus_object *obj = NULL;
	int ret;

	if (!bus || handle == OBUS_INVALID_HANDLE)
		return NULL;

	ret = obus_hash_lookup(&bus->objects_hash, handle, (void **)&obj);
	return (ret == 0) ? obj : NULL;
}

struct obus_object *obus_bus_object_first(struct obus_bus *bus, uint16_t uid)
{
	struct obus_object *obj;

	obus_list_walk_entry_forward(&bus->objects, obj, node) {
		if ((uid == OBUS_INVALID_UID) || (uid == obj->desc->uid))
			return obj;
	}
	return NULL;
}

struct obus_object *obus_bus_object_next(struct obus_bus *bus,
					 struct obus_object *prev,
					 uint16_t uid)
{
	struct obus_object *next = prev;
	struct obus_node *node;

	do {
		/* get next node in the list */
		node = obus_list_first(next ? &next->node : &bus->objects);
		if (node == &bus->objects)
			return NULL;

		next = obus_list_entry(node, struct obus_object, node);
		if (!next)
			return NULL;

	} while (uid != OBUS_INVALID_UID && next->desc->uid != uid);

	return next;
}
