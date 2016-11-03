/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_bus_api.c
 *
 * @brief object bus api
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

#define MTD_KEY(objuid, uid) \
	(uint32_t)((objuid) << 16 | (uid))

#define EVT_KEY(objuid, uid) \
	(uint32_t)((objuid) << 16 | (uid))

static int obus_bus_api_add_object(struct obus_bus_api *api,
				   const struct obus_object_desc *obj)
{
	int ret;
	uint16_t i;
	const struct obus_event_desc *event;
	const struct obus_method_desc *mtd;

	if (!api || !obj)
		return -EINVAL;

	/* add object in hash */
	ret = obus_hash_insert_const(&api->hashes[HASH_OBJ_DESC],
				     obj->uid, obj);
	if (ret < 0) {
		obus_error("can't insert OD[%03d]:'%s', error=%d(%s)",
			   obj->uid, obj->name, -ret, strerror(-ret));
		return ret;
	}

	/* add object calls */
	for (i = 0; i < obj->n_methods; i++) {
		mtd = &obj->methods[i];
		ret = obus_hash_insert_const(&api->hashes[HASH_MTD_DESC],
					     MTD_KEY(obj->uid, mtd->uid),
					     mtd);
		if (ret < 0) {
			obus_error("can't insert MD[%03d:%03d]:'%s:%s', "
				   "error=%d(%s)", obj->uid, mtd->uid,
				   obj->name, mtd->name, -ret, strerror(-ret));
			return ret;
		}
	}

	/* add object events */
	for (i = 0; i < obj->n_events; i++) {
		event = &obj->events[i];
		ret = obus_hash_insert_const(&api->hashes[HASH_EVT_DESC],
					     EVT_KEY(obj->uid, event->uid),
					     event);
		if (ret < 0) {
			obus_error("can't insert ED[%03d:%03d]:'%s:%s', "
				   "error=%d(%s)", obj->uid, event->uid,
				   obj->name, event->name, -ret,
				   strerror(-ret));
			return ret;
		}
	}

	return 0;
}

int obus_bus_api_init(struct obus_bus_api *api,
		      const struct obus_bus_desc *desc)
{
	int ret;
	uint16_t i;
	const struct obus_bus_event_desc *event;
	size_t n_methods, n_events;

	if (!api || !desc)
		return -EINVAL;

	memset(api, 0, sizeof(*api));
	api->desc = desc;

	/* init bus api object description hash */
	ret = obus_hash_init(&api->hashes[HASH_OBJ_DESC], desc->n_objects);
	if (ret < 0)
		goto error;

	/* init bus api methods description hash */
	n_methods = 0;
	for (i = 0; i < desc->n_objects; i++)
		n_methods += desc->objects[i]->n_methods;

	ret = obus_hash_init(&api->hashes[HASH_MTD_DESC], n_methods);
	if (ret < 0)
		goto destroy_obj_hash;

	/* init bus api events description hash */
	n_events = 0;
	for (i = 0; i < desc->n_objects; i++)
		n_events += desc->objects[i]->n_events;

	ret = obus_hash_init(&api->hashes[HASH_EVT_DESC], n_events);
	if (ret < 0)
		goto destroy_call_hash;

	/* init bus api events description hash */
	ret = obus_hash_init(&api->hashes[HASH_BUS_EVT_DESC], desc->n_events);
	if (ret < 0)
		goto destroy_event_hash;

	/* add objects description in hashes */
	for (i = 0; i < desc->n_objects; i++) {
		ret = obus_bus_api_add_object(api, desc->objects[i]);
		if (ret < 0)
			goto destroy_bus_event_hash;
	}

	/* add events description in hashes */
	for (i = 0; i < desc->n_events; i++) {
		event =  &desc->events[i];
		/* add event in hash */
		ret = obus_hash_insert_const(&api->hashes[HASH_BUS_EVT_DESC],
					     event->uid, event);
		if (ret < 0) {
			obus_error("can't insert BED[%03d]:'%s', error=%d(%s)",
				   event->uid, event->name, -ret,
				   strerror(-ret));
			goto destroy_bus_event_hash;
		}
	}

	return 0;

destroy_bus_event_hash:
	obus_hash_destroy(&api->hashes[HASH_BUS_EVT_DESC]);
destroy_event_hash:
	obus_hash_destroy(&api->hashes[HASH_EVT_DESC]);
destroy_call_hash:
	obus_hash_destroy(&api->hashes[HASH_MTD_DESC]);
destroy_obj_hash:
	obus_hash_destroy(&api->hashes[HASH_OBJ_DESC]);
error:
	return ret;
}

int obus_bus_api_destroy(struct obus_bus_api *api)
{
	if (!api)
		return -EINVAL;

	obus_hash_destroy(&api->hashes[HASH_EVT_DESC]);
	obus_hash_destroy(&api->hashes[HASH_MTD_DESC]);
	obus_hash_destroy(&api->hashes[HASH_OBJ_DESC]);
	obus_hash_destroy(&api->hashes[HASH_BUS_EVT_DESC]);
	memset(api, 0 , sizeof(*api));
	return 0;
}

const struct obus_object_desc *
obus_bus_api_object(struct obus_bus_api *api, uint16_t uid)
{
	const struct obus_object_desc *obj;
	int ret;

	if (!api || uid == OBUS_INVALID_UID)
		return NULL;

	ret = obus_hash_lookup_const(&api->hashes[HASH_OBJ_DESC], uid,
				     (const void **)&obj);
	return (ret == 0) ? obj : NULL;
}

const struct obus_method_desc *
obus_bus_api_method(struct obus_bus_api *api, uint16_t objuid, uint16_t mtduid)
{
	const struct obus_method_desc *call;
	int ret;

	if (!api || objuid == OBUS_INVALID_UID ||  mtduid == OBUS_INVALID_UID)
		return NULL;

	ret = obus_hash_lookup_const(&api->hashes[HASH_MTD_DESC],
				     MTD_KEY(objuid, mtduid),
				     (const void **)&call);
	return (ret == 0) ? call : NULL;
}

const struct obus_event_desc *
obus_bus_api_event(struct obus_bus_api *api, uint16_t objuid, uint16_t evtuid)
{
	const struct obus_event_desc *evt;
	int ret;

	if (!api || objuid == OBUS_INVALID_UID ||  evtuid == OBUS_INVALID_UID)
		return NULL;

	ret = obus_hash_lookup_const(&api->hashes[HASH_EVT_DESC],
				     EVT_KEY(objuid, evtuid),
				     (const void **)&evt);
	return (ret == 0) ? evt : NULL;
}

const struct obus_bus_event_desc *
obus_bus_api_bus_event(struct obus_bus_api *api, uint16_t uid)
{
	const struct obus_bus_event_desc *evt;
	int ret;

	if (!api || uid == OBUS_INVALID_UID)
		return NULL;

	ret = obus_hash_lookup_const(&api->hashes[HASH_BUS_EVT_DESC],
				     (uint32_t)uid,
				     (const void **)&evt);
	return (ret == 0) ? evt : NULL;
}

static int obus_bus_api_log_object(const struct obus_object_desc *obj)
{
	uint16_t i;
	const struct obus_method_desc *mtd;
	const struct obus_event_desc *evt;

	obus_info("OD[%03d]:'%s'", obj->uid, obj->name);

	for (i = 0; i < obj->n_methods; i++) {
		mtd = &obj->methods[i];
		obus_info("|--MD[%03d]:'%s'", mtd->uid, mtd->name);
	}

	for (i = 0; i < obj->n_events; i++) {
		evt = &obj->events[i];
		obus_info("|--ED[%03d]:'%s'", evt->uid, evt->name);
	}

	return 0;
}

/**
 * log bus api
 * @param api bus to be log
 * @return 0 on success
 */
int obus_bus_api_log(struct obus_bus_api *api)
{
	uint16_t i;

	obus_info("'%s' bus description:", api->desc->name);

	for (i = 0; i < api->desc->n_objects; i++)
		obus_bus_api_log_object(api->desc->objects[i]);

	for (i = 0; i < api->desc->n_events; i++)
		obus_info("BED[%03d]:'%s'", api->desc->events[i].uid,
			  api->desc->events[i].name);

	return 0;
}

