#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obus_c_utils.py
#
# @brief obus c utils code generator
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

import os, stat, subprocess, re

#===============================================================================
# write header include guard.
#===============================================================================
def writeHeaderIncludeGuard(out):
	guard = os.path.splitext(os.path.basename(out.fd.name))[0].upper()
	out.write("#ifndef _%s_H_\n", guard)
	out.write("#define _%s_H_\n", guard)

#===============================================================================
# write header extern c.
#===============================================================================
def writeHeaderExternC(out):
	out.write("OBUS_BEGIN_DECLS\n")

#===============================================================================
# write footer extern c.
#===============================================================================
def writeFooterExternC(out):
	out.write("OBUS_END_DECLS\n")

#===============================================================================
# write footer include guard.
#===============================================================================
def writeFooterIncludeGuard(out):
	guard = os.path.splitext(os.path.basename(out.fd.name))[0].upper()
	out.write("#endif /*_%s_H_*/\n", guard)

#===============================================================================
# get object name.
#===============================================================================
def getObjectName(obj):
	return obj.bus.name + "_" + obj.name

#===============================================================================
# indent c or h file.
#===============================================================================
def indentFile(filePath):

	outFilePath = filePath + ".out"

	# mark file as writable
	os.chmod(filePath, stat.S_IRUSR | stat.S_IWUSR)

	# call indent on our file
	if os.access("/usr/bin/indent", os.R_OK | os.X_OK):
		subprocess.call(["/usr/bin/indent", "-linux", filePath , "-o",
				outFilePath])
		# reopen file and remove specific INDENT line
		f = open(outFilePath, 'r')
		lines = f.readlines()
		f.close()
		os.unlink(outFilePath)
	else:
		# logging.error("'indent' package must be installed.")
		f = open(filePath, 'r')
		lines = f.readlines()
		f.close()

	p = re.compile(r"(\w+ \*)( +)(\w+)")

	# reopen in write mode
	f = open(filePath, 'w')
	for line in lines:
		if line.startswith('/* *INDENT'):
			continue

		m = p.search(line)
		if m:
			line = p.sub('\g<1>\g<3>', line)

		f.write(line)
	f.close()

	# mark file as readonly
	os.chmod(filePath, stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH)
