#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_vala.py
#
# @brief obus vala source code generator
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

import sys, os
import logging

from obusparser import ObusBus
from obusparser import ObusException
from obusparser import ObusType
from obusgen import Writer, writeHeader
from c import main as mainC

#===============================================================================
#===============================================================================
def toCamelcase(value, firstLow = False):
	def camelcase():
		if firstLow:
			yield str.lower
		while True:
			yield str.capitalize

	c = camelcase()
	return "".join(c.next()(x) if x else '_' for x in value.split("_"))

#===============================================================================
#===============================================================================
def getBusFileH(bus):
	return bus.name + "_bus.h"

#===============================================================================
#===============================================================================
def getObjectFileH(obj):
	return obj.bus.name + "_" + obj.name + ".h"

#===============================================================================
#===============================================================================
def getObjectTypeName(obj):
	return toCamelcase(obj.name, False)

#===============================================================================
#===============================================================================
def getObjectTypeNameC(obj):
	return obj.bus.name + "_" + obj.name

#===============================================================================
#===============================================================================
def getType(t):
	valatypes = {
		ObusType.Type.INT8 : 'int8' ,
		ObusType.Type.UINT8 : 'uint8',
		ObusType.Type.INT16 : 'int16',
		ObusType.Type.UINT16 : 'uint16',
		ObusType.Type.INT32 : 'int32',
		ObusType.Type.UINT32 : 'uint32',
		ObusType.Type.INT64 : 'int64',
		ObusType.Type.UINT64 : 'uint64_t',
		ObusType.Type.STRING : 'string',
		ObusType.Type.HANDLE : 'Obus.Handle',
		ObusType.Type.BOOL : 'bool',
		ObusType.Type.FLOAT : 'float',
		ObusType.Type.DOUBLE : 'double'
	}
	if t.isArray():
		if t.isEnum():
			return ("unowned %s[]" % toCamelcase(t.enum.name, False))
		else:
			return ("unowned %s[]" % valatypes[t.base])
	elif t.base == ObusType.Type.STRING:
		return "unowned string"
	elif t.isEnum():
		return toCamelcase(t.enum.name, False)
	else:
		return valatypes[t.base]

#===============================================================================
#===============================================================================
def listFiles(options, bus):
	vapiPath = os.path.join(options.outdir, bus.name.lower() + ".vapi")
	filePaths = [vapiPath]
	# No end of line but a space, we will print files from C generator as well
	sys.stdout.write(" ".join(filePaths) + " ")

#===============================================================================
#===============================================================================
def writeCodeAttributes(out, indent, **kwargs):
	nostr = ["instance_pos", "has_target"]
	codeattr = []
	for key in sorted(kwargs.keys()):
		if key in nostr:
			codeattr.append("%s = %s" % (key, kwargs[key]))
		else:
			codeattr.append("%s = \"%s\"" % (key, kwargs[key]))
	out.write(indent)
	out.write("[CCode (" + ", ".join(codeattr) + ")]\n")

#===============================================================================
#===============================================================================
def writeBus(out, bus):
	# Write bus descriptor
	writeCodeAttributes(out, "    ",
			cheader_filename = getBusFileH(bus),
			cname = bus.name + "_bus_desc",
			ctype = "const struct obus_bus_desc *")
	out.write("    public Obus.BusDesc *desc;\n")

	# Write bus event
	if not bus.events:
		return

	# Bus event class
	out.write("\n")
	out.write("    [Compact]\n")
	writeCodeAttributes(out, "    ",
			cheader_filename = getBusFileH(bus),
			cname = "struct " + bus.name + "_bus_event",
			free_function = bus.name + "_bus_event_destroy")
	out.write("    public class BusEvent {\n")

	# Bus event type enum
	writeCodeAttributes(out, "        ",
			cheader_filename = getBusFileH(bus),
			cname = "enum " + bus.name + "_bus_event_type",
			cprefix = bus.name.upper() + "_BUS_EVENT_")
	out.write("        public enum Type {\n")
	for event in bus.events.values():
		out.write("            " + event.name.upper() + ",\n")
	out.write("            COUNT;\n")
	writeCodeAttributes(out, "            ",
			cname = bus.name + "_bus_event_type_str")
	out.write("            public unowned string to_string();\n")
	out.write("        }\n")

	# Standard api
	out.write("\n")
	out.write("        public BusEvent(Type type);\n")
	writeCodeAttributes(out, "        ", instance_pos = "1.2")
	out.write("        public int send(Obus.Server srv);\n")

	# Api for each objects
	for obj in bus.objects.values():
		out.write("\n")
		if obj.events:
			out.write("        public int add_%s_event(%s object, %s.EventType type, ref %s.Info info);\n",
					obj.name, getObjectTypeName(obj),
					getObjectTypeName(obj), getObjectTypeName(obj))
		out.write("        public int register_%s(%s object);\n",
				obj.name, getObjectTypeName(obj))
		out.write("        public int unregister_%s(%s object);\n",
				obj.name, getObjectTypeName(obj))

	# End of BusEvent class
	out.write("    }\n")

#===============================================================================
#===============================================================================
def writeObject(out, obj):
	# Class header
	out.write("\n")
	out.write("    [Compact]\n")
	writeCodeAttributes(out, "    ",
			cheader_filename = getObjectFileH(obj),
			cname = "struct " + getObjectTypeNameC(obj),
			free_function = getObjectTypeNameC(obj) + "_destroy")
	out.write("    public class %s {\n", getObjectTypeName(obj))

	# Enums
	for enum in obj.enums.values():
		out.write("\n")
		writeCodeAttributes(out, "        ",
				cheader_filename = getObjectFileH(obj),
				cname = "enum " + getObjectTypeNameC(obj) + "_" + enum.name,
				cprefix = getObjectTypeNameC(obj).upper() + "_" + enum.name.upper() + "_")
		out.write("        public enum %s {\n", toCamelcase(enum.name, False))
		for val in enum.values.values():
			out.write("            %s,\n", val.name.upper())
		out.write("            __LAST__;\n")
		writeCodeAttributes(out, "            ",
				cname = getObjectTypeNameC(obj) + "_" + enum.name + "_str")
		out.write("            public unowned string to_string();\n")
		out.write("            public bool is_valid();\n")
		out.write("        }\n")

	# Event type enum
	if obj.events:
		out.write("\n")
		writeCodeAttributes(out, "        ",
				cheader_filename = getObjectFileH(obj),
				cname = "enum " + getObjectTypeNameC(obj) + "_event_type",
				cprefix = getObjectTypeNameC(obj).upper() + "_EVENT_")
		out.write("        public enum EventType {\n")
		for event in obj.events.values():
			out.write("            " + event.name.upper() + ",\n")
		out.write("            COUNT;\n")
		writeCodeAttributes(out, "            ",
				cname = getObjectTypeNameC(obj) + "_event_type_str")
		out.write("            public unowned string to_string();\n")
		out.write("        }\n")

	# InfoFields struct
	out.write("\n")
	out.write("        [Compact]\n")
	writeCodeAttributes(out, "        ",
			cheader_filename = getObjectFileH(obj),
			cname = "struct " + getObjectTypeNameC(obj) + "_info_fields")
	out.write("        public struct InfoFields {\n")
	for prop in obj.properties.values():
		out.write("            public bool %s;\n", prop.name)
	for mtd in obj.methods.values():
		out.write("            public bool method_%s;\n", mtd.name)
	out.write("        }\n")

	# Info struct
	out.write("\n")
	out.write("        [Compact]\n")
	writeCodeAttributes(out, "        ",
			cheader_filename = getObjectFileH(obj),
			cname = "struct " + getObjectTypeNameC(obj) + "_info",
			construct_function = getObjectTypeNameC(obj) + "_info_init")
	out.write("        public struct Info {\n")
	out.write("            public InfoFields fields;\n")
	# Private declarations
	out.write("\n")
	for prop in obj.properties.values():
		if prop.type.isArray():
			writeCodeAttributes(out, "            ",
					cname = prop.name,
					array_length_cname = "n_" + prop.name)
		else:
			writeCodeAttributes(out, "            ",
					cname = prop.name)
		out.write("            private %s _%s;\n", getType(prop.type), prop.name)
	for mtd in obj.methods.values():
		name = "method_" + mtd.name
		writeCodeAttributes(out, "            ",
				cname = name)
		out.write("            private %s _%s;\n", "Obus.MethodState", name)
	# Public declarations
	out.write("\n")
	for prop in obj.properties.values():
		out.write("            public %s %s {get {return this._%s;} set {this.fields.%s = true; this._%s = value;}}\n",
				getType(prop.type), prop.name, prop.name, prop.name, prop.name)
	for mtd in obj.methods.values():
		name = "method_" + mtd.name
		out.write("            public %s %s {get {return this._%s;} set {this.fields.%s = true; this._%s = value;}}\n",
				"Obus.MethodState", name, name, name, name)
	# Standard declarations
	out.write("\n")
	out.write("            public Info();\n")
	out.write("            public bool is_empty();\n")
	out.write("        }\n")

	# Accessors in class iself
	out.write("\n")
	for prop in obj.properties.values():
		out.write("        public %s %s {get {return this.info.%s;}}\n",
				getType(prop.type), prop.name, prop.name)
	for mtd in obj.methods.values():
		name = "method_" + mtd.name
		out.write("        public %s %s {get {return this.info.%s;}}\n",
				"Obus.MethodState", name, name)

	# Standard accessors
	out.write("\n")
	out.write("        private unowned Info* info {get;}\n")
	out.write("        public Obus.Handle handle {get;}\n")
	out.write("        public void* user_data {get; set;}\n")

	# Standard methods
	out.write("\n")
	if obj.methods:
		out.write("        public static unowned %s @new(Obus.Server srv, ref Info info, Handlers handlers);\n",
				getObjectTypeName(obj))
	else:
		out.write("        public static unowned %s @new(Obus.Server srv, ref Info info);\n",
				getObjectTypeName(obj))
	out.write("        public bool is_registered();\n")
	writeCodeAttributes(out, "        ", instance_pos = "1.2")
	out.write("        public int register(Obus.Server srv);\n")
	writeCodeAttributes(out, "        ", instance_pos = "1.2")
	out.write("        public int unregister(Obus.Server srv);\n")
	writeCodeAttributes(out, "        ", instance_pos = "1.2")
	if obj.events:
		out.write("        public int send_event(Obus.Server srv, EventType type, ref Info info);\n")
	out.write("        public static unowned %s? from_handle(Obus.Server srv, Obus.Handle handle);\n",
			getObjectTypeName(obj))
	out.write("        public static unowned %s? next(Obus.Server srv, %s? prev);\n",
			getObjectTypeName(obj), getObjectTypeName(obj))

	# Methods arguments types
	for mtd in obj.methods.values():
		if mtd.args:
			# ArgsFields struct
			out.write("\n")
			out.write("        [Compact]\n")
			writeCodeAttributes(out, "        ",
					cheader_filename = getObjectFileH(obj),
					cname = "struct " + getObjectTypeNameC(obj) + "_" + mtd.name + "_args_fields")
			out.write("        public struct %sArgsFields {\n", toCamelcase(mtd.name, False))
			for arg in mtd.args.values():
				out.write("            public bool %s;\n", arg.name)
			out.write("        }\n")
			# Args struct
			out.write("\n")
			out.write("        [Compact]\n")
			writeCodeAttributes(out, "        ",
					cheader_filename = getObjectFileH(obj),
					cname = "struct " + getObjectTypeNameC(obj) + "_" + mtd.name + "_args")
			out.write("        public struct %sArgs {\n", toCamelcase(mtd.name, False))
			out.write("            public %sArgsFields fields;\n", toCamelcase(mtd.name, False))
			# Private declarations
			out.write("\n")
			for arg in mtd.args.values():
				if arg.type.isArray():
					writeCodeAttributes(out, "            ",
							cname = arg.name,
							array_length_cname = "n_" + arg.name)
				else:
					writeCodeAttributes(out, "            ",
							cname = arg.name)
				out.write("            private %s _%s;\n", getType(arg.type), arg.name)
			# Public declarations
			out.write("\n")
			for arg in mtd.args.values():
				out.write("            public %s %s {get {return this._%s;}}\n",
						getType(arg.type), arg.name, arg.name)
			out.write("        }\n")
		# Delegate
		out.write("\n")
		writeCodeAttributes(out, "        ",
				cheader_filename = getObjectFileH(obj),
				cname = "ps_summary_method_" + mtd.name + "_handler_t",
				has_target = "false")
		if mtd.args:
			out.write("        public delegate void %sHandler(%s object, Obus.Handle handle, %sArgs args);\n",
					toCamelcase(mtd.name, False), getObjectTypeName(obj), toCamelcase(mtd.name, False))
		else:
			out.write("        public delegate void %sHandler(%s object, Obus.Handle handle, void* unused);\n",
					toCamelcase(mtd.name, False), getObjectTypeName(obj))

	# Handlers struct
	if obj.methods:
		out.write("\n")
		out.write("        [Compact]\n")
		writeCodeAttributes(out, "        ",
				cheader_filename = getObjectFileH(obj),
				cname = "struct " + getObjectTypeNameC(obj) + "_method_handlers")
		out.write("        public struct Handlers {\n")
		for mtd in obj.methods.values():
			out.write("            public %sHandler method_%s;\n",
					toCamelcase(mtd.name, False), mtd.name)
		out.write("        }\n")

	# End of class
	out.write("    }\n")

#===============================================================================
#===============================================================================
def main(options, xmlFile):
	# Only server api available
	if options.client:
		logging.error("Client api generation not available for vala")
		sys.exit(1)

	try:
		bus = ObusBus(xmlFile)
	except ObusException as ex:
		logging.error(ex)
		return

	if options.listFiles:
		listFiles(options, bus)
		mainC(options, xmlFile)
		return

	vapiPath = os.path.join(options.outdir, bus.name.lower() + ".vapi")
	out = Writer(vapiPath)
	writeHeader(out, bus.name + " bus vala interface file")
	out.write("\n")
	out.write("namespace %s {\n", toCamelcase(bus.name, False))
	writeBus(out, bus)
	for obj in bus.objects.values():
		writeObject(out, obj)
	out.write("}\n")
	out.close()

	# And now call c generator
	mainC(options, xmlFile)
