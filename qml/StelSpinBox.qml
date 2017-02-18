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
	property string text
	property int min
	property int max
	property int value: min
	property string valueText: value.toString()
	property bool large: false
	signal valueManuallyChanged
	height: childrenRect.height

	Column {
		width: parent.width
		height: childrenRect.height

		ImageButton {
			source: "images/downArrowEmpty.png"
			rotation: 180
			anchors.horizontalCenter: parent.horizontalCenter
			onClicked: {
				value = Math.min(max - 1, value + 1)
				root.valueManuallyChanged()
			}
			opacity: value >= max - 1 ? 0.5 : 1
		}
		TextInput {
			text: root.valueText
			width: parent.width
			color: "white"
			horizontalAlignment: TextInput.AlignHCenter
			font.pixelSize: root.large ? rootStyle.fontExtraLargeSize : rootStyle.fontNormalSize
			font.bold: root.large
			anchors.horizontalCenter: parent.horizontalCenter

			// Does not work with '-' on kindle fire.
			// inputMethodHints: Qt.ImhFormattedNumbersOnly

			onFocusChanged: {
				if (!focus) {
					root.value = parseInt(text)
					root.valueManuallyChanged()
				}
			}

			Keys.onReturnPressed: {
				root.value = parseInt(text)
				root.valueManuallyChanged()
				Qt.inputMethod.hide();
			}
		}
		ImageButton {
			source: "images/downArrowEmpty.png"
			anchors.horizontalCenter: parent.horizontalCenter
			onClicked: {
				value = Math.max(min, value - 1)
				root.valueManuallyChanged()
			}
			opacity: value <= min ? 0.5 : 1
		}
	}
}
