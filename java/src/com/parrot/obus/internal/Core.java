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

import android.util.Log;

import com.parrot.obus.internal.Descriptor.BusEventDesc;
import com.parrot.obus.internal.Descriptor.EventDesc;
import com.parrot.obus.internal.Descriptor.MethodDesc;
import com.parrot.obus.internal.Descriptor.ObjectDesc;

/**
 *
 */
public class Core {

	/** log received packets header */
	public static final boolean OBUS_PACKET_HEADER_LOG = false;

	/** Invalid object handle */
	public static final int OBUS_INVALID_HANDLE = 0;

	/** Invalid uid */
	public static final int OBUS_INVALID_UID = 0;

	/** Common bus events uid */
	public static final int BUSEVT_CONNECTED_UID = 1;
	public static final int BUSEVT_DISCONNECTED_UID = 2;
	public static final int BUSEVT_CONNECTION_REFUSED_UID = 3;
	
	/** Decode error exception */
	public static class DecodeError extends Exception {
		private static final long serialVersionUID = 1L;
		public DecodeError(String message) {
			super(message);
		}
	}

	/** obus object creator */
	public static interface ObusObjectCreator<T extends ObusObject> {
		public T create(ObjectDesc objDesc, ObusStruct struct);
	}

	/** obus event creator */
	public static interface ObusEventCreator<T extends ObusEvent> {
		public T create(EventDesc evtDesc, ObusObject obj, ObusStruct struct);
	}

	/** obus call creator */
	public static interface ObusCallCreator<T extends ObusCall> {
		public T create(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct);
	}

	/** obus bus event creator */
	public static interface ObusBusEventCreator<T extends ObusBusEvent> {
		public T create(BusEventDesc busEvtDesc);
	}

	/**
	 *
	 */
	public static class Logger {
		private final String tag;
		public Logger(String tag) {
			this.tag = tag;
		}
		public void error(String str) {
			Log.e(this.tag, str);
		}
		public void warning(String str) {
			Log.w(this.tag, str);
		}
		public void info(String str) {
			Log.i(this.tag, str);
		}
		public void debug(String str) {
			Log.d(this.tag, str);
		}
	}

	public static Logger getLogger(String tag) {
		return new Logger(tag);
	}
}
