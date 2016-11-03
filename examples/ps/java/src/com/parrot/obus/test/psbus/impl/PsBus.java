/******************************************************************************
* @file PsBus.java
*
* @brief PsBus
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus.impl;

import com.parrot.obus.internal.Core.ObusBusEventCreator;
import com.parrot.obus.internal.Descriptor.BusDesc;
import com.parrot.obus.internal.Descriptor.BusEventDesc;
import com.parrot.obus.internal.ObusBusEvent;
import com.parrot.obus.test.psbus.IPsBusEvent;

/** Bus */
@SuppressWarnings("javadoc")
public class PsBus {
	/** Bus event */
	public static final class Event extends ObusBusEvent implements IPsBusEvent {
		/** */
		private Event(BusEventDesc busEvtDesc) {
			super(busEvtDesc);
		}

		/** */
		@Override
		public Type getType() {
			return (Type)this.desc.busEvtType;
		}

		/** Event creator */
		private static final ObusBusEventCreator<Event> creator = new ObusBusEventCreator<Event>() {
			@Override
			public Event create(BusEventDesc busEvtDesc) {
				return new Event(busEvtDesc);
			}
		};

		/** Event descriptors */
		private static final BusEventDesc busEventDescConnected = new BusEventDesc(
				"connected",
				1,
				Type.CONNECTED,
				Event.creator);
		private static final BusEventDesc busEventDescDisconnected = new BusEventDesc(
				"disconnected",
				2,
				Type.DISCONNECTED,
				Event.creator);
		private static final BusEventDesc busEventDescConnectionRefused = new BusEventDesc(
				"connection_refused",
				3,
				Type.CONNECTION_REFUSED,
				Event.creator);
		private static final BusEventDesc busEventDescUpdated = new BusEventDesc(
				"updated",
				10,
				Type.UPDATED,
				Event.creator);
	}

	/** Bus descriptor */
	public static final BusDesc busDesc = new BusDesc("ps", 0);
	static {
		PsBus.busDesc.addObject(PsProcess.objectDesc);
		PsBus.busDesc.addObject(PsSummary.objectDesc);
		PsBus.busDesc.addEvent(Event.busEventDescConnected);
		PsBus.busDesc.addEvent(Event.busEventDescDisconnected);
		PsBus.busDesc.addEvent(Event.busEventDescConnectionRefused);
		PsBus.busDesc.addEvent(Event.busEventDescUpdated);
	}
}
