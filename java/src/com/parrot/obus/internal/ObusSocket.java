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

import com.parrot.obus.ObusAddress;
import java.nio.ByteBuffer;


/**
 * Obus Socket client implementation is done in native.
 */
public class ObusSocket {
	private static final Core.Logger _log = Core.getLogger("obus");
	private int fd = -1;

	static {
		System.loadLibrary("obus-jni");
	}

	/* native doConnect socket */
	private native boolean doConnect(String address);

	/* native doRead */
	private native int doRead(byte[] buf, int offset, int size);
	
	/* native doWrite */
	private native int doWrite(byte[] buf, int offset, int size);

	/* connect socket */
	public boolean connect(ObusAddress remote) {
		return doConnect(remote.getObusCanonical());
	}

	/* read from socket ( ret <= 0 on read failure ) */
	public int read(ByteBuffer buf) {
		return doRead(buf.array(), buf.arrayOffset() + buf.position(), 
					  buf.limit() - buf.position());
	}

	/* write buffer to socket ( ret <= 0 on write failure )*/
	public int write(ByteBuffer buf) {
		return doWrite(buf.array(), buf.arrayOffset() + buf.position(), 
					   buf.limit() - buf.position());
	}

	/* shutdown socket */
	public native void shutdown();

	/* close socket */
	public native void close();
}
