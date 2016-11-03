/******************************************************************************
 * libobus-java - obus client java binding library.
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

package com.parrot.obus.internal;

import java.util.ArrayList;

import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Descriptor.BusEventDesc;


/**
 *
 */
public abstract class ObusBusEvent  {
	private static final Core.Logger _log = Core.getLogger("obus");

	public final BusEventDesc desc;
	public final int uid;
	public final ArrayList<ObusObject> registerList;
	public final ArrayList<ObusObject> unregisterList;
	public final ArrayList<ObusEvent> eventList;
	public boolean dispatched;

	/**
	 *
	 */
	protected ObusBusEvent(BusEventDesc busEvtDesc) {
		this.desc = busEvtDesc;
		this.uid = busEvtDesc.uid;
		this.registerList = new ArrayList<ObusObject>();
		this.unregisterList = new ArrayList<ObusObject>();
		this.eventList = new ArrayList<ObusEvent>();
		this.dispatched = false;
	}

	/**
	 *
	 */
	public Enum<?> getType() {
		return this.desc.busEvtType;
	}

	/**
	 *
	 */
	public static ObusBusEvent create(BusEventDesc busEvtDesc) {
		/* Create ObusBusEvent object */
		return busEvtDesc.creator.create(busEvtDesc);
	}

	public static ObusBusEvent create(Bus bus, int uid) {
		/* Create ObusBusEvent object */
		BusEventDesc busEvtDesc = bus.getDesc().eventsDesc.get(uid);
		if (busEvtDesc != null) {
			return busEvtDesc.creator.create(busEvtDesc);
		}
		return null;
	}

	/**
	 *
	 */
	public void encode(Buffer buf) {
		/* TODO */
		throw new RuntimeException();
	}

	/**
	 *
	 */
	public static ObusBusEvent decode(Bus bus, Buffer buf) throws DecodeError{
		/* Read event uid */
		int uid = buf.readU16();
		BusEventDesc busEvtDesc = bus.getDesc().eventsDesc.get(uid);
		if (busEvtDesc == null) {
			throw new DecodeError("ObusBusEvent.decode: " +
					"can't create bus event uid=" + uid +
					", descriptor not found");
		}

		/* Create ObusBusEvent object */
		ObusBusEvent busEvt = ObusBusEvent.create(busEvtDesc);

		/* Read numbers or registrations/unregistrations/events */
		int regCount = buf.readU32();
		int unregCount = buf.readU32();
		int evtCount = buf.readU32();

		/* Decode registrations */
		busEvt.registerList.ensureCapacity(regCount);
		for (int i = 0; i < regCount; i++) {
			ObusObject obj = ObusObject.decode(bus, buf);
			if (obj != null) {
				busEvt.registerList.add(obj);
			}
		}

		/* Decode unregistrations */
		busEvt.unregisterList.ensureCapacity(unregCount);
		for (int i = 0; i < unregCount; i++) {
			/* Read object uid and handle */
			int uidObj = buf.readU16();
			int handle = buf.readU16();

			/* Find object, Check that uid is correct */
			ObusObject obj = bus.findObject(handle);
			if (obj == null) {
				_log.error("ObusBusEvent.decode: " +
						"object uid=" + uidObj + " handle=" + handle +
						" not registered");
			} else if (obj.uid != uidObj) {
				_log.error("ObusBusEvent.decode: " +
						"object uid=" + uidObj + " handle=" + handle +
						" bad internal uid=" + obj.uid);
			} else {
				/* Add in list */
				busEvt.unregisterList.add(obj);
			}
		}

		/* Decode events */
		busEvt.eventList.ensureCapacity(evtCount);
		for (int i = 0; i < evtCount; i++) {
			ObusEvent evt = ObusEvent.decode(bus, buf);
			if (evt != null) {
				busEvt.eventList.add(evt);
			}
		}

		return busEvt;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "{ " + this.getType() + " (" + this.uid + ")" + " }";
	}

}
