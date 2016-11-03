#===============================================================================
# obus-python - obus client python module.
#
# @file buseventbase.py
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

from obus import Enum
from obus import BusEventDesc
from obus.data import ObusBusEvent

#===============================================================================
#===============================================================================
class BusEventBase(ObusBusEvent):
	# obus event base
	Type = Enum("BusEventBase.Type", {
			"CONNECTED": 0,            # Connected to bus
			"DISCONNECTED": 1,         # Disconnected from bus
			"CONNECTION_REFUSED": 2,   # Connection refused
			"OBJECT_REGISTERED": 3,    # Object registered
			"OBJECT_UNREGISTERED": 4,  # Object unregistered
			"OBJECT_EVENT": 5,          # Object event raised
	})

	def __init__(self, busEvtDesc):
		ObusBusEvent.__init__(self, busEvtDesc)

	def isBaseEvent(self):
		return True

	# Create a 'CONNECTED' bus event.
	@staticmethod
	def createConnected(objects):
		busEvt = ObusBusEvent.create(_busEventDesc_connected)
		busEvt.addList.extend(objects)
		return busEvt

	# Create a 'DISCONNECTED' bus event.
	@staticmethod
	def createDisconnected(objects):
		busEvt = ObusBusEvent.create(_busEventDesc_disconnected)
		busEvt.removeList.extend(objects)
		return busEvt

	# Create a 'CONNECTION_REFUSED' bus event.
	@staticmethod
	def createConnectionRefused():
		busEvt = ObusBusEvent.create(_busEventDesc_connection_refused)
		return busEvt

	# Create a 'OBJECT_REGISTERED' bus event.
	@staticmethod
	def createObjectRegistered(obj):
		busEvt = ObusBusEvent.create(_busEventDesc_object_registered)
		busEvt.addList.append(obj)
		return busEvt

	# Create a 'OBJECT_UNREGISTERED' bus event.
	@staticmethod
	def createObjectUnregistered(obj):
		busEvt = ObusBusEvent.create(_busEventDesc_object_unregistered)
		busEvt.removeList.append(obj)
		return busEvt

	# Create a 'OBJECT_EVENT' bus event.
	@staticmethod
	def createObjectEvent(evt):
		busEvt = ObusBusEvent.create(_busEventDesc_object_event)
		busEvt.eventList.append(evt)
		return busEvt

#===============================================================================
# Fake uid (not actually used to encode/decode)
#===============================================================================

_EVT_CONNECTED_UID = -1
_EVT_DISCONNECTED_UID = -2
_EVT_CONNECTION_REFUSED_UID = -3
_EVT_OBJECT_REGISTERED_UID = -4
_EVT_OBJECT_UNREGISTERED_UID = -5
_EVT_OBJECT_EVENT_UID = -6

#===============================================================================
# Bus event descriptors
#===============================================================================

_busEventDesc_connected = BusEventDesc(
		"connected",
		_EVT_CONNECTED_UID,
		BusEventBase.Type.CONNECTED,
		BusEventBase)

_busEventDesc_disconnected = BusEventDesc(
		"disconnected",
		_EVT_DISCONNECTED_UID,
		BusEventBase.Type.DISCONNECTED,
		BusEventBase)

_busEventDesc_connection_refused = BusEventDesc(
		"connection_refused",
		_EVT_CONNECTION_REFUSED_UID,
		BusEventBase.Type.CONNECTION_REFUSED,
		BusEventBase)

_busEventDesc_object_registered = BusEventDesc(
		"object_registered",
		_EVT_OBJECT_REGISTERED_UID,
		BusEventBase.Type.OBJECT_REGISTERED,
		BusEventBase)

_busEventDesc_object_unregistered = BusEventDesc(
		"object_unregistered",
		_EVT_OBJECT_UNREGISTERED_UID,
		BusEventBase.Type.OBJECT_UNREGISTERED,
		BusEventBase)

_busEventDesc_object_event = BusEventDesc(
		"object_event",
		_EVT_OBJECT_EVENT_UID,
		BusEventBase.Type.OBJECT_EVENT,
		BusEventBase)
