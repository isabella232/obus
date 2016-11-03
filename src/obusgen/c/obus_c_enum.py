#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c.py
#
# @brief obus c enum code generator
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

from obus_c_utils import getObjectName

class ObusEnumWriter(object):
	""" ObusEnum C class writer """
	def __init__(self, enum):
		self.enum = enum

	def getName(self):
		""" get full enum name """
		return self.enum.obj.bus.name + "_" + \
			self.enum.obj.name + "_" + \
			self.enum.name

	def getDriverSymbol(self):
		""" get enum driver symbol name """
		return	self.enum.obj.bus.name + "_" + \
			self.enum.obj.name + "_" + \
			self.enum.name + "_driver"

	def __writeEnumIsValid(self, out, header):
		""" EnumIsValid method writer """
		if header:
			out.write("\n/**\n")
			out.write(" * @brief check if value is one of "\
				"enum %s values.\n", self.getName())
			out.write(" *\n")
			out.write(" * @param[in]  value  value to be checked.\n")
			out.write(" *\n")
			out.write(" * @retval 1 value is one of %s values.\n",
				self.getName())
			out.write(" * @retval 0 value is not one of %s values.\n",
				self.getName())
			out.write(" **/")

		out.write("\nint %s_is_valid(int32_t value)%s\n", self.getName(),
			(';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\treturn (")
			first = True
			for v in self.enum.values.values():
				if not first:
					out.write(" ||\n\t\t")
				else:
					first = False
				out.write("value == %s_%s", self.getName().upper(),
						v.name.upper())
			out.write(");\n}\n")

	def __writeEnumStr(self, out, header):
		""" EnumStr method writer """
		if header:
			out.write("\n/**\n")
			out.write(" * @brief get %s string value.\n", self.getName())
			out.write(" *\n")
			out.write(" * @param[in]  value  %s value to be converted into string.\n", self.enum.name)
			out.write(" *\n")
			out.write(" * @retval non NULL constant string value.\n")
			out.write(" **/")

		out.write("\nconst char *%s_str(enum %s value)%s\n",
			self.getName(), self.getName(),
			(';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tconst char *str;\n")
			out.write("\n")
			out.write("\tswitch (value) {\n")
			for v in self.enum.values.values():
				out.write("\tcase %s_%s:\n", self.getName().upper(),
					  v.name.upper())
				out.write("\t\tstr = \"%s\";\n", v.name.upper())
				out.write("\t\tbreak;\n")
			out.write("\tdefault:\n")
			out.write("\t\tstr = \"???\";\n")
			out.write("\t\tbreak;\n")
			out.write("\t}\n")
			out.write("\n")
			out.write("\treturn str;\n")
			out.write("}\n")

	def writeDeclaration(self, out):
		""" declare enumeration in a .h file """
		# declare enum
		out.write("\n/**\n")
		out.write(" * @brief %s %s enumeration.\n", getObjectName(self.enum.obj), self.enum.name)
		if self.enum.desc:
			out.write(" *\n")
			out.write(" * %s\n", self.enum.desc)
		out.write(" **/\n")
		out.write("enum %s {\n", self.getName())
		for v in self.enum.values.values():
			if v.desc:
				out.write("\t/** %s */\n", v.desc)
			out.write("\t%s_%s = %d,\n", self.getName().upper(),
				v.name.upper(), v.value)
		out.write("};\n")
		# declare enum value is valid
		self.__writeEnumIsValid(out, 1)
		self.__writeEnumStr(out, 1)

	def writeDriver(self, out):
		""" write enum driver in a .c file """
		# is_valid
		self.__writeEnumIsValid(out, 0)
		self.__writeEnumStr(out, 0)

		# set_value
		out.write("\nstatic void %s_set_value(void *addr, int32_t value)\n",
				self.getName())
		out.write("{\n")
		out.write("\tenum %s *v = addr;\n", self.getName())
		out.write("\t*v = (enum %s)value;\n", self.getName())
		out.write("}\n")

		# get_value
		out.write("\nstatic int32_t %s_get_value(const void *addr)\n",
				 self.getName())
		out.write("{\n")
		out.write("\tconst enum %s *v = addr;\n", self.getName())
		out.write("\treturn (int32_t)(*v);\n")
		out.write("}\n")

		# format
		out.write("\nstatic void %s_format(const void *addr, "\
				"char *buf, size_t size)\n", self.getName())
		out.write("{\n")
		out.write("\tconst enum %s *v = addr;\n", self.getName())
		out.write("\n")
		out.write("\tif (%s_is_valid((int32_t)(*v)))\n", self.getName())
		out.write("\t\tsnprintf(buf, size, \"%%s\", %s_str(*v));\n", self.getName())
		out.write("\telse\n")
		out.write("\t\tsnprintf(buf, size, \"??? (%%d)\", (int32_t)(*v));\n")
		out.write("}\n")

		# declare enum driver
		out.write("\nstatic const struct obus_enum_driver %s = {\n",
				self.getDriverSymbol())
		out.write("\t.name = \"%s\",\n", self.getName())
		out.write("\t.size = sizeof(enum %s),\n", self.getName())
		out.write("\t.default_value = %s_%s,\n", self.getName().upper(),
				self.enum.default.upper())
		out.write("\t.is_valid = %s_is_valid,\n", self.getName())
		out.write("\t.set_value = %s_set_value,\n", self.getName())
		out.write("\t.get_value = %s_get_value,\n", self.getName())
		out.write("\t.format = %s_format\n", self.getName())
		out.write("};\n")
