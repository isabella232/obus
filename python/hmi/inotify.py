#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file inotify.py
#
# @brief wxpython obus client xml inotify
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

import sys, os, logging
import threading
import select
import struct
import ctypes.util

import obus.looper as looper

#===============================================================================
#===============================================================================
_log = logging.getLogger("inotify")
_log.setLevel(logging.WARNING)

_libcName = ctypes.util.find_library('c')
_libc = ctypes.CDLL(_libcName, use_errno=True)

_IN_ACCESS = 0x00000001        # File was accessed
_IN_MODIFY = 0x00000002        # File was modified
_IN_ATTRIB = 0x00000004        # Metadata changed
_IN_CLOSE_WRITE = 0x00000008   # Writable file was closed
_IN_CLOSE_NOWRITE = 0x00000010 # Unwritable file closed
_IN_OPEN = 0x00000020          # File was opened
_IN_MOVED_FROM = 0x00000040    # File was moved from X
_IN_MOVED_TO = 0x00000080      # File was moved to Y
_IN_CREATE = 0x00000100        # Subfile was created
_IN_DELETE = 0x00000200        # Subfile was deleted
_IN_DELETE_SELF = 0x00000400   # Self (watched item itself) was deleted
_IN_MOVE_SELF = 0x00000800     # Self (watched item itself) was moved

_IN_ALL_EVENTS = (
	_IN_ACCESS |
	_IN_MODIFY |
	_IN_ATTRIB |
	_IN_CLOSE_WRITE |
	_IN_CLOSE_NOWRITE |
	_IN_OPEN |
	_IN_MOVED_FROM |
	_IN_MOVED_TO |
	_IN_CREATE |
	_IN_DELETE |
	_IN_DELETE_SELF |
	_IN_MOVE_SELF)

#===============================================================================
#===============================================================================
def _inotify_init():
	assert _libc is not None
	return _libc.inotify_init()

#===============================================================================
#===============================================================================
def _inotify_add_watch(fd, pathName, mask):
	assert _libc is not None
	pathName = ctypes.create_string_buffer(pathName)
	return _libc.inotify_add_watch(fd, pathName, mask)

#===============================================================================
#===============================================================================
def _inotify_rm_watch(fd, wd):
	assert _libc is not None
	return _libc.inotify_rm_watch(fd, wd)

#===============================================================================
#===============================================================================
class INotifyWin32(object):
	def __init__(self):
		pass
	def close(self):
		pass
	def add(self, filePath, cb, cbdata):
		pass
	def remove(self, filePath, cb, cbdata):
		pass

#===============================================================================
#===============================================================================
class INotifyLinux(object):
	class FileEntry(object):
		def __init__(self, fileName, cb, cbdata):
			self.fileName = fileName
			self.cb = cb;
			self.cbdata = cbdata;

	class DirEntry(object):
		def __init__(self, dirPath, wd):
			self.dirPath = dirPath
			self.wd = wd
			self.fileEntries = []

		def findFileEntry(self, fileName, cb, cbdata):
			for fileEntry in self.fileEntries:
				if fileEntry.fileName == fileName \
						and (fileEntry.cb == cb or cb is None) \
						and (fileEntry.cbdata == cbdata or cbdata is None):
					return fileEntry
			return None

	def __init__(self):
		# Table of registered path
		self.regTableByDirPath = {}
		self.regTableByWd = {}
		# Create inotify fd
		self.fdINotify = _inotify_init()
		# Create wakeup pipes
		self.fdPipes = os.pipe()
		# Handler to process events in main thread context
		self.evtHandler = looper.Handler(self._onEvt)
		# Create thread
		self.thread = threading.Thread(target=self._threadEntry)
		self.threadRunning = True
		self.thread.start()

	def close(self):
		# Stop thread
		self.threadRunning = False
		os.write(self.fdPipes[1], "STOP")
		self.thread.join()
		# Free resources
		os.close(self.fdINotify)
		os.close(self.fdPipes[0])
		os.close(self.fdPipes[1])

	def add(self, filePath, cb, cbdata):
		# Register the watch on the directory containing the file
		dirPath = os.path.dirname(filePath)
		fileName = os.path.basename(filePath)
		wd = _inotify_add_watch(self.fdINotify, dirPath, _IN_MOVED_TO | _IN_MODIFY)
		if wd in self.regTableByWd:
			dirEntry = self.regTableByWd[wd]
		else:
			dirEntry = INotifyLinux.DirEntry(dirPath, wd)
			self.regTableByWd[wd] = dirEntry
			self.regTableByDirPath[dirPath] = dirEntry
			_log.debug("Add directory watch %s -> %d",
					dirEntry.dirPath, dirEntry.wd)

		# Add file in entry
		fileEntry = dirEntry.findFileEntry(fileName, cb, cbdata)
		if fileEntry is not None:
			_log.warning("%s already registered with (cb=0x%x,cbdata=0x%x)",
					filePath, id(cb), id(cbdata))
		else:
			fileEntry = INotifyLinux.FileEntry(fileName, cb, cbdata)
			dirEntry.fileEntries.append(fileEntry)
			_log.debug("Register file %s (cb=0x%x,cbdata=0x%x)",
					filePath, id(cb), id(cbdata))

	def remove(self, filePath, cb, cbdata):
		# Check that directory is watched
		dirPath = os.path.dirname(filePath)
		fileName = os.path.basename(filePath)
		if dirPath not in self.regTableByDirPath:
			_log.warning("%s not registered with (cb=0x%x,cbdata=0x%x)",
					filePath, id(cb), id(cbdata))
			return

		# Check that file is registered
		dirEntry = self.regTableByDirPath[dirPath]
		fileEntry = dirEntry.findFileEntry(fileName, cb, cbdata)
		if fileEntry is None:
			_log.warning("%s not registered with (cb=0x%x,cbdata=0x%x)",
					filePath, id(cb), id(cbdata))
			return

		# Unregister file and remove watch of directory if no more needed
		_log.debug("Unregister file %s (cb=0x%x,cbdata=0x%x)",
				filePath, id(cb), id(cbdata))
		dirEntry.fileEntries.remove(fileEntry)
		if len(dirEntry.fileEntries) == 0:
			_log.debug("Remove directory watch %s -> %d",
					dirEntry.dirPath, dirEntry.wd)
			_inotify_rm_watch(self.fdINotify, dirEntry.wd)
			del self.regTableByWd[dirEntry.wd]
			del self.regTableByDirPath[dirEntry.dirPath]

	def _threadEntry(self):
		# Create a poll object
		_log.debug("thread: start")
		pollObj = select.poll()
		pollObj.register(self.fdINotify, select.POLLIN)
		pollObj.register(self.fdPipes[0], select.POLLIN)
		while self.threadRunning:
			# Do the poll
			res = pollObj.poll()
			for (fd, event) in res:
				if fd == self.fdINotify:
					self._readEvt()
		_log.debug("thread: exit")

	def _readEvt(self):
		buf = os.read(self.fdINotify, 4096)
		off = 0
		while off < len(buf):
			# Retrieve wd, mask, cookie and fname_len
			(wd, mask, cookie, fname_len) = struct.unpack(
					"iIII", buf[off:off+16])
			off += 16
			# Retrieve name (strip leading null bytes)
			(fname,) = struct.unpack("%ds" % fname_len, buf[off:off+fname_len])
			fname = fname.rstrip("\0")
			off += fname_len
			self._processEvt(wd, mask, cookie, fname)

	def _processEvt(self, wd, mask, cookie, fname):
		_log.debug("Process event wd=%d mask=0x%x cookie=%d fname=%s",
				wd, mask, cookie, fname)
		# Only interested in some events
		if mask != _IN_MOVED_TO and mask != _IN_MODIFY:
			return
		# Check that watch is valid
		if wd not in self.regTableByWd:
			_log.warning("Invalid watch wd=%d", wd)
			return
		# Process registered files
		dirEntry = self.regTableByWd[wd]
		for fileEntry in dirEntry.fileEntries:
			if fileEntry.fileName == fname:
				self.evtHandler.postMessage(fileEntry)

	def _onEvt(self, fileEntry):
		# Notify callback in main thread
		fileEntry.cb(fileEntry.cbdata)

#===============================================================================
#===============================================================================
if sys.platform == "win32":
	INotify = INotifyWin32
else:
	INotify = INotifyLinux
