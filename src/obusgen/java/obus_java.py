#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_java.py
#
# @brief obus java source code generator
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

import sys, os, errno
import logging
from obusparser import ObusBus
from obusparser import ObusException
from obusparser import ObusType
from obusgen import Writer, writeHeader

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

# Get the name of the package, based on bus name
# def getPackageName(busName):
# 	return busName + "bus"

# Get the name of the bus class, based on bus name
def getClassNameBus(busName):
	return toCamelcase(busName.title()) + "Bus"

# Get the name of the bus event interface , based on bus name
def getInterfaceNameBus(busName):
	return "I" + toCamelcase(busName.title()) + "BusEvent"

# Get the name of an object class, based on bus name and object name
def getClassNameObject(busName, objectName):
	return toCamelcase(busName.title()) + toCamelcase(objectName.title())

# Get the name of an mock object class, based on bus name and object name
def getClassNameMockObject(busName, objectName):
	return "Mock" + toCamelcase(busName.title()) + toCamelcase(objectName.title())

# Get the name of an object interface, based on bus name and object name
def getIntfNameObject(busName, objectName):
	return "I" + getClassNameObject(busName, objectName)

# Get the name of an enum class, based on enum name
def getEnumClassName(enumName):
	return toCamelcase(enumName)

# Get the name of a call class, based on method name
def getClassNameCall(methodName):
	return "Call" + toCamelcase(methodName)

# Get the name of a method call, based on method name
def getCallMethodName(methodName):
	return toCamelcase(methodName, True)

# Get the name of an enum value , based on enum name
def getEnumValueName(enum):
	return enum.upper()

# Get the name of a busEventDesc instance
def getBusEventDescName(evt):
	return "busEventDesc" + toCamelcase(evt)

# Get the name of a eventDesc instance
def getEventDescName(evt):
	return "eventDesc" + toCamelcase(evt)

# Get the name of a fieldDesc instance
def getFieldDescName(field):
	return "fieldDesc" + toCamelcase(field)

# Get the name of a property checker method
def getPropCheckerMethodName(prop):
	return "has" + toCamelcase(prop)

# Get the name of a property getter method
def getPropGetterMethodName(prop):
	return "get" + toCamelcase(prop)

# Get the name of a property setter method
def getPropSetterMethodName(prop):
	return "set" + toCamelcase(prop)

# Get the name of the parametre for the setter method
def getPropSetterParamName(prop):
	return toCamelcase(prop, True)

# Get the name of a property
def getPropName(prop):
	return toCamelcase(prop, True)

# Get the name of the prefix before a method state checker and getter
def getMethodStateNamePrefix(method):
	return "method_state_" + method

#===============================================================================
# Obus type class.
#===============================================================================

_typeMap = {
		ObusType.Type.UINT8: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_U8"),
		ObusType.Type.INT8: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_I8"),
		ObusType.Type.UINT16: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_U16"),
		ObusType.Type.INT16: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_I16"),
		ObusType.Type.UINT32: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_U32"),
		ObusType.Type.INT32: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_I32"),
		ObusType.Type.UINT64: ("Long", "setFieldLong", "getFieldLong", "FieldType.OBUS_FIELD_U64"),
		ObusType.Type.INT64: ("Long", "setFieldLong", "getFieldLong", "FieldType.OBUS_FIELD_I64"),
		ObusType.Type.HANDLE: ("int", "setFieldInt", "getFieldInt", "FieldType.OBUS_FIELD_U16"),
		ObusType.Type.STRING: ("String", "setFieldString", "getFieldString", "FieldType.OBUS_FIELD_STRING"),
		ObusType.Type.BOOL: ("boolean", "setFieldBool", "getFieldBool", "FieldType.OBUS_FIELD_BOOL"),
		ObusType.Type.FLOAT: ("Float", "setFieldFloat", "getFieldFloat", "FieldType.OBUS_FIELD_F32"),
		ObusType.Type.DOUBLE: ("Double", "setFieldDouble", "getFieldDouble", "FieldType.OBUS_FIELD_F64"),
}

class MyObusType(object):
	def __init__(self, obusType):
		self._obusType = obusType
		if obusType.isEnum():
			rawType = getEnumClassName(obusType.enum.name)
			self.paramType = self.returnType = rawType
			self.setField = "setFieldEnum"
			self.getField = "getFieldEnum"
			self.fieldType = "FieldType.OBUS_FIELD_ENUM"
			self.driver = rawType + "Driver"
			if obusType.isArray():
				self.cast = "(" + rawType + "[])"
			else:
				self.cast = "(" + rawType + ")"
		else:
			self.paramType = self.returnType = _typeMap[obusType.base][0]
			self.setField = _typeMap[obusType.base][1]
			self.getField = _typeMap[obusType.base][2]
			self.fieldType = _typeMap[obusType.base][3]
			self.driver = "null"
			self.cast = ""

		if obusType.isArray():
			self.paramType += "[]"
			self.returnType += "[]"
			self.setField += "Array"
			self.getField += "Array"
			self.fieldType += " | FieldType.OBUS_FIELD_ARRAY"

class MethodStateType(object):
	def __init__(self):
		self.paramType = "MethodState"
		self.returnType = "MethodState"
		self.driver = "Descriptor.FIELD_DRIVER_METHOD_STATE"
		self.setField = "setFieldEnum"
		self.getField = "getFieldEnum"
		self.fieldType = "FieldType.OBUS_FIELD_ENUM"
		self.cast = "(MethodState)"

#===============================================================================
# Generate bus class file.
#===============================================================================
def genBus(options, bus, out):
	busName = bus.name
	busClassName = getClassNameBus(busName)

	# Import packages
	out.write("package %s.impl;\n", options.javaPackage)
	out.write("\n")
	out.write("import com.parrot.obus.internal.Core.ObusBusEventCreator;\n")
	out.write("import com.parrot.obus.internal.Descriptor.BusDesc;\n")
	out.write("import com.parrot.obus.internal.Descriptor.BusEventDesc;\n")
	out.write("import com.parrot.obus.internal.ObusBusEvent;\n")
	out.write("import %s.%s;\n", options.javaPackage, getInterfaceNameBus(busName))
	out.write("\n")

	# Bus class
	out.write("/** Bus */\n")
	out.write("@SuppressWarnings(\"javadoc\")\n");
	out.write("public class %s {\n", busClassName)

	# Bus Event class
	out.write("\t/** Bus event */\n")
	out.write("\tpublic static final class Event extends ObusBusEvent implements %s {\n", getInterfaceNameBus(busName))

	# Event constructor
	out.write("\t\t/** */\n")
	out.write("\t\tprivate Event(BusEventDesc busEvtDesc) {\n")
	out.write("\t\t\tsuper(busEvtDesc);\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Event type getter
	out.write("\t\t/** */\n")
	out.write("\t\t@Override\n")
	out.write("\t\tpublic Type getType() {\n")
	out.write("\t\t\treturn (Type)this.desc.busEvtType;\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Event creator
	out.write("\t\t/** Event creator */\n")
	out.write("\t\tprivate static final ObusBusEventCreator<Event> creator =")
	out.write(" new ObusBusEventCreator<Event>() {\n")
	out.write("\t\t\t@Override\n")
	out.write("\t\t\tpublic Event create(BusEventDesc busEvtDesc) {\n")
	out.write("\t\t\t\treturn new Event(busEvtDesc);\n")
	out.write("\t\t\t}\n")
	out.write("\t\t};\n")
	out.write("\n")

	# Event descriptors
	out.write("\t\t/** Event descriptors */\n")
	for evt in bus.events.values():
		out.write("\t\tprivate static final BusEventDesc %s = new BusEventDesc(\n", getBusEventDescName(evt.name))
		out.write("\t\t\t\t\"%s\",\n", evt.name)
		out.write("\t\t\t\t%s,\n", evt.uid)
		out.write("\t\t\t\tType.%s,\n", getEnumValueName(evt.name))
		out.write("\t\t\t\tEvent.creator);\n")
	out.write("\t}\n")
	out.write("\n")

	# Bus descriptor
	out.write("\t/** Bus descriptor */\n")
	out.write("\tpublic static final BusDesc busDesc = new BusDesc(\"%s\", 0);\n", busName)
	out.write("\tstatic {\n")
	for obj in bus.objects.values():
		objClassName = getClassNameObject(busName, obj.name)
		out.write("\t\t%s.busDesc.addObject(%s.objectDesc);\n", busClassName, objClassName)
	for evt in bus.events.values():
		out.write("\t\t%s.busDesc.addEvent(Event.%s);\n", busClassName, getBusEventDescName(evt.name))
	out.write("\t}\n")
	out.write("}\n")

#===============================================================================
# Generate bus event interface file.
#===============================================================================
def genBusEventIntf(options, bus, out):
	busName = bus.name
	busClassName = getInterfaceNameBus(busName)

	# Import packages
	out.write("package %s;\n", options.javaPackage)
	out.write("\n")

	# Bus class
	out.write("@SuppressWarnings(\"javadoc\")\n");
	out.write("/** Bus Event interface */\n")
	out.write("public interface %s {\n", busClassName)

	# Event type enum
	out.write("\t/** Event type */\n")
	out.write("\tpublic static enum Type {\n")
	out.write("\t\t" + ",\n\t\t".join([
			"/** %s */\n\t\t%s" % (evt.desc or "", getEnumValueName(evt.name)) for evt in bus.events.values()
		]))
	out.write("\n\t}\n")
	out.write("\n")

	# Event type getter
	out.write("\t/** Event type getter */\n")
	out.write("\tpublic Type getType();\n")
	out.write("\n")

	out.write("}\n")


#===============================================================================
#===============================================================================
class AccessorKind(object):
	(QUERY, GET, SET) = range(0, 3)

#===============================================================================
# Generate accessors intreface  for a structure.
#===============================================================================
def genStructInftAccessors(structClassName, items, isMethodState, indent, kind, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
			myObusType = MethodStateType()
		else:
			name = item.name
			myObusType = MyObusType(item.type)

		if kind == AccessorKind.QUERY:
			out.write(indent + "public boolean %s();\n", getPropCheckerMethodName(name))

		elif kind == AccessorKind.GET:
			out.write(indent + "public %s %s();\n", myObusType.returnType, getPropGetterMethodName(name))

		elif kind == AccessorKind.SET:
			out.write(indent + "public void %s(%s %s);\n", getPropSetterMethodName(name), myObusType.paramType, getPropSetterParamName(name))

	out.write("\n")

#===============================================================================
# Generate accessors for a structure.
#===============================================================================
def genStructAccessors(structClassName, items, isMethodState, isOverride, indent, kind, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
			myObusType = MethodStateType()
		else:
			name = item.name
			myObusType = MyObusType(item.type)

		if isOverride:
			out.write(indent + "@Override\n")

		if kind == AccessorKind.QUERY:
			out.write(indent + "public boolean %s() {\n", getPropCheckerMethodName(name))
			out.write(indent + "\treturn this.struct.hasField(%s.%s);\n", structClassName, getFieldDescName(name))
			out.write(indent + "}\n")

		elif kind == AccessorKind.GET:
			out.write(indent + "public %s %s() {\n", myObusType.returnType, getPropGetterMethodName(name))
			out.write(indent + "\treturn %sthis.struct.%s(%s.%s);\n", myObusType.cast, myObusType.getField, structClassName, getFieldDescName(name))
			out.write(indent + "}\n")

		elif kind == AccessorKind.SET:
			out.write(indent + "public void %s(%s %s) {\n", getPropSetterMethodName(name), myObusType.paramType, getPropSetterParamName(name))
			out.write(indent + "\tthis.struct.%s(%s.%s, %s);\n", myObusType.setField, structClassName, getFieldDescName(name), getPropSetterParamName(name))
			out.write(indent + "}\n")

	out.write("\n")

#===============================================================================
# Generate accessors for a mock structure.
#===============================================================================
def genMockStructAccessors(structClassName, items, isMethodState, isOverride, indent, kind, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
			myObusType = MethodStateType()
		else:
			name = item.name
			myObusType = MyObusType(item.type)

		if isOverride:
			out.write(indent + "@Override\n")

		if kind == AccessorKind.QUERY:
			out.write(indent + "public boolean %s() {\n", getPropCheckerMethodName(name))
			out.write(indent + "\treturn this.%s;\n", getPropCheckerMethodName(name))
			out.write(indent + "}\n")

		elif kind == AccessorKind.GET:
			out.write(indent + "public %s %s() {\n", myObusType.returnType, getPropGetterMethodName(name))
			out.write(indent + "\treturn this.%s;\n", getPropName(name))
			out.write(indent + "}\n")

		elif kind == AccessorKind.SET:
			out.write(indent + "public void %s(%s %s) {\n", getPropSetterMethodName(name), myObusType.paramType, getPropSetterParamName(name))
			out.write(indent + "\tthis.%s = %s;\n", getPropName(name), getPropSetterParamName(name))
			out.write(indent + "}\n")

	out.write("\n")

#===============================================================================
# Generate setter for a mock structure that also set the checker field
#===============================================================================
def genMockEventStructSetter(structClassName, items, isMethodState, indent, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
			myObusType = MethodStateType()
		else:
			name = item.name
			myObusType = MyObusType(item.type)

		out.write(indent + "public void %s(%s %s) {\n", getPropSetterMethodName(name), myObusType.paramType, getPropSetterParamName(name))
		out.write(indent + "\tthis.%s = %s;\n", getPropName(name), getPropSetterParamName(name))
		out.write(indent + "\tthis.%s = true;\n", getPropCheckerMethodName(name))
		out.write(indent + "}\n")

	out.write("\n")

#===============================================================================
# Generate field for a structure.
#===============================================================================
def genStructFields(items, isMethodState, indent, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
			myObusType = MethodStateType()
		else:
			name = item.name
			myObusType = MyObusType(item.type)

		out.write(indent + "private %s %s;\n", myObusType.paramType, getPropName(name))

	out.write("\n")


#===============================================================================
# Generate field checker status for a structure.
#===============================================================================
def genStructCheckerFields(items, isMethodState, indent, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
		else:
			name = item.name

		out.write(indent + "private boolean %s;\n", getPropCheckerMethodName(name))

	out.write("\n")


#===============================================================================
# Generate field descriptors for a structure.
#===============================================================================
def genStructFieldsDesc(items, isMethodState, indent, kind, out):
	for item in items:
		if isMethodState:
			name = getMethodStateNamePrefix(item.name)
			myObusType = MethodStateType()
		else:
			name = item.name
			myObusType = MyObusType(item.type)

		out.write(indent + "private static final FieldDesc %s = new FieldDesc(\n", getFieldDescName(name))
		out.write(indent + "\t\t\"%s\",\n", name)
		out.write(indent + "\t\t%s,\n", item.uid)
		out.write(indent + "\t\t%s,\n", myObusType.fieldType)
		out.write(indent + "\t\tFieldRole.%s,\n", kind)
		out.write(indent + "\t\t%s);\n", myObusType.driver)

	out.write("\n")

#===============================================================================
# Generate structure descriptor.
#===============================================================================
def genStructDesc(structClassName, item, indent, out):
	out.write(indent + "private static final StructDesc structDesc = new StructDesc();\n")
	out.write(indent + "static {\n")

	if hasattr(item, "properties"):
		for prop in item.properties.values():
			out.write(indent + "\t%s.structDesc.addField(%s.%s);\n", structClassName, structClassName, getFieldDescName(prop.name))
	if hasattr(item, "methods"):
		for mtd in item.methods.values():
			out.write(indent + "\t%s.structDesc.addField(%s.%s);\n", structClassName, structClassName, getFieldDescName(getMethodStateNamePrefix(mtd.name)))
	if hasattr(item, "args"):
		for arg in item.args.values():
			out.write(indent + "\t%s.structDesc.addField(%s.%s);\n", structClassName, structClassName, getFieldDescName(arg.name))
	out.write(indent + "}\n")

	out.write("\n")


#===============================================================================
# Generate object event Interface (inside object interface itself).
#===============================================================================
def genObjEvtIntf(obj, objIntfName, out):
	# Event intreface
	out.write("\t/** Event */\n")
	out.write("\tpublic interface IEvent extends IBusObjectEvent {\n")
	# Event type enum
	out.write("\t\t/** Event type */\n")
	out.write("\t\tpublic static enum Type {\n")
	out.write("\t\t\t" + ",\n\t\t\t".join([
			"/** %s */\n\t\t\t%s" % (evt.desc or "", getEnumValueName(evt.name)) for evt in obj.events.values()
		]))
	out.write("\n\t\t}\n")
	out.write("\n")

	# Event type getter
	out.write("\t\t/** Event type getter */\n")
	out.write("\t\t@Override\n")
	out.write("\t\tpublic Type getType();\n")
	out.write("\n")

	# Property checkers
	out.write("\t\t/** Property checkers */\n")
	genStructInftAccessors(objIntfName, obj.properties.values(), False, "\t\t", AccessorKind.QUERY, out)

	# Method state checkers
	out.write("\t\t/** Method state checkers */\n")
	genStructInftAccessors(objIntfName, obj.methods.values(), True, "\t\t", AccessorKind.QUERY, out)

	# Property getters
	out.write("\t\t/** Property getters */\n")
	genStructInftAccessors(objIntfName, obj.properties.values(), False, "\t\t", AccessorKind.GET, out)

	# Method state getters
	out.write("\t\t/** Method state getters */\n")
	genStructInftAccessors(objIntfName, obj.methods.values(), True, "\t\t", AccessorKind.GET, out)

	out.write("\t}\n")


#===============================================================================
# Generate object event class (inside object class itself).
#===============================================================================
def genObjEvtClass(obj, objClassName, objIntfName, out):
	# Event class
	out.write("\t/** Event */\n")
	out.write("\tpublic static final class Event extends ObusEvent implements %s.IEvent {\n", objIntfName)

	# Event constructor
	out.write("\t\t/** */\n")
	out.write("\t\tprivate Event(EventDesc evtDesc, ObusObject obj, ObusStruct struct) {\n")
	out.write("\t\t\tsuper(evtDesc, obj, struct);\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Event type getter
	out.write("\t\t/** */\n")
	out.write("\t\t@Override\n")
	out.write("\t\tpublic Type getType() {\n")
	out.write("\t\t\treturn (Type)this.desc.evtType;\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Property checkers
	out.write("\t\t/** Property checkers */\n")
	genStructAccessors(objClassName, obj.properties.values(), False, True, "\t\t", AccessorKind.QUERY, out)

	# Method state checkers
	out.write("\t\t/** Method state checkers */\n")
	genStructAccessors(objClassName, obj.methods.values(), True, True, "\t\t", AccessorKind.QUERY, out)

	# Property getters
	out.write("\t\t/** Property getters */\n")
	genStructAccessors(objClassName, obj.properties.values(), False, True, "\t\t", AccessorKind.GET, out)

	# Method state getters
	out.write("\t\t/** Method state getters */\n")
	genStructAccessors(objClassName, obj.methods.values(), True, True, "\t\t", AccessorKind.GET, out)

	# Event creator
	out.write("\t\t/** Event creator */\n")
	out.write("\t\tprivate static final ObusEventCreator<Event> creator =")
	out.write(" new ObusEventCreator<Event>(){\n")
	out.write("\t\t\t@Override\n")
	out.write("\t\t\tpublic Event create(EventDesc evtDesc, ObusObject obj, ObusStruct struct) {\n")
	out.write("\t\t\t\treturn new Event(evtDesc, obj, struct);\n")
	out.write("\t\t\t}\n")
	out.write("\t\t};\n")
	out.write("\t}\n")
	out.write("\n")


#===============================================================================
# Generate Mock object event class (inside mock object class itself).
#===============================================================================
def genMockObjEvtClass(obj, objClassName, objIntfName, out):
	# Event class
	out.write("\t/** Event */\n")
	out.write("\tpublic static final class Event extends MockBusObjectEvent implements %s.IEvent {\n", objIntfName)

	out.write("\n")
	out.write("\t\tprivate Type type;\n")
	# Event properties
	genStructFields(obj.properties.values(), False, "\t\t", out)
	# Method state event properties
	genStructFields(obj.methods.values(), True, "\t\t", out)
	# Event properties checker state
	genStructCheckerFields(obj.properties.values(), False, "\t\t", out)
	# Method state checker properties
	genStructCheckerFields(obj.methods.values(), True, "\t\t", out)

	# Event constructor
	out.write("\t\t/** event constructor */\n")
	out.write("\t\tpublic Event(%s object, Type type) {\n", objClassName)
	out.write("\t\t\tsuper(object);\n")
	out.write("\t\t\tthis.type = type;\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Event type getter
	out.write("\t\t/** Event type getter */\n")
	out.write("\t\t@Override\n")
	out.write("\t\tpublic Type getType() {\n")
	out.write("\t\t\treturn this.type;\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Property checkers
	out.write("\t\t/** Property checkers */\n")
	genMockStructAccessors(objClassName, obj.properties.values(), False, True, "\t\t", AccessorKind.QUERY, out)

	# Method state checkers
	out.write("\t\t/** Method state checkers */\n")
	genMockStructAccessors(objClassName, obj.methods.values(), True, True, "\t\t", AccessorKind.QUERY, out)

	# Property getters
	out.write("\t\t/** Property getters */\n")
	genMockStructAccessors(objClassName, obj.properties.values(), False, True, "\t\t", AccessorKind.GET, out)

	# Method state getters
	out.write("\t\t/** Method state getters */\n")
	genMockStructAccessors(objClassName, obj.methods.values(), True, True, "\t\t", AccessorKind.GET, out)

	# Property setters
	out.write("\t\t/** Property setters */\n")
	genMockEventStructSetter(objClassName, obj.properties.values(), False, "\t\t", out)

	# Method state setters
	out.write("\t\t/** Method state setters */\n")
	genMockEventStructSetter(objClassName, obj.methods.values(), True, "\t\t", out)

	out.write("\t}\n")
	out.write("\n")

#===============================================================================
# Generate object call class (inside object class itself).
#===============================================================================
def genObjCallClass(mtd, out):
	callClassName = getClassNameCall(mtd.name)

	out.write("\t/** Call class */\n")
	out.write("\tpublic static final class %s extends ObusCall {\n", callClassName)

	# Call constructor
	out.write("\t\t/** */\n")
	out.write("\t\tprotected %s(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {\n", callClassName)
	out.write("\t\t\tsuper(mtdDesc, obj, struct);\n")
	out.write("\t\t}\n")
	out.write("\n")

	# Argument getters
	out.write("\t\t/** Argument getters */\n")
	genStructAccessors(callClassName, mtd.args.values(), False, False, "\t\t", AccessorKind.GET, out)

	# Argument setters
	out.write("\t\t/** Argument setters */\n")
	genStructAccessors(callClassName, mtd.args.values(), False, False, "\t\t", AccessorKind.SET, out)

	# Argument descriptors
	out.write("\t\t/** Argument descriptors */\n")
	genStructFieldsDesc(mtd.args.values(), False, "\t\t", "OBUS_ARGUMENT", out)

	# Structure descriptor
	out.write("\t\t/** Structure descriptor */\n")
	genStructDesc(callClassName, mtd, "\t\t", out)

	# Call creator
	out.write("\t\t/** Call creator */\n")
	out.write("\t\tprivate static final ObusCallCreator<%s> creator =", callClassName)
	out.write(" new ObusCallCreator<%s>() {\n", callClassName)
	out.write("\t\t\t@Override\n")
	out.write("\t\t\tpublic %s create(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {\n", callClassName)
	out.write("\t\t\t\treturn new %s(mtdDesc, obj, struct);\n", callClassName)
	out.write("\t\t\t}\n")
	out.write("\t\t};\n")
	out.write("\n")

	# Method descriptor
	out.write("\t\t/** Method descriptor */\n")
	out.write("\t\tprivate static final MethodDesc methodDesc = new MethodDesc(\n")
	out.write("\t\t\t\t\"%s\",\n", mtd.name)
	out.write("\t\t\t\t%s,\n", mtd.uid)
	out.write("\t\t\t\t%s.structDesc,\n", callClassName)
	out.write("\t\t\t\t%s.creator);\n", callClassName)

	out.write("\t}\n\n")


#===============================================================================
# Generate object enum  (inside object interface itself).
#===============================================================================
def genObjIntfEnum(obj, enum, out):
	enumClassName = getEnumClassName(enum.name)

	out.write("\t/** %s */\n", enum.desc or "")
	out.write("\tpublic enum %s {\n", enumClassName)
	out.write("\t\t" + ",\n\t\t".join([
			"/** %s */\n\t\t%s" % (val.desc or "", getEnumValueName(val.name)) for val in enum.values.values()
		]))
	out.write(";\n")
	out.write("\t}\n")
	out.write("\n")

#===============================================================================
# Generate object enum class (inside object class itself).
#===============================================================================
def genObjEnum(obj, enum, out):
	enumClassName = getEnumClassName(enum.name)

	out.write("\t\tpublic static final FieldDriverEnum<%s> %sDriver =", enumClassName, enumClassName)
	out.write(" new FieldDriverEnum<%s>(%s.class) {\n", enumClassName, enumClassName)

	out.write("\t\t@Override\n")
	out.write("\t\tpublic %s init() {\n", enumClassName)
	out.write("\t\t\treturn %s.%s;\n", enumClassName, getEnumValueName(enum.default))
	out.write("\t\t}\n")

	out.write("\t\t@Override\n")
	out.write("\t\tpublic int toInt(%s val) {\n", enumClassName)
	out.write("\t\t\tswitch (val) {\n")
	for val in enum.values.values():
		out.write("\t\t\t\tcase %s: return %s;\n", getEnumValueName(val.name), val.value)
	out.write("\t\t\t\tdefault: return 0;\n")
	out.write("\t\t\t}\n")
	out.write("\t\t}\n")

	out.write("\t\t@Override\n")
	out.write("\t\tpublic %s fromInt(int val) throws DecodeError {\n", enumClassName)
	out.write("\t\t\tswitch (val) {\n")
	for val in enum.values.values():
		out.write("\t\t\t\tcase %s: return %s.%s;\n", val.value, enumClassName, getEnumValueName(val.name))
	out.write("\t\t\tdefault: throw new DecodeError(\"Invalid %s %s: \" + val);\n", obj.name, enumClassName)
	out.write("\t\t\t}\n")
	out.write("\t\t}\n")

	out.write("\t};\n")
	out.write("\n")

#===============================================================================
# Generate object interface file.
#===============================================================================
def genObjIntf(options, bus, obj, out):
	busName = bus.name
	objName = obj.name
	objIntfName = getIntfNameObject(busName, objName)

	# Import packages
	out.write("package %s;\n", options.javaPackage)
	out.write("\n")
	out.write("import com.parrot.obus.MethodState;\n")
	out.write("import com.parrot.obus.IBusObject;\n")
	out.write("import com.parrot.obus.BusClient;\n")
	out.write("import com.parrot.obus.IBusObjectEvent;\n")
	out.write("import com.parrot.obus.IBusMethodCall;\n")
	out.write("\n")

	# Object class
	out.write("/** %s */\n", obj.desc or "")
	out.write("@SuppressWarnings(\"javadoc\")\n");
	out.write("public interface %s extends IBusObject {\n", objIntfName)

	# enums
	for enum in obj.enums.values():
		genObjIntfEnum(obj, enum, out)

	# Event Interface
	genObjEvtIntf(obj, objIntfName, out)
	out.write("\n")

	# Property getters
	out.write("\t/** Property getters */\n")
	genStructInftAccessors(objIntfName, obj.properties.values(), False, "\t", AccessorKind.GET, out)

	# Method state getters
	out.write("\t/** Method state getters */\n")
	genStructInftAccessors(objIntfName, obj.methods.values(), True, "\t", AccessorKind.GET, out)

	# methods
	for mtd in obj.methods.values():
		methodName = getCallMethodName(mtd.name)
		out.write("\t/** %s */\n", mtd.desc or "")
		out.write("\tpublic void %s(BusClient client", methodName);
		for arg in mtd.args.values():
			myObusType = MyObusType(arg.type)
			out.write(", %s %s", myObusType.paramType, getPropSetterParamName(arg.name))
		out.write(", IBusMethodCall.AckCb callback);\n\n")

	out.write("}\n")

#===============================================================================
# Generate object class file.
#===============================================================================
def genObj(options, bus, obj, out):
	busName = bus.name
	objName = obj.name
	objClassName = getClassNameObject(busName, objName)
	objIntfName = getIntfNameObject(busName, objName)

	# Import packages
	out.write("package %s.impl;\n", options.javaPackage)
	out.write("\n")

	out.write("import com.parrot.obus.internal.Core.ObusCallCreator;\n")
	out.write("import com.parrot.obus.internal.Core.DecodeError;\n")
	out.write("import com.parrot.obus.internal.Core.ObusEventCreator;\n")
	out.write("import com.parrot.obus.internal.Core.ObusObjectCreator;\n")
	out.write("import com.parrot.obus.internal.Descriptor;\n")
	out.write("import com.parrot.obus.internal.Descriptor.EventDesc;\n")
	out.write("import com.parrot.obus.internal.Descriptor.EventUpdateDesc;\n")
	out.write("import com.parrot.obus.internal.Descriptor.FieldDesc;\n")
	out.write("import com.parrot.obus.internal.Descriptor.FieldDriverEnum;\n")
	out.write("import com.parrot.obus.internal.Descriptor.FieldRole;\n")
	out.write("import com.parrot.obus.internal.Descriptor.FieldType;\n")
	out.write("import com.parrot.obus.internal.Descriptor.MethodDesc;\n")
	out.write("import com.parrot.obus.internal.Descriptor.ObjectDesc;\n")
	out.write("import com.parrot.obus.internal.Descriptor.StructDesc;\n")
	out.write("import com.parrot.obus.MethodState;\n")
	out.write("import com.parrot.obus.BusClient;\n")
	out.write("import com.parrot.obus.IBusMethodCall;\n")
	out.write("import com.parrot.obus.internal.ObusCall;\n")
	out.write("import com.parrot.obus.internal.ObusEvent;\n")
	out.write("import com.parrot.obus.internal.ObusObject;\n")
	out.write("import com.parrot.obus.internal.ObusStruct;\n")
	out.write("import %s.%s;\n", options.javaPackage, objIntfName)
	out.write("\n")

	# Object class
	out.write("/** Object */\n")
	out.write("@SuppressWarnings(\"javadoc\")\n");
	out.write("public final class %s extends ObusObject implements %s {\n", objClassName, objIntfName)

	# Event class
	genObjEvtClass(obj, objClassName, objIntfName, out)

	# Methods
	for mtd in obj.methods.values():
		genObjCallClass(mtd, out)

	# Enum
	for enum in obj.enums.values():
		genObjEnum(obj, enum, out)

	# Object constructor
	out.write("\t/** */\n")
	out.write("\tprivate %s(ObjectDesc objDesc, ObusStruct struct) {\n", objClassName)
	out.write("\t\tsuper(objDesc, struct);\n")
	out.write("\t}\n")
	out.write("\n")

	# Property getters
	out.write("\t/** Property getters */\n")
	genStructAccessors(objClassName, obj.properties.values(), False, True, "\t", AccessorKind.GET, out)

	# Method state getters
	out.write("\t/** Method state getters */\n")
	genStructAccessors(objClassName, obj.methods.values(), True, True, "\t", AccessorKind.GET, out)

	# Method call
	out.write("\t/** Method call */\n")
	for mtd in obj.methods.values():
		callClassName = getClassNameCall(mtd.name)
		methodName = getCallMethodName(mtd.name)
		out.write("\t@Override\n");
		out.write("\tpublic void %s(BusClient client", methodName);
		for arg in mtd.args.values():
			myObusType = MyObusType(arg.type)
			out.write(", %s %s", myObusType.paramType, getPropSetterParamName(arg.name))
		out.write(", IBusMethodCall.AckCb callback) {\n")
		out.write("\t\t%s call_ = (%s)ObusCall.create(%s.methodDesc, this);\n", callClassName, callClassName, callClassName)
		for arg in mtd.args.values():
			out.write("\t\tcall_.%s(%s);\n", getPropSetterMethodName(arg.name), getPropSetterParamName(arg.name))
		out.write("\t\tclient.callBusMethod(call_, callback);\n")
		out.write("\t}\n")
	out.write("\n")

	# Property descriptors
	out.write("\t/** Property descriptors */\n")
	genStructFieldsDesc(obj.properties.values(), False, "\t", "OBUS_PROPERTY", out)

	# Method state descriptors
	out.write("\t/** Method state descriptors */\n")
	genStructFieldsDesc(obj.methods.values(), True, "\t", "OBUS_METHOD", out)

	# Structure descriptor
	out.write("\t/** Structure descriptor */\n")
	genStructDesc(objClassName, obj, "\t", out)

	# Event descriptors
	out.write("\t/** Event descriptors */\n")
	for evt in obj.events.values():
		out.write("\tprivate static final EventDesc %s = new EventDesc(\n", getEventDescName(evt.name))
		out.write("\t\t\t\"%s\",\n", evt.name)
		out.write("\t\t\t%s,\n", evt.uid)
		out.write("\t\t\tEvent.Type.%s,\n", getEnumValueName(evt.name))
		out.write("\t\t\t%s.structDesc,\n", objClassName)
		out.write("\t\t\tEvent.creator);\n")
		if not evt.properties and not evt.methods:
			continue

		out.write("\tstatic {\n")
		for prop in evt.properties.values():
			out.write("\t\t%s.%s.addEventUpdateDesc(new EventUpdateDesc(%s.%s, 0));\n",
				objClassName, getEventDescName(evt.name), objClassName, getFieldDescName(prop.name))

		for mtd in evt.methods.values():
			out.write("\t\t%s.%s.addEventUpdateDesc(new EventUpdateDesc(%s.%s, 0));\n",
				objClassName, getEventDescName(evt.name), objClassName, getFieldDescName(getMethodStateNamePrefix(mtd.name)))
		out.write("\t}\n")
	out.write("\n")

	# Object creator
	out.write("\t/** Object creator */\n")
	out.write("\tprivate static final ObusObjectCreator<%s> creator =", objClassName)
	out.write(" new ObusObjectCreator<%s>() {\n", objClassName)
	out.write("\t\t@Override\n")
	out.write("\t\tpublic %s create(ObjectDesc objDesc, ObusStruct struct) {\n", objClassName)
	out.write("\t\t\treturn new %s(objDesc, struct);\n", objClassName)
	out.write("\t\t}\n")
	out.write("\t};\n")
	out.write("\n")

	# Object descriptor
	out.write("\t/** Object descriptor */\n")
	out.write("\tpublic static final ObjectDesc objectDesc = new ObjectDesc(\n")
	out.write("\t\t\t\"%s\",\n", obj.name)
	out.write("\t\t\t%s,\n", obj.uid)
	out.write("\t\t\t%s.structDesc,\n", objClassName)
	out.write("\t\t\t%s.creator);\n", objClassName)
	out.write("\tstatic {\n")
	for evt in obj.events.values():
		out.write("\t\t%s.objectDesc.addEvent(%s.%s);\n", objClassName, objClassName, getEventDescName(evt.name))
	for mtd in obj.methods.values():
		callClassName = getClassNameCall(mtd.name)
		out.write("\t\t%s.objectDesc.addMethod(%s.methodDesc);\n", objClassName, callClassName)
	out.write("\t}\n")

	out.write("}\n")


#===============================================================================
# Generate mock object class file.
#===============================================================================
def genMockObj(options, bus, obj, out):
	busName = bus.name
	objName = obj.name
	mockObjClassName = getClassNameMockObject(busName, objName)
	objClassName = getClassNameObject(busName, objName)
	objIntfName = getIntfNameObject(busName, objName)

	# Import packages
	out.write("package %s.mock;\n", options.javaPackage)
	out.write("\n")
	out.write("import %s.%s;\n", options.javaPackage, objIntfName)
	out.write("import %s.impl.%s;\n", options.javaPackage, objClassName)
	out.write("import com.parrot.obus.MethodState;\n")
	out.write("import com.parrot.obus.internal.Descriptor.ObjectDesc;\n")
	out.write("import com.parrot.obus.BusClient;\n")
	out.write("import com.parrot.obus.IBusMethodCall;\n")
	out.write("import com.parrot.obus.mock.MockBusObject;\n")
	out.write("import com.parrot.obus.mock.MockBusObjectEvent;\n")
	out.write("import com.parrot.obus.mock.MockMethodCall;\n")

	out.write("\n")

	# Mock Object class
	out.write("/** Mock Object */\n")
	out.write("@SuppressWarnings(\"javadoc\")\n");
	out.write("public class %s extends MockBusObject implements %s {\n", mockObjClassName, objIntfName)

	# Event class
	genMockObjEvtClass(obj, mockObjClassName, objIntfName, out)

	# Method enum used
	out.write("\t/** Methods enum */\n")
	out.write("\tpublic enum Methods {\n")
	out.write("\t\t" + ",\n\t\t".join([
			"/** %s */\n\t\t%s" % (mtd.desc or "", getEnumValueName(mtd.name)) for mtd in obj.methods.values()
		]))
	out.write("\n\t}\n\n")

	# Object properties
	genStructFields(obj.properties.values(), False, "\t", out)
	# Object state event properties
	genStructFields(obj.methods.values(), True, "\t", out)

	# Obj Descriptor getter
	out.write("\t/** ObjDesc getters */\n")
	out.write("\t@Override\n")
	out.write("\tpublic ObjectDesc getDescriptor() {\n")
	out.write("\t\treturn %s.objectDesc;\n", objClassName)
	out.write("\t}\n")

	# Property getters
	out.write("\t/** Property getters */\n")
	genMockStructAccessors(mockObjClassName, obj.properties.values(), False, True, "\t", AccessorKind.GET, out)

	# Method state getters
	out.write("\t/** Method state getters */\n")
	genMockStructAccessors(mockObjClassName, obj.methods.values(), True, True, "\t", AccessorKind.GET, out)

	# Property setters
	out.write("\t/** Property setters */\n")
	genMockStructAccessors(mockObjClassName, obj.properties.values(), False, False, "\t", AccessorKind.SET, out)

	# Method state setters
	out.write("\t/** Method state setters */\n")
	genMockStructAccessors(mockObjClassName, obj.methods.values(), True, False, "\t", AccessorKind.SET, out)

	# Method call
	out.write("\t/** Method call */\n")
	for mtd in obj.methods.values():
		methodName = getCallMethodName(mtd.name)
		out.write("\t@Override\n");
		out.write("\tpublic void %s(BusClient client", methodName);
		for arg in mtd.args.values():
			myObusType = MyObusType(arg.type)
			out.write(", %s %s", myObusType.paramType, getPropSetterParamName(arg.name))
		out.write(", IBusMethodCall.AckCb callback) {\n")
		out.write("\t\tclient.callBusMethod(new MockMethodCall<Methods>(Methods.%s, this", getEnumValueName(mtd.name))
		for arg in mtd.args.values():
			myObusType = MyObusType(arg.type)
			out.write(", %s", getPropSetterParamName(arg.name))
		out.write("), callback);\n")
		out.write("\t}\n")
	out.write("\n")

	# commit event method
	out.write("\t/** commit event */\n")
	out.write("\t@Override\n");
	out.write("\tpublic void commitEvent(MockBusObjectEvent event) {\n")

	out.write("\t\t%s.Event evt = (%s.Event)event;\n", mockObjClassName, mockObjClassName)
	for item in obj.properties.values():
		out.write("\t\tif (evt.%s) {\n", getPropCheckerMethodName(item.name))
		out.write("\t\t\tthis.%s = evt.%s();\n", getPropName(item.name), getPropGetterMethodName(item.name))
		out.write("\t\t}\n")
	for item in obj.methods.values():
		out.write("\t\tif (evt.%s) {\n", getPropCheckerMethodName(getMethodStateNamePrefix(item.name)))
		out.write("\t\t\tthis.%s = evt.%s();\n", getPropName(getMethodStateNamePrefix(item.name)), getPropGetterMethodName(getMethodStateNamePrefix(item.name)))
		out.write("\t\t}\n")
	out.write("\t}\n")

	out.write("}\n")

#===============================================================================
#===============================================================================
def listFiles(options, bus):
	intfPath = os.path.join(options.outdir, options.javaPackage.replace('.', '/'))
	implPath = os.path.join(intfPath, "impl")
	mockPath = os.path.join(intfPath, "mock")
	filePaths = []

	filePaths.append(os.path.join(intfPath, getInterfaceNameBus(bus.name) + ".java"))
	filePaths.append(os.path.join(implPath, getClassNameBus(bus.name) + ".java"))

	for obj in bus.objects.values():
		filePaths.append(os.path.join(intfPath, getIntfNameObject(bus.name, obj.name) + ".java"))
		filePaths.append(os.path.join(implPath, getClassNameObject(bus.name, obj.name) + ".java"))
		filePaths.append(os.path.join(mockPath, getClassNameMockObject(bus.name, obj.name) + ".java"))

	sys.stdout.write(" ".join(filePaths) + "\n")

#===============================================================================
#===============================================================================
def main(options, xmlFile):

	# Only client api available
	if not options.client:
		logging.error("Server api generation not available for java")
		sys.exit(1)

	try:
		bus = ObusBus(xmlFile)
	except ObusException as ex:
		logging.error(ex)
		return

	if options.listFiles:
		listFiles(options, bus)
		return

	# create package path
	intfPath = os.path.join(options.outdir, options.javaPackage.replace('.', '/'))
	implPath = os.path.join(intfPath, "impl")
	mockPath = os.path.join(intfPath, "mock")
	try:
		os.makedirs(intfPath)
	except OSError as ex:
		if ex.errno != errno.EEXIST:
			logging.error(ex)
			return
	try:
		os.makedirs(implPath)
	except OSError as ex:
		if ex.errno != errno.EEXIST:
			logging.error(ex)
			return
	try:
		os.makedirs(mockPath)
	except OSError as ex:
		if ex.errno != errno.EEXIST:
			logging.error(ex)
			return

	busName = bus.name

	# Generate bus Event interface file
	fileName = getInterfaceNameBus(bus.name) + ".java"
	filePath = os.path.join(intfPath, fileName)
	out = Writer(filePath)
	writeHeader(out, getInterfaceNameBus(bus.name))
	genBusEventIntf(options, bus, out)
	out.close()

	# Generate bus file
	fileName = getClassNameBus(bus.name) + ".java"
	filePath = os.path.join(implPath, fileName)
	out = Writer(filePath)
	writeHeader(out, getClassNameBus(bus.name))
	genBus(options, bus, out)
	out.close()

	# Generate object files
	for obj in bus.objects.values():
		# interface
		objName = obj.name
		fileName = getIntfNameObject(busName, objName) + ".java"
		filePath = os.path.join(intfPath, fileName)
		out = Writer(filePath)
		writeHeader(out, getIntfNameObject(busName, objName))
		genObjIntf(options, bus, obj, out)
		out.close()

		# implementation
		objName = obj.name
		fileName = getClassNameObject(busName, objName) + ".java"
		filePath = os.path.join(implPath, fileName)
		out = Writer(filePath)
		writeHeader(out, getClassNameObject(busName, objName))
		genObj(options, bus, obj, out)
		out.close()

		# mock implementation
		objName = obj.name
		fileName = getClassNameMockObject(busName, objName) + ".java"
		filePath = os.path.join(mockPath, fileName)
		out = Writer(filePath)
		writeHeader(out, getClassNameMockObject(busName, objName))
		genMockObj(options, bus, obj, out)
		out.close()

#===============================================================================
# For local testing.
#===============================================================================
if __name__ == "__main__":
	class _Options:
		def __init__(self):
			self.outdir = "."
			self.client = False
			self.lang = "java"
			self.javaPackage = "com.mypackage.test"
	main(_Options(), sys.argv[1])
