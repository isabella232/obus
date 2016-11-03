#===============================================================================
# obus-python - obus client python module.
#
# @file obus_call.py
#
# @brief obus python call
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
import obus
from obus import DecodeError
from .obus_struct import ObusStruct

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

#===============================================================================
#===============================================================================
class ObusCall(object):
	def __init__(self, mtdDesc, obj, struct):
		self.desc = mtdDesc
		self.uid = mtdDesc.uid
		self.obj = obj
		self.struct = struct
		self.handle = obus.OBUS_INVALID_HANDLE
		self.ack = obus.CallAck.INVALID

	def __str__(self):
		return ("{name='%s', handle=%d, ack=%s,"
				" obj={type='%s', handle=%d}, struct=%s}" %
				(self.desc.name, self.handle, self.ack,
				self.obj.desc.name, self.obj.handle, self.struct))

	def clone(self):
		clonedCall = ObusCall(self.desc, self.obj.clone(), self.struct.clone())

		clonedCall.handle = self.handle
		clonedCall.ack = self.ack

		return clonedCall

	@staticmethod
	def create(mtdDesc, obj):
		# Create ObusStruct then ObusCall object
		struct = ObusStruct.create(mtdDesc.structDesc)
		return mtdDesc.creator(mtdDesc, obj, struct)

	def encode(self, buf):
		# Write uid and handle of object
		buf.writeU16(self.obj.uid)
		buf.writeU16(self.obj.handle)

		# Write uid and handle of call
		buf.writeU16(self.uid)
		buf.writeU16(self.handle)

		# Reserve room for size
		sizeMarker = buf.prepareSizeMarker()
		self.struct.encode(buf)
		buf.writeSizeMarker(sizeMarker)

	@staticmethod
	def decode(bus, buf):
		# Read uid and handle of object
		uidObj = buf.readU16()
		handleObj = buf.readU16()
		# Read uid and handle of call
		uidCall = buf.readU16()
		handleCall = buf.readU16()

		# Get current position in buffer and size that we are supposed to read
		sizeData = buf.readU32()
		posData = buf.getPos()
		try:
			# Find object
			obj = bus.findObject(handleObj)
			if obj is None:
				raise DecodeError("ObusCall.decode: " +
						"object uid=%d handle=%d not registered" %
						(uidObj, handleObj))

			# Check that uid is correct
			if obj.uid != uidObj:
				raise DecodeError("ObusCall.decode: " +
						"object uid=%d handle=%d bad internal uid=%d" %
						(uidObj, handleObj, obj.uid))

			# get descriptor of method
			mdtDesc = obj.desc.methodsDesc.get(uidCall)
			if mdtDesc is None:
				raise DecodeError("ObusCall.decode: " +
						"object uid=%d can't create method uid=%d,"
						" descriptor not found" % (obj.uid, uidCall))

			# Decode data structure
			struct = ObusStruct.decode(mdtDesc.structDesc, buf)

			# Create ObusCall object with decoded struct
			# so we can't use ObusCall.create method)
			call = mdtDesc.creator(mdtDesc, obj, struct)
			call.handle = handleCall
			return call
		except DecodeError as ex:
			# Try to continue
			_log.error(str(ex))
			buf.setPos(posData+sizeData)
			return None
