#===============================================================================
# obus-python - obus client python module.
#
# @file obus_object.py
#
# @brief obus python object
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
class ObusObject(object):
	def __init__(self, objDesc, struct):
		self.desc = objDesc
		self.uid = objDesc.uid
		self.handle = obus.OBUS_INVALID_HANDLE
		self.struct = struct

	def __str__(self):
		return ("{type='%s', handle=%d, struct=%s}" %
				(self.desc.name, self.handle, self.struct))

	def clone(self):
		clonedObj = self.desc.creator(self.desc, self.struct.clone())
		clonedObj.handle = self.handle

		return clonedObj

	def hasField(self, fieldName):
		fieldDesc = self.struct.desc.findField(fieldName)
		if fieldDesc is None:
			return False

		return self.struct.hasField(fieldDesc)

	def getField(self, fieldName):
		fieldDesc = self.struct.desc.findField(fieldName)
		if fieldDesc is None:
			return None

		return self.struct.getField(fieldDesc)

	@staticmethod
	def create(objDesc):
		# Create ObusStruct then ObusObject object
		struct = ObusStruct.create(objDesc.structDesc)
		return objDesc.creator(objDesc, struct)

	def encode(self, buf):
		# Write uid and handle, then data structure
		buf.writeU16(self.uid)
		buf.writeU16(self.handle)
		# Reserve room for size
		sizeMarker = buf.prepareSizeMarker()
		self.struct.encode(buf)
		buf.writeSizeMarker(sizeMarker)

	@staticmethod
	def decode(bus, buf):
		# Read uid and handle
		uid = buf.readU16()
		handle = buf.readU16()

		# Get current position in buffer and size that we are supposed to read
		sizeData = buf.readU32()
		posData = buf.getPos()
		try:
			objDesc = bus.desc.objectsDesc.get(uid)
			if objDesc is None:
				raise DecodeError("ObusObject.decode: " +
						"Can't create object uid=%d, descriptor not found" % uid)

			# Decode data structure
			struct = ObusStruct.decode(objDesc.structDesc, buf)

			# Create ObusObject object with decoded struct
			# (so we can't use ObusObject.create method)
			obj = objDesc.creator(objDesc, struct)
			obj.handle = handle
			return obj
		except DecodeError as ex:
			# Try to continue
			_log.error(str(ex))
			buf.setPos(posData+sizeData)
			return None
