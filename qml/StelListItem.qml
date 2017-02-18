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
import Stellarium 1.0

Item {
	property string text
	property string rightText
	property bool checkbox: false
	property bool withArrow: false
	property bool selected: false
	property bool enabled: true
	property alias action: stelAction.action
	property string setting
	signal clicked;

	id: root
	width: parent.width
	height: 45*rootStyle.scale

	StelAction {
		id: stelAction
		onCheckedChanged: {
			if (root.setting)
				stellarium.writeSetting(root.setting, stelAction.checked)
		}
	}

	Rectangle {
		id: select
		anchors.fill: parent
		color: getColor(0)
		opacity: ((root.selected || mouseArea.pressed) && root.enabled) ? 0.75 : 0
		Behavior on opacity {NumberAnimation { duration: 50 }}
	}

	Text {
		id: text
		anchors {
			verticalCenter: parent.verticalCenter
			left: parent.left
			right: arrow.visible ? arrow.left : parent.right
			margins: rootStyle.margin
		}
		text: root.text
		font.pixelSize: rootStyle.fontNormalSize
		color: "white"
		opacity: root.enabled ? 0.9 : 0.4
		elide: Text.ElideRight
	}
	Text {
		id: rightText
		anchors {
			verticalCenter: parent.verticalCenter
			right: arrow.visible ? arrow.left : parent.right
			rightMargin: arrow.visible ? rootStyle.margin : 0
		}
		text: root.rightText
		font.pixelSize: rootStyle.fontNormalSize
		color: "white"
		opacity: root.enabled ? 1 : 0.5
	}
	// Separator line
	Rectangle {
		color: "grey"
		height: 2
		anchors {
			bottom: parent.bottom
			left: parent.left
			right: parent.right
		}
	}
	// Right arrow
	Image {
		id: arrow
		visible: withArrow
		anchors {
			right: parent.right
			verticalCenter: parent.verticalCenter
		}
		source: "images/navigation_next_item.png"
        scale: rootStyle.scale*0.5
		opacity: root.enabled ? 1 : 0.5
	}
	// Checkbox
	StelCheckBox {
		id: checkboxElem
		visible: root.checkbox
		checked: stelAction.checked
		anchors {
			right: parent.right
			margins: rootStyle.margin
			verticalCenter: parent.verticalCenter
		}
	}

	MouseArea {
		id: mouseArea
		anchors.fill: parent
		onClicked: if (root.enabled) root.clicked()
	}

	onClicked: {
		stelAction.trigger()
	}
}
