/**
 * @file ps_bus.h
 *
 * @brief obus ps bus client api
 *
 * @author obusgen 1.0.3 generated file, do not modify it.
 */
#ifndef _PS_BUS_H_
#define _PS_BUS_H_

#include "libobus.h"

OBUS_BEGIN_DECLS

/**
 * @brief ps bus descriptor.
 *
 * Reference to ps bus descripor.
 **/
extern const struct obus_bus_desc *ps_bus_desc;
/**
 * @brief ps bus event structure
 *
 * This opaque structure represent an ps bus event.
 **/
struct ps_bus_event;

/**
 * @brief ps bus event type enumeration.
 *
 * This enumeration describes all kind of ps bus events.
 **/
enum ps_bus_event_type {
	/** ps bus connected */
	PS_BUS_EVENT_CONNECTED = 0,
	/** ps bus disconnected */
	PS_BUS_EVENT_DISCONNECTED,
	/** ps bus connection refused */
	PS_BUS_EVENT_CONNECTION_REFUSED,
	/** System processes global update */
	PS_BUS_EVENT_UPDATED,
	/** for internal use only*/
	PS_BUS_EVENT_COUNT,
};

/**
 * @brief get ps_bus_event_type string value.
 *
 * @param[in] type bus event type to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *ps_bus_event_type_str(enum ps_bus_event_type type);

/**
 * @brief get ps bus event type.
 *
 * This function is used to get the type of a ps bus event.
 *
 * @param[in]  event  ps bus event.
 *
 * @retval  one of @ref ps_bus_event_type value.
 **/
enum ps_bus_event_type
ps_bus_event_get_type(const struct ps_bus_event *event);

/**
 * @brief get ps bus event from generic obus bus event.
 *
 * This function is used to get a ps bus event from an obus bus event.
 *
 * @param[in]  event       generic obus bus event.
 *
 * @retval  ps bus event or NULL if not a ps bus event.
 **/

struct ps_bus_event *
ps_bus_event_from_obus_event(struct obus_bus_event *event);

OBUS_END_DECLS

#endif /*_PS_BUS_H_*/
