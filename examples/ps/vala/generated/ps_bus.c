/******************************************************************************
* @file ps_bus.c
*
* @brief obus ps bus server api
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
#include "ps_bus.h"

static const struct obus_bus_event_desc ps_bus_events[] = {
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
	 .name = "updated",
	 }
};

/* referenced objects supported by ps bus */
extern const struct obus_object_desc ps_process_desc;
extern const struct obus_object_desc ps_summary_desc;

/* array of ps objects descriptors */
static const struct obus_object_desc *const objects[] = {
	&ps_process_desc,
	&ps_summary_desc,
};

/* ps bus description */
static const struct obus_bus_desc ps_desc = {
	.name = "ps",
	.n_objects = OBUS_SIZEOF_ARRAY(objects),
	.objects = objects,
	.n_events = OBUS_SIZEOF_ARRAY(ps_bus_events),
	.events = ps_bus_events,
	.crc = 0,
};

/*public reference to ps  */
const struct obus_bus_desc *ps_bus_desc = &ps_desc;

const char *ps_bus_event_type_str(enum ps_bus_event_type type)
{
	if (type >= OBUS_SIZEOF_ARRAY(ps_bus_events))
		return "???";

	return ps_bus_events[type].name;
}

struct ps_bus_event *
ps_bus_event_new(enum ps_bus_event_type type)
{
	const struct obus_bus_event_desc *desc;

	if (type >= PS_BUS_EVENT_COUNT)
		return NULL;

	desc = &ps_bus_events[type];
	return (struct ps_bus_event *)obus_bus_event_new(desc);
}

int ps_bus_event_destroy(struct ps_bus_event *event)
{
	return obus_bus_event_destroy((struct obus_bus_event *)event);
}

enum ps_bus_event_type
ps_bus_event_get_type(const struct ps_bus_event *event)
{
	long idx;
	const struct obus_bus_event *evt;
	const struct obus_bus_event_desc *desc;

	evt = (const struct obus_bus_event *)event;
	desc = obus_bus_event_get_desc(evt);
	idx = desc - ps_bus_events;

	if (idx < 0 || idx > PS_BUS_EVENT_COUNT)
		return PS_BUS_EVENT_COUNT;

	return (enum ps_bus_event_type)idx;
}


struct ps_bus_event *
ps_bus_event_from_obus_event(struct obus_bus_event *event)
{
	long idx;
	const struct obus_bus_event_desc *desc;

	desc = obus_bus_event_get_desc(event);
	idx = desc - ps_bus_events;

	if (idx < 0 || idx > PS_BUS_EVENT_COUNT)
		return NULL;

	return (struct ps_bus_event *)event;
}

int ps_bus_event_send(struct obus_server *server, struct ps_bus_event *event)
{
	struct obus_bus_event *evt = (struct obus_bus_event *)event;
	return obus_server_send_bus_event(server, evt);
}
