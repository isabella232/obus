#===============================================================================
# obus-python - obus client python module.
#
# @file protocol.py
#
# @brief obus python protocol layer
#
# @author yves-marie.morgan@parrot.com
#
# Copyright (c) 2013 Parrot S.A.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * Neither the name of the Parrot Company nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL PARROT COMPANY BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#===============================================================================

import logging
import struct

import obus
from obus import DecodeError
from obus.data import ObusObject
from obus.data import ObusEvent
from obus.data import ObusCall
from obus.data import ObusBusEvent
from .buffer import Buffer

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

# Current obus protocol version
OBUS_PROTOCOL_VERSION = 0x02

# obus magic
_OBUS_MAGIC_0 = ord("o")
_OBUS_MAGIC_1 = ord("b")
_OBUS_MAGIC_2 = ord("u")
_OBUS_MAGIC_3 = ord("s")
_OBUS_MAGIC = (_OBUS_MAGIC_0<<24)+(_OBUS_MAGIC_1<<16)+(_OBUS_MAGIC_2<<8)+_OBUS_MAGIC_3

# packet header format
# ******************************
# *  magic | size | type |
# *   4B      4B     1B
# ******************************
_OBUS_PKT_HDR_SIZE = 9
_OBUS_PKT_HDR_MAGIC_OFFSET = 0
_OBUS_PKT_HDR_SIZE_OFFSET = 4
_OBUS_PKT_HDR_TYPE_OFFSET = 8

# Packet type
OBUS_PKT_CONREQ = 0    # Connection request, from client to server
OBUS_PKT_CONRESP = 1   # Connection response, from server to client
OBUS_PKT_ADD = 2       # Add object, from server to client
OBUS_PKT_REMOVE = 3    # Remove object, from server to client
OBUS_PKT_BUS_EVENT = 4 # Bus event, from server to client
OBUS_PKT_EVENT = 5     # Object event, from server to client
OBUS_PKT_CALL = 6      # Method call, from client to server
OBUS_PKT_ACK = 7       # Method call acknowledgment, from server to client
OBUS_PKT_COUNT = 8
_packetTypeStr = {
	OBUS_PKT_CONREQ: "CONREQ",
	OBUS_PKT_CONRESP: "CONRESP",
	OBUS_PKT_ADD: "ADD",
	OBUS_PKT_REMOVE: "REMOVE",
	OBUS_PKT_BUS_EVENT: "BUS_EVENT",
	OBUS_PKT_EVENT: "EVENT",
	OBUS_PKT_CALL: "CALL",
	OBUS_PKT_ACK: "ACK",
}
def getPacketTypeStr(val):
	return _packetTypeStr.get(val, "UNKNOWN(%d)" % val)

# Connection response status
OBUS_CONRESP_STATUS_ACCEPTED = 0
OBUS_CONRESP_STATUS_REFUSED = 1
OBUS_CONRESP_STATUS_COUNT = 2
_connRespStatusStr = {
	OBUS_CONRESP_STATUS_ACCEPTED: "ACCEPTED",
	OBUS_CONRESP_STATUS_REFUSED: "REFUSED",
}
def getConnRespStatusStr(val):
	return _connRespStatusStr.get(val, "UNKNOWN(%d)" % val)

#===============================================================================
#===============================================================================
class Header(object):
	def __init__(self, magic, size, packetType):
		self.magic = magic
		self.size = size
		self.packetType = packetType

	def log(self, level):
		_log.log(level, "PACKET[%s]: %d bytes",
				getPacketTypeStr(self.packetType), self.size)

	def __repr__(self):
		return ("{packetType='%s', size=%d}" %
				(getPacketTypeStr(self.packetType), self.size))

#===============================================================================
#===============================================================================
class TxPacket(object):
	def __init__(self, packetType):
		self.packetType = packetType
		self.buf = Buffer()
		self.buf.skip(_OBUS_PKT_HDR_SIZE)

	def getData(self):
		return self.buf.getData()

	def finalizeHeader(self):
		size = len(self.buf)
		self.buf.rewind()
		self.buf.writeU32(_OBUS_MAGIC)
		self.buf.writeU32(size)
		self.buf.writeU8(self.packetType)

#===============================================================================
#===============================================================================
class TxPacketConnReq(TxPacket):
	def __init__(self, clientName, busName, crc):
		TxPacket.__init__(self, OBUS_PKT_CONREQ)
		self.buf.writeU8(OBUS_PROTOCOL_VERSION)
		self.buf.writeString(busName)
		self.buf.writeU32(crc)
		self.buf.writeString(clientName)
		self.finalizeHeader()

#===============================================================================
#===============================================================================
class TxPacketCall(TxPacket):
	def __init__(self, call):
		TxPacket.__init__(self, OBUS_PKT_CALL)
		call.encode(self.buf)
		self.finalizeHeader()

#===============================================================================
#===============================================================================
class RxPacket(object):
	def __init__(self, header):
		self.header = header

	def __repr__(self):
		return ("{header=%s}" % self.header)

#===============================================================================
#===============================================================================
class RxPacketConnReq(RxPacket):
	def __init__(self, bus, header, buf): # IGNORE:W0613
		RxPacket.__init__(self, header)
		self.version = buf.readU8()
		self.busName = buf.readString()
		self.crc = buf.readU32()
		self.clientName = buf.readString()

	def __repr__(self):
		return ("{header=%s, busName='%s', crc=0x%x, clientName='%s'}" %
				(self.header, self.busName, self.crc, self.clientName))

#===============================================================================
#===============================================================================
class RxPacketConnResp(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)
		self.status = buf.readU8()
		if self.status < 0 or self.status >= OBUS_CONRESP_STATUS_COUNT:
			raise DecodeError("RxPacketConnResp: " +
					"invalid connection response status %d" % self.status)
		objCount = buf.readU32()
		self.objects = []
		for _ in range(0, objCount):
			obj = ObusObject.decode(bus, buf)
			if obj is not None:
				self.objects.append(obj)

	def __repr__(self):
		return ("{header=%s, status='%s', objects=%s}" %
				(self.header, getConnRespStatusStr(self.status), self.objects))

#===============================================================================
#===============================================================================
class RxPacketAdd(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)
		self.obj = ObusObject.decode(bus, buf)
		if not self.obj:
			raise DecodeError("RxPacketAdd: " + "no object decoded")

	def __repr__(self):
		return ("{header=%s, obj=%s}" % (self.header, self.obj))

#===============================================================================
#===============================================================================
class RxPacketRemove(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)
		# Read uid and handle of object
		uid = buf.readU16()
		handle = buf.readU16()
		# Make sure object really exist
		self.obj = bus.findObject(handle)
		if self.obj is None:
			raise DecodeError("RxPacketRemove: " +
					"object uid=%d handle=%d not registered" % (uid, handle))
		# Check that uid is correct
		if self.obj.uid != uid:
			raise DecodeError("RxPacketRemove: " +
					"object uid=%d handle=%d bad internal uid=%d" %
					(uid, handle, self.obj.uid))

	def __repr__(self):
		return ("{header=%s, obj=%s}" % (self.header, self.obj))

#===============================================================================
#===============================================================================
class RxPacketBusEvent(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)
		self.busEvt = ObusBusEvent.decode(bus, buf)
		if not self.busEvt:
			raise DecodeError("RxPacketBusEvent: " + "no bus event decoded")

#===============================================================================
#===============================================================================
class RxPacketEvent(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)
		self.evt = ObusEvent.decode(bus, buf)
		if not self.evt:
			raise DecodeError("RxPacketEvent: " + "no event decoded")

	def __repr__(self):
		return ("{header=%s, evt=%s}" % (self.header, self.evt))

#===============================================================================
#===============================================================================
class RxPacketCall(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)
		self.call = ObusCall.decode(bus, buf)
		if not self.call:
			raise DecodeError("RxPacketCall: " + "no call decoded")

#===============================================================================
#===============================================================================
class RxPacketAck(RxPacket):
	def __init__(self, bus, header, buf):
		RxPacket.__init__(self, header)

		# Read call handle and ack status
		self.handle = buf.readU16()
		ackVal = buf.readU8()
		try:
			self.ack = obus.CallAck.fromInt(ackVal)
		except KeyError:
			raise DecodeError("RxPacketAck: " +
					"invalid call ack %d" % ackVal)

#===============================================================================
#===============================================================================
class RxRawPacket(object):
	_rxPacketClasses = {
		OBUS_PKT_CONREQ: RxPacketConnReq,
		OBUS_PKT_CONRESP: RxPacketConnResp,
		OBUS_PKT_ADD: RxPacketAdd,
		OBUS_PKT_REMOVE: RxPacketRemove,
		OBUS_PKT_BUS_EVENT: RxPacketBusEvent,
		OBUS_PKT_EVENT: RxPacketEvent,
		OBUS_PKT_CALL: RxPacketCall,
		OBUS_PKT_ACK: RxPacketAck,
	}

	def __init__(self, header, payloadBuf):
		self.header = header
		self.payloadBuf = payloadBuf

	def decode(self, bus):
		packet = None
		self.payloadBuf.rewind()
		try:
			if self.header.packetType not in self._rxPacketClasses:
				_log.warning("RxRawPacket: unhandled packet type: %s",
						getPacketTypeStr(self.header.packetType))
			else:
				packet = self._rxPacketClasses[self.header.packetType](
						bus, self.header, self.payloadBuf)
		except DecodeError as ex:
			_log.error(ex)
		except struct.error as ex:
			_log.error(ex)
		return packet

#===============================================================================
#===============================================================================
class Decoder(object):
	_STATE_IDLE = 0           # Idle state
	_STATE_HEADER_MAGIC_0 = 1 # Waiting for magic byte 0
	_STATE_HEADER_MAGIC_1 = 2 # Waiting for magic byte 1
	_STATE_HEADER_MAGIC_2 = 3 # Waiting for magic byte 2
	_STATE_HEADER_MAGIC_3 = 4 # Waiting for magic byte 3
	_STATE_HEADER = 5         # Reading header
	_STATE_PAYLOAD = 6        # Reading payload

	def __init__(self):
		self.headerBuf = None
		self.payloadBuf = None
		self.state = Decoder._STATE_IDLE
		self.bufSrc = None
		self.offSrc = 0
		self.lenSrc = 0
		self.header = None
		self._reset()

	def decode(self, buf, off):
		packet = None
		# If idle start a new parsing
		if self.state == Decoder._STATE_IDLE:
			self.state = Decoder._STATE_HEADER_MAGIC_0

		# Setup source buffer
		self.bufSrc = buf
		self.offSrc = off
		self.lenSrc = len(buf)-off
		while self.lenSrc > 0 and self.state != Decoder._STATE_IDLE:
			if self.state == Decoder._STATE_HEADER_MAGIC_0:
				self._reset()
				self.state = Decoder._STATE_HEADER_MAGIC_0
				self._copyOne(self.headerBuf)
				self._checkMagic(0, _OBUS_MAGIC_0)
			elif self.state == Decoder._STATE_HEADER_MAGIC_1:
				self._copyOne(self.headerBuf)
				self._checkMagic(1, _OBUS_MAGIC_1)
			elif self.state == Decoder._STATE_HEADER_MAGIC_2:
				self._copyOne(self.headerBuf)
				self._checkMagic(2, _OBUS_MAGIC_2)
			elif self.state == Decoder._STATE_HEADER_MAGIC_3:
				self._copyOne(self.headerBuf)
				self._checkMagic(3, _OBUS_MAGIC_3)
			elif self.state == Decoder._STATE_HEADER:
				self._copy(self.headerBuf, _OBUS_PKT_HDR_SIZE)
				if len(self.headerBuf) == _OBUS_PKT_HDR_SIZE:
					self._decodeHeader()
			elif self.state == Decoder._STATE_PAYLOAD:
				self._copy(self.payloadBuf, self.header.size - _OBUS_PKT_HDR_SIZE)
				if len(self.payloadBuf) == self.header.size - _OBUS_PKT_HDR_SIZE:
					# Return a raw packet with header and payload and let caller
					# decide in which thread to continue decoding the payload
					packet = RxRawPacket(self.header, self.payloadBuf)
					self.state = Decoder._STATE_IDLE
		return (self.offSrc, packet)

	def _reset(self):
		self.headerBuf = Buffer()
		self.payloadBuf = None
		self.header = None
		self.state = Decoder._STATE_IDLE

	def _checkMagic(self, idx, val):
		magic = ord(self.headerBuf.getData()[idx])
		if magic != val:
			_log.error("Bad magic %d: 0x%02x (0x%02x)", idx, magic, val)
			self.state = Decoder._STATE_HEADER_MAGIC_0
		else:
			self.state += 1

	def _copyOne(self, bufDst):
		bufDst.writeBuf(self.bufSrc[self.offSrc:self.offSrc+1])
		self.offSrc += 1
		self.lenSrc -= 1

	def _copy(self, bufDst, sizeDst):
		cpyLen = min(self.lenSrc, sizeDst - len(bufDst))
		bufDst.writeBuf(self.bufSrc[self.offSrc:self.offSrc+cpyLen])
		self.offSrc += cpyLen
		self.lenSrc -= cpyLen

	def _decodeHeader(self):
		self.headerBuf.rewind()
		try:
			magic = self.headerBuf.readU32()
			size = self.headerBuf.readU32()
			packetType = self.headerBuf.readU8()
			self.header = Header(magic, size, packetType)
			if self.header.packetType < 0 or self.header.packetType >= OBUS_PKT_COUNT:
				raise DecodeError("Decoder: " +
						"Bad packet type: %d" % self.header.packetType)
			if self.header.size < _OBUS_PKT_HDR_SIZE:
				raise DecodeError("Decoder: " +
						"Bad packet size: %d" % self.header.size)
			#self.header.log(logging.DEBUG)
			self.state = Decoder._STATE_PAYLOAD
			self.payloadBuf = Buffer()
		except DecodeError as ex:
			_log.error(ex)
			self.state = Decoder._STATE_HEADER_MAGIC_0
		except struct.error as ex:
			_log.error(ex)
			self.state = Decoder._STATE_HEADER_MAGIC_0
