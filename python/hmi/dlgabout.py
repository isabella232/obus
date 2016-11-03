#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file dlgabout.py
#
# @brief wxpython obus client about dialog
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

import os
import wx

#===============================================================================
#===============================================================================

APP_VERSION_MAJOR = 1
APP_VERSION_MINOR = 0
APP_VERSION_REV = 0
APP_VERSION_STR = "%d.%d.%d" % (APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_REV)

COPYRIGHT = "Copyright (C) 2014 Parrot S.A"
AUTHORS = "jean-baptiste.dubois@parrot.com, yves-marie.morgan@parrot.com"

_BMP_LOGO_PATH = os.path.join(os.path.dirname(__file__), "logo.bmp")
_BMP_ICON_PATH = os.path.join(os.path.dirname(__file__), "app.ico")

#===============================================================================
#===============================================================================
class DlgAbout(wx.Dialog):
	def __init__(self, parent):
		wx.Dialog.__init__(self, parent=parent)

		self.bmpLogo = wx.StaticBitmap(parent=self)
		self.bmpLogo.SetBitmap(wx.Bitmap(_BMP_LOGO_PATH))
		self.bmpIcon = wx.StaticBitmap(parent=self)
		self.bmpIcon.SetBitmap(wx.Bitmap(_BMP_ICON_PATH))

		self.stcVersion = wx.StaticText(parent=self,
				label=wx.GetApp().GetAppName() + " version " + APP_VERSION_STR)
		self.stcCopyright = wx.StaticText(parent=self, label=COPYRIGHT)
		self.stcWidgetsVersion = wx.StaticText(parent=self,
				label="Using wxWidgets version " + wx.VERSION_STRING)
		self.stcAuthors = wx.StaticText(parent=self,
				label="Authors: " + AUTHORS)
		self.btnOK = wx.Button(parent=self, id=wx.ID_OK, label="OK")

		sizer1 = wx.BoxSizer(wx.HORIZONTAL)
		sizer2 = wx.BoxSizer(wx.VERTICAL)

		sizer1.Add(self.bmpLogo, proportion=1, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer1.Add(sizer2, proportion=0, border=0, flag=wx.EXPAND)
		sizer1.AddStretchSpacer(prop=1)

		sizer2.Add(self.bmpIcon, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(wx.StaticLine(parent=self), proportion=0, border=2,
				flag=wx.ALL|wx.EXPAND)
		sizer2.Add(self.stcVersion, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(self.stcCopyright, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(self.stcWidgetsVersion, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(wx.StaticLine(parent=self), proportion=0, border=2,
				flag=wx.ALL|wx.EXPAND)
		sizer2.Add(self.stcAuthors, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(wx.StaticLine(parent=self), proportion=0, border=2,
				flag=wx.ALL|wx.EXPAND)
		sizer2.Add(self.btnOK, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL|wx.ALL)

		self.SetSizer(sizer1)
		self.Fit()
		self.CenterOnParent()
