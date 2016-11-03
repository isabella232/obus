/******************************************************************************
* @file net_bus.c
*
* @brief obus net bus client api
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#define OBUS_USE_PRIVATE
#include "libobus.h"
#include "libobus_private.h"
#include "net_bus.h"

static const struct obus_bus_event_desc net_bus_events[] = {
	{
	 .uid = 1,
	 .name = "connected",
	 }
	,
	{
	 .uid = 2,
	 .name = "disconnected",
	 }
	,
	{
	 .uid = 3,
	 .name = "connection_refused",
	 }
	,
	{
	 .uid = 10,
	 .name = "scan_completed",
	 }
};

/* referenced objects supported by net bus */
extern const struct obus_object_desc net_interface_desc;

/* array of net objects descriptors */
static const struct obus_object_desc *const objects[] = {
	&net_interface_desc,
};

/* net bus description */
static const struct obus_bus_desc net_desc = {
	.name = "net",
	.n_objects = OBUS_SIZEOF_ARRAY(objects),
	.objects = objects,
	.n_events = OBUS_SIZEOF_ARRAY(net_bus_events),
	.events = net_bus_events,
	.crc = 0,
};

/*public reference to net  */
const struct obus_bus_desc *net_bus_desc = &net_desc;

const char *net_bus_event_type_str(enum net_bus_event_type type)
{
	if (type >= OBUS_SIZEOF_ARRAY(net_bus_events))
		return "???";

	return net_bus_events[type].name;
}

enum net_bus_event_type
net_bus_event_get_type(const struct net_bus_event *event)
{
	long idx;
	const struct obus_bus_event *evt;
	const struct obus_bus_event_desc *desc;

	evt = (const struct obus_bus_event *)event;
	desc = obus_bus_event_get_desc(evt);
	idx = desc - net_bus_events;

	if (idx < 0 || idx > NET_BUS_EVENT_COUNT)
		return NET_BUS_EVENT_COUNT;

	return (enum net_bus_event_type)idx;
}


struct net_bus_event *
net_bus_event_from_obus_event(struct obus_bus_event *event)
{
	long idx;
	const struct obus_bus_event_desc *desc;

	desc = obus_bus_event_get_desc(event);
	idx = desc - net_bus_events;

	if (idx < 0 || idx > NET_BUS_EVENT_COUNT)
		return NULL;

	return (struct net_bus_event *)event;
}
