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
import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Descriptor.ObjectDesc;

/**
 * Obus object.
 */
public abstract class ObusObject implements IBusObject {
	private static final Core.Logger _log = Core.getLogger("obus");

	/**
	 *
	 */
	public final ObjectDesc desc;
	public final int uid;
	protected final ObusStruct struct;
	public int handle;	
	
	/**
	 *
	 */
	protected ObusObject(ObjectDesc objDesc, ObusStruct struct) {
		this.desc = objDesc;
		this.uid = objDesc.uid;
		this.handle = Core.OBUS_INVALID_HANDLE;
		this.struct = struct;
	}

	/**
	 *
	 */
	public static ObusObject create(ObjectDesc objDesc) {
		/* Create ObusStruct then ObusObject object */
		ObusStruct struct = ObusStruct.create(objDesc.structDesc);
		return objDesc.creator.create(objDesc, struct);
	}

	@Override
	public ObjectDesc getDescriptor() {
		return this.desc;
	}

	@Override
	public int getHandle() {
		return this.handle;
	}
		
	/**
	 *
	 */
	public void encode(Buffer buf) {
		/* Write uid and handle, then data structure */
		buf.writeU16(this.uid);
		buf.writeU16(this.handle);

		/* Write data structure */
		int marker = buf.prepareSizeMarker();
		this.struct.encode(buf);
		buf.writeSizeMarker(marker);
	}

	/**
	 *
	 */
	public static ObusObject decode(Bus bus, Buffer buf) {
		/* Read uid and handle */
		int uid = buf.readU16();
		int handle = buf.readU16();

		/* Get current position in buffer and size that we are supposed to read*/
		int sizeData = buf.readU32();
		int posData = buf.getPos();
		try {
			/* Get descriptor */
			ObjectDesc objDesc = bus.getDesc().objectsDesc.get(uid);
			if (objDesc == null) {
				throw new DecodeError("ObusObject.decode: " +
						"can't create object uid=" + uid +
						", descriptor not found");
			}

			/* Decode data structure */
			ObusStruct struct = ObusStruct.decode(objDesc.structDesc, buf);

			/* Create ObusObject object with decoded struct
			   (so we can't use ObusObject.create method) */
			ObusObject obj = objDesc.creator.create(objDesc, struct);
			obj.handle = handle;
			return obj;
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
		return "{" + this.desc.name + "("+ this.uid + "), handle=" + this.handle +
				", struct=" + this.struct.toString() + "}";
	}
}
