#===============================================================================
# obus-python - obus client python module.
#
# @file obus_event.py
#
# @brief obus python event
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

from obus import DecodeError
from .obus_struct import ObusStruct

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

#===============================================================================
#===============================================================================
class ObusEvent(object):
	def __init__(self, evtDesc, obj, struct):
		self.desc = evtDesc
		self.uid = evtDesc.uid
		self.obj = obj
		self.struct = struct
		self.is_committed = False

	def __str__(self):
		return ("{name='%s', obj={type='%s', handle=%d}, struct=%s}" %
				(self.desc.name, self.obj.desc.name, self.obj.handle, self.struct))

	def clone(self):
		clonedEvt = self.desc.creator(self.desc, self.obj.clone(), self.struct.clone())

		return clonedEvt

	def getType(self):
		return self.desc.evtType

	def commit(self):
		if not self.is_committed:
			# Merge data of event with object
			self.obj.struct.merge(self.struct)
		self.is_committed = True

	@staticmethod
	def create(evtDesc, obj):
		# Create ObusStruct then ObusEvent object
		struct = ObusStruct.create(evtDesc.structDesc)
		return evtDesc.creator(evtDesc, obj, struct)

	def encode(self, buf):
		# Write uid and handle of object
		buf.writeU16(self.obj.uid)
		buf.writeU16(self.obj.handle)
		# Write uid of event
		buf.writeU16(self.uid)
		# Reserve room for size
		sizeMarker = buf.prepareSizeMarker()
		self.struct.encode(buf)
		buf.writeSizeMarker(sizeMarker)

	@staticmethod
	def decode(bus, buf):
		# Read uid and handle of object and event uid
		uidObj = buf.readU16()
		handle = buf.readU16()
		uidEvt = buf.readU16()

		# Get current position in buffer and size that we are supposed to read
		sizeData = buf.readU32()
		posData = buf.getPos()
		try:
			# Find object
			obj = bus.findObject(handle)
			if obj is None:
				raise DecodeError("ObusEvent.decode: " +
						"object uid=%d handle=%d not registered" %
						(uidObj, handle))

			# Check that uid is correct
			if obj.uid != uidObj:
				raise DecodeError("ObusEvent.decode: " +
						"object uid=%d handle=%d bad internal uid=%d" %
						(uidObj, handle, obj.uid))

			# Get event desc
			evtDesc = obj.desc.eventsDesc.get(uidEvt)
			if evtDesc is None:
				raise DecodeError("ObusEvent.decode: " +
						"object uid=%d can't create event uid=%d,"
						" descriptor not found" % (obj.uid, uidEvt))

			# Decode data structure
			struct = ObusStruct.decode(evtDesc.structDesc, buf)

			# Create ObusEvent object with decoded struct
			# (so we can't use ObusEvent.create method)
			evt = evtDesc.creator(evtDesc, obj, struct)

			# Check event fields decoded according to event updates desc
			for fieldDesc in evt.struct.desc.fieldsDesc.values():
				if evt.struct.hasField(fieldDesc):
					if not evtDesc.hasField(fieldDesc):
						_log.warn("object '" + \
							evt.obj.desc.name + "' (handle=" + str(evt.obj.handle) + ")"\
							" event '" + evt.desc.name + \
							"' updates undeclared " + \
							fieldDesc.getRoleStr() + " '" + \
							fieldDesc.name + "'")

			return evt

		except DecodeError as ex:
			# Try to continue
			_log.error(str(ex))
			buf.setPos(posData+sizeData)
			return None
