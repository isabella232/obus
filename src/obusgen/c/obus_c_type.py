#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c_type.py
#
# @brief obus c type code generator
#
# @author jean-baptiste.dubois@parrot.com
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

from obusparser import ObusException
from obusparser import ObusType
from obus_c_enum import ObusEnumWriter

#===============================================================================
# get type.
#===============================================================================
def getType(t):
	# drivers dict
	ctypes = {
		ObusType.Type.INT8 : 'int8_t ' ,
		ObusType.Type.UINT8 : 'uint8_t ',
		ObusType.Type.INT16 : 'int16_t ',
		ObusType.Type.UINT16 : 'uint16_t ',
		ObusType.Type.INT32 : 'int32_t ',
		ObusType.Type.UINT32 : 'uint32_t ',
		ObusType.Type.INT64 : 'int64_t ',
		ObusType.Type.UINT64 : 'uint64_t ',
		ObusType.Type.STRING : 'const char *',
		ObusType.Type.HANDLE : 'obus_handle_t ',
		ObusType.Type.BOOL : 'obus_bool_t ',
		ObusType.Type.FLOAT : 'float ',
		ObusType.Type.DOUBLE : 'double '}

	s = ''
	if t.base in ctypes:
		s = ctypes[t.base]
	elif t.base == ObusType.Type.ENUM:
		s = 'enum {0} '.format(ObusEnumWriter(t.enum).getName())
	else:
		raise ObusException('type {0} not supported'.format(t))

	if t.isArray():
		if t.base == ObusType.Type.STRING:
			s = 'const char *const *'
		else:
			s = 'const ' + s + '*'

	return s

#===============================================================================
# get libobus type.
#===============================================================================
def getLibobusType(t):
	# libobus dict
	types = {
		ObusType.Type.INT8 : 'OBUS_FIELD_I8' ,
		ObusType.Type.UINT8 : 'OBUS_FIELD_U8',
		ObusType.Type.INT16 : 'OBUS_FIELD_I16',
		ObusType.Type.UINT16 : 'OBUS_FIELD_U16',
		ObusType.Type.INT32 : 'OBUS_FIELD_I32',
		ObusType.Type.UINT32 : 'OBUS_FIELD_U32',
		ObusType.Type.INT64 : 'OBUS_FIELD_I64',
		ObusType.Type.UINT64 : 'OBUS_FIELD_U64',
		ObusType.Type.STRING : 'OBUS_FIELD_STRING',
		ObusType.Type.HANDLE : 'OBUS_FIELD_U16',
		ObusType.Type.ENUM : 'OBUS_FIELD_ENUM',
		ObusType.Type.BOOL : 'OBUS_FIELD_BOOL',
		ObusType.Type.FLOAT : 'OBUS_FIELD_F32',
		ObusType.Type.DOUBLE : 'OBUS_FIELD_F64'}

	if t.base in types:
		s = types[t.base]
	else:
		raise ObusException('type {0} not supported'.format(t))

	if t.isArray():
		s += " | OBUS_FIELD_ARRAY"

	return s
