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

package com.parrot.obus;

import java.net.InetSocketAddress;

/**
 * obus socket address that handle both unix & inet socket family address.
 */
public class ObusAddress {
	public static enum Type {
		inet,
		unix
	};

	private final Type type;
	private final String unix;
	private final InetSocketAddress inet;

	/**
	 * Create an Inet ObusAddress from an InetSocketAddress object.
	 * 
	 * @param addr inet socket address.
	 */
	public ObusAddress(InetSocketAddress addr) {
		type = Type.inet;
		inet = addr;
		unix = null;
	}

	/**
	 * Create a Unix ObusAddress from path.
	 * 
	 * Unix socket address can be:
	 * - a filesystem path, ex:"/pathto/mysocket_file"
	 * - an abstract path not visible from filesystem (must start with '@')
	 *   ex: "@/obus/mybus"
	 * 
	 * @param unixPath unix socket path must start with '/' or "@/" for an
	 * abstract unix socket address.
	 */
	public ObusAddress(String unixPath) {
		int pos = 0;

		/* check unix path start position for abstract socket */
		if (unixPath.length() > 0 && unixPath.charAt(0) == '@')
			pos = 1;

		/* unix path shall start with '/' */
		if ((unixPath.length() < (pos + 2)) || unixPath.charAt(pos) != '/')
			throw new IllegalArgumentException("unixPath=" + unixPath);

		type = Type.unix;
		unix = unixPath;
		inet = null;
	}

	/**
	 * Create an ObusAddress from its obus canonical representation format.
	 * 
	 * @param addr inet socket address.
	 */
	static public ObusAddress createfromObusCanonical(String format) {
		String[] tokens;

		tokens = format.split(":");
		if (tokens.length == 0)
			throw new IllegalArgumentException("format=" + format);

		if (tokens[0].contentEquals("inet")) {
			if (tokens.length != 3)
				throw new IllegalArgumentException("format=" + format);

			InetSocketAddress addr = null;
			int port = Integer.parseInt(tokens[2]);
			addr = new InetSocketAddress(tokens[1], port); 
			return new ObusAddress(addr);
		} else if (tokens[0].contentEquals("unix")) {
			if (tokens.length != 2)
				throw new IllegalArgumentException("format=" + format);

			return new ObusAddress(tokens[1]);
		} else {
			throw new IllegalArgumentException("format=" + format);
		}
	}

	/**
	 * get obus canonical representation format of socket address
	 *
	 * format is "<family>:<address>[:port]"
	 *
	 * @return string
	 */
	public String getObusCanonical() {
		switch (type) {
		case inet:
			return "inet:" + inet.getAddress().getHostAddress() + ":" + inet.getPort();
		case unix:
			return "unix:" + unix;
		default:
			return null;
		}
	}

	/**
	 * (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return getObusCanonical();
	}
}
