#!/usr/bin/env python
#===============================================================================
# obusgen - obus source code generator.
#
# @file obusgen.py
#
# @brief obus source code generator
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

import sys, os, stat, logging
import optparse

#===============================================================================
#===============================================================================

# Available languages for generation
LANGUAGES = ["c", "java", "vala"]
# obusgen version
VERSION = "1.0.3"

#===============================================================================
# write header on top of .h, .c file.
#===============================================================================
def writeHeader(out, brief):
	out.write("/**\n")
	out.write(" * @file %s\n", os.path.basename(out.fd.name))
	out.write(" *\n")
	out.write(" * @brief %s\n", brief)
	out.write(" *\n")
	out.write(" * @author obusgen %s generated file, do not modify it.\n", VERSION)
	out.write(" */\n")

#===============================================================================
# Writer class to wrap a file and allow printf-like write method.
#===============================================================================
class Writer(object):
	def __init__(self, filePath):
		# unlink previous file
		try:
			os.unlink(filePath)
		except OSError as e:
			if e.errno == 2:  # No such file or directory
				pass
			else:
				raise e

		self.filePath = filePath
		self.fd = open(filePath, "w")
	def write(self, fmt, *args):
		if args:
			self.fd.write(fmt % (args))
		else:
			self.fd.write(fmt % ())

	def close(self):
		# close file
		self.fd.close()
		# mark file as readonly
		os.chmod(self.filePath, stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH)

#===============================================================================
#===============================================================================
def main():
	(options, args) = parseArgs()
	setupLog(options)

	# Setup full path of output directories
	options.outdir = os.path.abspath(options.outdir)

	logging.info("Language: %s", options.lang)
	logging.info("Output dir: %s", options.outdir)
	if not options.listFiles and not os.path.exists(options.outdir):
		os.makedirs(options.outdir, 0755)

	# Import package of language generation
	try:
		lang = __import__(options.lang)
	except ImportError:
		logging.error("Unable to import generator for language %s", options.lang)
		sys.exit(1)

	# Call generator for all given xml files
	for xmlFile in args:
		xmlFile = os.path.abspath(xmlFile)
		logging.info("Processing file %s", xmlFile)
		# TODO: check integrity of xml
		lang.main(options, xmlFile)

#===============================================================================
# Setup option parser and parse command line.
#===============================================================================
def parseArgs():
	# Setup parser
	usage = "usage: %prog [options] <xml>..."
	parser = optparse.OptionParser(usage = usage)

	# Main options
	parser.add_option("-o", "--output",
		dest = "outdir",
		action = "store",
		default = ".",
		metavar = "DIR",
		help = "Name of output directory [default: current directory]")
	parser.add_option("-l", "--lang",
		dest = "lang",
		action = "store",
		default = "c",
		metavar = "LANG",
		help = "Language for generation: %s [default: %%default]" % ", ".join(LANGUAGES))
	parser.add_option("-p", "--package",
		dest = "javaPackage",
		action = "store",
		default = "",
		help = "java package name (ex: com.mycompany.mypackage)")
	parser.add_option("-c", "--client",
		dest = "client",
		action = "store_true",
		default = False,
		help = "Generate code for client instead of server")
	parser.add_option("-s", "--server",
		dest = "client",
		action = "store_false",
		default = False,
		help = "Generate code for server")
	parser.add_option("-f", "--files",
		dest = "listFiles",
		action = "store_true",
		default = False,
		help = "List on stdout full path of files that will be generated and exit")

	# Other options
	parser.add_option("--version",
		dest = "print_version",
		action = "store_true",
		default = False,
		help = "display obusgen version and exit")
	parser.add_option("-q",
		dest = "quiet",
		action = "store_true",
		default = False,
		help = "be quiet")
	parser.add_option("-v",
		dest = "verbose",
		action = "count",
		default = 0,
		help = "verbose output (more verbose if specified twice)")

	# Parse arguments and check validity
	(options, args) = parser.parse_args()

	if options.print_version:
		sys.stdout.write(VERSION + "\n")
		exit(0)

	if len(args) < 1:
		parser.error("Missing input xml file")
	elif options.lang not in LANGUAGES:
		parser.error("Bad language: %s (%s)" % \
			(options.lang, ", ".join(LANGUAGES)))
	elif options.lang == "java" and not options.javaPackage:
		parser.error("Missing java package name: use \"-p\" option")

	return (options, args)

#===============================================================================
# Setup logging system.
#===============================================================================
def setupLog(options):
	logging.basicConfig(
		level = logging.WARNING,
		format = "[%(levelname)s] %(message)s",
		stream = sys.stderr)
	logging.addLevelName(logging.CRITICAL, "C")
	logging.addLevelName(logging.ERROR, "E")
	logging.addLevelName(logging.WARNING, "W")
	logging.addLevelName(logging.INFO, "I")
	logging.addLevelName(logging.DEBUG, "D")

	# Setup log level
	if options.quiet == True:
		logging.getLogger().setLevel(logging.CRITICAL)
	elif options.verbose >= 2:
		logging.getLogger().setLevel(logging.DEBUG)
	elif options.verbose >= 1:
		logging.getLogger().setLevel(logging.INFO)

#===============================================================================
#===============================================================================
if __name__ == "__main__":
	main()
