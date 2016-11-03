#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c.py
#
# @brief obus c bus event code generator
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

class ObusBusEventsWriter(object):
	""" ObusBusEvents C class writer """
	def __init__(self, bus):
		self.bus = bus

	def getEventsDescSymbol(self):
		""" get bus events description symbol name """
		return self.bus.name + "_bus_events"

	def declareEventStruct(self, out):
		out.write("/**\n")
		out.write(" * @brief %s bus event structure\n", self.bus.name)
		out.write(" *\n")
		out.write(" * This opaque structure represent an %s bus event.\n", self.bus.name)
		out.write(" **/\n")
		out.write("struct %s_bus_event;\n\n", self.bus.name)

	def declareEvents(self, out):
		if not self.bus.events:
			return

		# declare bus enum
		out.write("\n/**\n")
		out.write(" * @brief %s bus event type enumeration.\n", self.bus.name)
		out.write(" *\n")
		out.write(" * This enumeration describes all kind of %s bus events.\n", self.bus.name)
		out.write(" **/\n")
		out.write("enum %s_bus_event_type {\n", self.bus.name)
		first = True
		for event in self.bus.events.values():
			if event.desc:
				out.write("\t/** %s */\n", event.desc)
			out.write("\t%s_BUS_EVENT_%s", self.bus.name.upper(), event.name.upper())
			if first:
				out.write(" = 0")
				first = False
			out.write(",\n")
		out.write("\t/** for internal use only*/\n")
		out.write("\t%s_BUS_EVENT_COUNT,\n", self.bus.name.upper())
		out.write("};\n")

	def writeEventTypeStr(self, out, header):
		""" EventStr method writer """
		if not self.bus.events:
			return

		if header:
			out.write("\n/**\n")
			out.write(" * @brief get %s_bus_event_type string value.\n", self.bus.name)
			out.write(" *\n")
			out.write(" * @param[in] type bus event type to be converted into string.\n")
			out.write(" *\n")
			out.write(" * @retval non NULL constant string value.\n")
			out.write(" **/")

		out.write("\nconst char *%s_bus_event_type_str(enum %s_bus_event_type type)%s\n",
			self.bus.name, self.bus.name, (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tif (type >= OBUS_SIZEOF_ARRAY(%s))\n", self.getEventsDescSymbol())
			out.write("\t\treturn \"???\";\n")
			out.write("\n")
			out.write("\treturn %s[type].name;\n", self.getEventsDescSymbol());
			out.write("}\n")

	def writeEventsDesc(self, out):
		""" generate bus events desc """
		if not self.bus.events:
			return

		out.write("\nstatic const struct obus_bus_event_desc %s[] = {\n",
			self.getEventsDescSymbol())
		first = True
		for event in self.bus.events.values():
			if first:
				first = False
			else:
				out.write("\t,\n")

			out.write("\t{\n")
			out.write("\t\t.uid = %d,\n", event.uid)
			out.write("\t\t.name = \"%s\",\n", event.name)
			out.write("\t}\n")
		out.write("};\n")

	def writeEventsApi(self, out, options, header):
		""" generate events api for client """
		if not self.bus.events:
			return

		if not options.client:
			if header:
				out.write("\n/**\n")
				out.write(" * @brief create %s bus event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * This function is used to create a new %s event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * @param[in]  type   %s bus event type.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * @retval     %s bus event or NULL on error.\n", self.bus.name)
				out.write(" **/\n")
			else:
				out.write("\n/* *INDENT-COMMENT-FIX-ISSUE* */\n")

			out.write("/* *INDENT-OFF* */\nstruct %s_bus_event *\n/* *INDENT-ON* */\n%s_bus_event_new(enum "\
					"%s_bus_event_type type)%s\n", self.bus.name, self.bus.name, self.bus.name, (';' if header else ''))
			if not header:
				out.write("{\n")
				out.write("\tconst struct obus_bus_event_desc *desc;\n")
				out.write("\n")
				out.write("\tif (type >= %s_BUS_EVENT_COUNT)\n", self.bus.name.upper())
				out.write("\t\treturn NULL;\n")
				out.write("\n")
				out.write("\tdesc = &%s[type];\n", self.getEventsDescSymbol())
				out.write("\treturn (struct %s_bus_event *)"\
						"obus_bus_event_new(desc);\n",
						self.bus.name)
				out.write("}\n\n")

			if header:
				out.write("\n/**\n")
				out.write(" * @brief destroy %s bus event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * This function is used to destroy a %s event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * @param[in]  event  %s bus event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * @retval      0     success.\n")
				out.write(" **/")

			out.write("\nint %s_bus_event_destroy(struct %s_bus_event"\
				" *event)%s\n", self.bus.name, self.bus.name, (';' if header else ''))
			if not header:
				out.write("{\n")
				out.write("\treturn obus_bus_event_destroy((struct "\
						"obus_bus_event *)event);\n")
				out.write("}\n\n")

		# api for both server and client
		if header:
			out.write("\n/**\n")
			out.write(" * @brief get %s bus event type.\n", self.bus.name)
			out.write(" *\n")
			out.write(" * This function is used to get the type of a %s bus event.\n", self.bus.name)
			out.write(" *\n")
			out.write(" * @param[in]  event  %s bus event.\n", self.bus.name)
			out.write(" *\n")
			out.write(" * @retval  one of @ref %s_bus_event_type value.\n", self.bus.name)
			out.write(" **/\n")
		else:
			out.write("\n/* *INDENT-COMMENT-FIX-ISSUE* */\n")

		out.write("/* *INDENT-OFF* */\nenum %s_bus_event_type\n/* *INDENT-ON* */\n%s_bus_event_get_type(const "\
					"struct %s_bus_event *event)%s\n", self.bus.name, self.bus.name, self.bus.name, (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tlong idx;\n")
			out.write("\tconst struct obus_bus_event *evt;\n")
			out.write("\tconst struct obus_bus_event_desc *desc;\n")
			out.write("\n")
			out.write("\tevt = (const struct obus_bus_event *)event;\n")
			out.write("\tdesc = obus_bus_event_get_desc(evt);\n")
			out.write("\tidx = desc - %s;\n", self.getEventsDescSymbol())
			out.write("\n")
			out.write("\tif (idx < 0 || idx > %s_BUS_EVENT_COUNT)\n",
					self.bus.name.upper())
			out.write("\t\treturn %s_BUS_EVENT_COUNT;\n", self.bus.name.upper())
			out.write("\n")
			out.write("\treturn (enum %s_bus_event_type)idx;\n", self.bus.name)
			out.write("}\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief get %s bus event from generic obus bus event.\n", self.bus.name)
			out.write(" *\n")
			out.write(" * This function is used to get a %s bus event from an obus bus event.\n", self.bus.name)
			out.write(" *\n")
			out.write(" * @param[in]  event       generic obus bus event.\n")
			out.write(" *\n")
			out.write(" * @retval  %s bus event or NULL if not a %s bus event.\n", self.bus.name, self.bus.name)
			out.write(" **/\n")
		else:
			out.write("\n/* *INDENT-COMMENT-FIX-ISSUE* */\n")

		out.write("\n/* *INDENT-OFF* */\nstruct %s_bus_event *\n/* *INDENT-ON* */\n%s_bus_event_from_obus_event("\
			"struct obus_bus_event *event)%s\n", self.bus.name, self.bus.name, (';' if header else ''))

		if not header:
			out.write("{\n")
			out.write("\tlong idx;\n")
			out.write("\tconst struct obus_bus_event_desc *desc;\n")
			out.write("\n")
			out.write("\tdesc = obus_bus_event_get_desc(event);\n")
			out.write("\tidx = desc - %s;\n", self.getEventsDescSymbol())
			out.write("\n")
			out.write("\tif (idx < 0 || idx > %s_BUS_EVENT_COUNT)\n",
					self.bus.name.upper())
			out.write("\t\treturn NULL;\n")
			out.write("\n")
			out.write("\treturn (struct  %s_bus_event *)event;\n", self.bus.name)
			out.write("}\n")

		# for server only
		if not options.client:
			if header:
				out.write("\n/**\n")
				out.write(" * @brief send a %s bus event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * This function send a %s bus event.\n", self.bus.name)
				out.write(" * Associated object event are sent and object contents are updated.\n")
				out.write(" * Associated objects to be registered are registered.\n")
				out.write(" * Associated objects to be unregistered are unregistered and destroyed.\n")
				out.write(" *\n")
				out.write(" * @param[in]  server  %s bus server.\n", self.bus.name)
				out.write(" * @param[in]  event   %s bus event.\n", self.bus.name)
				out.write(" *\n")
				out.write(" * @retval     0      bus event is sent\n")
				out.write(" * @retval  -EINVAL   invalid parameters.\n")
				out.write(" * @retval     0      one of @%s_bus_event_type value.\n", self.bus.name)
				out.write(" **/")

			out.write("\nint %s_bus_event_send(struct obus_server *server, "\
					"struct %s_bus_event *event)%s\n", self.bus.name, self.bus.name,
					(';' if header else ''))
			if not header:
				out.write("{\n")
				out.write("\tstruct obus_bus_event *evt = (struct obus_bus_event *)event;\n")
				out.write("\treturn obus_server_send_bus_event(server, evt);\n")
				out.write("}\n")
