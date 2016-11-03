#!/usr/bin/env python
#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file wxpyobus.py
#
# @brief wxpython obus client
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

import os, sys, logging, optparse
sys.dont_write_bytecode = True

import hmi
from hmi.busdata import BusInfo

#===============================================================================
#===============================================================================
def main():
	setupLog()

	# parse command parser
	usage = "usage: %prog [options] <address> <xml> [<address2> <xml2> ...]\n\n"
	usage += "<address>: obus server address (ex: 192.168.0.1:58000)\n"
	usage += "<xml>:     obus server xml file path"
	parser = optparse.OptionParser(usage=usage)

	parser.add_option("-m", "--monitor",
		dest="monitor",
		action="store_false",
		default=False,
		help="monitor and reload xml file.")

	(options, args) = parser.parse_args()

	app = hmi.App()

	if len(args) == 0:
		# Load bus info from config file from $HOME dir
		for i in range(0, 10):
			xmlFile = app.configFile.Read("BUS/%d/XMLFILE" % i)
			addrRaw = app.configFile.Read("BUS/%d/ADDR" % i)
			enabled = app.configFile.ReadInt("BUS/%d/ENABLED" % i)
			autoReload = app.configFile.ReadInt("BUS/%d/AUTORELOAD" % i)
			primaryFields = app.configFile.Read("BUS/%d/PRIMARYFIELDS" % i)
			if xmlFile:
				addr = (addrRaw.split(":")[0], int(addrRaw.split(":")[1]))
				busInfo = BusInfo(xmlFile, addr, enabled, autoReload, primaryFields)
				app.mainFrame.addBus(busInfo)

	elif len(args) % 2:
		parser.error("Missing input xml file")
		return
	else:
		for i in range(0, len(args), 2):
			addrRaw = args[i]
			xmlFile = os.path.abspath(args[i + 1])
			enabled = True
			autoReload = options.monitor
			primaryFields = {}
			if xmlFile:
				addr = (addrRaw.split(":")[0], int(addrRaw.split(":")[1]))
				busInfo = BusInfo(xmlFile, addr, enabled, autoReload, primaryFields)
				app.mainFrame.addBus(busInfo)
	app.MainLoop()

#===============================================================================
#===============================================================================
def setupLog():
	logging.basicConfig(
		level=logging.DEBUG,
		format="[%(levelname)s][%(asctime)s][%(name)s] %(message)s",
		datefmt="%Y-%m-%d %H:%M:%S",
		stream=sys.stderr)
	logging.addLevelName(logging.CRITICAL, "C")
	logging.addLevelName(logging.ERROR, "E")
	logging.addLevelName(logging.WARNING, "W")
	logging.addLevelName(logging.INFO, "I")
	logging.addLevelName(logging.DEBUG, "D")

#===============================================================================
#===============================================================================
if __name__ == "__main__":
	main()
