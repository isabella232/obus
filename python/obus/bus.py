#===============================================================================
# obus-python - obus client python module.
#
# @file bus.py
#
# @brief obus bus client
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

#===============================================================================
#===============================================================================
class Bus(object):
	def __init__(self, busDesc):
		self.desc = busDesc
		self.objects = {}

	def clone(self):
		clonedBus = Bus(self.desc)

		for key, value in self.objects.iteritems():
			clonedBus.objects[key] = value.clone()

		return clonedBus

	def registerObject(self, obj):
		if obj.handle == obus.OBUS_INVALID_HANDLE:
			raise KeyError("object uid=%d invalid handle" % obj.uid)
		if obj.handle in self.objects:
			raise KeyError("object uid=%d handle=%d already registered" %
					(obj.uid, obj.handle))
		# Add in table
		self.objects[obj.handle] = obj

	def unregisterObject(self, obj):
		if obj.handle not in self.objects:
			raise KeyError("object uid=%d handle=%d not registered" %
					(obj.uid, obj.handle))
		# Remove from table
		del self.objects[obj.handle]

	def findObject(self, handle):
		return self.objects.get(handle, None)

	def getAllObjects(self):
		return self.objects.values()

	def getObjects(self, objDesc):
		return [obj for obj in self.objects.values() if obj.uid == objDesc.uid]
