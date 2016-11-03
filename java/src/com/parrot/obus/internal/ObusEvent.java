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

import com.parrot.obus.IBusObject;
import com.parrot.obus.IBusObjectEvent;
import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Descriptor.EventDesc;
import com.parrot.obus.internal.Descriptor.FieldDesc;

/**
 * Obus object event.
 */
public abstract class ObusEvent implements IBusObjectEvent {
	private static final Core.Logger _log = Core.getLogger("obus");

	/**
	 *
	 */
	public final EventDesc desc;
	public final int uid;
	public final ObusObject obj;
	protected final ObusStruct struct;
	private boolean is_committed;

	/**
	 *
	 */
	protected ObusEvent(EventDesc evtDesc, ObusObject obj, ObusStruct struct) {
		this.desc = evtDesc;
		this.uid = evtDesc.uid;
		this.obj = obj;
		this.struct = struct;
		this.is_committed = false;
	}
	
	
	/**
	 *
	 */
	public static ObusEvent create(EventDesc evtDesc, ObusObject obj) {
		/* Create ObusStruct then ObusEvent object */
		ObusStruct struct = ObusStruct.create(evtDesc.structDesc);
		return evtDesc.creator.create(evtDesc, obj, struct);
	}

	/**
	 *
	 */
	@Override
	public Enum<?> getType() {
		return this.desc.evtType;
	}

	
	@Override
	public IBusObject getObject() {
		return this.obj;
	}
	
	/**
	 *
	 */
	public void commit() {
		if (!this.is_committed) {
			/* Merge data of event with object */
			this.obj.struct.merge(this.struct);
		}
		this.is_committed = true;
	}

	/**
	 *
	 */
	public void encode(Buffer buf) {
		/* Write uid and handle of object */
		buf.writeU16(this.obj.uid);
		buf.writeU16(this.obj.handle);
		/* Write uid of event */
		buf.writeU16(this.uid);

		/* Write data structure */
		int marker = buf.prepareSizeMarker();
		this.struct.encode(buf);
		buf.writeSizeMarker(marker);
	}

	/**
	 *
	 */
	public static ObusEvent decode(Bus bus, Buffer buf) {
		/* Read uid and handle of object */
		int uidObj = buf.readU16();
		int handle = buf.readU16();
		/* Read event uid */
		int uidEvt = buf.readU16();

		/* Get current position in buffer and size that we are supposed to read*/
		int sizeData = buf.readU32();
		int posData = buf.getPos();
		try {
			/* Find object */
			ObusObject obj = bus.findObject(handle);
			if (obj == null) {
				throw new DecodeError("ObusEvent.decode: " +
						"object uid=" + uidObj + " handle=" + handle +
						" not registered");
			}
			/* Check that uid is correct */
			if (obj.uid != uidObj) {
				throw new DecodeError("ObusEvent.decode: " +
						"object uid=" + uidObj + " handle=" + handle +
						" bad internal uid=" + obj.uid);
			}

			/* Get event descriptor */
			EventDesc evtDesc = obj.desc.eventsDesc.get(uidEvt);
			if (evtDesc == null) {
				throw new DecodeError("ObusEvent.decode: " +
						"object uid=" + obj.uid +
						" can't create event uid=" + uidEvt +
						", descriptor not found");
			}

			/* Decode data structure */
			ObusStruct struct = ObusStruct.decode(evtDesc.structDesc, buf);

			/* Create ObusEvent object with decoded struct
			   (so we can't use ObusEvent.create method) */
			ObusEvent evt = evtDesc.creator.create(evtDesc, obj, struct);

			/* Check event fields decoded according to event updates desc */
			for (FieldDesc fieldDesc : evt.struct.getDesc().getFieldsDesc().values()) {
				if (evt.struct.hasField(fieldDesc)) {
					if (!evt.desc.hasUpdateField(fieldDesc)) {
						_log.warning("object '" + evt.obj.desc.name +  "' (handle=" + evt.obj.getHandle() + ")" +
								" event '" + evt.desc.name + " updates undeclared " +
								fieldDesc.getRoleStr() + " '" + fieldDesc.name + "'");
					}
				}
			}

			return evt;
		} catch (DecodeError e) {
			/* Log error and try to continue */
			_log.error(e.toString());
			buf.setPos(posData + sizeData);
			return null;
		}
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "{" + this.getType() + " ("+ this.uid + "), obj={" + this.obj.desc.name + 
					", handle=" + this.obj.handle + "}, struct=" + this.struct.toString() + "}";
	}
}
