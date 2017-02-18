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
	height: 55*rootStyle.scale
	opacity: textInput.activeFocus ? 1 : 0.4
	function takeFocus() {textInput.focus = true}
	
	signal returnPressed

	function listItemClicked(mData) {
		Qt.inputMethod.hide()
		textInput.color = "#AAAAAA"
		textInput.font.italic = true
		textInput.focus = false
		stellarium.gotoObject(mData)
		textInput.text = qsTr("Search")
		resultsList.model = []
		root.returnPressed()
	}
	
	Rectangle {
		color: 'transparent'
		border.width: 1*rootStyle.scale
		border.color: 'white'
		anchors.fill: parent
		anchors.margins: rootStyle.margin

		Image {
			id: loupeImage
			source: "images/search.png"
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.leftMargin: rootStyle.margin
			width: 24*rootStyle.scale
			height: 24*rootStyle.scale
		}
		
		TextInput {
			id: textInput
			anchors {
				top: parent.top
				left: loupeImage.right
				right: parent.right
				bottom: parent.bottom
				leftMargin: rootStyle.margin
			}
			inputMethodHints: Qt.ImhNoPredictiveText
			verticalAlignment: Text.AlignVCenter
			font.pixelSize: rootStyle.fontNormalSize
			color: "#AAAAAA"
			text: qsTr("Search")
			font.italic: true

			function accept() {
				focus = false
				Qt.inputMethod.hide()
				// Pick the right name from the list (if any).
				var objName = text.trim()
				for (var i = 0; i < resultsList.model.length; i++) {
					if (resultsList.model[i].toLowerCase() === objName.toLowerCase()) {
						objName = resultsList.model[i]
						break;
					}
				}
				resultsList.model = []
				color = "#AAAAAA"
				stellarium.gotoObject(objName)
				text = qsTr("Search")
				font.italic = true
				root.returnPressed()
			}

			onAccepted: {accept();}
			Keys.onReturnPressed: {accept();}

			onTextChanged: {
				resultsList.model = (text === qsTr("Search")) ? [] : stellarium.search(text)
			}
			
			onActiveFocusChanged: {
				if (activeFocus && text === qsTr("Search"))
				{
					text = ""
					textInput.font.italic = false
					textInput.color = 'white'
				}
			}
		}
		
		ListView {
			id: resultsList
			width: parent.width
			height: Math.min(320*rootStyle.scale, 60*rootStyle.scale + count*45*rootStyle.scale)
			anchors.left: parent.left
			anchors.top: textInput.bottom
			clip: true
			delegate: StelListItem {
				withArrow: false
				text: modelData
				onClicked: {
					root.listItemClicked(modelData)
				}
			}
			model: []
		}
		
	}
}
