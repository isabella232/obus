#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c_event.py
#
# @brief obus c event code generator
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

class ObusEventsWriter(object):
	""" ObusEvents C class writer """
	def __init__(self, obj):
		self.obj = obj

	def getEventsDescSymbol(self):
		""" get events description symbol name """
		return getObjectName(self.obj) + "_events_desc"

	def declareEvents(self, out):
		if not self.obj.events:
			return
		# declare event enum
		out.write("\n/**\n")
		out.write(" * @brief %s event type enumeration.\n", getObjectName(self.obj))
		out.write(" *\n")
		out.write(" * This enumeration describes all kind of %s events.\n", getObjectName(self.obj))
		out.write(" **/\n")
		out.write("enum %s_event_type {\n", getObjectName(self.obj))
		first = True
		for event in self.obj.events.values():
			if event.desc:
				out.write("\t/** %s */\n", event.desc)
			out.write("\t%s_EVENT_%s", getObjectName(self.obj).upper(),
				event.name.upper())
			if first:
				out.write(" = 0")
				first = False
			out.write(",\n")
		out.write("\t/** for internal use only*/\n")
		out.write("\t%s_EVENT_COUNT,\n", getObjectName(self.obj).upper())
		out.write("};\n")

	def writeEventTypeStr(self, out, header):
		""" EventStr method writer """
		if not self.obj.events:
			return

		if header:
			out.write("\n/**\n")
			out.write(" * @brief get %s_event_type string value.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @param[in]  type  event type to be converted into string.\n")
			out.write(" *\n")
			out.write(" * @retval non NULL constant string value.\n")
			out.write(" **/")

		out.write("\nconst char *%s_event_type_str(enum %s_event_type type)%s\n",
			getObjectName(self.obj), getObjectName(self.obj),
			(';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tif(type >= OBUS_SIZEOF_ARRAY(%s_events_desc))\n", getObjectName(self.obj))
			out.write("\t\treturn \"???\";\n")
			out.write("\n")
			out.write("\treturn %s_events_desc[type].name;\n", getObjectName(self.obj));
			out.write("}\n")

	def getEventUpdatesDescSymbol(self, event):
		return "event_{0}_updates".format(event.name)

	def isEventhasUpdates(self, event):
		return event.properties or event.methods

	def writeEventUpdatesDesc(self, event, out):

		# only declare update struct if event has updates
		if not self.isEventhasUpdates(event):
			return

		out.write("\nstatic const struct obus_event_update_desc %s[] = {\n",
				self.getEventUpdatesDescSymbol(event))

		first = True
		for prop in event.properties.values():
			if first:
				first = False
			else:
				out.write("\n,\n")

			out.write("\t{\n")
			out.write("\t\t.field = &%s_info_fields[%s_%s_FIELD_%s],\n", getObjectName(self.obj),
					self.obj.bus.name.upper(), self.obj.name.upper(), prop.name.upper())
			out.write("\t\t.flags = 0,\n")
			out.write("\t}\n")

		for mtd in event.methods.values():
			if first:
				first = False
			else:
				out.write("\n,\n")

			out.write("\t{\n")
			out.write("\t\t.field = &%s_info_fields[%s_%s_FIELD_METHOD_%s],\n", getObjectName(self.obj),
					self.obj.bus.name.upper(), self.obj.name.upper(), mtd.name.upper())
			out.write("\t\t.flags = 0,\n")
			out.write("\t}\n")

		out.write("\n};\n")

	def writeEventsDesc(self, out):
		""" generate object events args struct """
		if not self.obj.events:
			return

		# generate event updates desc
		for event in self.obj.events.values():
				self.writeEventUpdatesDesc(event, out)

		out.write("\nstatic const struct obus_event_desc %s[] = {\n",
			self.getEventsDescSymbol())
		first = True
		for event in self.obj.events.values():
			if first:
				first = False
			else:
				out.write("\n,\n")

			out.write("\t{\n")
			out.write("\t\t.uid = %d,\n", event.uid)
			out.write("\t\t.name = \"%s\",\n", event.name)

			if self.isEventhasUpdates(event):
				out.write("\t\t.updates = %s,\n",
						self.getEventUpdatesDescSymbol(event))
				out.write("\t\t.n_updates = OBUS_SIZEOF_ARRAY(%s),\n",
						self.getEventUpdatesDescSymbol(event))
			else:
				out.write("\t\t.updates = NULL,\n")
				out.write("\t\t.n_updates = 0,\n")

			out.write("\t}\n")
		out.write("\n};\n")

	def writeEventsApi(self, out, header):
		""" generate events api for client """
		if not self.obj.events:
			return

		if not header:
			out.write("\nstatic inline struct obus_event *\n"\
				"%s_obus_event(struct %s_event *event)\n", getObjectName(self.obj),
					 getObjectName(self.obj))
			out.write("{\n")
			out.write("\treturn event && (obus_event_get_object_desc("\
					"(struct obus_event *)event) == &%s_desc) ? "\
					"(struct obus_event *)event : NULL;\n", getObjectName(self.obj))
			out.write("}\n")

			out.write("\nstatic inline const struct obus_event *\n"\
					"%s_const_obus_event(const struct %s_event *event)\n",
					 getObjectName(self.obj), getObjectName(self.obj))
			out.write("{\n")
			out.write("\treturn event && (obus_event_get_object_desc("\
					"(const struct obus_event *)event) == &%s_desc) ? "\
					"(const struct obus_event *)event : NULL;\n",
					getObjectName(self.obj))
			out.write("}\n")


		if header:
			out.write("\n/**\n")
			out.write(" * @brief get %s event type.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * This function is used to retrieved %s event type.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @param[in]  event  %s event.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @retval     type   %s event type.\n", getObjectName(self.obj))
			out.write(" **/")
		else:
			out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

		out.write("\nenum %s_event_type\n%s_event_get_type(const struct "\
				"%s_event *event)%s\n", getObjectName(self.obj), getObjectName(self.obj),
				getObjectName(self.obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tconst struct obus_event_desc *desc;\n")
			out.write("\tdesc = obus_event_get_desc(%s_const_obus_event(event));\n",
					 getObjectName(self.obj))
			out.write("\treturn desc ? (enum %s_event_type)(desc - %s_events_desc)"\
					" : %s_EVENT_COUNT;\n", getObjectName(self.obj),
					getObjectName(self.obj), getObjectName(self.obj).upper())
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief log %s event.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * This function log %s event and its associated fields values.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @param[in]  event   %s event.\n", getObjectName(self.obj))
			out.write(" * @param[in]  level   obus log level.\n")
			out.write(" **/\n")

		out.write("void %s_event_log(const struct %s_event *event, "\
				"enum obus_log_level level)%s\n", getObjectName(self.obj),
				getObjectName(self.obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tobus_event_log(%s_const_obus_event(event), level);\n",
					getObjectName(self.obj))
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief check %s event contents is empty.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * This function check if each event field has its presence bit cleared.\n")
			out.write(" *\n")
			out.write(" * @param[in]  event   %s event.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @retval     1     Each field has its presence bit cleared.\n")
			out.write(" * @retval     0     One field (or more) has its presence bit set.\n")
			out.write(" **/\n")

		out.write("int %s_event_is_empty(const struct %s_event *event)%s\n", \
				getObjectName(self.obj), getObjectName(self.obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\treturn obus_event_is_empty(%s_const_obus_event(event));\n",
					 getObjectName(self.obj))
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief commit %s event contents in object.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * This function copy %s event contents in object.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @param[in]  event   %s event.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @retval     0     Commit succeed.\n")
			out.write(" * @retval     <0    Commit failed.\n")
			out.write(" *\n")
			out.write(" * @note: if not call by client, event commit is done internally\n")
			out.write(" * on client provider event callback return.\n")
			out.write(" **/\n")
		else:
			out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

		out.write("int %s_event_commit(struct %s_event *event)%s\n",
				getObjectName(self.obj), getObjectName(self.obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\treturn obus_event_commit(%s_obus_event(event));\n",
					 getObjectName(self.obj))
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief read %s event associated fields values.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * This function is used to read event fields values.\n")
			out.write(" *\n")
			out.write(" * @param[in]  event   %s event.\n", getObjectName(self.obj))
			out.write(" *\n")
			out.write(" * @retval  info  pointer to a constant object fields values.\n")
			out.write(" * @retval  NULL  event is NULL or not an %s object event.\n", getObjectName(self.obj))
			out.write(" **/\n")

		out.write("/* *INDENT-OFF* */\nconst struct %s_info *\n/* *INDENT-ON* */\n%s_event_get_info(const struct "\
				"%s_event *event)%s\n", getObjectName(self.obj), getObjectName(self.obj),
				getObjectName(self.obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\treturn (const struct %s_info *)obus_event_get_info("\
					"%s_const_obus_event(event));\n", getObjectName(self.obj),
					getObjectName(self.obj))
			out.write("}\n\n")
