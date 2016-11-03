#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file mainframe.py
#
# @brief wxpython obus client main frame
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

import obus

from . import utils
from .busdata import BusItf
from .dlgabout import DlgAbout
from .inotify import INotify

#===============================================================================
#===============================================================================

_PNG_OK_PATH = os.path.join(os.path.dirname(__file__), "ok.png")
_PNG_KO_PATH = os.path.join(os.path.dirname(__file__), "ko.png")

#===============================================================================
#===============================================================================
TREE_ITEM_KIND_ROOT = 0
TREE_ITEM_KIND_BUS = 1
TREE_ITEM_KIND_OBJ_TYPE = 2
TREE_ITEM_KIND_OBJ = 3

def packTreeItemData(kind, busItf, obj):
	return wx.TreeItemData((kind, (busItf, obj)))

def unpackTreeItemData(treeItemData):
	(kind, data) = treeItemData.GetData()
	(busItf, obj) = data
	return (kind, busItf, obj)

class _ObusTreeCtrl(wx.TreeCtrl):
	def OnCompareItems(self, item1, item2):
		(kind1, busItf1, data1) = unpackTreeItemData(self.GetItemData(item1))
		(kind2, busItf2, data2) = unpackTreeItemData(self.GetItemData(item2))
		if kind1 == TREE_ITEM_KIND_OBJ and kind2 == TREE_ITEM_KIND_OBJ:
			# Compare handle of objects
			return cmp(data1.handle, data2.handle)
		# By default, use text of item for sorting
		return cmp(self.GetItemText(item1), self.GetItemText(item2))

#===============================================================================
#===============================================================================
class MainFrame(wx.Frame):
	_ID_TREECTRL = 1
	_ID_TXTLOG = 2

	_ID_CMD_BUS_CONNECT = 100
	_ID_CMD_BUS_DISCONNECT = 101

	_ID_CMD_SET_PRIMARY_FIELD = wx.ID_HIGHEST

	def __init__(self):
		wx.Frame.__init__(self, parent=None,
				title=wx.GetApp().GetAppName(),
				size=(800, 600),
				style=wx.DEFAULT_FRAME_STYLE|wx.TAB_TRAVERSAL)

		wx.EVT_CLOSE(self, self.onClose)

		# List of bus interface and inotify object
		self.busItfList = []
		self.inotify = INotify()

		# Create 2 splitters
		self.splitterVert = wx.SplitterWindow(parent=self)
		self.splitterHorz = wx.SplitterWindow(parent=self.splitterVert)

		# Create TreeCtrl that will go in left pane of splitterVert
		self.treeCtrl = _ObusTreeCtrl(parent=self.splitterVert,
				id=MainFrame._ID_TREECTRL,
				style=wx.TR_DEFAULT_STYLE|wx.TR_SINGLE| wx.TR_HIDE_ROOT)
		self.treeRoot = self.treeCtrl.AddRoot("root",
				data=packTreeItemData(TREE_ITEM_KIND_ROOT, None, None))
		wx.EVT_TREE_SEL_CHANGED(self, MainFrame._ID_TREECTRL,
				self.onTreeSelChanged)
		wx.EVT_TREE_ITEM_RIGHT_CLICK(self, MainFrame._ID_TREECTRL,
				self.onTreeItemRightClick)

		# Create Panel for log that will go in bottom pane of splitterHorz
		self.pnlLog = _PnlLog(parent=self.splitterHorz)

		# Top pane of splitterHorz can be if several kind
		self.pnlInfoEmpty = _PnlInfoEmpty(self.splitterHorz)
		self.pnlInfoObjType = _PnlInfoObjType(self.splitterHorz)
		self.pnlInfoObj = _PnlInfoObj(self.splitterHorz, self.pnlLog)
		self.pnlInfoEmpty.Show(False)
		self.pnlInfoObjType.Show(False)
		self.pnlInfoObj.Show(False)
		self.pnlInfo = None

		# Setup splitter
		self.splitterVert.SplitVertically(self.treeCtrl, self.splitterHorz)
		self.splitterHorz.SplitHorizontally(self.pnlInfoEmpty, self.pnlLog)
		self.splitterVert.SetSashPosition(200)
		self.splitterHorz.SetSashPosition(300)
		self.pnlInfo = self.pnlInfoEmpty
		self.pnlInfoEmpty.Show(True)

		# Setup main menu
		menuBar = wx.MenuBar()
		menuFile = wx.Menu()
		menuFile.Append(wx.ID_EXIT, "&Quit\tAlt+F4",
				"Quit the application")
		menuHelp = wx.Menu()
		menuHelp.Append(wx.ID_ABOUT, "&About...\tAlt+F1",
				"Display informations about the program")
		menuBar.Append(menuFile, "&File")
		menuBar.Append(menuHelp, "&?")
		wx.EVT_MENU(self, wx.ID_EXIT, self.onFileQuit)
		wx.EVT_MENU(self, wx.ID_ABOUT, self.onHelpAbout)

		wx.EVT_MENU(self, MainFrame._ID_CMD_BUS_CONNECT, self.onCmdBusConnect)
		wx.EVT_MENU(self, MainFrame._ID_CMD_BUS_DISCONNECT, self.onCmdBusDisconnect)
		wx.EVT_MENU_RANGE(self, MainFrame._ID_CMD_SET_PRIMARY_FIELD,
				MainFrame._ID_CMD_SET_PRIMARY_FIELD + 65535, self.onCmdSetPrimaryField)

		# Setup MenuBar and StatusBar
		self.SetMenuBar(menuBar)
		self.SetStatusBar(wx.StatusBar(parent=self))

	def Destroy(self, *args, **kwargs):
		while self.busItfList:
			self.removeBus(self.busItfList[0])
		self.inotify.close()
		return wx.Frame.Destroy(self, *args, **kwargs)

	def onClose(self, evt):
		self.Destroy()

	def addBus(self, busInfo):
		# Create BusItf
		busItf = BusItf(self, busInfo)
		self.busItfList.append(busItf)
		# Add bus at root and a child for each type of object in bus
		busItf.treeIdBus = self.treeCtrl.AppendItem(self.treeRoot,
				"%s (%s)" % (busItf.getName(), busItf.getStatusStr()),
				data=packTreeItemData(TREE_ITEM_KIND_BUS, busItf, None))
		for objDesc in busItf.getObjectsDesc():
			# set primary Field to obj desc
			if objDesc.uid in busInfo.primaryFields:
				objDesc.primaryFieldUid = busInfo.primaryFields[objDesc.uid]
			# add object in tree
			busItf.treeIdObj[objDesc.uid] = self.treeCtrl.AppendItem(
					busItf.treeIdBus, objDesc.name,
					data=packTreeItemData(TREE_ITEM_KIND_OBJ_TYPE, busItf, objDesc))
		self.treeCtrl.Expand(busItf.treeIdBus)
		# Start bus if needed
		if busInfo.enabled:
			busItf.start()
		# Update status
		self.treeCtrl.SetItemText(busItf.treeIdBus, "%s (%s)" %
				(busItf.getName(), busItf.getStatusStr()))
		# Register for modification of xml file
		if busInfo.autoReload:
			self.inotify.add(busInfo.xmlFile, self.onBusModified, busItf)

	def removeBus(self, busItf):
		# Remove from inotify, stop bus and ree resources
		if busItf.busInfo.autoReload:
			self.inotify.remove(busItf.busInfo.xmlFile, self.onBusModified, busItf)
		busItf.stop()
		self.treeCtrl.Delete(busItf.treeIdBus)
		self.busItfList.remove(busItf)
		busItf.treeIdBus = None
		busItf.treeIdObj = {}

	def onBusModified(self, busItf):
		self.pnlLog.log(busItf, "Bus description has changed")
		started = busItf.isStarted()
		busInfo = busItf.busInfo
		busInfo.enabled = started
		self.removeBus(busItf)
		self.addBus(busInfo)

	def setPnlInfo(self, pnl):
		if self.pnlInfo != pnl \
				and self.splitterHorz.ReplaceWindow(self.pnlInfo, pnl):
			self.pnlInfo.Show(False)
			self.pnlInfo.clearInfo()
			pnl.Show(True)
			self.pnlInfo = pnl

	def onFileQuit(self, evt):
		# Close main frame
		self.Close()

	def onHelpAbout(self, evt):
		DlgAbout(self).ShowModal()

	def onCmdBusConnect(self, evt):
		item = self.treeCtrl.GetSelection()
		if not item.IsOk():
			return
		(kind, busItf, data) = unpackTreeItemData(self.treeCtrl.GetItemData(item))
		if kind == TREE_ITEM_KIND_BUS:
			# Start and update status
			busItf.start()
			self.treeCtrl.SetItemText(busItf.treeIdBus, "%s (%s)" %
					(busItf.getName(), busItf.getStatusStr()))

	def onCmdBusDisconnect(self, evt):
		item = self.treeCtrl.GetSelection()
		if not item.IsOk():
			return
		(kind, busItf, data) = unpackTreeItemData(self.treeCtrl.GetItemData(item))
		if kind == TREE_ITEM_KIND_BUS:
			# Stop and update status
			busItf.stop()
			self.treeCtrl.SetItemText(busItf.treeIdBus, "%s (%s)" %
					(busItf.getName(), busItf.getStatusStr()))

	def getObjectTreeLabel(self, obj):
		# get object label
		label = str(obj.handle)
		while len(label) < 5:
			label = "0" + label

		if obj.desc.primaryFieldUid != -1:
			label += " : " + str(obj.struct.getField(
				obj.desc.structDesc.fieldsDesc[obj.desc.primaryFieldUid]))

		return label

	def setObjectPrimaryField(self, busItf, objDesc, uid):
		if uid <= 0:
			objDesc.primaryFieldUid = -1
		else:
			objDesc.primaryFieldUid = uid

		# refresh HMI
		self.treeCtrl.Freeze()
		root = busItf.treeIdObj[objDesc.uid]
		(item, cookie) = self.treeCtrl.GetFirstChild(root)
		while item.IsOk():
			(kind, busItf, data) = unpackTreeItemData(self.treeCtrl.GetItemData(item))
			if kind == TREE_ITEM_KIND_OBJ:
				obj = data
				# update text
				self.treeCtrl.SetItemText(item, self.getObjectTreeLabel(obj))

			item, cookie = self.treeCtrl.GetNextChild(root, cookie)

		self.treeCtrl.Thaw()

	def onCmdSetPrimaryField(self, evt):
		item = self.treeCtrl.GetSelection()
		if not item.IsOk():
			return
		(kind, busItf, data) = unpackTreeItemData(self.treeCtrl.GetItemData(item))
		if kind == TREE_ITEM_KIND_OBJ_TYPE:
			# Stop and update status
			objDesc = data
			uid = evt.GetId() - MainFrame._ID_CMD_SET_PRIMARY_FIELD
			self.setObjectPrimaryField(busItf, objDesc, uid)

	def onTreeSelChanged(self, evt):
		if not evt.GetItem().IsOk():
			self.setPnlInfo(self.pnlInfoEmpty)
			return
		(kind, busItf, data) = unpackTreeItemData(self.treeCtrl.GetItemData(evt.GetItem()))
		if kind == TREE_ITEM_KIND_OBJ_TYPE:
			self.pnlInfoObjType.setInfo(busItf, objDesc=data)
			self.setPnlInfo(self.pnlInfoObjType)
		elif kind == TREE_ITEM_KIND_OBJ:
			self.pnlInfoObj.setInfo(busItf, obj=data)
			self.setPnlInfo(self.pnlInfoObj)
		else:
			self.setPnlInfo(self.pnlInfoEmpty)

	def onTreeItemRightClick(self, evt):
		if not evt.GetItem().IsOk():
			return
		self.treeCtrl.SelectItem(evt.GetItem())
		(kind, busItf, data) = unpackTreeItemData(self.treeCtrl.GetItemData(evt.GetItem()))
		if kind == TREE_ITEM_KIND_BUS:
			# Display popup menu
			menu = wx.Menu()
			menu.Append(id=MainFrame._ID_CMD_BUS_CONNECT, text="Connect")
			menu.Enable(MainFrame._ID_CMD_BUS_CONNECT, busItf.canStart())
			menu.Append(id=MainFrame._ID_CMD_BUS_DISCONNECT, text="Disconnect")
			menu.Enable(MainFrame._ID_CMD_BUS_DISCONNECT, busItf.canStop())
			self.treeCtrl.PopupMenu(menu)
		elif kind == TREE_ITEM_KIND_OBJ_TYPE:
			# Select object item tree label based on property
			primaryMenu = wx.Menu()
			objDesc = data
			primaryMenu.AppendRadioItem(id=MainFrame._ID_CMD_SET_PRIMARY_FIELD, text="none")
			for fieldDesc in objDesc.structDesc.fieldsDesc.values():
				if fieldDesc.role == obus.FieldRole.OBUS_PROPERTY:
					primaryMenu.AppendRadioItem(id=MainFrame._ID_CMD_SET_PRIMARY_FIELD + fieldDesc.uid,
							text=str(fieldDesc.name))

			if objDesc.primaryFieldUid == -1:
				primaryMenu.Check(MainFrame._ID_CMD_SET_PRIMARY_FIELD, True)
			else:
				primaryMenu.Check(MainFrame._ID_CMD_SET_PRIMARY_FIELD + objDesc.primaryFieldUid, True)

			menu = wx.Menu()
			menu.AppendSubMenu(submenu=primaryMenu, text="Set Primary Property")
			self.treeCtrl.PopupMenu(menu)

	def onBusEvent(self, busItf, busEvt):
		if busEvt.isBaseEvent():
			# Log event
			if busEvt.getType() == obus.BusEventBase.Type.CONNECTED:
				self.pnlLog.log(busItf, "Connected")
			elif busEvt.getType() == obus.BusEventBase.Type.DISCONNECTED:
				self.pnlLog.log(busItf, "Disconnected")
			elif busEvt.getType() == obus.BusEventBase.Type.CONNECTION_REFUSED:
				self.pnlLog.log(busItf, "Connection refused")
			# Update status
			self.treeCtrl.SetItemText(busItf.treeIdBus, "%s (%s)" %
					(busItf.getName(), busItf.getStatusStr()))
		else:
			self.pnlLog.log(busItf, "Bus event: %s", busEvt.getType())

		# Dispatch bus event with controls frozen
		self.Freeze()
		busItf.dispatchBusEvent(busEvt)
		# Sort objects in tree
		for treeId in busItf.treeIdObj.values():
			self.treeCtrl.SortChildren(treeId)
		# Update panel with info
		self.pnlInfo.updateInfo()
		self.Thaw()

	def onObjectAdd(self, busItf, obj, busEvt):
		self.pnlLog.log(busItf, "Object add: %s:%d",
				obj.desc.name, obj.handle)
		if self.pnlLog.verbose:
			self.pnlLog.log(None, "    -> %s", str(obj))

		# Add in tree
		treeId = self.treeCtrl.AppendItem(busItf.treeIdObj[obj.uid],
				self.getObjectTreeLabel(obj),
				data=packTreeItemData(TREE_ITEM_KIND_OBJ, busItf, obj))
		obj.treeId = treeId

	def onObjectRemove(self, busItf, obj, busEvt):
		self.pnlLog.log(busItf, "Object remove: %s:%d",
				obj.desc.name, obj.handle)
		if self.pnlLog.verbose:
			self.pnlLog.log(None, "    -> %s", str(obj))
		# Remove from tree
		self.treeCtrl.Delete(obj.treeId)
		obj.treeId = None

	def onObjectEvent(self, busItf, objEvt, busEvt):
		self.pnlLog.log(busItf, "Object event: %s %s:%d",
				objEvt.getType(), objEvt.obj.desc.name, objEvt.obj.handle)
		if self.pnlLog.verbose:
			self.pnlLog.log(None, "    -> %s", str(objEvt))

		# Update Tree Ctl with primaryFieldUid
		objEvt.commit()
		if objEvt.obj.desc.primaryFieldUid != -1:
			fieldDesc = objEvt.desc.structDesc.fieldsDesc[objEvt.obj.desc.primaryFieldUid]
			if objEvt.struct.hasField(fieldDesc):
				self.treeCtrl.SetItemText(objEvt.obj.treeId,
						self.getObjectTreeLabel(objEvt.obj))

	def onObjectCallAck(self, busItf, call):
		self.pnlLog.log(busItf, "Object call ack: %s %s:%d %s:%d",
				call.ack, call.desc.name, call.handle,
				call.obj.desc.name, call.obj.handle)

#===============================================================================
#===============================================================================
class _PnlLog(wx.Panel):
	_ID_BTN_CLEAR = 1
	_ID_CHK_VERBOSE = 2
	_ID_TXTLOG = 3

	def __init__(self, parent):
		wx.Panel.__init__(self, parent=parent)
		self.verbose = wx.GetApp().configFile.ReadInt("LOG/VERBOSE")

		# Create controls for log
		self.btnClear = wx.Button(parent=self,
				id=_PnlLog._ID_BTN_CLEAR, label="Clear")
		self.chkVerbose = wx.CheckBox(parent=self,
				id=_PnlLog._ID_CHK_VERBOSE, label="Verbose")
		self.chkVerbose.SetValue(self.verbose)

		# Create TextCtrl for log
		self.txtLog = wx.TextCtrl(parent=self, id=_PnlLog._ID_TXTLOG,
				style=wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_NOHIDESEL|
				wx.TE_DONTWRAP|wx.TE_RICH2)

		wx.EVT_BUTTON(self, _PnlLog._ID_BTN_CLEAR, self.onBtnClear)
		wx.EVT_CHECKBOX(self, _PnlLog._ID_CHK_VERBOSE, self.onChkVerbose)

		sizer1 = wx.BoxSizer(wx.VERTICAL)
		sizer2 = wx.StaticBoxSizer(wx.StaticBox(parent=self, label="Log"), wx.HORIZONTAL)

		sizer1.Add(sizer2, proportion=0, border=0, flag=wx.EXPAND)
		sizer1.Add(self.txtLog, proportion=1, border=2,
				flag=wx.EXPAND|wx.ALL)

		sizer2.Add(self.btnClear, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(self.chkVerbose, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL)

		self.SetSizer(sizer1)

	def log(self, busItf, fmt, *args):
		line = fmt % (args) if args is not None else fmt
		if busItf is not None:
			line = busItf.getName() + ": " + line

		# Get selection and current position
		(startSel, endSel) = self.txtLog.GetSelection()
		pos = self.txtLog.GetInsertionPoint()
		end = (self.txtLog.GetLastPosition() == pos)
		self.txtLog.Freeze()

		# Add text
		self.txtLog.AppendText(line + "\n")

		# Put back selection and enable back update
		if not end:
			self.txtLog.SetSelection(startSel, endSel)
		else:
			self.txtLog.ShowPosition(self.txtLog.GetLastPosition())
		self.txtLog.Thaw()
		self.txtLog.Refresh(False, None)

	def onBtnClear(self, evt):
		self.txtLog.Clear()

	def onChkVerbose(self, evt):
		self.verbose = self.chkVerbose.GetValue()
		wx.GetApp().configFile.WriteInt("LOG/VERBOSE", self.verbose)

#===============================================================================
#===============================================================================
class _PnlInfoEmpty(wx.Panel):
	def __init__(self, parent):
		wx.Panel.__init__(self, parent=parent)

	def setInfo(self):
		pass

	def updateInfo(self):
		pass

	def clearInfo(self):
		pass

#===============================================================================
#===============================================================================
class _PnlInfoObjType(wx.Panel):
	def __init__(self, parent):
		wx.Panel.__init__(self, parent=parent)
		self.busItf = None
		self.objDesc = None
		self.sortedColumn = 0
		self.sortAscending = True
		self.colData = {}

		# Create ListCtrl for objects
		self.lstObjs = wx.ListCtrl(parent=self,
				style=wx.LC_REPORT|wx.LC_SINGLE_SEL|wx.LC_HRULES|wx.LC_VRULES)
		wx.EVT_LIST_COL_CLICK(self, self.lstObjs.GetId(), self._onListCtrlColClick)

		# Put in a sizer
		sizer = wx.BoxSizer(orient=wx.HORIZONTAL)
		sizer.Add(self.lstObjs, proportion=1, flag=wx.EXPAND)
		self.SetSizer(sizer)

	def setInfo(self, busItf, objDesc):
		# Setup fields if type of object has changed
		oldDesc = self.objDesc
		self.busItf = busItf
		self.objDesc = objDesc
		if oldDesc != self.objDesc:
			self._setupFields()
		# Update content
		self.updateInfo()

	def updateInfo(self):
		# Update/Add object. store handle as item data because it must be a 'long'
		objects = self.busItf.getObjects(self.objDesc)
		for obj in objects:
			itemId = self.lstObjs.FindItemData(-1, obj.handle)
			if itemId < 0:
				# Add a new item, filling only column 0 with handle
				itemId = self.lstObjs.GetItemCount()
				listItem = wx.ListItem()
				listItem.SetId(itemId)
				listItem.SetText(str(obj.handle))
				listItem.SetData(obj.handle)
				self.lstObjs.InsertItem(listItem)
			listItem = self.lstObjs.GetItem(itemId)
			# Fill other columns with fields
			idx = 1
			for fieldDesc in self.objDesc.structDesc.fieldsDesc.values():
				listItem.SetColumn(idx)
				listItem.SetText(str(obj.struct.getField(fieldDesc)))
				self.lstObjs.SetItem(listItem)
				idx += 1
		# Remove objects from list
		handles = [obj.handle for obj in objects]
		itemId = 0
		while itemId < self.lstObjs.GetItemCount():
			listItem = self.lstObjs.GetItem(itemId)
			if listItem.GetData() not in handles:
				self.lstObjs.DeleteItem(itemId)
			else:
				itemId += 1
		utils.listCtrl_AdjustColumns(self.lstObjs)
		self._sort(self.sortedColumn, self.sortAscending)

	def clearInfo(self):
		self.busItf = None
		self.objDesc = None
		self.colData = {}
		self.lstObjs.ClearAll()

	def _sortCb(self, item1, item2):
		obj1 = self.busItf.findObject(item1)
		obj2 = self.busItf.findObject(item2)
		if self.sortedColumn == 0:
			if self.sortAscending:
				return cmp(obj1.handle, obj2.handle)
			else:
				return cmp(obj2.handle, obj1.handle)
		else:
			field1 = obj1.struct.getField(self.colData[self.sortedColumn])
			field2 = obj2.struct.getField(self.colData[self.sortedColumn])
			if self.sortAscending:
				return cmp(field1, field2)
			else:
				return cmp(field2, field1)

	def _sort(self, column, ascending):
		self.sortedColumn = column
		self.sortAscending = ascending
		self.lstObjs.SortItems(self._sortCb)

	def _onListCtrlColClick(self, evt):
		column = evt.GetColumn()
		self._sort(column,
				True if (column != self.sortedColumn) else not self.sortAscending)

	def _setupFields(self):
		self.colData = {}
		self.lstObjs.ClearAll()
		self.lstObjs.InsertColumn(0, "handle")
		idx = 1
		for fieldDesc in self.objDesc.structDesc.fieldsDesc.values():
			self.lstObjs.InsertColumn(idx, fieldDesc.name)
			self.colData[idx] = fieldDesc
			idx += 1

#===============================================================================
#===============================================================================
class _PnlInfoObj(wx.Panel):
	_ID_LST_PROPS = 1
	_ID_CBX_METHOD = 2
	_ID_STC_ARGS_BMP = 3
	_ID_CBX_ARGS = 4
	_ID_BTN_CALL = 5
	_ID_STC_ARGS_DESC = 6
	_ID_TXT_ARGS_DESC = 7
	def __init__(self, parent, pnlLog):
		wx.Panel.__init__(self, parent=parent)
		self.busItf = None
		self.obj = None
		self.pnlLog = pnlLog

		# Cache of latest call arguments
		self.callArgsCache = {}

		# OK/KO images
		self.bmpOK = wx.Bitmap(_PNG_OK_PATH)
		self.bmpKO = wx.Bitmap(_PNG_KO_PATH)

		# Create ListCtrl for properties
		self.lstProps = wx.ListCtrl(parent=self,
				id=_PnlInfoObj._ID_LST_PROPS,
				style=wx.LC_REPORT|wx.LC_SINGLE_SEL|wx.LC_HRULES|wx.LC_VRULES)
		self.lstProps.InsertColumn(0, "Property")
		self.lstProps.InsertColumn(1, "Type")
		self.lstProps.InsertColumn(2, "Value")

		# Create controls for Call
		self.cbxMethod = wx.ComboBox(parent=self,
				id=_PnlInfoObj._ID_CBX_METHOD,
				style=wx.CB_DROPDOWN|wx.CB_READONLY)
		self.stcArgsBmp = wx.StaticBitmap(parent=self,
				id=_PnlInfoObj._ID_STC_ARGS_BMP,
				size=wx.Size(16, 16))
		self.cbxArgs = wx.ComboBox(parent=self,
				id=_PnlInfoObj._ID_CBX_ARGS,
				style=wx.CB_DROPDOWN|wx.TE_PROCESS_ENTER)
		self.btnCall = wx.Button(parent=self,
				id=_PnlInfoObj._ID_BTN_CALL, label="Call")
		self.stcArgsDesc = wx.StaticText(parent=self,
				id=_PnlInfoObj._ID_CBX_ARGS, label="Args type:")
		self.txtArgsDesc = wx.TextCtrl(parent=self,
				id=_PnlInfoObj._ID_TXT_ARGS_DESC,
				style=wx.TE_READONLY)

		# Both enter in args ComboBox and call Button will do the call
		wx.EVT_LIST_ITEM_SELECTED(self, _PnlInfoObj._ID_LST_PROPS, self.OnLstProps)
		wx.EVT_COMBOBOX(self, _PnlInfoObj._ID_CBX_METHOD, self.onCbxMethod)
		wx.EVT_TEXT_ENTER(self, _PnlInfoObj._ID_CBX_ARGS, self.onBtnCall)
		wx.EVT_TEXT(self, _PnlInfoObj._ID_CBX_ARGS, self.onCbxArgsChanged)
		wx.EVT_BUTTON(self, _PnlInfoObj._ID_BTN_CALL, self.onBtnCall)

		sizer1 = wx.BoxSizer(orient=wx.VERTICAL)
		sizer2 = wx.BoxSizer(orient=wx.HORIZONTAL)
		sizer3 = wx.BoxSizer(orient=wx.HORIZONTAL)

		sizer1.Add(self.lstProps, proportion=1, border=2, flag=wx.ALL|wx.EXPAND)
		sizer1.Add(sizer2, proportion=0, border=0, flag=wx.EXPAND)
		sizer1.Add(sizer3, proportion=0, border=0, flag=wx.EXPAND)

		sizer2.Add(self.cbxMethod, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(self.stcArgsBmp, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer2.Add(self.cbxArgs, proportion=1, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.EXPAND)
		sizer2.Add(self.btnCall, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL)

		sizer3.Add(self.stcArgsDesc, proportion=0, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL)
		sizer3.Add(self.txtArgsDesc, proportion=1, border=2,
				flag=wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.EXPAND)

		self.SetSizer(sizer1)

	def setInfo(self, busItf, obj):
		# Setup properties if type of object has changed
		oldDesc = None if self.obj is None else self.obj.desc
		self.busItf = busItf
		self.obj = obj
		if self.obj.desc != oldDesc:
			self._setupProperties()
		# Update content
		self.updateInfo()

	def updateInfo(self):
		# Update value of fields
		idx = 0
		for fieldDesc in self.obj.desc.structDesc.fieldsDesc.values():
			listItem = wx.ListItem()
			listItem.SetId(idx)
			listItem.SetColumn(2)
			listItem.SetText(str(self.obj.struct.getField(fieldDesc)))
			self.lstProps.SetItem(listItem)
			idx += 1
		utils.listCtrl_AdjustColumns(self.lstProps)

	def clearInfo(self):
		# Clear current content
		self.lstProps.DeleteAllItems()
		self.busItf = None
		self.obj = None

	def _setupProperties(self):
		# Setup name and type
		self.lstProps.DeleteAllItems()
		idx = 0
		for fieldDesc in self.obj.desc.structDesc.fieldsDesc.values():
			listItem = wx.ListItem()
			listItem.SetId(idx)
			listItem.SetColumn(0)
			listItem.SetText(fieldDesc.name)
			self.lstProps.InsertItem(listItem)
			listItem.SetColumn(1)
			listItem.SetText(fieldDesc.rawType)
			self.lstProps.SetItem(listItem)
			idx += 1
		# Methods
		self.cbxMethod.Clear()
		for mtdDesc in self.obj.desc.methodsDesc.values():
			utils.comboBox_AddItemData(self.cbxMethod, mtdDesc.name, mtdDesc)
		utils.comboBox_SetSelectedItem(self.cbxMethod, 0)
		self._updateTxtArgsDesc()
		self._updateCbxArgs()
		self._updateStcArgsBmp()

	def _updateTxtArgsDesc(self):
		mtdDesc = utils.comboBox_GetSelectedItemData(self.cbxMethod, None)
		if mtdDesc is None:
			self.txtArgsDesc.SetValue("")
		else:
			argsDesc = ["%s:%s" % (fieldDesc.name, fieldDesc.rawType)
					for fieldDesc in mtdDesc.structDesc.fieldsDesc.values()]
			if not argsDesc:
				self.txtArgsDesc.SetValue("NO ARGUMENTS")
			else:
				self.txtArgsDesc.SetValue(" ".join(argsDesc))

	def _updateCbxArgs(self):
		mtdDesc = utils.comboBox_GetSelectedItemData(self.cbxMethod, None)
		if mtdDesc is None:
			self.cbxArgs.Clear()
		else:
			key = "%s:%s:%s" % (self.busItf.getName(), self.obj.desc.uid, mtdDesc.uid)
			if key not in self.callArgsCache:
				self.cbxArgs.Clear()
			else:
				self.cbxArgs.SetItems(self.callArgsCache[key])
				utils.comboBox_SetSelectedItem(self.cbxArgs, 0)

	def _updateStcArgsBmp(self):
		mtdDesc = utils.comboBox_GetSelectedItemData(self.cbxMethod, None)
		if mtdDesc is not None:
			# Try to decode call arguments
			ok = True
			strArgs = self.cbxArgs.GetValue()
			if strArgs:
				strArgs += ",_nolog=True"
			else:
				strArgs += "_nolog=True"
			namespace = {"obj": self.obj}
			data = "call = obj.createCall_%s(%s)" % (mtdDesc.name, strArgs)
			try:
				exec data in namespace
			except StandardError as ex:
				ok = False
			self.stcArgsBmp.SetBitmap(self.bmpOK if ok else self.bmpKO)
		else:
			# Clear bitmap
			self.stcArgsBmp.SetBitmap(self.bmpOK)

	def OnLstProps(self, evt):
		# Get field description of selected item
		selectedItem = utils.listCtrl_GetSelectedItem(self.lstProps)
		if selectedItem < 0:
			return
		fieldDesc = self.obj.desc.structDesc.fieldsDesc.values()[selectedItem]
		# Select ComboBox for methods
		if fieldDesc.role == obus.FieldRole.OBUS_METHOD:
			mtdDesc = self.obj.desc.methodsDesc[fieldDesc.uid]
			utils.comboBox_SetSelectedItemData(self.cbxMethod, mtdDesc)
			self._updateTxtArgsDesc()
			self._updateCbxArgs()
			self._updateStcArgsBmp()


	def onCbxMethod(self, evt):
		self._updateTxtArgsDesc()
		self._updateCbxArgs()
		self._updateStcArgsBmp()

	def onCbxArgsChanged(self, evt):
		self._updateStcArgsBmp()

	def onBtnCall(self, evt):
		mtdDesc = utils.comboBox_GetSelectedItemData(self.cbxMethod, None)
		if mtdDesc is not None:
			strArgs = self.cbxArgs.GetValue()
			namespace = {"obj": self.obj}
			data = "call = obj.createCall_%s(%s)" % (mtdDesc.name, strArgs)
			try:
				exec data in namespace
				call = namespace["call"]
				self.busItf.sendMethodCall(call)
				self.pnlLog.log(self.busItf, "Object call: %s:%d %s:%d",
						call.desc.name, call.handle,
						call.obj.desc.name, call.obj.handle)
				if self.pnlLog.verbose:
					self.pnlLog.log(None, "    -> %s", str(call))
			except StandardError as ex:
				self.pnlLog.log(None, str(ex))
			# Save arguments in cache
			key = "%s:%s:%s" % (self.busItf.getName(), self.obj.desc.uid, mtdDesc.uid)
			if key not in self.callArgsCache:
				self.callArgsCache[key] = [strArgs]
			else:
				if strArgs in self.callArgsCache[key]:
					self.callArgsCache[key].remove(strArgs)
				elif len(self.callArgsCache[key]) >= 16:
					self.callArgsCache[key].remove(15)
				self.callArgsCache[key].insert(0, strArgs)
			self._updateCbxArgs()
