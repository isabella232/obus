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

import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.MethodCallAck;
import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Descriptor.MethodDesc;

/**
 *
 */
public class ObusCall implements IBusMethodCall {
	private static final Core.Logger _log = Core.getLogger("obus");

	/**
	 *
	 */
	public final MethodDesc desc;
	public final int uid;
	public final ObusObject obj;
	protected final ObusStruct struct;
	public int handle;
	public MethodCallAck ack;
	public AckCb callback;
	/**
	 * @param cb 
	 *
	 */
	protected ObusCall(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {
		this.desc = mtdDesc;
		this.uid = mtdDesc.uid;
		this.obj = obj;
		this.struct = struct;
		this.handle = Core.OBUS_INVALID_HANDLE;
		this.ack = MethodCallAck.INVALID;
	}

	/**
	 *
	 */
	public static ObusCall create(MethodDesc mtdDesc, ObusObject obj) {
		/* Create ObusStruct then ObusCall object */
		ObusStruct struct = ObusStruct.create(mtdDesc.structDesc);
		return mtdDesc.creator.create(mtdDesc, obj, struct);
	}

	
	/**
	 *
	 */
	public void encode(Buffer buf) {
		/* Write uid and handle of object */
		buf.writeU16(this.obj.uid);
		buf.writeU16(this.obj.handle);

		/* Write uid and handle of call */
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
	public static ObusCall decode(Bus bus, Buffer buf) {
		/* Read uid and handle of object */
		int uidObj = buf.readU16();
		int handleObj = buf.readU16();
		/* Read uid and handle of call */
		int uidCall = buf.readU16();
		int handleCall = buf.readU16();

		/* Get current position in buffer and size that we are supposed to read*/
		int sizeData = buf.readU32();
		int posData = buf.getPos();
		try {
			/* Find object */
			ObusObject obj = bus.findObject(handleObj);
			if (obj == null) {
				throw new DecodeError("ObusCall.decode: " +
						"object uid=" + uidObj + " handle=" + handleObj +
						" not registered");
			}

			/* Check that uid is correct */
			if (obj.uid != uidObj) {
				throw new DecodeError("ObusCall.decode: " +
						"object uid=" + uidObj + " handle=" + handleObj +
						" bad internal uid=" + obj.uid);
			}

			/* Get method descriptor */
			MethodDesc mdtDesc = obj.desc.methodsDesc.get(uidCall);
			if (mdtDesc == null) {
				throw new DecodeError("ObusCall.decode: " +
						"object uid=" + obj.uid +
						" can't create method uid=" + uidCall +
						", descriptor not found");
			}

			/* Decode data structure */
			ObusStruct struct = ObusStruct.decode(mdtDesc.structDesc, buf);

			/* Create ObusCall object with decoded struct
			   (so we can't use ObusCall.create method) */
			ObusCall call = mdtDesc.creator.create(mdtDesc, obj, struct);
			call.handle = handleCall;
			return call;
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
		return "{" + this.desc.name + " ("+ this.uid + "), ack=" + this.ack +
				", obj={" + this.obj.desc.name +  ", handle=" + this.obj.handle +
				"}, struct=" + this.struct.toString() + "}";
	}

}
