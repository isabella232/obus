#===============================================================================
# wxpyobus - obus client gui in python.
#
# @file utils.py
#
# @brief wxpython obus utilities
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

#===============================================================================
#===============================================================================
def listCtrl_AdjustColumns(listCtrl):
	# Adjust width of columns
	colCount = listCtrl.GetColumnCount()
	listCtrl.Freeze()
	for col in range(0, colCount):
		# Compute size based on header or data, keep max of 2 values
		listCtrl.SetColumnWidth(col, wx.LIST_AUTOSIZE_USEHEADER)
		width1 = listCtrl.GetColumnWidth(col)
		listCtrl.SetColumnWidth(col, wx.LIST_AUTOSIZE)
		width2 = listCtrl.GetColumnWidth(col)
		if width2 < width1:
			listCtrl.SetColumnWidth(col, width1)
	listCtrl.Thaw()

#===============================================================================
#===============================================================================
def listCtrl_GetSelectedItem(listCtrl):
	# Search the selected item
	selectedItem = -1
	itemCount = listCtrl.GetItemCount()
	for item in range(0, itemCount):
		if listCtrl.GetItemState(item, wx.LIST_STATE_SELECTED) != 0:
			selectedItem = item
	return selectedItem

#===============================================================================
#===============================================================================
def comboBox_AddItemData(comboBox, strItem, itemData):
	# Add item
	index = comboBox.Append(strItem)
	comboBox.SetClientData(index, itemData)

#===============================================================================
#===============================================================================
def comboBox_GetSelectedItem(comboBox):
	# Get selected item index
	return comboBox.GetSelection()

#===============================================================================
#===============================================================================
def comboBox_SetSelectedItem(comboBox, selectedItem):
	# Select the item
	comboBox.SetSelection(selectedItem)

#===============================================================================
#===============================================================================
def comboBox_GetSelectedItemData(comboBox, defaultValue):
	# Get selected item index
	index = comboBox.GetSelection()
	if index != wx.NOT_FOUND:
		return comboBox.GetClientData(index)
	else:
		return defaultValue

#===============================================================================
#===============================================================================
def comboBox_SetSelectedItemData(comboBox, itemData):
	# Select the item having the specified data
	for index in range(0, comboBox.GetCount()):
		if comboBox.GetClientData(index) == itemData:
			comboBox.SetSelection(index)
			return
	comboBox.SetSelection(wx.NOT_FOUND)
