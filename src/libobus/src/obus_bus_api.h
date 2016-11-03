/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_bus_api.h
 *
 * @brief obus bus api
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

#ifndef _OBUS_BUS_API_H_
#define _OBUS_BUS_API_H_

enum {
	HASH_OBJ_DESC = 0,
	HASH_MTD_DESC,
	HASH_EVT_DESC,
	HASH_BUS_EVT_DESC,
	HASH_DESC_COUNT
};

/* obus bus api */
struct obus_bus_api {
	const struct obus_bus_desc *desc;
	struct obus_hash hashes[HASH_DESC_COUNT];
};

/**
 * create bus api from bus description
 * @param api bus api to be initialized
 * @param info bus info
 * @return 0 on success
 */
int obus_bus_api_init(struct obus_bus_api *api,
		      const struct obus_bus_desc *desc);

/**
 * destroy bus api
 * @param api bus api to be destroy
 * @return 0 on success
 */
int obus_bus_api_destroy(struct obus_bus_api *api);

/**
 * log bus api
 * @param api bus api
 * @return 0
 */
int obus_bus_api_log(struct obus_bus_api *api);

/**
 * get object description given its uid
 * @param api bus api
 * @param uid object uid
 * @return object description or NULL if not found
 */
const struct obus_object_desc *
obus_bus_api_object(struct obus_bus_api *api, uint16_t uid);

/**
 * get object method description given object uid and method uid
 * @param api bus api
 * @param objuid object uid
 * @param mtduid property uid
 * @return method description or NULL if not found
 */
const struct obus_method_desc *
obus_bus_api_method(struct obus_bus_api *api, uint16_t objuid, uint16_t mtduid);

/**
 * get object event description given event uid
 * @param api bus api
 * @param objuid object uid
 * @param evtuid object event uid
 * @return object event description or NULL if not found
 */
const struct obus_event_desc *
obus_bus_api_event(struct obus_bus_api *api, uint16_t objuid, uint16_t evtuid);

/**
 * get bus event description given event uid
 * @param api bus api
 * @param uid event uid
 * @return event description or NULL if not found
 */
const struct obus_bus_event_desc *
obus_bus_api_bus_event(struct obus_bus_api *api, uint16_t uid);


#endif /* _OBUS_BUS_API_H_ */
