#===============================================================================
# obus-python - obus client python module.
#
# @file __init__.py
#
# @brief obus python init
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

#===============================================================================
# Decode error exception.
#===============================================================================
class DecodeError(Exception):
	pass

#===============================================================================
#===============================================================================
class Enum(dict):
	def __init__(self, enumName, items):
		dict.__init__(self, items)
		self.__dict__["_enumName"] = enumName
	# Look in map if member is present
	def __getattr__(self, name):
		if name in self:
			return name
		raise AttributeError("%s: invalid attribute '%s'" % (self.getName(), name))

	# Prevent modification of internal map
	def __setattr__(self, name, val):
		raise AttributeError

	def getName(self):
		return self.__dict__["_enumName"]

	def hasItem(self, val):
		return val in self

	def fromInt(self, val):
		for key in self:
			if self[key] == val:
				return key
		raise KeyError()

	def toInt(self, val):
		return self[val]

#===============================================================================
# obus bus event callback.
#===============================================================================
class BusEventCb(object): # IGNORE:R0921
	def onBusEvent(self, busEvt):
		raise NotImplementedError()

#===============================================================================
# obus object callback.
#===============================================================================
class ObjectCb(object): # IGNORE:R0921
	def onObjectAdd(self, obj, busEvt):
		raise NotImplementedError()
	def onObjectRemove(self, obj, busEvt):
		raise NotImplementedError()
	def onObjectEvent(self, evt, busEvt):
		raise NotImplementedError()
	def onObjectCallAck(self, call):
		raise NotImplementedError()

#===============================================================================
#===============================================================================

OBUS_OBJECT_LOG = False

OBUS_INVALID_HANDLE = 0

# obus method state (values part of protocol)
MethodState = Enum("MethodState", { # IGNORE:C0103
		"NOT_SUPPORTED": 0, # Method is not supported
		"ENABLED": 1,       # Method is enabled
		"DISABLED": 2,      # Method is disabled
	})

# obus call acknowledge status (values part of protocol)
CallAck = Enum("CallAck", { # IGNORE:C0103
		"INVALID": 0,              # Used when ack is not set yet
		"ACKED": 1,                # Call acknowledge
		"ABORTED": 2,              # Call aborted (object removed...)
		"METHOD_DISABLED": 3,      # Method is disabled
		"METHOD_NOT_SUPPORTED": 4, # Method is not supported
		"INVALID_ARGUMENTS": 5,    # Invalid call arguments
		"REFUSED": 6,              # Call refused by server ...
})

from .descriptors import * # IGNORE:W0401
from .xmlbus import loadBus
from .buseventbase import BusEventBase
from .client import Client
