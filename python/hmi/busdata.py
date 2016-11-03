#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file busdata.py
#
# @brief wxpython obus client bus data
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

import os
import obus
import ast

#===============================================================================
#===============================================================================
class BusInfo(object):
	def __init__(self, xmlFile, addr, enabled, autoReload, primaryFields):
		self.xmlFile = xmlFile
		self.addr = addr
		self.enabled = enabled
		self.autoReload = autoReload
		if primaryFields:
			self.primaryFields = ast.literal_eval(primaryFields)
		else:
			self.primaryFields = {}

#===============================================================================
#===============================================================================
class BusItf(obus.BusEventCb, obus.ObjectCb):
	def __init__(self, mainFrame, busInfo):
		obus.BusEventCb.__init__(self)
		obus.ObjectCb.__init__(self)
		# Save parameters
		self._mainFrame = mainFrame
		self.busInfo = busInfo
		# Load bus data
		try:
			self._status = "disconnected"
			self._bus = obus.loadBus(busInfo.xmlFile)
		except obus.DecodeError as ex:
			self._bus = None
			self._client = None
			self._status = str(ex)
		else:
			# Create obus client
			self._client = obus.Client(mainFrame.GetTitle(), self.getBusDesc(), self)
			for objDesc in self.getObjectsDesc():
				self._client.registerObjectCb(objDesc, self)
		# To store Ids in TreeCtrl
		self.treeIdBus = None
		self.treeIdObj = {}

	def getName(self):
		if self._bus is not None:
			return self._bus.BUS_DESC.name
		else:
			return os.path.basename(self.busInfo.xmlFile)

	def getStatusStr(self):
		return self._status

	def start(self):
		if self._client is not None:
			self._status = "connecting"
			self._client.start(self.busInfo.addr)

	def stop(self):
		if self._client is not None:
			self._client.stop()
			self._status = "disconnected"

	def canStart(self):
		return (self._client is not None) and (not self._client.isStarted())

	def canStop(self):
		return (self._client is not None) and (self._client.isStarted())

	def isStarted(self):
		return (self._client is not None) and (self._client.isStarted())

	def dispatchBusEvent(self, busEvt):
		self._client.dispatchBusEvent(busEvt)

	def commitObjectEvent(self, objEvt):
		self._client.commitObjectEvent(objEvt)

	def sendMethodCall(self, call):
		self._client.sendMethodCall(call)

	def findObject(self, handle):
		return self._client.findObject(handle)

	def getAllObjects(self):
		return self._client.getAllObjects()

	def getObjects(self, objDesc):
		return self._client.getObjects(objDesc)

	def getBusDesc(self):
		return None if self._bus is None else self._bus.BUS_DESC

	def getObjectsDesc(self):
		return [] if self._bus is None else self._bus.BUS_DESC.objectsDesc.values()

	def onBusEvent(self, busEvt):
		if busEvt.isBaseEvent():
			if busEvt.getType() == obus.BusEventBase.Type.CONNECTED:
				self._status = "connected"
			elif busEvt.getType() == obus.BusEventBase.Type.DISCONNECTED:
				self._status = "connecting"
			elif busEvt.getType() == obus.BusEventBase.Type.CONNECTION_REFUSED:
				self._status = "refused"
		self._mainFrame.onBusEvent(self, busEvt)

	def onObjectAdd(self, obj, busEvt):
		self._mainFrame.onObjectAdd(self, obj, busEvt)

	def onObjectRemove(self, obj, busEvt):
		self._mainFrame.onObjectRemove(self, obj, busEvt)

	def onObjectEvent(self, objEvt, busEvt):
		self._mainFrame.onObjectEvent(self, objEvt, busEvt)

	def onObjectCallAck(self, call):
		self._mainFrame.onObjectCallAck(self, call)
