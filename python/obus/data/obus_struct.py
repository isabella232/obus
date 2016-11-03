#===============================================================================
# obus-python - obus client python module.
#
# @file obus_struct.py
#
# @brief obus python struct
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
from obus import FieldDesc
from obus import FieldType

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

#===============================================================================
# Obus structure content.
#===============================================================================
class ObusStruct(object):
	# Create a new obus structure.
	# @param structDesc : structure descriptor.
	def __init__(self, structDesc):
		self.desc = structDesc
		self.fields = {}

	def __str__(self):
		return str(self.fields)

	def clone(self):
		clonedStruct = ObusStruct(self.desc)

		for fieldDesc in self.desc.fieldsDesc.values():
			if self.hasField(fieldDesc):
				fieldValue = ObusStruct.cloneField(fieldDesc, self.getField(fieldDesc))
				clonedStruct.setField(fieldDesc, fieldValue)

		return clonedStruct

	@staticmethod
	def cloneField(fieldDesc, value):
		clonedField = None

		if (fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0:
			clonedField = []
			for val in value:
				clonedField.append(val)
		else:
			clonedField = value

		return clonedField

	# Determine if a field is present (non null).
	# @param fieldDesc : descriptor of field to query.
	# @return true if the field is present, false otherwise.
	def hasField(self, fieldDesc):
		return fieldDesc.name in self.fields

	# Get a field.
	# @param fieldDesc : descriptor of field to query.
	# @return field.
	def getField(self, fieldDesc):
		value = None

		if self.hasField(fieldDesc):
			value = self.fields[fieldDesc.name]

		return value

	# Set a field.
	# @param fieldDesc : descriptor of field to set.
	# @param val : value to set.
	def setField(self, fieldDesc, val):
		self.fields[fieldDesc.name] = val

	# Check that the structure is complete (no missing fields).
	def checkComplete(self):
		# Make sure there is no missing fields
		for fieldDesc in self.desc.fieldsDesc.values():
			if not self.hasField(fieldDesc):
				raise DecodeError("ObusStruct.checkComplete: " +
						" Missing field %s" % fieldDesc.name)

	# Merge the content of this structure with another one.
	# @param other : other structure to merge with this one.
	def merge(self, other):
		if self.desc != other.desc:
			raise Exception()

		# Merge fields
		for fieldDesc in self.desc.fieldsDesc.values():
			if other.hasField(fieldDesc):
				# Direct copy
				self.fields[fieldDesc.name] = other.fields[fieldDesc.name]

	# Create a default empty structure.
	# @param structDesc : structure descriptor.
	@staticmethod
	def create(structDesc):
		# Create ObusStruct object
		return ObusStruct(structDesc)

	# Encode the structure.
	# @param buf : output buffer.
	def encode(self, buf):
		# Get number of present fields
		fieldCount = 0
		for fieldDesc in self.desc.fieldsDesc.values():
			if self.hasField(fieldDesc):
				fieldCount += 1

		# Write number of fields, then encode fields
		buf.writeU16(fieldCount)
		for fieldDesc in self.desc.fieldsDesc.values():
			if self.hasField(fieldDesc):
				# Write field uid, type, then encode field
				buf.writeU16(fieldDesc.uid)
				buf.writeU8(fieldDesc.type)
				field = self.fields[fieldDesc.name]
				if (fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0:
					ObusStruct._encodeArray(fieldDesc, buf, field)
				else:
					ObusStruct._encodeField(fieldDesc, buf, field)

	# Decode a structure.
	# @param structDesc : structure descriptor.
	# @param buf : input buffer.
	# @return decoded structure.
	@staticmethod
	def decode(structDesc, buf):
		# Create ObusStruct
		struct = ObusStruct.create(structDesc)

		# Read number of fields
		fieldCount = buf.readU16()
		for _ in range(0, fieldCount):
			# Read field uid and type
			uid = buf.readU16()
			fieldType = buf.readU8()

			# Get field descriptor from uid
			fieldDesc = structDesc.fieldsDesc.get(uid)
			if fieldDesc is None:
				# The uid is not known to us, skip this field
				_log.warning("ObusStruct.decode: " +
						"Can't decode field uid=%d, descriptor not found", uid)
				if (fieldType&FieldType.OBUS_FIELD_ARRAY) != 0:
					ObusStruct._skipArray(fieldType, buf)
				else:
					ObusStruct._skipField(fieldType, buf)
			elif fieldDesc.type != fieldType:
				# The uid is known but with another type, skip this field
				_log.warning("ObusStruct.decode: " +
						"Can't decode field uid=%d, type mismatch (descriptor:%d decoded=%d)",
						uid, fieldDesc.type, fieldType)
				if (fieldType&FieldType.OBUS_FIELD_ARRAY) != 0:
					ObusStruct._skipArray(fieldType, buf)
				else:
					ObusStruct._skipField(fieldType, buf)
			else:
				# Decode field
				if (fieldDesc.type&FieldType.OBUS_FIELD_ARRAY) != 0:
					field = ObusStruct._decodeArray(fieldDesc, buf)
					struct.fields[fieldDesc.name] = field
				else:
					field = ObusStruct._decodeField(fieldDesc, buf)
					struct.fields[fieldDesc.name] = field

		return struct

	@staticmethod
	def _encodeField(fieldDesc, buf, val):
		if (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_BOOL:
			buf.writeU8(1 if val is True else 0)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U8:
			buf.writeU8(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I8:
			buf.writeI8(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U16:
			buf.writeU16(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I16:
			buf.writeI16(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U32:
			buf.writeU32(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I32:
			buf.writeI32(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U64:
			buf.writeU64(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I64:
			buf.writeI64(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_STRING:
			buf.writeString(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_ENUM:
			buf.writeI32(fieldDesc.driver.toInt(val))
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_F32:
			buf.writeF32(val)
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_F64:
			buf.writeF64(val)
		else:
			# Should not occur, as we known our own types
			raise ValueError("ObusStruct._encodeField: " +
					"Unknown field type")

	@staticmethod
	def _decodeField(fieldDesc, buf):
		if (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_BOOL:
			return True if buf.readU8() == 1 else False
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U8:
			return buf.readU8()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I8:
			return buf.readI8()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U16:
			return buf.readU16()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I16:
			return buf.readI16()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U32:
			return buf.readU32()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I32:
			return buf.readI32()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U64:
			return buf.readU64()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I64:
			return buf.readI64()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_STRING:
			return buf.readString()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_ENUM:
			return fieldDesc.driver.fromInt(buf.readI32())
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_F32:
			return buf.readF32()
		elif (fieldDesc.type&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_F64:
			return buf.readF64()
		else:
			raise DecodeError("ObusStruct._decodeField: " +
					"Unknown field type: %d" % fieldDesc.type&FieldType.OBUS_FIELD_MASK)

	@staticmethod
	def _skipField(fieldType, buf):
		if (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_BOOL:
			buf.readU8()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U8:
			buf.readU8()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I8:
			buf.readI8()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U16:
			buf.readU16()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I16:
			buf.readI16()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U32:
			buf.readU32()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I32:
			buf.readI32()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_U64:
			buf.readU64()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_I64:
			buf.readI64()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_STRING:
			buf.readString()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_ENUM:
			buf.readI32()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_F32:
			buf.readF32()
		elif (fieldType&FieldType.OBUS_FIELD_MASK) == FieldType.OBUS_FIELD_F64:
			buf.readF64()
		else:
			raise DecodeError("ObusStruct._skipField: " +
					"Unknown field type: %d" % fieldType&FieldType.OBUS_FIELD_MASK)

	# Encode a field that is an array.
	# @param fieldDesc : descriptor of field to encode.
	# @param buf : output buffer.
	# @param array : array to  encode.
	@staticmethod
	def _encodeArray(fieldDesc, buf, array):
		# Write number of items, then encode items
		buf.writeU32(len(array))
		for item in array:
			ObusStruct._encodeField(fieldDesc, buf, item)

	# Decode a field that is an array.
	# @param fieldDesc : descriptor of field to decode.
	# @param buf : input buffer.
	# @return decoded array as a simple Object.
	@staticmethod
	def _decodeArray(fieldDesc, buf):
		# Read number of items
		# TODO: check that value is reasonable
		itemCount = buf.readU32()

		# Decode items
		array = []
		for _ in range(0, itemCount):
			item = ObusStruct._decodeField(fieldDesc, buf)
			array.append(item)
		return array

	@staticmethod
	def _skipArray(fieldType, buf):
		# Read number of items
		# TODO: check that value is reasonable
		itemCount = buf.readU32()

		# Skip items
		for _ in range(0, itemCount):
			ObusStruct._skipField(fieldType, buf)
