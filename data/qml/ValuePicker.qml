/*
 * Stellarium
 * Copyright (C) 2014 Guillaume Chereau
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

StelDialog {
	id: root
	withMargins: true

	property int min: 0
	property int max: 10
	property int value: 0
	property string valueText: value.toString()

	width: rootStyle.niceMenuWidth
	height: childrenRect.height

	Item {
		width: parent.width
		height: 50 * rootStyle.scale
		ImageButton {
			id: buttonLeft
			source: "images/downArrowEmpty.png"
			rotation: 90
			anchors {
				left: parent.left
				margins: rootStyle.margin
				verticalCenter: parent.verticalCenter
			}
			onClicked: {
				root.value = Math.max(min, value - 1)
			}
			opacity: value <= min ? 0.5 : 1
		}
		TextInput {
			text: root.valueText
			color: "white"
			horizontalAlignment: TextInput.AlignHCenter
			font.pixelSize: rootStyle.fontExtraLargeSize
			font.bold: true
			anchors {
				left: buttonLeft.right
				right: buttonRight.left
				verticalCenter: parent.verticalCenter
			}

			// Does not work with '-' on kindle fire.
			// inputMethodHints: Qt.ImhFormattedNumbersOnly

			onFocusChanged: {
				if (!focus) {
					root.value = parseInt(text)
				}
			}

			Keys.onReturnPressed: {
				root.value = parseInt(text)
				Qt.inputMethod.hide();
			}
		}
		ImageButton {
			id: buttonRight
			source: "images/downArrowEmpty.png"
			rotation: -90
			anchors {
				right: parent.right
				margins: rootStyle.margin
				verticalCenter: parent.verticalCenter
			}
			onClicked: {
				root.value = Math.min(max, value + 1)
			}
			opacity: value >= max ? 0.5 : 1
		}
	}
}