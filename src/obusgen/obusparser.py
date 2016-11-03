#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obusparser.py
#
# @brief obus idl parser
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

import sys
import logging
import xml.etree.ElementTree as ET
from collections import OrderedDict

def to_underscored(string):
	"""camel_case_to_underscored """
	if not string:
		return None

	words = []
	from_char_position = 0
	for current_char_position, char in enumerate(string):
		if char.isupper() and from_char_position < current_char_position:
			words.append(string[from_char_position:current_char_position].lower())
			from_char_position = current_char_position
	words.append(string[from_char_position:].lower())
	return '_'.join(words)

def reindent(s, numSpaces):
	s = s.split('\n')
	s = [(numSpaces * ' ') + line for line in s]
	s = '\n'.join(s)
	return s

class Enum(set):
	def __getattr__(self, name):
		if name in self:
			return name
		raise AttributeError

class ObusException(Exception):
	"""obus exception"""
	def __init__(self, msg):
		Exception.__init__(self, msg)

class Obus:
	"""obus constants"""
	# obus invalid uid value
	INVALID_UID = 0
	# obus invalid handle value
	INVALID_HANDLE = 0
	# obus event uid min value
	BUS_EVENT_UID_MIN_VALUE = 10

	def __init__(self):
		pass

class ObusType:
	"""obus type """
	# base type
	Type = Enum(["INVALID", "INT8", "UINT8", "INT16", "UINT16", \
		"INT32", "UINT32", "INT64", 'UINT64', 'STRING', 'HANDLE', \
		'ENUM', 'BOOL', 'FLOAT', 'DOUBLE'])

	def __init__(self, obj, node):
		self.obj = obj
		self._isArray = False
		self.enum = None
		self.base = ObusType.Type.INVALID

		# base type dict
		base = {
				'int8'  : ObusType.Type.INT8,
				'uint8' : ObusType.Type.UINT8,
				'int16'  : ObusType.Type.INT16,
				'uint16' : ObusType.Type.UINT16,
				'int32'  : ObusType.Type.INT32,
				'uint32' : ObusType.Type.UINT32,
				'int64'  : ObusType.Type.INT64,
				'uint64' : ObusType.Type.UINT64,
				'string' : ObusType.Type.STRING,
				'handle' : ObusType.Type.HANDLE,
				'enum'   : ObusType.Type.ENUM,
				'bool'   : ObusType.Type.BOOL,
				'float'  : ObusType.Type.FLOAT,
				'double'  : ObusType.Type.DOUBLE}

		# get field type
		t = to_underscored(node.get('type'))
		if not t:
			raise ObusException('\'{0}\' without type'.format(node.tag))

		# transform type in a list of  several base type list
		types = t.split(':')

		# check number of types
		if not types:
			raise ObusException('\'{0}\' invalid type'.format(node.tag))

		# array should be first
		if types[0] == 'array':
			self._isArray = True
			# 2nd type is mandatary
			if len(types) < 2:
				raise ObusException('\'{0}\' invalid type \'{1}\': ' \
						'missing array base type'\
						.format(node.tag, node.get('type')))
			elif types[1] not in base:
				raise ObusException('\'{0}\' invalid type \'{1}\': ' \
						'invalid array base type  \'{2}\''\
						.format(node.tag, node.get('type'), types[1]))
			else:
				# get base type
				self.base = base[types[1]]
				if self.base == ObusType.Type.ENUM:
					if len(types) < 3:
						raise ObusException('\'{0}\' invalid type \'{1}\': ' \
							'missing array enum type'\
							.format(node.tag, node.get('type')))

					# check object enum exists
					if types[2] not in self.obj.enums:
						raise ObusException('\'{0}\' invalid type \'{1}\': ' \
							'enum \'{2}\' not defined !'\
							.format(node.tag, node.get('type'), types[2]))

					# get object enum ref
					self.enum = self.obj.enums[types[2]]

		elif types[0] not in base:
			raise ObusException('\'{0}\' invalid type \'{1}\': ' \
					'invalid base type  \'{2}\''\
					.format(node.tag, node.get('type'), types[0]))
		else:
			# get base type
			self.base = base[types[0]]
			if self.base == ObusType.Type.ENUM:
				if len(types) < 2:
					raise ObusException('\'{0}\' invalid type \'{1}\': ' \
						'missing enum type'\
						.format(node.tag, node.get('type')))

				# check object enum exists
				if types[1] not in self.obj.enums:
					raise ObusException('\'{0}\' invalid type \'{1}\': ' \
						'enum \'{2}\' not defined !'\
						.format(node.tag, node.get('type'), types[1]))

				# get object enum ref
				self.enum = self.obj.enums[types[1]]

	def isArray(self):
		return self._isArray

	def isEnum(self):
		return self.base == ObusType.Type.ENUM

	def __str__(self):
		if self.base == ObusType.Type.ENUM:
			return '<ObusType> = [isArray={0}, base=ENUM, name={1}]'\
					.format(self.isArray(), self.enum.name)
		else:
			return '<ObusType> = [isArray={0}, base={1}]'\
					.format(self.isArray(), self.base)


class ObusEnumValue:
	"""obus enumeration value"""
	def __init__(self, enum, node):
		self.enum = enum

		# get label name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('enum label without name')

		# get label value
		value = node.get('value')
		if not value:
			raise ObusException('object \'{0}\' enum \'{1}\' label \'{2}\'' \
					' without value'.format(self.enum.obj.name,
					self.enum.name, self.name))

		# convert value to int
		try:
			self.value = int(value, 0)
		except ValueError:
			raise ObusException('object \'{0}\' enum \'{1}\' label \'{2}\'' \
					'value {3} is not numeric'.format(self.enum.obj.name,
					self.enum.name, self.name, value))

		# get enum value description
		self.desc = node.get('desc')

	def __str__(self):
		return '<ObusEnumValue> = [name=\'{0}\', value={1}]'\
					.format(self.name, self.value)

class ObusEnum:
	"""obus enumeration container"""
	def __init__(self, obj, node):
		self.obj = obj
		self.values = OrderedDict()

		# get enum name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('enum without name')

		# get enum default value
		self.default = to_underscored(node.get('default'))
		if not self.name:
			raise ObusException('object \'{0}\' enum \'{1}\' without '\
					'default value'.format(self.obj.name, self.name))

		# parse enumeration values
		for child in node.findall('label'):
			# parse value
			v = ObusEnumValue(self, child)

			# check label value not already exists in enum
			if v.name in self.values:
				raise ObusException('object \'{0}\' enum \'{1}\' label ' \
					'\'%s\' already used'.format(self.name, v.name))

			self.values[v.name] = v

		# check enum is not empty
		if not self.values:
			raise ObusException('object \'{0}\' enum \'{1}\' has no labels '\
					'...'.format(self.obj.name, self.name))

		# check enum default value exist
		if self.default not in self.values:
			raise ObusException('object \'{0}\' enum \'{1}\' default value '\
					'\'{1}\' is not defined'.format(self.obj.name, \
					self.name, self.default))

		# get enum description
		self.desc = node.get('desc')

	def __str__(self):
		s = '<ObusEnum> = [name=\'{0}\', default=\'{1}\']'\
					.format(self.name, self.default)

		for v in self.values.values():
			s += reindent('\n{0}'.format(v), 4)

		return s


class ObusMethodArg:
	"""obus method argument """
	def __init__(self, method, node):
		self.method = method

		# get method argument
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('method argument without name')

		# get method arg uid
		uid = node.get('uid')

		# convert uid to int
		try:
			self.uid = int(uid)
		except ValueError:
			raise ObusException('object \'{0}\' method \'{1}\' '\
								'argument \'{2}\' invalid uid {3}'.format(\
								self.method.obj.name, self.method.name, \
								self.name, uid))

		# check uid is numeric > 0
		if self.uid == Obus.INVALID_UID:
			raise ObusException('object \'{0}\' method \'{1}\' '\
					'argument \'{2}\' invalid uid {3}'.format(\
					self.method.obj.name, self.method.name, \
					self.name, uid))

		# get method argument description
		self.desc = node.get('desc')

		# parse method arg type
		self.type = ObusType(method.obj, node)

	def __str__(self):
		return '<ObusMethodArg> = [uid={0},name=\'{1}\', type={2}]'\
					.format(self.uid, self.name, self.type)

class ObusProperty:
	"""obus property """
	def __init__(self, obj, node):
		self.obj = obj

		# get property name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('property without name')

		# get property uid
		uid = node.get('uid')

		# convert uid to int
		try:
			self.uid = int(uid)
		except ValueError:
			raise ObusException('object \'{0}\' property \'{1}\' ' \
				'invalid uid {2}'\
				.format(self.obj.name, self.name, uid))

		# check uid is numeric > 0
		if self.uid == Obus.INVALID_UID:
			raise ObusException('object \'{0}\' property \'{1}\' ' \
				'invalid uid {2}'\
				.format(self.obj.name, self.name, uid))

		# get property description
		self.desc = node.get('desc')

		# parse parse property type
		self.type = ObusType(self.obj, node)

	def __str__(self):
		return '<ObusProperty> = [uid={0}, name=\'{1}\', type={2}]'\
					.format(self.uid, self.name, self.type)


class ObusMethod:
	"""obus method """
	def __init__(self, obj, node):
		self.obj = obj
		self.args = OrderedDict()

		# get method name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('method without name')

		# get method uid
		uid = node.get('uid')

		# convert uid to int
		try:
			self.uid = int(uid)
		except ValueError:
			raise ObusException('object \'{0}\' method \'{1}\' invalid uid '\
					'{2}'.format(self.obj.name, self.name, uid))

		# check uid is numeric > 0
		if self.uid == Obus.INVALID_UID:
			raise ObusException('object \'{0}\' method \'{1}\' invalid uid '\
					'{2}'.format(self.obj.name, self.name, uid))

		# get method description
		self.desc = node.get('desc')

		# parser methods arguments
		for child in node.findall('arg'):
			# parse argument
			arg = ObusMethodArg(self, child)

			# check another arg with same name does not exists
			if arg.name in self.args:
				raise ObusException('object \'{0}\' method \'{1}\' argument' \
					'\'{2}\' already used'.format(self.obj.name, \
					self.name, arg.name))

			# add argument in list
			self.args[arg.name] = arg

	def __str__(self):
		s = '<ObusMethod> = [uid={1}, name=\'{0}\']' \
					.format(self.uid, self.name)

		for arg in self.args.values():
			s += reindent('\n{0}'.format(arg), 4)

		return s


class ObusEvent:
	"""obus event"""
	def __init__(self, obj, node):
		self.obj = obj
		self.properties = OrderedDict()
		self.methods = OrderedDict()

		# get bus event name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('event name missing')

		# get bus event uid
		uid = node.get('uid')

		# convert uid to int
		try:
			self.uid = int(uid)
		except ValueError:
			raise ObusException('object \'{0}\' method \'{1}\' invalid uid '\
					'{2}'.format(self.obj.name, self.name, uid))

		# check uid is numeric > 0
		if self.uid == Obus.INVALID_UID:
			raise ObusException('object \'{0}\' event \'{1}\' invalid uid '\
					'{2}'.format(self.obj.name, self.name, uid))

		# get event description
		self.desc = node.get('desc')

		# get list of updated properties
		for update in node.findall('update'):
			pname = update.get('property')
			mname = update.get('method')

			if pname and mname:
				raise ObusException('object \'{0}\' event \'{1}\' ' \
					'update property and method not allowed !\n' \
					'Split line ...'.format(self.obj.name, self.name, pname))

			if pname:
				if pname not in self.obj.properties:
					raise ObusException('object \'{0}\' event \'{1}\' ' \
					'update property \'{2}\' not found'.format(self.obj.name, self.name, pname))

				prop = self.obj.properties[pname]
				if prop.name in self.properties:
					raise ObusException('object \'{0}\' event \'{1}\' ' \
					'update property \'{2}\' already added'.format(self.obj.name, self.name, pname))

				# add properties in event
				self.properties[prop.name] = prop
			elif mname:
				if mname not in self.obj.methods:
					raise ObusException('object \'{0}\' event \'{1}\' ' \
					'update method \'{2}\' not found'.format(self.obj.name, self.name, mname))

				mtd = self.obj.methods[mname]
				if mtd.name in self.methods:
					raise ObusException('object \'{0}\' event \'{1}\'' \
					'update method \'{2}\' already added'.format(self.obj.name, self.name, mname))

				# add method in event
				self.methods[mtd.name] = mtd
			else:
				raise ObusException('object \'{0}\' event \'{1}\' ' \
					'update without property or method'.format(self.obj.name, self.name))


	def __str__(self):
		return '<ObusEvent> = [uid={1}, name=\'{0}\', update={{3}}]' \
				.format(self.uid, self.name)

class ObusObject:
	"""obus object"""
	def __isUidUsed(self, uid):
		for prop in self.properties.values():
			if prop.uid == uid:
				return True

		for mtd in self.methods.values():
			if mtd.uid == uid:
				return True

		return False

	def __init__(self, bus, node):
		self.bus = bus
		self.enums = OrderedDict()
		self.properties = OrderedDict()
		self.methods = OrderedDict()
		self.events = OrderedDict()

		# get object name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('object name missing')

		# refusing object named 'bus' for c file generation issue.
		if self.name == 'bus':
			raise ObusException('invalid object name \'{0}\'' \
					.format(self.name))

		# get object uid
		uid = node.get('uid')

		# convert uid to int
		try:
			self.uid = int(uid)
		except ValueError:
			raise ObusException('object \'{0}\' invalid uid {1}' \
				.format(self.name, uid))

		# check uid is numeric > 0
		if self.uid == Obus.INVALID_UID:
			raise ObusException('object \'{0}\' invalid uid {1}' \
				.format(self.name, uid))

		# get object description
		self.desc = node.get('desc')

		# handle enums
		for child in node.findall('enum'):
			# parse enum
			e = ObusEnum(self, child)

			# check another enum with same name does not exists
			if e.name in self.enums:
				raise ObusException('enum \'{0}\' already used' \
						.format(e.name))

			# add enum in dict
			self.enums[e.name] = e

		# handle property
		for child in node.findall('property'):
			# parse property
			p = ObusProperty(self, child)

			# check another property with same name does not exists
			if p.name in self.properties:
				raise ObusException('object \'{0}\' property \'{1}\'' \
					'already used'.format(self.name, p.name))

			# check another property or method with same uid does not exists
			if self.__isUidUsed(p.uid):
				raise ObusException('object \'{0}\' property \'{1}\' ' \
					'uid {2} already used by another property ' \
					'or a method'.format(self.name, p.name, p.uid))

			# add property in dict
			self.properties[p.name] = p

		# handle methods
		for child in node.findall('method'):
			# parse method
			m = ObusMethod(self, child)

			# check another method with same name does not exists
			if m.name in self.methods:
				raise ObusException('object \'{0}\' method \'{1}\' already ' \
					'used'.format(self.name, m.name))

			# check another property or method with same uid does not exists
			if self.__isUidUsed(m.uid):
				raise ObusException('object \'{0}\' method \'{1}\' '\
					'uid {2} already used by another property ' \
					'or a method'.format(self.name, m.name, m.uid))

			# add method in dict
			self.methods[m.name] = m

		# handle events
		for child in node.findall('event'):
			# parse event
			e = ObusEvent(self, child)

			# check another method with same name does not exists
			if e.name in self.events:
				raise ObusException('object \'{0}\' event \'{1}\' already ' \
					'used'.format(self.name, e.name))

			# check another event with same uid does not exists
			for event in self.events.values():
				if event.uid == e.uid:
					raise ObusException('object \'{0}\' event \'{1}\' uid ' \
						'{2} is already used'.format(self.name, e.name, \
						e.uid))

			# add event in list
			self.events[e.name] = e

	def __str__(self):
		s = '<ObusObject> = [uid={0}, name=\'{1}\']' \
				.format(self.uid, self.name)

		for e in self.enums.values():
			s += reindent('\n{0}'.format(e), 4)

		for p in self.properties.values():
			s += reindent('\n{0}'.format(p), 4)

		for m in self.methods.values():
			s += reindent('\n{0}'.format(m), 4)

		for e in self.events.values():
			s += reindent('\n{0}'.format(e), 4)

		return s

class ObusBusEvent:
	"""obus bus event"""
	def __init__(self, bus, node = None):
		self.bus = bus

		if node is None:
			return

		# get bus event name
		self.name = to_underscored(node.get('name'))
		if not self.name:
			raise ObusException('bus event name missing')

		# get bus event uid
		uid = node.get('uid')

		# convert uid to int
		try:
			self.uid = int(uid)
		except ValueError:
			raise ObusException('bus event \'{0}\' invalid uid '\
					'{1}\''.format(self.name, uid))

		# check uid is > 10
		if (self.uid < Obus.BUS_EVENT_UID_MIN_VALUE):
			raise ObusException(' bus event \'{0}\' invalid uid {1}: ' \
					'must be >= {2}'.format(self.name, uid, \
					Obus.BUS_EVENT_UID_MIN_VALUE))

		# get bus event description
		self.desc = node.get('desc')

	def __str__(self):
		return '<ObusBusEvent> = [uid={1}, name=\'{0}\']' \
			.format(self.name, self.uid)

class ObusBus:
	"""obus bus"""
	def __init__(self, xmlFile):
		self.xmlFile = xmlFile
		self.objects = OrderedDict()
		self.events = OrderedDict()

		# open & parse xml file
		tree = ET.parse(self.xmlFile)
		bus = tree.getroot()

		# get bus
		if bus.tag != 'bus':
			raise ObusException('no xml \'<bus>\' node found')

		# get bus name
		self.name = to_underscored(bus.get('name'))
		if not self.name:
			raise ObusException('bus name missing')

		# get bus description
		self.desc = bus.get('desc')

		# get optional javapackage
		self.javaPackage = bus.get('javapackage')

		# find objects
		for node in bus.findall('object'):
			# create object
			obj = ObusObject(self, node)

			# check no ohter objects have same name
			if obj.name in self.objects:
				raise ObusException('object name \'{0}\' already '\
					'used'.format(obj.name))

			# check no ohter objects have same uid
			for obj2 in self.objects.values():
				if obj2.uid == obj.uid:
					raise ObusException('object uid {0} already '\
						'used'.format(obj.uid))

			# add object in dict
			self.objects[obj.name] = obj

		# create internal bus events
		# connected event
		event = ObusBusEvent(self)
		event.uid = 1
		event.name = "connected"
		event.desc = self.name + " bus connected"
		self.events[event.name] = event

		# disconnected
		event = ObusBusEvent(self)
		event.uid = 2
		event.name = "disconnected"
		event.desc = self.name + " bus disconnected"
		self.events[event.name] = event

		# connection refused
		event = ObusBusEvent(self)
		event.uid = 3
		event.name = "connection_refused"
		event.desc = self.name + " bus connection refused"
		self.events[event.name] = event

		# find bus events
		for node in bus.findall('event'):
			# create & parse bus event
			event = ObusBusEvent(self, node)

			# check no ohter event have same name
			if event.name in self.events:
				raise ObusException('bus event name \'{0}\' already '\
						'used'.format(event.name))

			# check no ohter objects have same uid
			for name in self.events:
				if self.events[name].uid == event.uid:
					raise ObusException('bus event uid {0} '\
						'already used'.format(event.uid))

			# add bus event in dict
			self.events[event.name] = event

	def __str__(self):
		s = '<ObusBus> = [name=\'{0}\', javapackage=\'{1}\']' \
				.format(self.name, self.javaPackage)

		for obj in self.objects.values():
			s += reindent('\n{0}'.format(obj), 4)

		for event in self.events.values():
			s += reindent('\n{0}'.format(event), 4)

		return s

#===============================================================================
#===============================================================================
def main(xmlFile):
	logging.basicConfig(
		level = logging.INFO,
		format = "[%(levelname)s] %(message)s",
		stream = sys.stderr)
	try:
		bus = ObusBus(xmlFile)
	except ObusException as ex:
		logging.error(ex)
	else:
		logging.info(bus)

#===============================================================================
# For local testing.
#===============================================================================
if __name__ == "__main__":
	main(sys.argv[1])

