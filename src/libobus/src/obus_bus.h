/******************************************************************************
 * libobus - linux interprocess objects synchronization protocol.
 *
 * @file obus_bus.h
 *
 * @brief obus bus
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

#ifndef _OBUS_BUS_H_
#define _OBUS_BUS_H_

/* obus bus */
struct obus_bus {
	/* bus api */
	struct obus_bus_api api;
	/* bus object calls hash */
	struct obus_hash calls_hash;
	/* bus object calls list */
	struct obus_node calls;
	/* bus object hash */
	struct obus_hash objects_hash;
	/* bus registered object list */
	struct obus_node objects;
	/* bus object provider hash */
	struct obus_hash providers_hash;
	/* bus object  providers list */
	struct obus_node providers;
};

/**
 * create bus from bus description
 * @param log log context
 * @param desc bus description
 * @return bus or NULL on error
 */
int obus_bus_init(struct obus_bus *bus, const struct obus_bus_desc *desc);

/**
 * destroy bus
 * @param bus bus
 * @return 0 on success
 */
int obus_bus_destroy(struct obus_bus *bus);

/**
 * clear bus by destroying all registered objects
 * @param bus bus
 * @return 0 on success
 */
int obus_bus_clear(struct obus_bus *bus);

/**
 * abort pending call
 * @param bus bus
 * @param obj object or NULL to abort all bus calls
 * @return 0 on success
 */
void obus_bus_abort_call(struct obus_bus *bus, struct obus_object *obj);

/**
 * register object provider
 * @param bus bus
 * @param prov object provider
 * @return 0 on succeed
 */
int obus_bus_register_provider(struct obus_bus *bus,
			       struct obus_provider *prov);

/**
 * register object provider
 * @param bus bus
 * @param prov object provider
 * @return 0 on succeed
 */
int obus_bus_unregister_provider(struct obus_bus *bus,
				 struct obus_provider *prov);

/**
 * get provider
 * @param bus bus
 * @param uid object uid
 * @return provider
 */
struct obus_provider *obus_bus_provider(struct obus_bus *bus, uint16_t uid);

/**
 * register object call in bus
 * @param bus bus
 * @param call object call
 * @return 0 on succeed
 */
int obus_bus_register_call(struct obus_bus *bus, struct obus_call *call);

/**
 * unregister object call from bus
 * @param bus bus
 * @param call object call
 * @return 0 on succeed
 */
int obus_bus_unregister_call(struct obus_bus *bus, struct obus_call *call);

/**
 * get call given its handle
 * @param bus bus
 * @param handle call handle
 * @return call
 */
struct obus_call *obus_bus_call(struct obus_bus *bus, obus_handle_t handle);

/**
 * add object in bus
 * @param bus bus
 * @param object object
 * @return 0 on succeed
 */
int obus_bus_add_object(struct obus_bus *bus, struct obus_object *object);

/**
 * remove object from bus
 * @param bus bus
 * @param object object
 * @return 0 on succeed
 */
int obus_bus_remove_object(struct obus_bus *bus, struct obus_object *object);

/**
 * register object in bus
 * @param bus bus
 * @param object object
 * @return 0 on succeed
 */
int obus_bus_register_object(struct obus_bus *bus, struct obus_object *object);

/**
 * unregister object from bus
 * @param bus bus
 * @param object object
 * @return 0 on succeed
 */
int obus_bus_unregister_object(struct obus_bus *bus,
			       struct obus_object *object);

/**
 * get registered object given its handle
 * @param bus
 * @param handle
 * @return object
 */
struct obus_object *obus_bus_object(struct obus_bus *bus, obus_handle_t handle);

/**
 * get registered first object given its uid
 * @param bus object bus
 * @param uid object uid or OBUS_INVALID_UID
 * @return first object or NULL if not existing
 */
struct obus_object *obus_bus_object_first(struct obus_bus *bus, uint16_t uid);

/**
 * get next registered first object given its uid
 * @param bus object bus
 * @param uid object uid or OBUS_INVALID_UID
 * @param prev previous object
 * @return next object or NULL if not existing
 */
struct obus_object *obus_bus_object_next(struct obus_bus *bus,
					 struct obus_object *prev,
					 uint16_t uid);

#endif /* _OBUS_BUS_H_ */
