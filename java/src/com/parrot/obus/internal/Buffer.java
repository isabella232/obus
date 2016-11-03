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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import com.parrot.obus.internal.Core.DecodeError;

/**
 * Wrapper around java.nio.ByteBuffer for obus specific encoding/decoding.
 */
public class Buffer {
	/** Allocation step when writing */
	private static final int ALLOC_STEP = 1024;

	/** Internal buffer with data */
	private ByteBuffer data;

	public Buffer() {
		this.data = ByteBuffer.allocate(ALLOC_STEP);
		this.data.order(ByteOrder.BIG_ENDIAN);
	}

	public Buffer(int size) {
		this.data = ByteBuffer.allocate(size);
		this.data.order(ByteOrder.BIG_ENDIAN);
	}

	public ByteBuffer getByteBuffer() {
		return this.data;
	}

	/**
	 * To be called when writing is finished to setup length
	 * @return length of data.
	 */
	public int finish() {
		this.data.limit(this.data.position());
		return this.data.position();
	}

	public int length() {
		return this.data.position();
	}

	/**
	 * Get current position in buffer.
	 * @return current position in buffer.
	 */
	public int getPos() {
		return this.data.position();
	}

	/**
	 * Change position in buffer.
	 * @param pos : new position.
	 */
	public void setPos(int pos) {
		this.data.position(pos);
	}

	/** Prepare the buffer to put a marker to write later a size.
	 * It memorize the position and write a dummy U32.
	 * @return the memorized position before the dummy U32.
	 */
	public int prepareSizeMarker() {
		int curPos = this.getPos();
		this.writeU32(0);
		return curPos;
	}

	/** Write a size at a previously memorized marker.
	 * @param marker : marker returned by prepareSizeMarker.
	 */
	public void writeSizeMarker(int marker) {
		int curPos = this.getPos();
		this.setPos(marker);
		this.writeU32(curPos - marker - 4);
		this.setPos(curPos);
	}

	/**
	 * Skip some bytes in write buffer. Useful to skip header part without
	 * really writing anything. Only the position in the buffer is changed.
	 * Data should be written latter after a rewind.
	 * @param count : number of bytes to skip.
	 */
	public void skip(int count) {
		this.ensureSize(count);
		this.data.position(this.data.position() + count);
	}

	/**
	 * Rewind the buffer to reset the position at 0 and keep current data.
	 */
	public void rewind() {
		this.data.rewind();
	}

	public void writeU8(int val) {
		this.ensureSize(1);
		this.data.put((byte)(val&0xff));
	}

	public void writeI8(int val) {
		this.ensureSize(1);
		this.data.put((byte)(val));
	}

	public void writeU16(int val) {
		this.ensureSize(2);
		this.data.putShort((short)(val&0xffff));
	}

	public void writeI16(int val) {
		this.ensureSize(2);
		this.data.putShort((short)(val));
	}

	public void writeU32(int val) {
		this.ensureSize(4);
		this.data.putInt(val);
	}

	public void writeI32(int val) {
		this.ensureSize(4);
		this.data.putInt(val);
	}

	public void writeU64(long val) {
		this.ensureSize(8);
		this.data.putLong(val);
	}

	public void writeI64(long val) {
		this.ensureSize(8);
		this.data.putLong(val);
	}

	public void writeString(String string) {
		if (string == null) {
			this.ensureSize(4);
			this.data.putInt(0);
		} else {
			byte[] buf = string.getBytes();
			this.ensureSize(4 + buf.length + 1);
			this.data.putInt(buf.length + 1);
			this.data.put(buf);
			this.data.put((byte)0);
		}
	}

	public void writeF32(float val) {
		this.ensureSize(4);
		this.data.putFloat(val);
	}

	public void writeF64(double val) {
		this.ensureSize(8);
		this.data.putDouble(val);
	}

	public void writeBuf(byte[] buf) {
		this.ensureSize(buf.length);
		this.data.put(buf);
	}

	public void writeBuf(ByteBuffer buf, int length) {
		/* Use a slice because we don't want to copy remaining data, only up
		   to the given length */
		this.ensureSize(length);
		ByteBuffer bufSlice = buf.slice();
		bufSlice.limit(length);
		this.data.put(bufSlice);
		/* Don't forget to update original buffer position */
		buf.position(buf.position() + length);
	}

	public int readU8() {
		byte val = this.data.get();
		if (val >= 0) {
			return val;
		} else {
			return val+256;
		}
	}

	public int readI8() {
		byte val = this.data.get();
		return val;
	}

	public int readU16() {
		short val = this.data.getShort();
		if (val >= 0) {
			return val;
		} else {
			return val+65536;
		}
	}

	public int readI16() {
		short val = this.data.getShort();
		return val;
	}

	public int readU32() {
		return this.data.getInt();
	}

	public int readI32() {
		return this.data.getInt();
	}

	public long readU64() {
		return this.data.getLong();
	}

	public long readI64() {
		return this.data.getLong();
	}

	public String readString() throws DecodeError {
		int size = this.data.getInt();
		if (size >= 65536) {
			throw new DecodeError("String size too big: " + size);
		}
		if (size == 0) {
			return null;
		} else {
			/* A final null byte is present */
			byte[] buf = new byte[size-1];
			this.data.get(buf);
			this.data.get();
			return new String(buf);
		}
	}

	public float readF32() {
		return this.data.getFloat();
	}

	public double readF64() {
		return this.data.getDouble();
	}

	public byte[] readBuf(int count) {
		byte[] buf = new byte[count];
		this.data.get(buf);
		return buf;
	}

	/**
	 * Make sure there is enough room in write buffer.
	 * @param size : number of bytes that are needed for next write operation.
	 * @remarks More bytes may be allocated.
	 */
	private void ensureSize(int size) {
		/* Check remaining bytes in buffer */
		if (this.data.remaining() < size) {
			int allocStep = (size < ALLOC_STEP) ? ALLOC_STEP : size;
			/* setup new buffer */
			this.data.limit(this.data.position());
			this.data.rewind();
			ByteBuffer newData = ByteBuffer.allocate(this.data.capacity() + allocStep);
			newData.put(this.data);
			this.data = newData;
		}
	}
}
