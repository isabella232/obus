#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c.py
#
# @brief obus c source code generator
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

import os
import sys
import logging
from obusparser import ObusBus
from obusparser import ObusException
from obus_c_method import ObusMethodsWriter
from obus_c_event import ObusEventsWriter
from obus_c_bus_event import ObusBusEventsWriter
from obus_c_type import getType
from obus_c_type import getLibobusType
from obus_c_enum import ObusEnumWriter
from obus_c_utils import getObjectName, indentFile
from obusgen import Writer, writeHeader

def genHeader(out, obj, client, header):
	brief = 'obus {0} object {1} api'.format(getObjectName(obj), \
		'client' if client else 'server')

	writeHeader(out, brief)
	if header:
		guard = os.path.splitext(os.path.basename(out.fd.name))[0].upper()
		out.write("#ifndef _%s_H_\n", guard)
		out.write("#define _%s_H_\n", guard)
		out.write("\n")
		out.write("#include \"libobus.h\"\n")
		out.write("\n/* *INDENT-OFF* */\nOBUS_BEGIN_DECLS\n/* *INDENT-ON* */\n")
		out.write("\n")
	else:
		out.write("#ifndef _GNU_SOURCE\n")
		out.write("#define _GNU_SOURCE\n")
		out.write("#endif\n")
		out.write("\n")
		out.write("#include <stdio.h>\n")
		out.write("#include <stdlib.h>\n")
		out.write("#include <errno.h>\n")
		out.write("#include <stdint.h>\n")
		out.write("#include <string.h>\n")
		out.write("\n")
		out.write("#define OBUS_USE_PRIVATE\n")
		out.write("#include \"libobus.h\"\n")
		out.write("#include \"libobus_private.h\"\n")
		out.write("#include \"%s.h\"\n", getObjectName(obj))
		out.write("\n")

def genFooter(out, header):
	if header:
		guard = os.path.splitext(os.path.basename(out.fd.name))[0].upper()
		out.write("\n/* *INDENT-OFF* */\nOBUS_END_DECLS\n/* *INDENT-ON* */\n")
		out.write("\n#endif /*_%s_H_*/\n", guard)

def genObjectStruct(out, obj):
	# declare enum object fields

	first = True
	out.write("\nenum %s_field_type {\n", getObjectName(obj))
	for prop in obj.properties.values():
		out.write("\t%s_FIELD_%s%s,\n", getObjectName(obj).upper(),
			prop.name.upper(), ' = 0' if first else '')
		first = False

	for mtd in obj.methods.values():
		out.write("\t%s_FIELD_METHOD_%s%s,\n", getObjectName(obj).upper(),
			mtd.name.upper(), ' = 0' if first else '')
		first = False

	out.write("};\n")

	out.write("\nstatic const struct obus_field_desc %s_info_fields[] = {\n",
			getObjectName(obj))
	for prop in obj.properties.values():
		# out.write("/* *INDENT-OFF* */\n")
		out.write("\t[%s_FIELD_%s] = {\n", getObjectName(obj).upper(),
					prop.name.upper())
		# out.write("/* *INDENT-ON* */\n")
		out.write("\t\t.uid = %d,\n", prop.uid)
		out.write("\t\t.name = \"%s\",\n", prop.name)
		out.write("\t\t.offset = obus_offsetof(struct %s_info, %s),\n",
				getObjectName(obj), prop.name)

		out.write("\t\t.role = OBUS_PROPERTY,\n")
		out.write("\t\t.type = %s,\n", getLibobusType(prop.type))
		if prop.type.isEnum():
			out.write("\t\t.enum_drv = &%s,\n",
				ObusEnumWriter(prop.type.enum).getDriverSymbol())

		if prop.type.isArray():
			out.write("\t\t.nb_offset = obus_offsetof(struct %s_info, n_%s),"\
					"\n", getObjectName(obj), prop.name)
		# out.write("/* *INDENT-OFF* */\n")
		out.write("\t},\n")
		# out.write("/* *INDENT-ON* */\n")

	for mtd in obj.methods.values():
		# out.write("/* *INDENT-OFF* */\n")
		out.write("\t[%s_FIELD_METHOD_%s] = {\n", getObjectName(obj).upper(),
					mtd.name.upper())
		# out.write("/* *INDENT-ON* */\n")
		out.write("\t\t.uid = %d,\n", mtd.uid)
		out.write("\t\t.name = \"%s\",\n", mtd.name)
		out.write("\t\t.offset = obus_offsetof(struct %s_info, method_%s),\n",
				 getObjectName(obj), mtd.name)
		out.write("\t\t.role = OBUS_METHOD,\n")
		out.write("\t\t.type = OBUS_FIELD_ENUM,\n")
		out.write("\t\t.enum_drv = &obus_method_state_driver,\n")
		# out.write("/* *INDENT-OFF* */\n")
		out.write("\t},\n")
		# out.write("/* *INDENT-ON* */\n")
	out.write("\n};\n")

	out.write("\nstatic const struct obus_struct_desc %s_info_desc = {\n",
			getObjectName(obj))
	out.write("\t.size = sizeof(struct %s_info),\n", getObjectName(obj))
	out.write("\t.fields_offset = obus_offsetof(struct %s_info, fields),\n",
			 getObjectName(obj))
	out.write("\t.n_fields = OBUS_SIZEOF_ARRAY(%s_info_fields),\n",
			getObjectName(obj))
	out.write("\t.fields = %s_info_fields,\n", getObjectName(obj))
	out.write("};\n")

def genObjectDesc(out, obj):
	out.write("\nconst struct obus_object_desc %s_desc = {\n",
			getObjectName(obj))
	out.write("\t.uid = %s_%s_UID,\n", obj.bus.name.upper(), obj.name.upper())
	out.write("\t.name = \"%s\",\n", obj.name)
	out.write("\t.info_desc = &%s_info_desc,\n", getObjectName(obj))

	if obj.events:
		out.write("\t.n_events = OBUS_SIZEOF_ARRAY(%s_events_desc),\n",
				 getObjectName(obj))
		out.write("\t.events = %s_events_desc,\n", getObjectName(obj))
	else:
		out.write("\t.n_events = 0,\n")
		out.write("\t.events = NULL,\n")

	if obj.methods:
		out.write("\t.n_methods = OBUS_SIZEOF_ARRAY(%s_methods_desc),\n",
				 getObjectName(obj))
		out.write("\t.methods = %s_methods_desc,\n", getObjectName(obj))
	else:
		out.write("\t.n_methods = 0,\n")
		out.write("\t.methods = NULL,\n")

	out.write("};\n")

def genProviderApi(out, obj, options, header):
	if not options.client:
		return

	if header:
		out.write("\n/* %s object provider api */\n", getObjectName(obj))

		out.write("\n/**\n")
		out.write("\n * @struct %s_provider\n", getObjectName(obj))
		out.write("\n * @brief callbacks for events on %s objects\n",
				getObjectName(obj))
		out.write(" */\n")
		out.write("\nstruct %s_provider {\n", getObjectName(obj))
		out.write("\t/** for internal use only */\n")
		out.write("\tstruct obus_provider *priv;\n")
		out.write("\t/** called on a %s object apparition */\n",
				getObjectName(obj))
		out.write("\tvoid (*add) (struct %s *object, struct " \
				"%s_bus_event *bus_event, void *user_data);\n",
				getObjectName(obj), obj.bus.name)
		out.write("\t/** called on a %s object removal */\n",
				getObjectName(obj))
		out.write("\tvoid (*remove) (struct %s *object, struct " \
				"%s_bus_event *bus_event, void *user_data);\n",
				getObjectName(obj), obj.bus.name)
		out.write("\t/** called on %s object events */\n",
				getObjectName(obj))
		out.write("\tvoid (*event) (struct %s *object, struct %s_event "\
				"*event, struct %s_bus_event *bus_event, "\
				"void *user_data);\n", getObjectName(obj),
				getObjectName(obj), obj.bus.name)
		out.write("};\n")

	out.write("\n/**\n")
	out.write(" * @brief subscribe to events concerning %s objects.\n",
			getObjectName(obj))
	out.write(" *\n")
	out.write(" * @param[in] client bus client.\n")
	out.write(" * @param[in] provider callback set for reacting on %s events."\
			"\n", getObjectName(obj))
	out.write(" * @param[in] user_data data passed to callbacks on events.\n")
	out.write(" *\n")
	out.write(" * @retval 0 success.\n")
	out.write(" **/")

	out.write("\nint %s_subscribe(struct obus_client *client, struct "\
			"%s_provider *provider, void *user_data)%s\n", getObjectName(obj),
			 getObjectName(obj), (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\tstruct obus_provider *p;\n")
		out.write("\tint ret;\n")
		out.write("\tif (!client || !provider || !provider->add || "\
				"!provider->remove || !provider->event)\n")
		out.write("\t\treturn -EINVAL;\n")
		out.write("\n")
		out.write("\tp = calloc(1, sizeof(*p));\n")
		out.write("\tif (!p)\n")
		out.write("\t\treturn -ENOMEM;\n")
		out.write("\n")
		out.write("\tp->add = (obus_provider_add_cb_t)provider->add;\n")
		out.write("\tp->remove = (obus_provider_remove_cb_t)provider->remove;\n")
		out.write("\tp->event = (obus_provider_event_cb_t)provider->event;\n")
		out.write("\tp->desc = &%s_desc;\n", getObjectName(obj))
		out.write("\tp->user_data = user_data;\n")
		out.write("\n")
		out.write("\tret = obus_client_register_provider(client, p);\n")
		out.write("\tif (ret < 0) {\n")
		out.write("\t\tfree(p);\n")
		out.write("\t\treturn ret;\n")
		out.write("\t}\n")
		out.write("\n")
		out.write("\tprovider->priv = p;\n")
		out.write("\treturn 0;\n")
		out.write("}\n\n")

	out.write("\n/**\n")
	out.write(" * @brief unsubscribe to events concerning %s objects.\n",
			getObjectName(obj))
	out.write(" *\n")
	out.write(" * @param[in] client bus client.\n")
	out.write(" * @param[in] provider passed to %s_subscribe.\n",
			getObjectName(obj))
	out.write(" *\n")
	out.write(" * @retval 0 success.\n")
	out.write(" **/")

	out.write("int %s_unsubscribe(struct obus_client *client, "\
			"struct %s_provider *provider)%s\n", getObjectName(obj),
			getObjectName(obj), (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\tint ret;\n")
		out.write("\tif (!client || !provider)\n")
		out.write("\t\treturn -EINVAL;\n")
		out.write("\n")
		out.write("\tret = obus_client_unregister_provider(client, "\
				"provider->priv);\n")
		out.write("\tif (ret < 0)\n")
		out.write("\t\treturn ret;\n")
		out.write("\n")
		out.write("\tfree(provider->priv);\n")
		out.write("\tprovider->priv = NULL;\n")
		out.write("\treturn 0;\n")
		out.write("}\n")


def genObjectApi(out, obj, options, header):
	if header:
		out.write("\n/**\n")
		out.write(" * @brief %s object structure\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This opaque structure represent an %s object.\n", getObjectName(obj))
		out.write(" **/\n")
		out.write("struct %s;\n\n", getObjectName(obj))

		if options.client:
			out.write("/**\n")
			out.write(" * @brief %s object event structure\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This opaque structure represent an %s object event.\n", getObjectName(obj))
			out.write(" **/\n")
			out.write("struct %s_event;\n\n", getObjectName(obj))

		out.write("/**\n")
		out.write(" * @brief %s bus event structure\n", obj.bus.name)
		out.write(" *\n")
		out.write(" * This opaque structure represent an %s bus event.\n", obj.bus.name)
		out.write(" **/\n")
		out.write("struct %s_bus_event;\n", obj.bus.name)

		out.write("\n/**\n")
		out.write(" * @brief %s object info fields structure.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This structure contains a presence bit for each fields\n")
		out.write(" * (property or method state) in %s object.\n", getObjectName(obj))
		out.write(" * When a bit is set, the corresponding field in\n")
		out.write(" * @ref %s_info structure must be taken into account.\n", getObjectName(obj))
		out.write(" **/\n")
		out.write("struct %s_info_fields {\n", getObjectName(obj))
		for prop in obj.properties.values():
			out.write("\t/** %s field presence bit */\n", prop.name)
			out.write("\tunsigned int %s:1;\n", prop.name)
		for mtd in obj.methods.values():
			out.write("\t/** %s method presence bit */\n", mtd.name)
			out.write("\tunsigned int method_%s:1;\n", mtd.name)
		out.write("};\n")

		out.write("\n/**\n")
		out.write(" * @brief %s object info structure.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This structure represent %s object contents.\n", getObjectName(obj))
		out.write(" **/\n")
		out.write("struct %s_info {\n", getObjectName(obj))
		out.write("\t/** fields presence bit structure */\n")
		out.write("\tstruct %s_info_fields fields;\n", getObjectName(obj))
		for prop in obj.properties.values():
			if prop.desc:
				out.write("\t/** %s */\n", prop.desc)
			out.write("\t%s%s;\n", getType(prop.type), prop.name)
			if prop.type.isArray():
				out.write("\t/** size of %s array */\n", prop.name)
				out.write("\tuint32_t n_%s;\n", prop.name)

		for mtd in obj.methods.values():
			out.write("\t/** method %s state */\n", mtd.name)
			out.write("\tenum obus_method_state method_%s;\n", mtd.name)
		out.write("};\n")

	if not options.client and obj.methods:
		if header:
			# generate methods args desc
			out.write("\n")
			ObusMethodsWriter(obj).declareMethodsArgs(out, options.client)

			out.write("\n/**\n")
			out.write(" * @brief %s method handlers structure.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This structure contains pointer to %s methods implementations.\n", getObjectName(obj))
			out.write(" * Server must implement theses callback and fill up\n")
			out.write(" * this structure before creating a %s object.\n", getObjectName(obj))
			out.write(" **/\n")
			out.write("struct %s_method_handlers {\n", getObjectName(obj))
			first = True
			for mtd in obj.methods.values():
				if first:
					first = False
				else:
					out.write("\n")

				if mtd.args:
					args = 'const struct {0}_{1}_args *args'.format(\
							getObjectName(obj), mtd.name)
				else:
					args = 'void *unused'

				out.write("\t/**\n")
				out.write("\t * @brief %s method %s handler.\n", getObjectName(obj), mtd.name)
				out.write("\t *\n")
				if mtd.desc:
					out.write("\t * %s\n", mtd.desc)
					out.write("\t *\n")

				out.write("\t * @param[in]  object  %s object.\n", getObjectName(obj))
				out.write("\t * @param[in]  handle  client call sequence id.\n")
				if mtd.args:
					out.write("\t * @param[in]  args    method %s call arguments.\n", mtd.name)
				else:
					out.write("\t * @param[in]  unused  unused argument.\n")
				out.write("\t **/\n")
				out.write("\tvoid (*method_%s) (struct %s *object, "\
						"obus_handle_t handle, %s);\n", mtd.name,
						getObjectName(obj), args)
			out.write("};\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief check @ref %s_method_handlers structure is valid.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @param[in]  handlers  %s methods handlers.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval     1         all methods have non NULL handler.\n")
			out.write(" * @retval     0         one method (or more) has a NULL handler.\n")
			out.write(" **/")

		out.write("\nint %s_method_handlers_is_valid("\
			"const struct %s_method_handlers *handlers)%s\n", \
			getObjectName(obj), getObjectName(obj), (';' if header else ''))

		if not header:
			out.write("{\n")
			out.write("\treturn handlers")
			for mtd in obj.methods.values():
				out.write(" &&\n\t\thandlers->method_%s", mtd.name)
			out.write(";\n")
			out.write("}\n")

	if not options.client and obj.methods:
		for mtd in obj.methods.values():
				if not mtd.args:
					continue

				if header:
					out.write("\n/**\n")
					out.write(" * @brief check @ref %s_%s_args structure is complete (all arguments are present).\n", getObjectName(obj), mtd.name)
					out.write(" *\n")
					out.write(" * @param[in]  args %s %s method arguments.\n", getObjectName(obj), mtd.name)
					out.write(" *\n")
					out.write(" * @retval     1         all methods arguments are present.\n")
					out.write(" * @retval     0         one method (or more) argument is missing.\n")
					out.write(" **/")

				out.write("\nint {0}_{1}_args_is_complete("\
					"const struct {0}_{1}_args *args){2}\n".format( \
					getObjectName(obj), mtd.name, (';' if header else '')))

				if not header:
					out.write("{\n")
					out.write("\treturn args")
					for arg in mtd.args.values():
						out.write(" &&\n\t\targs->fields.%s", arg.name)
					out.write(";\n")
					out.write("}\n")

	if not options.client:
		if header:
			out.write("\n/**\n")
			out.write(" * @brief initialize @ref %s_info structure.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function initialize @ref %s_info structure.\n", getObjectName(obj))
			out.write(" * Each field has its presence bit cleared.\n")
			out.write(" *\n")
			out.write(" * @param[in]  info  pointer to allocated @ref %s_info structure.\n", getObjectName(obj))
			out.write(" **/")

		out.write("\nvoid %s_info_init(struct %s_info *info)%s\n",
			getObjectName(obj), getObjectName(obj), (';' if header else ''))

		if not header:
			out.write("{\n")
			out.write("\tif (info)\n")
			out.write("\t\tmemset(info, 0, sizeof(*info));\n")
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief check @ref %s_info structure contents is empty.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function check if each field has its presence bit cleared\n")
			out.write(" *\n")
			out.write(" * @param[in]  info  @ref %s_info structure.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval     1     Each field has its presence bit cleared.\n")
			out.write(" * @retval     0     One field (or more) has its presence bit set.\n")
			out.write(" **/")
		out.write("\nint %s_info_is_empty(const struct %s_info *info)%s\n",
				getObjectName(obj), getObjectName(obj),
				(';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\treturn \tinfo")
			for prop in obj.properties.values():
				out.write(" &&\n\t\t!info->fields.%s", prop.name)
			for mtd in obj.methods.values():
				out.write(" &&\n\t\t!info->fields.method_%s", mtd.name)
			out.write(";\n")
			out.write("}\n")

		if obj.methods:
			if header:
				out.write("\n/**\n")
				out.write(" * @brief set @ref %s_info methods state.\n", getObjectName(obj))
				out.write(" *\n")
				out.write(" * This function set all %s methods state to given argument state\n", getObjectName(obj))
				out.write(" *\n")
				out.write(" * @param[in]  info   @ref %s_info structure.\n", getObjectName(obj))
				out.write(" * @param[in]  state  new methods state.\n")
				out.write(" **/")

			out.write("\nvoid %s_info_set_methods_state(struct %s_info *info, "\
					"enum obus_method_state state)%s\n",
					getObjectName(obj), getObjectName(obj),
					(';' if header else ''))
			if not header:
				out.write("{\n")
				for mtd in obj.methods.values():
					out.write("\tOBUS_SET(info, method_%s, state);\n",
						mtd.name)
				out.write("}\n")

	if not header:
		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")
		out.write("\n/* *INDENT-OFF* */\nstatic inline struct %s *\n/* *INDENT-ON* */\n%s_from_object("\
			"struct obus_object *object)\n", getObjectName(obj),
			getObjectName(obj))
		out.write("{\n")
		out.write("\tconst struct obus_object_desc *desc;\n")
		out.write("\n")
		out.write("\tif (!object)\n")
		out.write("\t\treturn NULL;\n")
		out.write("\n")
		out.write("\tdesc = obus_object_get_desc(object);\n")
		out.write("\tif (desc != &%s_desc)\n", getObjectName(obj))
		out.write("\t\treturn NULL;\n")
		out.write("\n")
		out.write("\treturn (struct %s *)object;\n", getObjectName(obj))
		out.write("}\n")

		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")
		out.write("\n/* *INDENT-OFF* */\nstatic inline struct obus_object *\n/* *INDENT-ON* */\n%s_object(struct %s "\
			"*object)\n", getObjectName(obj), getObjectName(obj))
		out.write("{\n")
		out.write("\tstruct obus_object *obj;\n")
		out.write("\n")
		out.write("\tobj = (struct obus_object *)object;\n")
		out.write("\tif (%s_from_object(obj) != object)\n", getObjectName(obj))
		out.write("\t\treturn NULL;\n")
		out.write("\n")
		out.write("\treturn (struct obus_object *)object;\n")
		out.write("}\n")

		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")
		out.write("\n/* *INDENT-OFF* */\nstatic inline const struct obus_object *\n/* *INDENT-ON* */\n%s_const_object"\
			"(const struct %s *object)\n", getObjectName(obj),
			getObjectName(obj))
		out.write("{\n")
		out.write("\tconst struct obus_object *obj;\n")
		out.write("\n")
		out.write("\tobj = (const struct obus_object *)object;\n")
		out.write("\tif (obus_object_get_desc(obj) != &%s_desc)\n", getObjectName(obj))
		out.write("\t\treturn NULL;\n")
		out.write("\n")
		out.write("\treturn obj;\n")
		out.write("}\n")

	if not options.client:
		if header:
			out.write("\n/**\n")
			out.write(" * @brief create a %s object.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function allocate %s object and initialize each\n", getObjectName(obj))
			out.write(" * object field with the given @ref info field value only if @ref info field\n")
			out.write(" * presence bit is set (see @ref %s_info_fields).\n", getObjectName(obj))
			out.write(" * This function also set a unique handle for this object.\n")
			out.write(" * Registering or unregistering object in bus do not alter handle value\n");
			out.write(" *\n")
			out.write(" * @param[in]  server    %s bus server.\n", obj.bus.name)
			out.write(" * @param[in]  info      %s object initial values (may be NULL).\n", getObjectName(obj))
			if obj.methods:
				out.write(" * @param[in]  handlers  %s method handlers.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  object  success.\n")
			out.write(" * @retval  NULL    failure.\n")
			out.write(" *\n")
			out.write(" * @note: object does not keep any reference to @ref info so that user shall\n")
			out.write(" * allocate it on the stack.\n")
			out.write(" *\n")
			out.write(" * @note: if @ref info param is NULL, %s object is initialized with\n", getObjectName(obj))
			out.write(" * default values.\n")
			out.write(" *\n")
			out.write(" * @note: object is not yet registered on bus,\n")
			out.write(" * call one of theses functions below to register it:\n")
			out.write(" * @ref %s_register\n", getObjectName(obj))
			out.write(" * @ref %s_bus_event_register_%s\n", obj.bus.name, obj.name)
			out.write(" **/")
		else:
			out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

		out.write("\n/* *INDENT-OFF* */\nstruct %s *\n/* *INDENT-ON* */\n%s_new(struct obus_server *srv, const struct %s_info *info",
				getObjectName(obj), getObjectName(obj), getObjectName(obj))
		if obj.methods:
			out.write(", const struct %s_method_handlers *handlers",
					getObjectName(obj))
		out.write(")%s\n", (';' if header else ''))

		if not header:
			out.write("{\n")
			if obj.methods:
				out.write("\tobus_method_handler_cb_t cbs[%s_METHOD_COUNT];\n",
						getObjectName(obj).upper())
			else:
				out.write("\tobus_method_handler_cb_t *cbs = NULL;\n")
			out.write("\tstruct obus_struct st = {\n\t\t.u.const_addr = info,\n\t\t"\
					".desc = %s_desc.info_desc\n\t};\n", getObjectName(obj))

			out.write("\n")
			for mtd in obj.methods.values():
				out.write("\tcbs[%s_METHOD_%s] = (obus_method_handler_cb_t)"\
						"handlers->method_%s;\n", \
						getObjectName(obj).upper(), mtd.name.upper(), mtd.name)
			out.write("\n")
			out.write("\treturn (struct %s*)obus_server_new_object(srv, &%s_desc, cbs,"\
					" info ? &st : NULL);\n", getObjectName(obj),
					getObjectName(obj))
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief destroy a %s object.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function release %s object memory.\n", getObjectName(obj))
			out.write(" * Only non registered objects can be destroyed.\n")
			out.write(" *\n")
			out.write(" * @param[in]  object  %s object to be destroyed.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  0       success.\n")
			out.write(" * @retval  -EPERM  @ref object is registered.\n")
			out.write(" * @retval  -EINVAL invalid @ref object.\n")
			out.write(" **/\n")

		out.write("int %s_destroy(struct %s *object)%s\n", getObjectName(obj),
				 getObjectName(obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\treturn obus_object_destroy(%s_object(object));\n",
					getObjectName(obj))
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief register a %s object.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function register a %s object in %s bus.\n", getObjectName(obj), obj.bus.name)
			out.write(" *\n")
			out.write(" * @param[in]  server  %s bus server.\n", obj.bus.name)
			out.write(" * @param[in]  object  %s object to be registered.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  0       success.\n")
			out.write(" * @retval  -EPERM  @ref object is already registered.\n")
			out.write(" * @retval  -EINVAL invalid parameters.\n")
			out.write(" * @retval  < 0     other errors.\n")
			out.write(" **/\n")
		out.write("int %s_register(struct obus_server *server, "\
				"struct %s *object)%s\n", getObjectName(obj),
				getObjectName(obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tstruct obus_object *obj = (struct obus_object *)object;\n");
			out.write("\treturn obus_server_register_object(server, obj);\n")
			out.write("}\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief unregister a %s object.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function unregister a %s object in %s bus.\n", getObjectName(obj), obj.bus.name)
			out.write(" *\n")
			out.write(" * @param[in]  server  %s bus server.\n", obj.bus.name)
			out.write(" * @param[in]  object  %s object to be unregistered.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  0       success.\n")
			out.write(" * @retval  -EPERM  @ref object is not registered.\n")
			out.write(" * @retval  -EINVAL invalid parameters.\n")
			out.write(" * @retval  < 0     other errors.\n")
			out.write(" **/\n")

		out.write("int %s_unregister(struct obus_server *server, "\
				"struct %s *object)%s\n", getObjectName(obj),
				getObjectName(obj), (';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tstruct obus_object *obj = (struct obus_object *)object;\n");
			out.write("\treturn obus_server_unregister_object(server, obj);\n")
			out.write("}\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief check if is a %s object registered.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function check whether a %s object is registered or not.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @param[in]  object  %s object to checked.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  0  object is not registered.\n")
			out.write(" * @retval  1  object is registered.\n")
			out.write(" **/\n")

		out.write("int %s_is_registered(const struct %s *object)%s\n",
				getObjectName(obj), getObjectName(obj),
				(';' if header else ''))
		if not header:
			out.write("{\n")
			out.write("\tconst struct obus_object *obj = (const struct obus_object *)object;\n");
			out.write("\treturn obus_object_is_registered(obj);\n")
			out.write("}\n")

	if header:
		out.write("\n/**\n")
		out.write(" * @brief read current %s object fields values.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function is used to read current object fields values.\n")
		out.write(" *\n")
		out.write(" * @param[in]  object  %s object.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @retval  info  pointer to a constant object fields values.\n")
		out.write(" * @retval  NULL  object is NULL or not an %s object.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @note: object info pointer returned never changed during object life cycle\n")
		out.write(" * so that user may keep a reference on this pointer until object destruction.\n")
		out.write(" * this is not the case for info pointers members.\n")
		out.write(" **/\n")
	else:
		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

	out.write("\n/* *INDENT-OFF* */\nconst struct %s_info *\n/* *INDENT-ON* */\n%s_get_info(const struct %s"\
			" *object)%s\n", getObjectName(obj), getObjectName(obj),
			getObjectName(obj), (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\treturn (const struct %s_info *)obus_object_get_info("\
				"%s_const_object(object));\n", getObjectName(obj),
				getObjectName(obj))
		out.write("}\n\n")

	if header:
		out.write("\n/**\n")
		out.write(" * @brief log %s object.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function log object and its current fields values.\n")
		out.write(" *\n")
		out.write(" * @param[in]  object  %s object.\n", getObjectName(obj))
		out.write(" * @param[in]  level   obus log level.\n")
		out.write(" **/\n")

	out.write("void %s_log(const struct %s *object, enum obus_log_level "\
			"level)%s\n", getObjectName(obj), getObjectName(obj),
			(';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\tobus_object_log(%s_const_object(object), level);\n",
				getObjectName(obj))
		out.write("}\n\n")

	if header:
		out.write("\n/**\n")
		out.write(" * @brief set %s object user data pointer.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function store a user data pointer in a %s object.\n", getObjectName(obj))
		out.write(" * This pointer is never used by libobus and can be retrieved using\n")
		out.write(" * @ref %s_get_user_data function.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @param[in]  object      %s object.\n", getObjectName(obj))
		out.write(" * @param[in]  user_data   user data pointer.\n")
		out.write(" *\n")
		out.write(" * @retval  0        success.\n")
		out.write(" * @retval  -EINVAL  object is NULL.\n")
		out.write(" **/\n")

	out.write("int %s_set_user_data(struct %s *object, void *user_data)"\
			"%s\n", getObjectName(obj), getObjectName(obj),
			(';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\treturn obus_object_set_user_data(%s_object(object), "\
				"user_data);\n", getObjectName(obj))
		out.write("}\n\n")

	if header:
		out.write("\n/**\n")
		out.write(" * @brief get %s object user data pointer.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function retrieve user data pointer stored in a %s object\n", getObjectName(obj))
		out.write(" * by a previous call to @ref %s_set_user_data function\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @param[in]  object  %s object.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @retval  user_data  user data pointer.\n")
		out.write(" **/\n")

	out.write("void *%s_get_user_data(const struct %s *object)%s\n", \
		getObjectName(obj), getObjectName(obj), (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\treturn obus_object_get_user_data(" \
				"%s_const_object(object));\n", getObjectName(obj))
		out.write("}\n\n")

	if header:
		out.write("\n/**\n")
		out.write(" * @brief get registered %s object obus handle.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function retrieve %s object obus handle.\n", getObjectName(obj))
		out.write(" * obus handle is an unsigned 16 bits integer.\n")
		out.write(" * object handle is generated during object creation.\n")
		out.write(" * object handle can be used to reference an object into another one.\n")
		out.write(" *\n")
		out.write(" * @param[in]  object  %s object.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @retval  handle               registered object obus handle.\n")
		out.write(" * @retval  OBUS_INVALID_HANDLE  if object is not registered.\n")
		out.write(" **/\n")
	else:
		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

	out.write("obus_handle_t %s_get_handle(const struct %s *object)%s\n",
			getObjectName(obj), getObjectName(obj), (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\treturn obus_object_get_handle("\
				"%s_const_object(object));\n", getObjectName(obj))
		out.write("}\n\n")

	if header:
		out.write("\n/**\n")
		out.write(" * @brief get %s object from obus handle.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function retrieve %s object given its obus handle.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @param[in]  %s  %s bus %s\n", 'client' if options.client else 'server', obj.bus.name, 'client' if options.client else 'server')
		out.write(" * @param[in]  handle  %s object handle.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @retval  object  %s object.\n", getObjectName(obj))
		out.write(" * @retval  NULL    invalid parameters.\n",)
		out.write(" * @retval  NULL    corresponding handle object is not a %s.\n", getObjectName(obj))
		out.write(" **/\n")
	else:
		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

	mode = 'client' if options.client else 'server'
	out.write("/* *INDENT-OFF* */\nstruct %s *\n/* *INDENT-ON* */\n%s_from_handle(struct obus_%s *%s, "\
			"obus_handle_t handle)%s\n", getObjectName(obj), getObjectName(obj),
			mode, mode, (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\tstruct obus_object *obj;\n")
		out.write("\n")
		out.write("\tobj = obus_%s_get_object(%s, handle);", mode, mode)
		out.write("\treturn %s_from_object(obj);\n", getObjectName(obj))
		out.write("}\n\n")


	if header:
		out.write("\n/**\n")
		out.write(" * @brief get next registered %s object in bus.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * This function retrieve the next registered %s object in bus.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @param[in]  %s    %s bus %s\n", 'client' if options.client else 'server', obj.bus.name, 'client' if options.client else 'server')
		out.write(" * @param[in]  previous  previous %s object in list (may be NULL).\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @retval  object  next %s object.\n", getObjectName(obj))
		out.write(" * @retval  NULL    invalid parameters.\n",)
		out.write(" * @retval  NULL    no more %s objects in bus.\n", getObjectName(obj))
		out.write(" *\n")
		out.write(" * @note: if @p previous is NULL, then the first\n")
		out.write(" * registered %s object is returned.\n", getObjectName(obj))
		out.write(" **/\n")
	else:
		out.write("/* *INDENT-COMMENT-FIX-ISSUE* */\n")

	out.write("/* *INDENT-OFF* */\nstruct %s *\n/* *INDENT-ON* */\n%s_next(struct obus_%s *%s, struct %s "\
			"*previous)%s\n", getObjectName(obj), getObjectName(obj), mode,
			mode, getObjectName(obj), (';' if header else ''))
	if not header:
		out.write("{\n")
		out.write("\tstruct obus_object *next, *prev;\n")
		out.write("\n")
		out.write("\tprev = (struct obus_object *)previous;\n")
		out.write("\tnext = obus_%s_object_next(%s, prev, %s_desc.uid);\n",
			mode, mode, getObjectName(obj))
		out.write("\treturn %s_from_object(next);\n", getObjectName(obj))
		out.write("}\n")

	# generate send event for server
	if not options.client and obj.events:
		if header:
			out.write("\n/**\n")
			out.write(" * @brief send a %s object event.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * This function send an event on a %s object.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @param[in]  server    %s bus server.\n", obj.bus.name)
			out.write(" * @param[in]  object    %s object.\n", getObjectName(obj))
			out.write(" * @param[in]  type      %s event type.\n", getObjectName(obj))
			out.write(" * @param[in]  info      associated %s content to be updated.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  0          event sent and object content updated.\n")
			out.write(" * @retval  -EINVAL    invalid parameters.\n")
			out.write(" * @retval  -EPERM     object is not registered in bus.\n")
			out.write(" *\n")
			out.write(" * @note: Partial info members copy is done inside function.\n")
			out.write(" * No reference to info members is kept.\n")
			out.write(" **/")

		out.write("\nint %s_send_event(struct obus_server *server, struct %s"\
				" *object, enum %s_event_type type, const struct %s_info "\
				"*info)%s\n", getObjectName(obj), getObjectName(obj),
				getObjectName(obj), getObjectName(obj),
				(';' if header else ''))

		if not header:
			out.write("{\n")
			out.write("\tint ret;\n")
			out.write("\tstruct obus_event event;\n")
			out.write("\tstruct obus_struct st = {\n\t\t.u.const_addr = info,\n\t\t"\
					".desc = %s_desc.info_desc\n\t};\n", getObjectName(obj))
			out.write("\n")
			out.write("\tif (!object || !server || type >= %s_EVENT_COUNT)\n",
					getObjectName(obj).upper())
			out.write("\t\treturn -EINVAL;\n")
			out.write("\n")
			out.write("\tret = obus_event_init(&event, %s_object(object), "\
					"&%s_events_desc[type], &st);\n", getObjectName(obj),
					getObjectName(obj))
			out.write("\tif (ret < 0)\n")
			out.write("\t\treturn ret;\n")
			out.write("\n")
			out.write("\treturn obus_server_send_event(server, &event);\n")
			out.write("}\n\n")

	if not options.client:
		if obj.events:
			if header:
				out.write("\n/**\n")
				out.write(" * @brief send a %s object event through a %s bus event.\n", getObjectName(obj), obj.bus.name)
				out.write(" *\n")
				out.write(" * This function create a %s object event and attach it\n", getObjectName(obj))
				out.write(" * to an existing %s bus event. Created %s object event\n", obj.bus.name, getObjectName(obj))
				out.write(" * will be sent within corresponding bus event.\n",)
				out.write(" * Unlike @%s_send_event, object content will not be updated\n", getObjectName(obj))
				out.write(" * when this function returns but when\n")
				out.write(" * @%s_bus_event_send will be invoked.\n", getObjectName(obj))
				out.write(" *\n")
				out.write(" * @param[in]  bus_event  %s bus event.\n", obj.bus.name)
				out.write(" * @param[in]  object     %s object.\n", getObjectName(obj))
				out.write(" * @param[in]  type       %s event type.\n", getObjectName(obj))
				out.write(" * @param[in]  info       associated %s content to be updated.\n", getObjectName(obj))
				out.write(" *\n")
				out.write(" * @retval  0          event created and associated to bus event.\n")
				out.write(" * @retval  -EINVAL    invalid parameters.\n")
				out.write(" * @retval  -ENOMEM    memory error.\n")
				out.write(" *\n")
				out.write(" * @note: Partial info members copy is done inside function.\n")
				out.write(" * No reference to info members is kept.\n")
				out.write(" **/\n")

			out.write("int %s_bus_event_add_%s_event(struct %s_bus_event "\
					"*bus_event, struct %s"\
					" *object, enum %s_event_type type, const struct %s_info "\
					"*info)%s\n", obj.bus.name, obj.name, obj.bus.name,
					getObjectName(obj), getObjectName(obj),
					getObjectName(obj), (';' if header else ''))

			if not header:
				out.write("{\n")
				out.write("\tint ret;\n")
				out.write("\tstruct obus_event *event;\n")
				out.write("\tstruct obus_struct st = {\n\t\t.u.const_addr = info,\n\t\t"\
					".desc = %s_desc.info_desc\n\t};\n", getObjectName(obj))
				out.write("\n")
				out.write("\tif (!object || !bus_event || type >= %s_EVENT_COUNT)\n",
						getObjectName(obj).upper())
				out.write("\t\treturn -EINVAL;\n")
				out.write("\n")
				out.write("\tevent = obus_event_new(%s_object(object), "\
						"&%s_events_desc[type], &st);\n", getObjectName(obj),
						getObjectName(obj))
				out.write("\tif (!event)\n")
				out.write("\t\treturn -ENOMEM;\n")
				out.write("\n")
				out.write("\tret = obus_bus_event_add_event("\
						"(struct obus_bus_event *)bus_event, event);\n")
				out.write("\tif (ret < 0)\n")
				out.write("\t\tobus_event_destroy(event);\n")
				out.write("\n")
				out.write("\treturn ret;\n")
				out.write("}\n\n")

			if header:
				out.write("\n/**\n")
				out.write(" * @brief register a %s object through a %s bus event.\n", getObjectName(obj), obj.bus.name)
				out.write(" *\n")
				out.write(" * This function set a %s object to be registered when\n", getObjectName(obj))
				out.write(" * associated %s bus event will be sent. Unlike @%s_register,\n", obj.bus.name, getObjectName(obj))
				out.write(" * object will not be registered when this function returns but when\n")
				out.write(" * @%s_bus_event_send will be invoked.\n", getObjectName(obj))
				out.write(" *\n")
				out.write(" * @param[in]  bus_event  %s bus event.\n", obj.bus.name)
				out.write(" * @param[in]  object     %s object to be registered.\n", getObjectName(obj))
				out.write(" *\n")
				out.write(" * @retval  0          object registration request associated to bus event.\n")
				out.write(" * @retval  -EINVAL    invalid parameters.\n")
				out.write(" * @retval  -EPERM     object is already attached to an existing bus event.\n")
				out.write(" *\n")
				out.write(" **/\n")

		out.write("int %s_bus_event_register_%s(struct %s_bus_event "\
				"*bus_event, struct %s *object)%s\n", obj.bus.name, obj.name,
				obj.bus.name, getObjectName(obj),
				(';' if header else ''))

		if not header:
			out.write("{\n")
			out.write("\treturn obus_bus_event_register_object("\
					"(struct obus_bus_event *)bus_event, %s_object(object));\n",
					 getObjectName(obj))
			out.write("}\n\n")

		if header:
			out.write("\n/**\n")
			out.write(" * @brief unregister a %s object through a %s bus event.\n", getObjectName(obj), obj.bus.name)
			out.write(" *\n")
			out.write(" * This function set %s object to be unregistered when\n", getObjectName(obj))
			out.write(" * associated %s bus event will be sent. Unlike @%s_register,\n", obj.bus.name, getObjectName(obj))
			out.write(" * object will not be unregistered when this function returns but when\n")
			out.write(" * @%s_bus_event_send will be invoked.\n", getObjectName(obj))
			out.write(" * Object is then automatically destroyed on\n")
			out.write(" * @%s_bus_event_destroy call.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @param[in]  bus_event  %s bus event.\n", obj.bus.name)
			out.write(" * @param[in]  object     %s object to be registered.\n", getObjectName(obj))
			out.write(" *\n")
			out.write(" * @retval  0          object registration request associated to bus event.\n")
			out.write(" * @retval  -EINVAL    invalid parameters.\n")
			out.write(" * @retval  -EPERM     object is already attached to an existing bus event.\n")
			out.write(" *\n")
			out.write(" **/\n")

		out.write("int %s_bus_event_unregister_%s(struct %s_bus_event "\
				"*bus_event, struct %s *object)%s\n",
				obj.bus.name, obj.name, obj.bus.name, getObjectName(obj),
				(';' if header else ''))

		if not header:
			out.write("{\n")
			out.write("\treturn obus_bus_event_unregister_object("\
					"(struct obus_bus_event *)bus_event, %s_object(object));\n",
					 getObjectName(obj))
			out.write("}\n\n")

def genBusHeader(out, bus, client, header):
	brief = 'obus {0} bus {1} api'.format(bus.name, \
		'client' if client else 'server')
	writeHeader(out, brief)
	if header:
		guard = os.path.splitext(os.path.basename(out.fd.name))[0].upper()
		out.write("#ifndef _%s_H_\n", guard)
		out.write("#define _%s_H_\n", guard)
		out.write("\n")
		out.write("#include \"libobus.h\"\n")
		out.write("\n/* *INDENT-OFF* */\nOBUS_BEGIN_DECLS\n/* *INDENT-ON* */\n")
		out.write("\n")
	else:
		out.write("#ifndef _GNU_SOURCE\n")
		out.write("#define _GNU_SOURCE\n")
		out.write("#endif\n")
		out.write("\n")
		out.write("#include <stdio.h>\n")
		out.write("#include <stdlib.h>\n")
		out.write("#include <errno.h>\n")
		out.write("#include <stdint.h>\n")
		out.write("#include <string.h>\n")
		out.write("\n")
		out.write("#define OBUS_USE_PRIVATE\n")
		out.write("#include \"libobus.h\"\n")
		out.write("#include \"libobus_private.h\"\n")
		out.write("#include \"%s_bus.h\"\n", bus.name)
		out.write("\n")

def genBusFooter(out, header):
	if header:
		guard = os.path.splitext(os.path.basename(out.fd.name))[0].upper()
		out.write("\n/* *INDENT-OFF* */\nOBUS_END_DECLS\n/* *INDENT-ON* */\n")
		out.write("\n#endif /*_%s_H_*/\n", guard)

#===============================================================================
# Generate object file.
#===============================================================================
def genObject(obj, options, header):
	# create object file
	fileName = getObjectName(obj) + ('.h' if header else '.c')
	filePath = os.path.join(options.outdir, fileName)
	out = Writer(filePath)
	genHeader(out, obj, options.client, header)

	# generate object uid
	if header:
		out.write("/**\n")
		out.write(" * @brief %s object uid\n", getObjectName(obj))
		out.write(" **/\n")
		out.write("#define %s_%s_UID %d\n", obj.bus.name.upper(),
				obj.name.upper(), obj.uid)

	# generate enums
	for enum in obj.enums.values():
		if header:
			ObusEnumWriter(enum).writeDeclaration(out)
		else:
			ObusEnumWriter(enum).writeDriver(out)

	# generate object events
	if header:
		ObusEventsWriter(obj).declareEvents(out)
	else:
		# generate object struct
		genObjectStruct(out, obj)
		# generate events
		ObusEventsWriter(obj).writeEventsDesc(out)

	ObusEventsWriter(obj).writeEventTypeStr(out, header)

	if not header:
		# generate object methods struct
		ObusMethodsWriter(obj).writeMethodsDesc(out)
		# generate object desc
		genObjectDesc(out, obj)

	# generate object api
	genObjectApi(out, obj, options, header)

	# generate events api for client
	if options.client:
		ObusEventsWriter(obj).writeEventsApi(out, header)

	if options.client:
		# generate methods api
		ObusMethodsWriter(obj).writeMethodsApi(out, header)
		# generate object provider api
		genProviderApi(out, obj, options, header)

	genFooter(out, header)
	out.close()
	indentFile(filePath)

#===============================================================================
# Generate bus file.
#===============================================================================
def genBus(bus, options, header):
	# create object file
	fileName = bus.name + '_bus' + ('.h' if header else '.c')
	filePath = os.path.join(options.outdir, fileName)
	out = Writer(filePath)
	genBusHeader(out, bus, options.client, header)

	if header:
		out.write("\n/**\n")
		out.write(" * @brief %s bus descriptor.\n", bus.name)
		out.write(" *\n")
		out.write(" * Reference to %s bus descripor.\n", bus.name)
		out.write(" **/\n")
		out.write("extern const struct obus_bus_desc *%s_bus_desc;\n", bus.name)

		ObusBusEventsWriter(bus).declareEventStruct(out)
		ObusBusEventsWriter(bus).declareEvents(out)
		ObusBusEventsWriter(bus).writeEventTypeStr(out, header)
		ObusBusEventsWriter(bus).writeEventsApi(out, options, header)
	else:

		# write events desc
		ObusBusEventsWriter(bus).writeEventsDesc(out)

		out.write("\n/* referenced objects supported by %s bus */\n", bus.name)
		for obj in bus.objects.values():
			out.write("extern const struct obus_object_desc %s_desc;\n",
				getObjectName(obj))

		out.write("\n/* array of %s objects descriptors */", bus.name)
		out.write("\nstatic const struct obus_object_desc *const objects[] = {\n")
		for obj in bus.objects.values():
			out.write("\t&%s_desc,\n", getObjectName(obj))
		out.write("};\n")

		out.write("/* %s bus description */", bus.name)
		out.write("\nstatic const struct obus_bus_desc %s_desc = {\n",
				 bus.name)
		out.write("\t.name = \"%s\",\n", bus.name)
		out.write("\t.n_objects = OBUS_SIZEOF_ARRAY(objects),\n")
		out.write("\t.objects = objects,\n")
		if bus.events:
			out.write("\t.n_events = OBUS_SIZEOF_ARRAY(%s),\n", ObusBusEventsWriter(bus).getEventsDescSymbol())
			out.write("\t.events = %s,\n", ObusBusEventsWriter(bus).getEventsDescSymbol())
		else:
			out.write("\t.n_events = 0,\n")
			out.write("\t.events = NULL,\n")

		out.write("\t.crc = 0,\n")
		out.write("};\n")

		out.write("\n/*public reference to %s  */\n", bus.name)
		out.write("const struct obus_bus_desc *%s_bus_desc = &%s_desc;\n", bus.name, bus.name)

		ObusBusEventsWriter(bus).writeEventTypeStr(out, header)
		ObusBusEventsWriter(bus).writeEventsApi(out, options, header)

	genBusFooter(out, header)
	out.close()
	indentFile(filePath)

#===============================================================================
#===============================================================================
def listFiles(options, bus):
	fileNames = [bus.name + '_bus.h', bus.name + '_bus.c']
	for obj in bus.objects.values():
		fileNames.append(getObjectName(obj) + '.h')
		fileNames.append(getObjectName(obj) + '.c')

	filePaths = [os.path.join(options.outdir, fileName)
			for fileName in fileNames]

	sys.stdout.write(" ".join(filePaths) + "\n")

#===============================================================================
#===============================================================================
def main(options, xmlFile):

	try:
		bus = ObusBus(xmlFile)
	except ObusException as ex:
		logging.error(ex)
		sys.exit(1)
	else:
		if options.listFiles:
			listFiles(options, bus)
			return

		# gernerate bus .c file
		genBus(bus, options, True)
		# gernerate bus .h file
		genBus(bus, options, False)

		# Generate bus file
		for name in bus.objects.keys():
			# genereate object .c file
			genObject(bus.objects[name], options, True)
			# genereate object .h file
			genObject(bus.objects[name], options, False)

#===============================================================================
# For local testing.
#===============================================================================
if __name__ == "__main__":
	class _Options:
		def __init__(self):
			self.outdir = "."
			self.client = True
			self.lang = "c"
	main(_Options(), sys.argv[1])
