#===============================================================================
# obus-python - obus client python module.
#
# @file obus_busevent.py
#
# @brief obus python bus event
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
from .obus_object import ObusObject
from .obus_event import ObusEvent

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

#===============================================================================
#===============================================================================
class ObusBusEvent(object):
	def __init__(self, busEvtDesc):
		self.desc = busEvtDesc
		self.uid = busEvtDesc.uid
		self.addList = []
		self.removeList = []
		self.eventList = []
		self.dispatched = False

	def __str__(self):
		return ("{uid=%d, name=%s, add=%d, remove=%d, event=%d}" %
				(self.uid, self.desc.name, len(self.addList),
				len(self.removeList), len(self.eventList)))

	def isBaseEvent(self): # IGNORE:R0201
		return False

	def getType(self):
		return self.desc.busEvtType

	def clone(self):
		clonedEvt = ObusBusEvent.create(self.desc)

		for add in self.addList:
			clonedEvt.addList.append(add.clone())

		for remove in self.removeList:
			clonedEvt.removeList.append(remove.clone())

		for evt in self.eventList:
			clonedEvt.eventList.append(evt.clone())

		return clonedEvt

	@staticmethod
	def create(busEvtDesc):
		# Create ObusBusEvent object
		return busEvtDesc.creator(busEvtDesc)

	def encode(self, buf):
		# Write event uid
		buf.writeU16(self.uid)

		# Write numbers or registrations/unregistrations/events
		buf.writeU32(len(self.addList))
		buf.writeU32(len(self.removeList))
		buf.writeU32(len(self.eventList))

		# Encode registrations
		for obj in self.addList:
			obj.encode(buf)

		# Encode unregistrations
		for obj in self.removeList:
			buf.writeU16(obj.uid)
			buf.writeU16(obj.handle)

		# Encode events
		for evt in self.eventList:
			evt.encode(buf)

	@staticmethod
	def decode(bus, buf):
		# Read event uid
		uid = buf.readU16()
		busEvtDesc = bus.desc.eventsDesc.get(uid)
		if busEvtDesc is None:
			raise DecodeError("ObusBusEvent.decode: " +
					"can't create bus event uid=%d, descriptor not found" % uid)

		# Create ObusBusEvent object
		busEvt = ObusBusEvent.create(busEvtDesc)

		# Read numbers or registrations/unregistrations/events
		addCount = buf.readU32()
		removeCount = buf.readU32()
		eventCount = buf.readU32()

		# Decode registrations
		for _ in range(0, addCount):
			obj = ObusObject.decode(bus, buf)
			if obj is not None:
				busEvt.addList.append(obj)

		# Decode unregistrations
		for _ in range(0, removeCount):
			# Read object uid and handle
			uidObj = buf.readU16()
			handle = buf.readU16()

			# Find object
			obj = bus.findObject(handle)
			if obj is None:
				# Just log, no exception, continue loop
				_log.error("ObusBusEvent.decode: " +
						"object uid=%d handle=%d not registered",
						uidObj, handle)
			# Check that uid is correct
			elif obj.uid != uidObj:
				# Just log, no exception, continue loop
				_log.error("ObusBusEvent.decode: " +
						"object uid=%d handle=%d bad internal uid=%d",
						uidObj, handle, obj.uid)
			else:
				# Add in list
				busEvt.removeList.append(obj)

		# Decode events
		for _ in range(0, eventCount):
			evt = ObusEvent.decode(bus, buf)
			if evt is not None:
				busEvt.eventList.append(evt)

		return busEvt
