#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c_method.py
#
# @brief obus c method code generator
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

from obus_c_type import getType
from obus_c_type import getLibobusType
from obus_c_enum import ObusEnumWriter
from obus_c_utils import getObjectName

class ObusMethodsWriter(object):
	""" ObusMethods C class writer """
	def __init__(self, obj):
		self.obj = obj

	def getMethodsDescSymbol(self):
		""" get methods description symbol name """
		return getObjectName(self.obj) + "_methods_desc"

	def declareMethodStatusCallback(self, out):
		out.write("\n/**\n")
		out.write(" * generic %s client method status callback\n", getObjectName(self.obj))
		out.write(" **/\n")
		out.write("typedef void (*%s_method_status_cb_t) (struct %s "\
			"*object, obus_handle_t handle, enum obus_call_status status);\n",
			getObjectName(self.obj), getObjectName(self.obj))

	def declareMethodsArgs(self, out, client):
		if not self.obj.methods:
			return

		# declare methods args
		for mtd in self.obj.methods.values():
			ObusMethodWriter(mtd).declareMethodArgs(out)

	def writeMethodsApi(self, out, header):
		# generate method api
		if header:
			self.declareMethodStatusCallback(out)

		for mtd in self.obj.methods.values():
			if header:
				ObusMethodWriter(mtd).declareMethodArgs(out)

			ObusMethodWriter(mtd).writeMethodCallApi(header, out)

	def writeMethodsDesc(self, out):
		""" generate object methods args struct """
		if not self.obj.methods:
			return

		# declare method enum
		out.write("\nenum %s_method_type {\n", getObjectName(self.obj))
		first = True
		for mtd in self.obj.methods.values():
			out.write("\t%s_METHOD_%s", getObjectName(self.obj).upper(), \
				mtd.name.upper())
			if first:
				out.write(" = 0")
				first = False
			out.write(",\n")
		out.write("\t%s_METHOD_COUNT,\n", getObjectName(self.obj).upper())
		out.write("};\n")

		for mtd in self.obj.methods.values():
			ObusMethodWriter(mtd).writeMethodArgsDesc(out)

		out.write("\nstatic const struct obus_method_desc %s[] = {\n",
				self.getMethodsDescSymbol())
		first = True
		for mtd in self.obj.methods.values():
			if first:
				first = False
			else:
				out.write("\t,\n")

			out.write("\t{")
			out.write("\t\t.uid = %d,\n", mtd.uid)
			out.write("\t\t.name = \"%s\",\n", mtd.name)
			out.write("\t\t.args_desc = &%s,\n",
				ObusMethodWriter(mtd).getMethodArgsNameDescSymbol())
			out.write("\t}\n")
		out.write("};\n")

class ObusMethodWriter(object):
	""" ObusMethod C class writer """
	def __init__(self, mtd):
		self.mtd = mtd

	def getName(self):
		""" get method full name """
		return self.mtd.obj.bus.name + "_" + \
			self.mtd.obj.name + "_" + \
			self.mtd.name

	def getMethodArgsName(self):
		""" get method arguments array symbol name """
		return self.mtd.obj.bus.name + "_" + \
			self.mtd.obj.name + "_" + \
			self.mtd.name + "_args"

	def getMethodArgsNameDescSymbol(self):
		""" get method arguments description symbol name """
		return self.getMethodArgsName() + "_desc"

	def declareMethodArgs(self, out):
		""" declare method arguments struct in a .h file """
		if not self.mtd.args:
			return

		out.write("/**\n")
		out.write(" * @brief %s method %s arguments presence structure.\n", getObjectName(self.mtd.obj), self.mtd.name)
		out.write(" *\n")
		out.write(" * This structure contains a presence bit for each\n")
		out.write(" * of %s method %s argument.\n", getObjectName(self.mtd.obj), self.mtd.name)
		out.write(" * When a bit is set, the corresponding argument in\n")
		out.write(" * @ref %s_fields structure must be taken into account.\n", self.getMethodArgsName())
		out.write(" **/\n")
		out.write("struct %s_fields {\n", self.getMethodArgsName())
		for arg in self.mtd.args.values():
			out.write("\t/** presence bit for argument %s */\n", arg.name)
			out.write("\tunsigned int %s:1;\n", arg.name)
		out.write("};\n")

		out.write("\n/**\n")
		out.write(" * @brief %s method %s arguments structure.\n", getObjectName(self.mtd.obj), self.mtd.name)
		out.write(" *\n")
		out.write(" * This structure contains %s method %s arguments values.\n", getObjectName(self.mtd.obj), self.mtd.name)
		out.write(" **/\n")
		out.write("struct %s {\n", self.getMethodArgsName())
		out.write("\t/** arguments presence bit structure */\n")
		out.write("\tstruct %s_fields fields;\n", self.getMethodArgsName())
		for arg in self.mtd.args.values():
			if arg.desc:
				out.write("\t/** %s */\n", arg.desc)
			out.write("\t%s%s;\n", getType(arg.type), arg.name)
			if arg.type.isArray():
				out.write("\tuint32_t n_%s;\n", arg.name)

		out.write("};\n")

	def writeMethodArgsDesc(self, out):
		""" write method arguments srtruct in a .c file """

		if self.mtd.args:
			out.write("\nstatic const struct obus_field_desc "\
				"%s_fields[] = {\n", self.getMethodArgsName())
			first = True
			for arg in self.mtd.args.values():
				if first:
					first = False
				else:
					out.write("\t,\n")

				out.write("\t{")
				out.write("\t\t.uid = %d,\n", arg.uid)
				out.write("\t\t.name = \"%s\",\n", arg.name)
				out.write("\t\t.offset = obus_offsetof(struct %s, %s),\n",
						 self.getMethodArgsName(), arg.name)

				out.write("\t\t.role = OBUS_ARGUMENT,\n")
				out.write("\t\t.type = %s,\n", getLibobusType(arg.type))
				if arg.type.isEnum():
					out.write("\t\t.enum_drv = &%s,\n",
					ObusEnumWriter(arg.type.enum).getDriverSymbol())
				if arg.type.isArray():
					out.write("\t\t.nb_offset = obus_offsetof(struct %s, n_%s),\n",
							 self.getMethodArgsName(), arg.name)
				out.write("\t}\n")
			out.write("};\n")

		out.write("\nstatic const struct obus_struct_desc %s = {\n",
				self.getMethodArgsNameDescSymbol())
		if self.mtd.args:
			out.write("\t.size = sizeof(struct %s),\n", self.getMethodArgsName())
			out.write("\t.fields_offset = obus_offsetof(struct %s, fields),\n",
				self.getMethodArgsName())
			out.write("\t.n_fields = OBUS_SIZEOF_ARRAY(%s_fields),\n",
				self.getMethodArgsName())
			out.write("\t.fields = %s_fields,\n", self.getMethodArgsName())
		else:
			out.write("\t.size = 0,\n")
			out.write("\t.fields_offset = 0,\n")
			out.write("\t.n_fields = 0,\n")
			out.write("\t.fields = NULL,\n")

		out.write("};\n")

	def writeMethodCallApi(self, header, out):
		if self.mtd.args:
			if header:
				out.write("\n/**\n")
				out.write(" * @brief initialize @ref %s structure.\n", self.getMethodArgsName())
				out.write(" *\n")
				out.write(" * This function initialize @ref %s structure.\n", self.getMethodArgsName())
				out.write(" * Each argument field has its presence bit cleared.\n")
				out.write(" *\n")
				out.write(" * @param[in]  args  pointer to allocated @ref %s structure.\n", self.getMethodArgsName())
				out.write(" **/\n")

			out.write("void %s_init(struct %s *args)%s\n",
					self.getMethodArgsName(), self.getMethodArgsName(),
					(';' if header else ''))
			if not header:
				out.write("{\n")
				out.write("\tif (args)\n")
				out.write("\t\tmemset(args, 0, sizeof(*args));\n")
				out.write("}\n\n")

			if header:
				out.write("\n/**\n")
				out.write(" * @brief check @ref %s structure contents is empty.\n", self.getMethodArgsName())
				out.write(" *\n")
				out.write(" * This function check if each argument field has its presence bit cleared.\n")
				out.write(" *\n")
				out.write(" * @param[in]  args  @ref %s structure.\n", self.getMethodArgsName())
				out.write(" *\n")
				out.write(" * @retval     1     Each argument field has its presence bit cleared.\n")
				out.write(" * @retval     0     One argument field (or more) has its presence bit set.\n")
				out.write(" **/\n")

			out.write("int %s_is_empty(const struct %s *args)%s\n",
					self.getMethodArgsName(), self.getMethodArgsName(),
					(';' if header else ''))
			if not header:
				out.write("{\n")
				out.write("\treturn (args")
				for arg in self.mtd.args.values():
					out.write(" &&\n\t\t!args->fields.%s", arg.name)
				out.write(");\n")
				out.write("}\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief call method '%s'.\n", self.mtd.name)
			out.write(" *\n")
			out.write(" * This function call method '%s' on a %s object\n", self.mtd.name, getObjectName(self.mtd.obj))
			if self.mtd.desc:
				out.write(" *\n")
				out.write(" * %s\n", self.mtd.desc[0].upper() + self.mtd.desc[1:].lower())
			out.write(" *\n")
			out.write(" * @param[in]   client  obus client context.\n")
			out.write(" * @param[in]   object  %s object.\n", getObjectName(self.mtd.obj))
			if self.mtd.args:
				out.write(" * @param[in]   args    call arguments.\n")
			out.write(" * @param[in]   cb      call status callback.\n")
			out.write(" * @param[out]  handle  call handle.\n")
			out.write(" *\n")
			out.write(" * @retval      0      Call request has been sent to server.\n")
			out.write(" * @retval   -EINVAL   Invalid function arguments.\n")
			out.write(" * @retval   -EPERM    Client is not connected.\n")
			out.write(" * @retval   -EPERM    Object is not registered.\n")
			out.write(" * @retval   -EPERM    Method is not ENABLED.\n")
			out.write(" * @retval   -ENOMEM   Memory error.\n")
			out.write(" **/\n")

		out.write("int %s_call_%s(struct obus_client *client,\n",
				getObjectName(self.mtd.obj), self.mtd.name)
		out.write("\t\t\tstruct %s *object,\n", getObjectName(self.mtd.obj))
		if self.mtd.args:
			out.write("\t\t\tconst struct %s *args,\n", self.getMethodArgsName())
		out.write("\t\t\t%s_method_status_cb_t cb,\n", getObjectName(self.mtd.obj))
		out.write("\t\t\tuint16_t *handle)%s\n", (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tconst struct obus_method_desc *desc = "\
					"&%s_methods_desc[%s_METHOD_%s];\n", \
					getObjectName(self.mtd.obj),
					getObjectName(self.mtd.obj).upper(),
					self.mtd.name.upper())

			out.write("\tstruct obus_struct st = {.u.const_addr = %s, "\
				".desc = desc->args_desc};\n",
				"args" if self.mtd.args else "NULL")

			out.write("\treturn obus_client_call(client , %s_object(object), "\
					"desc, &st, (obus_method_call_status_handler_cb_t)cb, "\
					"handle);\n", getObjectName(self.mtd.obj))
			out.write("}\n")
