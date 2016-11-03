#===============================================================================
# obus-python - obus client python module.
#
# @file descriptors.py
#
# @brief obus python client
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
from .bus import Bus
from .transport import PacketHandler
from .transport import Transport
from .buseventbase import BusEventBase
from . import protocol

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

# Connection state
_STATE_IDLE = 0
_STATE_CONNECTING = 1
_STATE_CONNECTED = 2
_STATE_DISCONNECTING = 3
_STATE_DISCONNECTED = 4
_STATE_REFUSED = 5
_stateStr = {
	_STATE_IDLE: "IDLE",
	_STATE_CONNECTING: "CONNECTING",
	_STATE_CONNECTED: "CONNECTED",
	_STATE_DISCONNECTING: "DISCONNECTING",
	_STATE_DISCONNECTED: "DISCONNECTED",
	_STATE_REFUSED: "REFUSED",
}
def _getStateStr(val):
	return _stateStr.get(val, "UNKNOWN(%d)" % val)

#===============================================================================
#===============================================================================
class Client(PacketHandler):
	def __init__(self, name, busDesc, busEventCb):
		PacketHandler.__init__(self)
		self.busEventCb = busEventCb # Bus event callback
		self.name = name             # Client name
		self.bus = Bus(busDesc)      # Client bus
		self.state = _STATE_IDLE     # Client connection status
		self.transport = None        # Transport layer
		self.handleCall = 1          # Next call handle
		self.pendingCalls = {}       # Pending method calls
		self.objCbs = {}             # Object callbacks

	# Start the client.
	# @param addr : address to connect to.
	def start(self, addr):
		if self.state != _STATE_IDLE:
			raise Exception()
		self.state = _STATE_CONNECTING
		self.transport = Transport()
		self.transport.start(self, addr)

	# Stop the client.
	def stop(self):
		if self.state != _STATE_IDLE:
			self.transport.stop()
			self._disconnect()
			self.state = _STATE_IDLE
			self.transport = None

	def setAutoreconnect(self, reconnect):
		if self.transport != None:
			self.transport.setAutoreconnect(reconnect)

	# Determine if the client is connected (in obus way, not only socket).
	# @return true if socket is connected AND protocol is established.
	def isConnected(self):
		return (self.state == _STATE_CONNECTED)

	def isStarted(self):
		return (self.transport is not None)

	def registerObjectCb(self, objDesc, objCb):
		if objDesc.uid in self.objCbs:
			raise Exception("callback for object uid=%d already registered" %
					objDesc.uid)
		self.objCbs[objDesc.uid] = objCb

	def unregisterObjectCb(self, objDesc):
		if objDesc.uid not in self.objCbs:
			raise Exception("callback for object uid=%d not registered" %
					objDesc.uid)
		del self.objCbs[objDesc.uid]

	def cloneBus(self):
		return self.bus.clone()

	def findObjectCb(self, objDesc):
		return self.objCbs.get(objDesc.uid)

	def commitObjectEvent(self, objEvt): # IGNORE:R0201
		objEvt.commit()

	def findObject(self, handle):
		return self.bus.findObject(handle)

	def getAllObjects(self):
		return self.bus.getAllObjects()

	def getObjects(self, objDesc):
		return self.bus.getObjects(objDesc)

	def callMethod(self, obj, methodName, **kwargs):
		namespace = {"obj": obj}
		data = "callMethod = obj.createCall_%s" % (methodName)
		exec data in namespace
		callMethod = namespace["callMethod"]

		call = callMethod(**kwargs)
		self.sendMethodCall(call)

		return call

	def sendMethodCall(self, call):
		# Setup handle for call
		self.handleCall += 1
		if self.handleCall >= 65536:
			self.handleCall = 1
		call.handle = self.handleCall
		self.pendingCalls[call.handle] = call

		# Create packet and send it
		try:
			packet = protocol.TxPacketCall(call)
		except StandardError as ex:
			# Remove from list if encoding failed
			del self.pendingCalls[call.handle]
			raise ex
		self.transport.writePacket(packet)
		return call.handle

	def dispatchBusEvent(self, busEvt):
		# Make sure dispatch was not already done
		if busEvt.dispatched:
			return

		# Set the flag now to be sure further callback don't dispatch again
		busEvt.dispatched = True

		# Dispatch registrations
		for obj in busEvt.addList:
			try:
				if obus.OBUS_OBJECT_LOG:
					_log.info("add object: %s", obj)

				# Register object in bus
				self.bus.registerObject(obj)

				# Call object callback add
				objCb = self.findObjectCb(obj.desc)
				if objCb is not None:
					objCb.onObjectAdd(obj, busEvt)
			except KeyError as ex:
				_log.error(ex)

		# Dispatch events
		for evt in busEvt.eventList:
			if obus.OBUS_OBJECT_LOG:
				_log.info("event object: %s", evt)

			# Call object callback event
			objCb = self.findObjectCb(evt.obj.desc)
			if objCb is not None:
				objCb.onObjectEvent(evt, busEvt)

			# Commit event if not done by user
			evt.commit()
			if obus.OBUS_OBJECT_LOG:
				_log.info("updated object: %s", evt.obj)

		# Dispatch unregistrations
		for obj in busEvt.removeList:
			try:
				if obus.OBUS_OBJECT_LOG:
					_log.info("remove object: %s", obj)

				# Call object callback remove
				objCb = self.findObjectCb(obj.desc)
				if objCb is not None:
					objCb.onObjectRemove(obj, busEvt)

				# Unregister object from bus
				self._removePendingCalls(obj)
				self.bus.unregisterObject(obj)
			except KeyError as ex:
				_log.error(ex)

	def _notifyBusEvent(self, busEvt):
		# Notify user
		self.busEventCb.onBusEvent(busEvt)

		# Dispatch if not already done
		if not busEvt.dispatched:
			self.dispatchBusEvent(busEvt)

	def _disconnect(self):
		oldState = self.state
		self.state = _STATE_DISCONNECTING

		# Notify client of connection lost
		if oldState == _STATE_CONNECTED:
			busEvt = BusEventBase.createDisconnected(self.bus.getAllObjects())
			self._notifyBusEvent(busEvt)

		# Reconnection always enabled
		self.state = _STATE_CONNECTING

	def _removePendingCalls(self, obj):
		# Iterate over a copy of values as we will modify the list
		for call in self.pendingCalls.values():
			if call.obj == obj:
				_log.warning("Object %d:%d removed with pending call handle=%d" %
						(obj.uid, obj.handle, call.handle))
				# Send a fake ack to user
				call.ack = obus.CallAck.ABORTED
				objCb = self.findObjectCb(call.obj.desc)
				if objCb is not None:
					objCb.onObjectCallAck(call)

				# Remove from list
				del self.pendingCalls[call.handle]

	def _sendConnectionRequest(self):
		# Create a TxPacketConnReq packet and send it
		packet = protocol.TxPacketConnReq(self.name,
				self.bus.desc.name, self.bus.desc.crc)
		self.transport.writePacket(packet)

	def _recvConnectionResponse(self, packet):
		_log.info("server connection response: %s (%d objects)",
				protocol.getConnRespStatusStr(packet.status), len(packet.objects))

		# Check connection state
		if self.state != _STATE_CONNECTING:
			_log.warning("ignoring connection response in %s state",
					_getStateStr(self.state))
			return

		# Handle connection refused
		if packet.status != protocol.OBUS_CONRESP_STATUS_ACCEPTED:
			self.state = _STATE_REFUSED
			busEvt = BusEventBase.createConnectionRefused()
			self._notifyBusEvent(busEvt)
			return

		# Connection succeed
		self.state = _STATE_CONNECTED
		busEvt = BusEventBase.createConnected(packet.objects)
		self._notifyBusEvent(busEvt)

	def _recvAdd(self, packet):
		# Create a 'object_register' bus event and notify it
		busEvt = BusEventBase.createObjectRegistered(packet.obj)
		self._notifyBusEvent(busEvt)

	def _recvRemove(self, packet):
		# Create a 'object_unregister' bus event and notify it
		busEvt = BusEventBase.createObjectUnregistered(packet.obj)
		self._notifyBusEvent(busEvt)

	def _recvBusEvent(self, packet):
		self._notifyBusEvent(packet.busEvt)

	def _recvEvent(self, packet):
		# Create a 'object_event' bus event and notify it
		busEvt = BusEventBase.createObjectEvent(packet.evt)
		self._notifyBusEvent(busEvt)

	def _recvAck(self, packet):
		# Retrieve pending call
		call = self.pendingCalls.get(packet.handle)
		if call is None:
			_log.warning("Received ack for not pending call handle=%d", packet.handle)
		else:
			# Save ack in call
			call.ack = packet.ack
			if obus.OBUS_OBJECT_LOG:
				_log.info("call ack: %s", call)

			# Call object callback ack
			objCb = self.findObjectCb(call.obj.desc)
			if objCb is not None:
				objCb.onObjectCallAck(call)
			del self.pendingCalls[call.handle]

	def onConnected(self):
		_log.info("client %s socket connected", self.name)
		# Send bus connection request
		self._sendConnectionRequest()

	def onDisconnected(self):
		_log.info("client %s socket disconnected", self.name)
		self._disconnect()

	# Packet handlers
	_rxPacketHandlers = {
		protocol.OBUS_PKT_CONRESP: _recvConnectionResponse,
		protocol.OBUS_PKT_ADD: _recvAdd,
		protocol.OBUS_PKT_REMOVE: _recvRemove,
		protocol.OBUS_PKT_BUS_EVENT: _recvBusEvent,
		protocol.OBUS_PKT_EVENT: _recvEvent,
		protocol.OBUS_PKT_ACK: _recvAck,
	}

	def recvPacket(self, rawPacket):
		# Finish decoding in main thread (to synchronize with bus)
		packet = rawPacket.decode(self.bus)
		if packet is not None:
			if packet.header.packetType not in self._rxPacketHandlers:
				_log.warning("Client: unhandled packet type: '%s'",
						protocol.getPacketTypeStr(packet.header.packetType))
			else:
				self._rxPacketHandlers[packet.header.packetType](self, packet)
