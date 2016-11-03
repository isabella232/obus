#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file app.py
#
# @brief wxpython obus client app
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

import wx
import obus.looper as looper

from .mainframe import MainFrame

#===============================================================================
#===============================================================================
class WxLoop(object):
	class MyEvent(wx.PyEvent):
		def __init__(self, handler, msg):
			wx.PyEvent.__init__(self)
			self.handler = handler
			self.msg = msg

	def __init__(self):
		self._evtType = wx.NewEventType()
		wx.GetApp().Connect(wx.ID_ANY, wx.ID_ANY, self._evtType, WxLoop.onEvt)

	def postMessage(self, handler, msg):
		evt = WxLoop.MyEvent(handler, msg)
		evt.SetEventType(self._evtType)
		wx.GetApp().AddPendingEvent(evt)

	@staticmethod
	def onEvt(evt):
		evt.handler.cb(evt.msg)

#===============================================================================
#===============================================================================
class App(wx.App):
	def __init__(self):
		self.mainFrame = None
		self.configFile = None
		wx.App.__init__(self, redirect=False)

	def OnInit(self):
		self.SetAppName("wxpyobus")
		self.configFile = wx.FileConfig()

		# Prepare looper
		looper.prepareLoop(WxLoop())

		# Create main frame
		self.mainFrame = MainFrame()

		# Show
		self.mainFrame.Show(True)
		self.SetTopWindow(self.mainFrame)

		# Initialization finished
		return True

	def OnExit(self):
		# Exit
		pass
