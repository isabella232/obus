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

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.parrot.obus.MethodCallAck;
import com.parrot.obus.internal.Core.DecodeError;

/**
 * Obus protocol classes.
 */
public class Protocol {
	private static final Core.Logger _log = Core.getLogger("obus");

	/* obus protocol version */
	public static final int OBUS_PROTOCOL_VERSION = 2;

	/* obus magic */
	private static final int OBUS_MAGIC_0 = 'o';
	private static final int OBUS_MAGIC_1 = 'b';
	private static final int OBUS_MAGIC_2 = 'u';
	private static final int OBUS_MAGIC_3 = 's';
	private static final int OBUS_MAGIC =
			(OBUS_MAGIC_0<<24)+(OBUS_MAGIC_1<<16)+(OBUS_MAGIC_2<<8)+OBUS_MAGIC_3;

	/* packet header format
	 * ******************************
	 * *  magic | size | type |
	 * *   4B      4B     1B
	 * ******************************
	 */
	private static final int OBUS_PKT_HDR_SIZE = 9;

	/** Packet type */
	public static enum PacketType {
		CONREQ(0),    /**< Connection request, from client to server */
		CONRESP(1),   /**< Connection response, from server to client */
		ADD(2),       /**< add/register object, from server to client */
		REMOVE(3),    /**< remove/unregister object, from server to client */
		BUS_EVENT(4), /**< Bus event, from server to client */
		EVENT(5),     /**< Object event, from server to client */
		CALL(6),      /**< Method call, from client to server */
		ACK(7);       /**< Method call acknowledgment, from server to client */
		private final int value;
		private PacketType(int value) {this.value = value;}
		public static int toInt(PacketType type) {return type.value;}
		public static PacketType fromInt(int val) throws DecodeError {
			switch (val) {
			case 0: return CONREQ;
			case 1: return CONRESP;
			case 2: return ADD;
			case 3: return REMOVE;
			case 4: return BUS_EVENT;
			case 5: return EVENT;
			case 6: return CALL;
			case 7: return ACK;
			default: throw new DecodeError("Invalid packet type: " + val);
			}
		}
	}

	/** Connection response status */
	public static enum ConnRespStatus {
		ACCEPTED(0),
		REFUSED(1);
		private final int value;
		private ConnRespStatus(int value) {this.value = value;}
		public static int toInt(ConnRespStatus status) {return status.value;}
		public static ConnRespStatus fromInt(int val) throws DecodeError {
			switch (val) {
			case 0: return ACCEPTED;
			case 1: return REFUSED;
			default: throw new DecodeError("Invalid connection response: " + val);
			}
		}
	}

	/**
	 *
	 */
	public static class Header {
		public final int magic;
		public final int size;
		public final PacketType packetType;

		public Header(int magic, int size, PacketType packetType) {
			this.magic = magic;
			this.size = size;
			this.packetType = packetType;
		}

		public void log() {
			if (Core.OBUS_PACKET_HEADER_LOG) {
				_log.debug("PACKET[" + this.packetType + "]: " + this.size + " bytes");
			}
		}
	}

	/**
	 *
	 */
	public static class TxPacket {
		protected final PacketType packetType;
		protected final Buffer buf;

		public TxPacket(PacketType packetType) {
			this.packetType = packetType;
			this.buf = new Buffer();
			this.buf.skip(OBUS_PKT_HDR_SIZE);
		}

		public ByteBuffer getByteBuffer() {
			return this.buf.getByteBuffer();
		}

		protected void finalizeHeader() {
			int size = this.buf.finish();
			this.buf.rewind();
			this.buf.writeU32(OBUS_MAGIC);
			this.buf.writeU32(size);
			this.buf.writeU8(PacketType.toInt(this.packetType));
		}
	}

	/**
	 *
	 */
	public static class TxPacketConnReq extends TxPacket {
		public TxPacketConnReq(String clientName, String busName, int crc) {
			super(PacketType.CONREQ);
			this.buf.writeU8(OBUS_PROTOCOL_VERSION);
			this.buf.writeString(busName);
			this.buf.writeU32(crc);
			this.buf.writeString(clientName);
			this.finalizeHeader();
		}
	}

	/**
	 *
	 */
	public static class TxPacketConnResp extends TxPacket {
		public TxPacketConnResp(ConnRespStatus status, ObusObject[] objects) {
			super(PacketType.CONRESP);
			this.buf.writeU8(ConnRespStatus.toInt(status));
			this.buf.writeU32(objects.length);
			for (ObusObject obj: objects) {
				obj.encode(this.buf);
			}
			this.finalizeHeader();
		}
	}

	/**
	 *
	 */
	public static class TxPacketAdd extends TxPacket {
		public TxPacketAdd(ObusObject obj) {
			super(PacketType.ADD);
			obj.encode(this.buf);
			this.finalizeHeader();
		}
	}

	/**
	 *
	 */
	public static class TxPacketRemove extends TxPacket {
		public TxPacketRemove(ObusObject obj) {
			super(PacketType.REMOVE);
			this.buf.writeU16(obj.uid);
			this.buf.writeU16(obj.handle);
			this.finalizeHeader();
		}
	}

	/**
	 *
	 */
	public static class TxPacketEvent extends TxPacket {
		public TxPacketEvent(ObusEvent evt) {
			super(PacketType.EVENT);
			evt.encode(this.buf);
			this.finalizeHeader();
		}
	}

	/**
	 *
	 */
	public static class TxPacketCall extends TxPacket {
		public TxPacketCall(ObusCall call) {
			super(PacketType.CALL);
			call.encode(this.buf);
			this.finalizeHeader();
		}
	}

	/**
	 *
	 */
	public static class RxPacket {
		public final Header header;

		public RxPacket(Header header) {
			this.header = header;
		}
	}

	/**
	 *
	 */
	public static class RxPacketConnReq extends RxPacket {
		public final int version;
		public final String busName;
		public final String clientName;
		public final int crc;

		public RxPacketConnReq(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			this.version = buf.readU8();
			this.busName = buf.readString();
			this.crc = buf.readU32();
			this.clientName = buf.readString();
		}
	}

	/**
	 *
	 */
	public static class RxPacketConnResp extends RxPacket {
		public final ConnRespStatus status;
		public final List<ObusObject> objects;

		public RxPacketConnResp(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			this.status = ConnRespStatus.fromInt(buf.readU8());
			int objCount = buf.readU32();
			this.objects = new ArrayList<ObusObject>(objCount);
			for (int i = 0; i < objCount; i++) {
				ObusObject obj = ObusObject.decode(bus, buf);
				if (obj != null) {
					this.objects.add(obj);
				}
			}
		}
	}

	/**
	 *
	 */
	public static class RxPacketAdd extends RxPacket {
		public final ObusObject obj;

		public RxPacketAdd(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			this.obj = ObusObject.decode(bus, buf);
			if (this.obj == null) {
				throw new DecodeError("RxPacketAdd: no object decoded");
			}
		}
	}

	/**
	 *
	 */
	public static class RxPacketRemove extends RxPacket {
		public final ObusObject obj;

		public RxPacketRemove(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			/* Read uid and handle of object */
			int uid = buf.readU16();
			int handle = buf.readU16();

			/* Make sure object really exist */
			this.obj = bus.findObject(handle);
			if (this.obj == null) {
				throw new DecodeError("RxPacketRemove: " +
						"object uid=" + uid +
						" handle=" + handle + " not registered");
			}

			/* Check that uid is correct */
			if (this.obj.uid != uid) {
				throw new DecodeError("RxPacketRemove: " +
						"object uid=" + uid +
						" handle=" + handle +
						" bad internal uid=" + this.obj.uid);
			}
		}
	}

	/**
	 *
	 */
	public static class RxPacketBusEvent extends RxPacket {
		public final ObusBusEvent busEvt;

		public RxPacketBusEvent(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			this.busEvt = ObusBusEvent.decode(bus, buf);
			if (this.busEvt == null) {
				throw new DecodeError("RxPacketBusEvent: no bus event decoded");
			}
		}
	}

	/**
	 *
	 */
	public static class RxPacketEvent extends RxPacket {
		public final ObusEvent evt;

		public RxPacketEvent(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			this.evt = ObusEvent.decode(bus, buf);
			if (this.evt == null) {
				throw new DecodeError("RxPacketEvent: no event decoded");
			}
		}
	}

	/**
	 *
	 */
	public static class RxPacketCall extends RxPacket {
		public final ObusCall call;

		public RxPacketCall(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);
			this.call = ObusCall.decode(bus, buf);
			if (this.call == null) {
				throw new DecodeError("RxPacketCall: no call decoded");
			}
		}
	}

	/**
	 *
	 */
	public static class RxPacketAck extends RxPacket {
		public final int handle;
		public final MethodCallAck ack;

		public RxPacketAck(Bus bus, Header header, Buffer buf) throws DecodeError {
			super(header);

			/* Read call handle and ack status */
			this.handle = buf.readU16();
			int ackValue = buf.readU8();
			switch (ackValue) {
			case 0: 
				this.ack = MethodCallAck.INVALID;
				break;
			case 1: 
				this.ack = MethodCallAck.ACKED;
				break;
			case 2: 
				this.ack = MethodCallAck.ABORTED;
				break;
			case 3: 
				this.ack = MethodCallAck.METHOD_DISABLED;
				break;
			case 4: 
				this.ack = MethodCallAck.METHOD_NOT_SUPPORTED;
				break;
			case 5: 
				this.ack = MethodCallAck.INVALID_ARGUMENTS;
				break;
			case 6: 
				this.ack = MethodCallAck.REFUSED;
				break;
			default: throw new DecodeError("Invalid call ack: " + ackValue);
			}
		}
	}

	/**
	 *
	 */
	public static class RxRawPacket {
		private final Header header;
		private final Buffer payloadBuf;

		/**
		 *
		 */
		public RxRawPacket(Header header, Buffer payloadBuf) {
			this.header = header;
			this.payloadBuf = payloadBuf;
		}

		/**
		 * Finish decoding of payload into a fully decode packet.
		 * @param bus : bus to use during decoding of payload.
		 * @return decoded packet or null if an error occurred.
		 */
		public RxPacket decode(Bus bus) {
			RxPacket packet = null;
			this.payloadBuf.rewind();
			try {
				switch (this.header.packetType) {
				case CONREQ:
					packet = new RxPacketConnReq(bus,
							this.header, this.payloadBuf);
					break;
				case CONRESP:
					packet = new RxPacketConnResp(bus,
							this.header, this.payloadBuf);
					break;
				case ADD:
					packet = new RxPacketAdd(bus,
							this.header, this.payloadBuf);
					break;
				case REMOVE:
					packet = new RxPacketRemove(bus,
							this.header, this.payloadBuf);
					break;
				case BUS_EVENT:
					packet = new RxPacketBusEvent(bus,
							this.header, this.payloadBuf);
					break;
				case EVENT:
					packet = new RxPacketEvent(bus,
							this.header, this.payloadBuf);
					break;
				case CALL:
					packet = new RxPacketCall(bus,
							this.header, this.payloadBuf);
					break;
				case ACK:
					packet = new RxPacketAck(bus,
							this.header, this.payloadBuf);
					break;
				default:
					_log.warning("RxPacket: unhandled packet type: " +
							this.header.packetType);
					break;
				}
			} catch (DecodeError e) {
				/* Some decoding error */
				_log.error(e.toString());
			} catch (BufferUnderflowException e) {
				/* Not enough data in payload */
				_log.error(e.toString());
			}

			/* Check if after decoding some bytes are still in payload */
			if (packet != null) {
				int remaining = this.payloadBuf.getByteBuffer().remaining();
				if (remaining != 0) {
					_log.warning("RxPacket: " + remaining +
							" bytes remaining after decoding of packet " +
							this.header.packetType);
				}
			}
			return packet;
		}
	}

	/**
	 *
	 */
	public static class Decoder {
		/** Internal state */
		private static enum State {
			IDLE,           /**< Idle state */
			HEADER_MAGIC_0, /**< Waiting for magic byte 0 */
			HEADER_MAGIC_1, /**< Waiting for magic byte 1 */
			HEADER_MAGIC_2, /**< Waiting for magic byte 2 */
			HEADER_MAGIC_3, /**< Waiting for magic byte 3 */
			HEADER,         /**< Reading header */
			PAYLOAD         /**< Reading payload */
		}

		private Buffer headerBuf;
		private Buffer payloadBuf;
		private State state;
		private ByteBuffer bufSrc;
		private Header header;

		/**
		 * Create a new obus decoder.
		 */
		public Decoder() {
			this.headerBuf = null;
			this.payloadBuf = null;
			this.state = State.IDLE;
			this.bufSrc = null;
			this.header = null;
			this.reset();
		}

		/**
		 * Try to decode a raw packet from given buffer.
		 * @param buf : buffer with received data.
		 * @return a full raw packet of null if more data needed.
		 */
		public RxRawPacket decode(ByteBuffer buf) {
			RxRawPacket packet = null;
			/* If idle start a new parsing */
			if (this.state == State.IDLE) {
				this.state = State.HEADER_MAGIC_0;
			}

			/* Setup source buffer */
			this.bufSrc = buf;
			while (this.bufSrc.remaining() > 0 && this.state != State.IDLE) {
				switch (this.state) {
				case IDLE: /* FALLTHROUGH */
				case HEADER_MAGIC_0:
					this.reset();
					this.state = State.HEADER_MAGIC_0;
					this.copyOne(this.headerBuf);
					this.checkMagic(0, OBUS_MAGIC_0, State.HEADER_MAGIC_1);
					break;

				case HEADER_MAGIC_1:
					this.copyOne(this.headerBuf);
					this.checkMagic(1, OBUS_MAGIC_1, State.HEADER_MAGIC_2);
					break;

				case HEADER_MAGIC_2:
					this.copyOne(this.headerBuf);
					this.checkMagic(2, OBUS_MAGIC_2, State.HEADER_MAGIC_3);
					break;

				case HEADER_MAGIC_3:
					this.copyOne(this.headerBuf);
					this.checkMagic(3, OBUS_MAGIC_3, State.HEADER);
					break;

				case HEADER:
					this.copy(this.headerBuf, OBUS_PKT_HDR_SIZE);
					if (this.headerBuf.length() == OBUS_PKT_HDR_SIZE) {
						this.decodeHeader();
					}
					break;

				case PAYLOAD:
					this.copy(this.payloadBuf, this.header.size-OBUS_PKT_HDR_SIZE);
					if (this.payloadBuf.length() == this.header.size-OBUS_PKT_HDR_SIZE) {
						/* Return a raw packet with header and payload and
						   let caller decide in which thread to continue
						   decoding the payload */
						packet = new RxRawPacket(this.header, this.payloadBuf);
						this.state = State.IDLE;
					}
					break;
				}
			}
			return packet;
		}

		/**
		 * Reset the decoder.
		 */
		private void reset() {
			this.header = null;
			this.headerBuf = new Buffer(OBUS_PKT_HDR_SIZE);
			this.payloadBuf = null;
			this.state = State.IDLE;
		}

		/** Check one of the magic byte.
		 * @param idx : index of the magic byte o check.
		 * @param val : expected value.
		 * @param nextState : next state if ok.
		 */
		private void checkMagic(int idx, int val, State nextState) {
			int magic = this.headerBuf.getByteBuffer().get(idx)&0xff;
			if (magic != val) {
				_log.error(String.format(
						"Bad magic %d: 0x%02x (0x%02x)", idx, magic, val));
				this.state = State.HEADER_MAGIC_0;
			} else {
				this.state = nextState;
			}
		}

		/**
		 * Copy one byte from internal source buffer into given
		 * destination buffer.
		 * @param bufDst : destination buffer.
		 */
		private void copyOne(Buffer bufDst) {
			bufDst.writeU8(this.bufSrc.get());
		}

		/**
		 * Copy up to given size from internal source buffer into given
		 * destination buffer.
		 * @param bufDst : destination buffer.
		 * @param sizeDst : maximum size to copy.
		 */
		private void copy(Buffer bufDst, int sizeDst) {
			int cpyLen = sizeDst - bufDst.length();
			if (cpyLen > this.bufSrc.remaining()) {
				cpyLen = this.bufSrc.remaining();
			}
			bufDst.writeBuf(this.bufSrc, cpyLen);
		}

		/**
		 * Decode the header to determine size of payload that will follow.
		 */
		private void decodeHeader() {
			this.headerBuf.rewind();
			try {
				/* Decode header fields */
				int magic = this.headerBuf.readU32();
				int size = this.headerBuf.readU32();
				PacketType packetType = PacketType.fromInt(this.headerBuf.readU8());
				this.header = new Header(magic, size, packetType);
				if (this.header.size < OBUS_PKT_HDR_SIZE) {
					throw new DecodeError("Bad packet size: " + this.header.size);
				}
				this.header.log();
				this.state = State.PAYLOAD;
				this.payloadBuf = new Buffer(this.header.size - OBUS_PKT_HDR_SIZE);
			} catch (DecodeError e) {
				_log.error(e.toString());
				this.state = State.HEADER_MAGIC_0;
			} catch (BufferUnderflowException e) {
				_log.error(e.toString());
				this.state = State.HEADER_MAGIC_0;
			}
		}
	}
}
