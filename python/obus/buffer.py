#===============================================================================
# obus-python - obus client python module.
#
# @file buffer.py
#
# @brief obus buffer client
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
import struct
from cStringIO import StringIO

#===============================================================================
#===============================================================================
class Buffer(object):
	def __init__(self):
		self.data = StringIO()

	def __len__(self):
		pos = self.data.tell()
		self.data.seek(0, os.SEEK_END)
		size = self.data.tell()
		self.data.seek(pos, os.SEEK_SET)
		return size

	def getData(self):
		return self.data.getvalue()

	def getPos(self):
		return self.data.tell()

	def setPos(self, pos):
		self.data.seek(pos, os.SEEK_SET)

	def prepareSizeMarker(self):
		curPos = self.getPos()
		self.writeU32(0)
		return curPos

	def writeSizeMarker(self, marker):
		curPos = self.getPos()
		self.setPos(marker)
		self.writeU32(curPos - marker - 4)
		self.setPos(curPos)

	def skip(self, count):
		self.data.seek(count, os.SEEK_CUR)

	def rewind(self):
		self.data.seek(0, os.SEEK_SET)

	def writeU8(self, val):
		self.writeBuf(struct.pack(">B", val))

	def writeI8(self, val):
		self.writeBuf(struct.pack(">b", val))

	def writeU16(self, val):
		self.writeBuf(struct.pack(">H", val))

	def writeI16(self, val):
		self.writeBuf(struct.pack(">h", val))

	def writeU32(self, val):
		self.writeBuf(struct.pack(">I", val))

	def writeI32(self, val):
		self.writeBuf(struct.pack(">i", val))

	def writeU64(self, val):
		self.writeBuf(struct.pack(">Q", val))

	def writeI64(self, val):
		self.writeBuf(struct.pack(">q", val))

	def writeString(self, string):
		if string is None:
			buf = struct.pack(">I", 0)
		else:
			if isinstance(string, unicode):
				string = str(string)
			buf = struct.pack(">I%dsB" % len(string), len(string) + 1, string, 0)
		self.writeBuf(buf)

	def writeF32(self, val):
		self.writeBuf(struct.pack(">f", val))

	def writeF64(self, val):
		self.writeBuf(struct.pack(">d", val))

	def writeBuf(self, buf):
		self.data.write(buf)

	def readU8(self):
		return struct.unpack(">B", self.data.read(1))[0]

	def readI8(self):
		return struct.unpack(">b", self.data.read(1))[0]

	def readU16(self):
		return struct.unpack(">H", self.data.read(2))[0]

	def readI16(self):
		return struct.unpack(">h", self.data.read(2))[0]

	def readU32(self):
		return struct.unpack(">I", self.data.read(4))[0]

	def readI32(self):
		return struct.unpack(">i", self.data.read(4))[0]

	def readU64(self):
		return struct.unpack(">Q", self.data.read(8))[0]

	def readI64(self):
		return struct.unpack(">q", self.data.read(8))[0]

	def readString(self):
		size = self.readU32()
		if size > 0:
			return struct.unpack(">%dsB" % (size - 1), self.readBuf(size))[0]
		else:
			return None

	def readF32(self):
		return struct.unpack(">f", self.data.read(4))[0]

	def readF64(self):
		return struct.unpack(">d", self.data.read(8))[0]

	def readBuf(self, count):
		return self.data.read(count)
