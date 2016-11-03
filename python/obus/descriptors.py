#===============================================================================
# obus-python - obus client python module.
#
# @file descriptors.py
#
# @brief obus python descriptors
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

import obus
from obus import DecodeError

#===============================================================================
# Type of field (vales part of protocol).
#===============================================================================
class FieldType:
	OBUS_FIELD_U8 = 0
	OBUS_FIELD_I8 = 1
	OBUS_FIELD_U16 = 2
	OBUS_FIELD_I16 = 3
	OBUS_FIELD_U32 = 4
	OBUS_FIELD_I32 = 5
	OBUS_FIELD_U64 = 6
	OBUS_FIELD_I64 = 7
	OBUS_FIELD_ENUM = 8
	OBUS_FIELD_STRING = 9
	OBUS_FIELD_BOOL = 10
	OBUS_FIELD_F32 = 11
	OBUS_FIELD_F64 = 12
	OBUS_FIELD_ARRAY = (1 << 7)
	OBUS_FIELD_MASK = (0x7f)

#===============================================================================
# Role (Not part of protocol).
#===============================================================================
class FieldRole:
	OBUS_PROPERTY = 0
	OBUS_METHOD = 1
	OBUS_ARGUMENT = 2

#===============================================================================
# Field descriptor.
#===============================================================================
class FieldDesc(object):
	def __init__(self, name, uid, type, role, driver, rawType):
		self.name = name       # Field name
		self.uid = uid         # Field uid
		self.type = type       # Field type
		self.role = role       # Field role
		self.driver = driver   # Field driver for enum fields
		self.rawType = rawType # Raw type (from xml)

	def getRoleStr(self):
		if self.role == FieldRole.OBUS_PROPERTY:
			return "property"
		elif self.role == FieldRole.OBUS_METHOD:
			return "method"
		elif self.role == FieldRole.OBUS_ARGUMENT:
			return "argument"
		else:
			return "<invalid>"

	def isEnum(self):
		return self.type == obus.FieldType.OBUS_FIELD_ENUM

	def isEnumItemValid(self, value):
		return self.isEnum() and self.driver.hasItem(value)

#===============================================================================
# Struct descriptor.
#===============================================================================
class StructDesc(object):
	def __init__(self):
		self.fieldsDesc = {} # Fields descriptors

	def addField(self, fieldDesc):
		if fieldDesc.uid in self.fieldsDesc:
			raise KeyError()
		self.fieldsDesc[fieldDesc.uid] = fieldDesc

	def findField(self, name):
		for fieldDesc in self.fieldsDesc.values():
			if fieldDesc.name == name:
				return fieldDesc
		return None

#===============================================================================
# EventUpdate descriptor.
#===============================================================================
class EventUpdateDesc(object):
	def __init__(self, fieldDesc, flags):
		self.fieldDesc = fieldDesc       # update field desc
		self.flags = flags               # update flags

#===============================================================================
# Event descriptor.
#===============================================================================
class EventDesc(object):
	def __init__(self, name, uid, evtType, structDesc, creator):
		self.updatesDesc = {}        # Event update Fields descriptors
		self.name = name             # Event name
		self.uid = uid               # Event uid
		self.evtType = evtType       # Event type
		self.structDesc = structDesc # Event struct descriptor
		self.creator = creator       # Creator of specific ObusEvent objects

	def addUpdateDesc(self, updateDesc):
		if updateDesc.fieldDesc.uid in self.updatesDesc:
			raise KeyError()
		self.updatesDesc[updateDesc.fieldDesc.uid] = updateDesc

	def hasField(self, fieldDesc):
		if fieldDesc.uid in self.updatesDesc:
			return True
		else:
			return False

#===============================================================================
# Method descriptor.
#===============================================================================
class MethodDesc(object):
	def __init__(self, name, uid, structDesc, creator):
		self.name = name             # Method name
		self.uid = uid               # Method uid
		self.structDesc = structDesc # Method struct descriptor
		self.creator = creator       # Creator of specific ObusCall objects

#===============================================================================
# Object descriptor.
#===============================================================================
class ObjectDesc(object):
	def __init__(self, name, uid, structDesc, creator):
		self.name = name             # Object type name
		self.uid = uid               # Object type uid
		self.primaryFieldUid = -1    # Object primary field uid
		self.structDesc = structDesc # Object struct descriptor
		self.creator = creator       # Creator of specific ObusObject objects
		self.eventsDesc = {}         # Object events descriptors
		self.methodsDesc = {}        # Object methods descriptors

	def addEvent(self, evtDesc):
		if evtDesc.uid in self.eventsDesc:
			raise KeyError()
		self.eventsDesc[evtDesc.uid] = evtDesc

	def addMethod(self, mtdDesc):
		if mtdDesc.uid in self.methodsDesc:
			raise KeyError()
		self.methodsDesc[mtdDesc.uid] = mtdDesc

#===============================================================================
# Bus event descriptor.
#===============================================================================
class BusEventDesc(object):
	def __init__(self, name, uid, busEvtType, creator):
		self.name = name             # Bus event name
		self.uid = uid               # Bus event uid
		self.busEvtType = busEvtType # Bus event type
		self.creator = creator       # Creator of specific ObusBusEvent objects

#===============================================================================
# Bus descriptor.
#===============================================================================
class BusDesc(object):
	def __init__(self, name, crc):
		self.name = name      # bus name
		self.crc = crc        # bus crc32
		self.objectsDesc = {} # bus objects type descriptors
		self.eventsDesc = {}  # bus events type descriptors

	def addObject(self, objDesc):
		if objDesc.uid in self.objectsDesc:
			raise KeyError()
		self.objectsDesc[objDesc.uid] = objDesc

	def addEvent(self, busEvtDesc):
		if busEvtDesc.uid in self.eventsDesc:
			raise KeyError()
		self.eventsDesc[busEvtDesc.uid] = busEvtDesc

#===============================================================================
# Generic enum field driver.
#===============================================================================
class FieldDriverEnum(object):
	def __init__(self, enum, defVal):
		self._enum = enum
		self._defVal = defVal

	def init(self):
		return self._defVal

	def hasItem(self, val):
		return self._enum.hasItem(val)

	def toInt(self, val):
		return self._enum.toInt(val)

	def fromInt(self, val):
		try:
			return self._enum.fromInt(val)
		except KeyError:
			raise DecodeError("Invalid %s: %d" % (self._enum.getName(), val))

	def format(self, val):
		return str(val)

#===============================================================================
#===============================================================================
FIELD_DRIVER_METHOD_STATE = FieldDriverEnum(
		obus.MethodState, obus.MethodState.NOT_SUPPORTED)
