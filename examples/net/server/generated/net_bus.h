/******************************************************************************
* @file net_bus.h
*
* @brief obus net bus server api
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
#ifndef _NET_BUS_H_
#define _NET_BUS_H_

#include "libobus.h"

OBUS_BEGIN_DECLS

/**
 * @brief net bus descriptor.
 *
 * Reference to net bus descripor.
 **/
extern const struct obus_bus_desc *net_bus_desc;
/**
 * @brief net bus event structure
 *
 * This opaque structure represent an net bus event.
 **/
struct net_bus_event;

/**
 * @brief net bus event type enumeration.
 *
 * This enumeration describes all kind of net bus events.
 **/
enum net_bus_event_type {
	/* net bus connected */
	NET_BUS_EVENT_CONNECTED = 0,
	/* net bus disconnected */
	NET_BUS_EVENT_DISCONNECTED,
	/* net bus connection refused */
	NET_BUS_EVENT_CONNECTION_REFUSED,
	/* network system scan completed */
	NET_BUS_EVENT_SCAN_COMPLETED,
	/* for internal use only */
	NET_BUS_EVENT_COUNT,
};

/**
 * @brief get net_bus_event_type string value.
 *
 * @param[in]  value bus event type to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *net_bus_event_type_str(enum net_bus_event_type type);

/**
 * @brief create net bus event.
 *
 * This function is used to create a new net event.
 *
 * @param[in]  type   net bus event type.
 *
 * @retval     net bus event or NULL on error.
 **/
struct net_bus_event *
net_bus_event_new(enum net_bus_event_type type);

/**
 * @brief destroy net bus event.
 *
 * This function is used to destroy a net event.
 *
 * @param[in]  event  net bus event.
 *
 * @retval      0     success.
 **/
int net_bus_event_destroy(struct net_bus_event *event);

/**
 * @brief get net bus event type.
 *
 * This function is used to get the type of a net bus event.
 *
 * @param[in]  event  net bus event.
 *
 * @retval  one of @net_bus_event_type value.
 **/
enum net_bus_event_type
net_bus_event_get_type(const struct net_bus_event *event);

/**
 * @brief get net bus event from generic obus bus event.
 *
 * This function is used to get a net bus event from an obus bus event.
 *
 * @param[in]  event       generic obus bus event.
 *
 * @retval  net bus event or NULL if not a net bus event.
 **/

struct net_bus_event *
net_bus_event_from_obus_event(struct obus_bus_event *event);

/**
 * @brief send a net bus event.
 *
 * This function send a net bus event.
 * Associated object event are sent and object contents are updated.
 * Associated objects to be registered are registered.
 * Associated objects to be unregistered are unregistered and destroyed.
 *
 * @param[in]  server  net bus server.
 * @param[in]  event   net bus event.
 *
 * @retval     0      bus event is sent
 * @retval  -EINVAL   invalid parameters.
 * @retval     0      one of @net_bus_event_type value.
 **/
int net_bus_event_send(struct obus_server *server, struct net_bus_event *event);

OBUS_END_DECLS

#endif /*_NET_BUS_H_*/
