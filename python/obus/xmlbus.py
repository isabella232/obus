#===============================================================================
# obus-python - obus client python module.
#
# @file utils.py
#
# @brief obus python utilities
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

import types
import xml.etree.ElementTree as ET

import obus
import obus.data

_log = logging.getLogger("obus")

#===============================================================================
# Obus type class.
#===============================================================================
class _ObusType(object):
	def __init__(self, rawType):
		self.isArray = False
		self.isEnum = False
		self.rawType = rawType
		# Split array/enum from rawType
		if self.rawType.startswith("array:"):
			self.isArray = True
			self.rawType = self.rawType[6:]
		if self.rawType.startswith("enum:"):
			self.isEnum = True
			self.rawType = self.rawType[5:]

		self.driver = None
		if self.rawType == "bool":
			self.type = obus.FieldType.OBUS_FIELD_BOOL
		elif self.rawType == "uint8":
			self.type = obus.FieldType.OBUS_FIELD_U8
		elif self.rawType == "int8":
			self.type = obus.FieldType.OBUS_FIELD_I8
		elif self.rawType == "uint16":
			self.type = obus.FieldType.OBUS_FIELD_U16
		elif self.rawType == "int16":
			self.type = obus.FieldType.OBUS_FIELD_I16
		elif self.rawType == "uint32":
			self.type = obus.FieldType.OBUS_FIELD_U32
		elif self.rawType == "int32":
			self.type = obus.FieldType.OBUS_FIELD_I32
		elif self.rawType == "uint64":
			self.type = obus.FieldType.OBUS_FIELD_U64
		elif self.rawType == "int64":
			self.type = obus.FieldType.OBUS_FIELD_I64
		elif self.rawType == "handle":
			self.type = obus.FieldType.OBUS_FIELD_U16
		elif self.rawType == "string":
			self.type = obus.FieldType.OBUS_FIELD_STRING
		elif self.isEnum:
			self.type = obus.FieldType.OBUS_FIELD_ENUM
		elif self.rawType == "float":
			self.type = obus.FieldType.OBUS_FIELD_F32
		elif self.rawType == "double":
			self.type = obus.FieldType.OBUS_FIELD_F64
		else:
			raise Exception("Unknown type: " + self.rawType)

		if self.isArray:
			self.type |= obus.FieldType.OBUS_FIELD_ARRAY


#===============================================================================
#===============================================================================
def _genObjClass(ctx, objClassName):
	data = (
		"import obus.data\n"
		"class %s(obus.data.ObusObject):\n"
		"    def __init__(self, objDesc, struct):\n"
		"        obus.data.ObusObject.__init__(self, objDesc, struct)\n"
	) % objClassName
	exec data in ctx
	return ctx[objClassName]

#===============================================================================
#===============================================================================
def _genEvtClass(ctx, evtClassName):
	data = (
		"import obus.data\n"
		"class %s(obus.data.ObusEvent):\n"
		"    def __init__(self, evtDesc, obj, struct):\n"
		"        obus.data.ObusEvent.__init__(self, evtDesc, obj, struct)\n"
	) % evtClassName
	exec data in ctx
	return ctx[evtClassName]

#===============================================================================
#===============================================================================
def _genBusEvtClass(ctx, busEvtClassName):
	data = (
		"import obus.data\n"
		"class %s(obus.data.ObusBusEvent):\n"
		"    def __init__(self, busEvtDesc):\n"
		"        obus.data.ObusBusEvent.__init__(self, busEvtDesc)\n"
	) % busEvtClassName
	exec data in ctx
	return ctx[busEvtClassName]

#===============================================================================
#===============================================================================
def _genCreateCallFct(ctx, createCallFctName, xmlMtd):
	data = (
		"def %s(obj, **kwargs):\n"
		"    call = obus.data.ObusCall.create(obj.desc.methodsDesc[%d], obj)\n"
		""
		"    for paramName in kwargs.keys():\n"
		"        if paramName == \"_nolog\":\n"
		"            continue\n"
		"        if call.desc.structDesc.findField(paramName) is None:\n"
		"            raise ValueError(\"Param %%s unknown\" %% (paramName))\n"
		""
		"    for fieldDesc in call.desc.structDesc.fieldsDesc.values():\n"
		"        if fieldDesc.name not in kwargs:\n"
		"            if not \"_nolog\" in kwargs:\n"
		"                _log.warning(\"%s: Missing argument %%s\", fieldDesc.name)\n"
		"        elif fieldDesc.isEnum() and not fieldDesc.isEnumItemValid(kwargs[fieldDesc.name]):\n"
		"            raise ValueError(\"Invalid value %%s for param %%s\" %% (kwargs[fieldDesc.name], fieldDesc.name))\n"
		"        else:\n"
		"           call.struct.setField(fieldDesc, kwargs[fieldDesc.name])\n"
		"    return call\n"
	) % (createCallFctName, int(xmlMtd.get("uid")), createCallFctName)
	exec data in ctx
	return ctx[createCallFctName]

#===============================================================================
#===============================================================================
def _genAccessors(cls, fieldsDesc):
	for fieldDesc in fieldsDesc.values():
		name = fieldDesc.name
		fieldDescGen = "x.struct.desc.fieldsDesc[%d]" % fieldDesc.uid
		namespace = {}
		exec ("hasField = lambda x: x.struct.hasField(%s)" % fieldDescGen) \
				in namespace
		exec ("getField = lambda x: x.struct.getField(%s)" % fieldDescGen) \
				in namespace
		exec ("setField = lambda x,y: x.struct.setField(%s, y)" % fieldDescGen) \
				in namespace
		setattr(cls, "has_field_" + name, namespace["hasField"])
		setattr(cls, "get_" + name, namespace["getField"])
		setattr(cls, "set_" + name, namespace["setField"])

#===============================================================================
#===============================================================================
def _genStructDesc(xmlItem, objClass):
	structDesc = obus.StructDesc()
	# Properties
	for xmlProp in xmlItem.findall("property"):
		obusType = _ObusType(xmlProp.get("type"))
		if obusType.isEnum:
			obusType.driver = getattr(objClass,
					"_driver_" + obusType.rawType.title())
		structDesc.addField(obus.FieldDesc(
				xmlProp.get("name"),
				int(xmlProp.get("uid")),
				obusType.type,
				obus.FieldRole.OBUS_PROPERTY,
				obusType.driver,
				xmlProp.get("type")))
	# Methods state
	for xmlMethod in xmlItem.findall("method"):
		structDesc.addField(obus.FieldDesc(
				"method_state_" + xmlMethod.get("name"),
				int(xmlMethod.get("uid")),
				obus.FieldType.OBUS_FIELD_ENUM,
				obus.FieldRole.OBUS_METHOD,
				obus.FIELD_DRIVER_METHOD_STATE,
				"enum:method_state"))
	# Arguments of methods
	for xmlArg in xmlItem.findall("arg"):
		obusType = _ObusType(xmlArg.get("type"))
		if obusType.isEnum:
			obusType.driver = getattr(objClass,
					"_driver_" + obusType.rawType.title())
		structDesc.addField(obus.FieldDesc(
				xmlArg.get("name"),
				int(xmlArg.get("uid")),
				obusType.type,
				obus.FieldRole.OBUS_ARGUMENT,
				obusType.driver,
				xmlArg.get("type")))
	return structDesc

#===============================================================================
#===============================================================================
def loadBus(xmlFile):
	# Parse the xml
	try:
		xmlTree = ET.parse(xmlFile)
	except ET.ParseError as ex:
		raise obus.DecodeError(str(ex))
	xmlBus = xmlTree.getroot()
	busName = xmlBus.get("name")

	# Create a new module for the bus data
	module = types.ModuleType(busName)
	setattr(module, "_log", _log)

	# Create bus descriptor
	busDesc = obus.BusDesc(busName, 0)

	# Process objects in bus
	for xmlObj in xmlBus.findall("object"):
		objName = xmlObj.get("name")
		objClassName = busName.title() + objName.title()
		objClass = _genObjClass(module.__dict__, objClassName)

		# Create an enum object in objClass
		for xmlEnum in xmlObj.findall("enum"):
			enumName = xmlEnum.get("name")
			items = {}
			for xmlLabel in xmlEnum.findall("label"):
				items[xmlLabel.get("name").upper()] = int(xmlLabel.get("value"), 0)
			enumObj = obus.Enum(objClassName + enumName.title(), items)
			defVal = getattr(enumObj, xmlEnum.get("default").upper())
			driver = obus.FieldDriverEnum(enumObj, defVal)
			setattr(objClass, enumName.title(), enumObj)
			setattr(objClass, "_driver_" + enumName.title(), driver)

		# Create an evtClass in objClass
		evtClassName = "_" + objClassName + "Event"
		evtClass = _genEvtClass(module.__dict__, evtClassName)
		items = {}
		for xmlEvt in xmlObj.findall("event"):
			items[xmlEvt.get("name").upper()] = len(items)
		evtTypeEnum = obus.Enum(objClassName + "Event.Type", items)
		setattr(evtClass, "Type", evtTypeEnum)
		setattr(objClass, "Event", evtClass)

		# Create structure descriptor
		structDesc = _genStructDesc(xmlObj, objClass)

		# Generate accessors in objClass and evtClass
		_genAccessors(objClass, structDesc.fieldsDesc)
		_genAccessors(evtClass, structDesc.fieldsDesc)

		# Create object descriptor
		objDesc = obus.ObjectDesc(objName,
				int(xmlObj.get("uid")),
				structDesc,
				objClass)
		setattr(objClass, "OBJECT_DESC", objDesc)

		# Register event descriptors in object
		for xmlEvt in xmlObj.findall("event"):
			# create event
			event =  obus.EventDesc(
					xmlEvt.get("name"),
					int(xmlEvt.get("uid")),
					getattr(evtTypeEnum, xmlEvt.get("name").upper()),
					structDesc,
					evtClass)

			# find update fields for this event
			for update in xmlEvt.findall("update"):
				fieldDesc = None
				name = update.get('property')
				if not name:
					name = "method_state_" + update.get('method')

				if name:
					fieldDesc = structDesc.findField(name)

				# add update desc
				if fieldDesc:
					event.addUpdateDesc(obus.EventUpdateDesc(fieldDesc, 0))

			objDesc.addEvent(event)

		# Create methods
		for xmlMtd in xmlObj.findall("method"):
			createCallFctName = "_" + objClassName + "_createCall_" + xmlMtd.get("name")
			createCallFct = _genCreateCallFct(module.__dict__, createCallFctName, xmlMtd)
			structDescMtd = _genStructDesc(xmlMtd, objClass)
			objDesc.addMethod(obus.MethodDesc(
					xmlMtd.get("name"),
					int(xmlMtd.get("uid")),
					structDescMtd,
					obus.data.ObusCall))
			setattr(objClass, "createCall_" + xmlMtd.get("name"), createCallFct)

		# Register object descriptor in bus
		busDesc.addObject(objDesc)

	# Create a busEvtClass in module
	busEvtClassName = "_BusEvent"
	busEvtClass = _genBusEvtClass(module.__dict__, busEvtClassName)
	items = {}
	for xmlBusEvt in xmlBus.findall("event"):
		items[xmlBusEvt.get("name").upper()] = len(items)
	busEvtTypeEnum = obus.Enum("_BusEvent.Type", items)
	setattr(busEvtClass, "Type", busEvtTypeEnum)
	setattr(module, "Event", busEvtClass)

	# Bus events descriptors
	for xmlBusEvt in xmlBus.findall("event"):
		busDesc.addEvent(obus.BusEventDesc(
					xmlBusEvt.get("name"),
					int(xmlBusEvt.get("uid")),
					getattr(busEvtTypeEnum, xmlBusEvt.get("name").upper()),
					obus.data.ObusBusEvent
				))

	# Save bus descriptor
	module.BUS_DESC = busDesc

	return module
