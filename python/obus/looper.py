#===============================================================================
# obus-python - obus client python module.
#
# @file protocol.py
#
# @brief obus python looper
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

import threading
import Queue as queue

#===============================================================================
#===============================================================================

_tls = threading.local()

#===============================================================================
#===============================================================================
class _Loop(object):
	def __init__(self):
		self._queue = queue.Queue()
		self._running = False

	def postMessage(self, handler, msg):
		self._queue.put((handler, msg))

	def run(self):
		self._running = True
		while self._running:
			try:
				# Use timeout so we can interrupt wait
				(handler, msg) = self._queue.get(timeout=0.1)
				handler.cb(msg)
			except queue.Empty:
				pass

	def exit(self):
		self._running = False

#===============================================================================
#===============================================================================
class Handler(object):
	def __init__(self, cb):
		self._loop = _tls.loop
		self.cb = cb

	def postMessage(self, msg):
		self._loop.postMessage(self, msg)

#===============================================================================
#===============================================================================
def prepareLoop(loop=None):
	# Make sure that current thread does not already have a loop object
	if hasattr(_tls, "loop") and _tls.loop is not None:
		raise Exception("Current thread already have a loop object")
	# Create a new loop object
	if loop is None:
		_tls.loop = _Loop()
	else:
		_tls.loop = loop

#===============================================================================
#===============================================================================
def runLoop():
	_tls.loop.run()
	_tls.loop = None

#===============================================================================
#===============================================================================
def exitLoop():
	_tls.loop.exit()
