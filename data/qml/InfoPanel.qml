/*
 * Stellarium
 * Copyright (C) 2013 Fabien Chereau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */

import QtQuick 2.2

Item {
	id: root
	width: rootStyle.niceMenuWidth
	property bool fullInfoVisible: false
	
	Rectangle {
		id: nameAndInfoFrame
		opacity: 0.1
		color: "white"
		anchors {
			left: root.left
			right: root.right
			top: root.top
		}
		height: Math.max(nameAndInfo.height + 2*rootStyle.margin-2.*rootStyle.scale, 40*rootStyle.scale)
		
		MouseArea {
			anchors.fill: parent
			onClicked: {
				root.fullInfoVisible = !root.fullInfoVisible
			}
		}
	}
	
	Rectangle {
		id: nameAndInfoLine
		color: "white"
		opacity: 0.5
		anchors {
			left: nameAndInfoFrame.left
			right: nameAndInfoFrame.right
			top: nameAndInfoFrame.bottom
		}
		height: 1 * rootStyle.scale
	}
	
	ImageButton {
		source: "images/closeInfo.png"
		onClicked: stellarium.unselectObject()
		visible: !stellarium.tracking
		anchors.right: nameAndInfoFrame.right
		anchors.top: nameAndInfoFrame.top
		height: 30*rootStyle.scale
		width: 30*rootStyle.scale
	}
	
	Row {
		id: buttons
		height: childrenRect.height
		anchors {
			left: nameAndInfoFrame.left
			top: nameAndInfoFrame.bottom
		}

		ImageButton {
			source: "images/centerOnObject.png"
			action: "actionGoto_Selected_Object"
			visible: !stellarium.tracking
		}

		ImageButton {
			source: "images/autoZoomIn.png"
			action: "actionZoom_In_Auto"
			visible: stellarium.tracking
		}

		ImageButton {
			source: "images/autoZoomOut.png"
			action: "actionZoom_Out_Auto"
			visible: stellarium.tracking
		}
	}
	
	ImageButton {
		id: moreButton
		source: "images/downArrow.png"
		onClicked: {
			root.fullInfoVisible = !root.fullInfoVisible
		}
		rotation: root.fullInfoVisible ? 180 : 0
        height: 27*rootStyle.scale // Modified: moreButton too small to click, +50% size (Cheng Xinlun, Apr 18,2017)
        width: 27*rootStyle.scale // Modified: moreButton too small to click, +50% size (Cheng Xinlun, Apr 18,2017)
		anchors {
			horizontalCenter: nameAndInfoLine.horizontalCenter
			top: nameAndInfoLine.bottom
		}
		opacity: 0.7
	}

	
	Item {
		id: nameAndInfo
		anchors {
			top: parent.top
			left: parent.left
			margins: rootStyle.margin
		}
		width: root.width
		height: selectedObjectName.height+selectedObjectShortInfo.height+2 + (root.fullInfoVisible ? selectedObjectInfo.height : 0)
		
		Text {
			id: selectedObjectName
			visible: stellarium.hasSelectedObject
			text: stellarium.selectedObjectName? stellarium.selectedObjectName : qsTr("Unnamed")
			color: "white"
			font.pixelSize: rootStyle.fontNormalSize
			font.weight: Font.Bold
			anchors {
				left: parent.left
				top: parent.top
			}
			onTextChanged: {
				root.fullInfoVisible = false
			}
		}

		Text {
			id: selectedObjectShortInfo
			anchors {
				left: parent.left
				top: selectedObjectName.bottom
				topMargin: rootStyle.margin*0.5
			}
			visible: stellarium.hasSelectedObject
			text: stellarium.selectedObjectShortInfo
			font.pixelSize: rootStyle.fontSmallSize
			font.weight: Font.Light
			color: "white"
		}
		Text {
			id: selectedObjectInfo
			anchors {
				left: parent.left
				top: selectedObjectShortInfo.bottom
				topMargin: rootStyle.margin*0.5
			}
			visible: root.fullInfoVisible
			text: root.fullInfoVisible ? stellarium.selectedObjectInfo : ""
			font.pixelSize: rootStyle.fontSmallSize
			font.weight: Font.Light
			color: "white"
			
			Rectangle {
				color: "white"
				opacity: 0.1
				anchors {
					left: selectedObjectInfo.left
					top: selectedObjectInfo.top
					topMargin: -2* rootStyle.scale
				}
				height: 1 * rootStyle.scale
				width: nameAndInfoFrame.width-nameAndInfo.anchors.leftMargin*2
			}
		}
	}
}
