#===============================================================================
# obus-python - obus client python module.
#
# @file transport.py
#
# @brief obus python transport layer
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

import sys
import errno
import logging
import threading
import time
import socket

from . import looper
from .protocol import Decoder

#===============================================================================
#===============================================================================
_log = logging.getLogger("obus")

_MSG_RX_CONNECTED = 1
_MSG_RX_DISCONNECTED = 2
_MSG_RX_PACKET = 3
_MSG_TX_STOP = 3
_MSG_TX_PACKET = 4

#===============================================================================
#===============================================================================
class PacketHandler(object): # IGNORE:R0921
	def onConnected(self):
		raise NotImplementedError()
	def onDisconnected(self):
		raise NotImplementedError()
	def recvPacket(self, rawPacket):
		raise NotImplementedError()

#===============================================================================
#===============================================================================
class Transport(object):
	def __init__(self):
		self.packetHandler = None
		self.running = False
		self.readThread = None
		self.writeThread = None
		self.readHandler = None
		self.writeHandler = None
		self.sock = None
		self.sockAddr = None
		self.cond = threading.Condition()
		self.reconnect = True

	def start(self, packetHandler, addr):
		# Save parameters
		self.packetHandler = packetHandler
		self.sockAddr = addr
		_log.info("start connection to %s:%d", self.sockAddr[0], self.sockAddr[1])

		# Handler to process Rx message in main thread context
		self.readHandler = looper.Handler(self._onRxMessage)

		# Thread for Rx and Tx
		self.readThread = threading.Thread(target=self._reader)
		self.writeThread = threading.Thread(target=self._writer)

		# Start everything !
		self.running = True
		self.readThread.start()
		self.writeThread.start()

		# Make sure the write handler has been created by writer thread
		try:
			self.cond.acquire()
			while self.writeHandler is None:
				self.cond.wait()
		finally:
			self.cond.release()

	def stop(self):
		# Ask everyone to stop
		try:
			_log.info("stop connection to %s:%d", self.sockAddr[0], self.sockAddr[1])
			self.cond.acquire()
			self.running = False
			self.writeHandler.postMessage((_MSG_TX_STOP, None))
			if self.sock is not None:
				self.sock.shutdown(socket.SHUT_RDWR)
				self.sock.close()
		except (OSError, IOError) as ex:
			_log.debug("shutdown: err=%d(%s)", ex.errno, ex.strerror)
		finally:
			self.cond.release()

		# Wait everyone
		self.readThread.join()
		self.writeThread.join()

		# Cleanup
		self.packetHandler = None
		self.sock = None
		self.sockAddr = None
		self.readThread = None
		self.writeThread = None
		self.readHandler = None
		self.writeHandler = None

	def setAutoreconnect(self, reconnect):
		self.reconnect = reconnect

	def writePacket(self, packet):
		self.writeHandler.postMessage((_MSG_TX_PACKET, packet))

	@staticmethod
	def activateSocketKeepalive(sock, keepidle=5, keepintvl=1, keepcnt=2):
		if sys.platform != "win32":
			sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
			sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, keepidle)
			sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, keepintvl)
			sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, keepcnt)

	def _reader(self):
		while True:
			try:
				# Open a socket
				try:
					self.cond.acquire()
					if not self.running:
						break
					self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
				finally:
					self.cond.release()

				# Connect to address (use timeout so we can abort 'quickly')
				self.sock.settimeout(1.0)
				self.sock.connect(self.sockAddr)
				self.sock.setblocking(True)
				Transport.activateSocketKeepalive(self.sock)

				# Notify connection
				self.readHandler.postMessage((_MSG_RX_CONNECTED, None))

				# Read loop
				decoder = Decoder()
				try:
					while True:
						buf = self.sock.recv(1024)
						if not buf:
							# EOF found
							break

						# Decode data into raw packets and notify them
						off = 0
						while off < len(buf):
							(off, packet) = decoder.decode(buf, off)
							if packet is not None:
								self.readHandler.postMessage((_MSG_RX_PACKET, packet))

				except socket.timeout:
					pass
				except (OSError, IOError) as ex:
					_log.info("recv: err=%d(%s)", ex.errno, ex.strerror)
					pass

				# Notify disconnection
				self.readHandler.postMessage((_MSG_RX_DISCONNECTED, None))

			except socket.timeout:
				pass
			except (OSError, IOError) as ex:
				# Some errors are expected
				if ex.errno != errno.ECONNREFUSED \
						and ex.errno != errno.ENETUNREACH \
						and ex.errno != errno.ENETDOWN \
						and ex.errno != errno.EHOSTUNREACH \
						and ex.errno != errno.EHOSTDOWN:
					_log.error("connect: err=%d(%s)", ex.errno, ex.strerror)

			# Cleanup socket
			try:
				self.cond.acquire()
				if self.sock is not None:
					self.sock.close()
			finally:
				self.sock = None
				self.cond.release()

			if self.reconnect == False:
				break

			# Retry again in 1s
			if self.running:
				time.sleep(1.0)

	def _onRxMessage(self, msg):
		(what, packet) = msg
		if self.packetHandler is None:
			if what == _MSG_RX_PACKET:
				_log.warning("Rx packet lost")
			return
		if what == _MSG_RX_CONNECTED:
			self.packetHandler.onConnected()
		elif what == _MSG_RX_DISCONNECTED:
			self.packetHandler.onDisconnected()
		elif what == _MSG_RX_PACKET:
			self.packetHandler.recvPacket(packet)

	def _writer(self):
		# Prepare a message loop
		looper.prepareLoop()

		# Setup the write handler and wakeup start
		try:
			self.cond.acquire()
			self.writeHandler = looper.Handler(self._onTxMessage)
			self.cond.notifyAll()
		finally:
			self.cond.release()

		# Go !
		looper.runLoop()

	def _onTxMessage(self, msg):
		(what, packet) = msg
		if what == _MSG_TX_PACKET:
			try:
				# Get socket, keep a local ref to avoid the object to be
				# destroyed and set to None.
				self.cond.acquire()
				sock = self.sock
				self.cond.release()

				# Get buffer, and write it
				if sock is not None:
					sock.sendall(packet.getData())
				else:
					_log.warning("Tx packet lost")

			except (OSError, IOError) as ex:
				# Expected when stopped or disconnected
				_log.debug("send: err=%d(%s)", ex.errno, ex.strerror)

		elif what == _MSG_TX_STOP:
			looper.exitLoop()
