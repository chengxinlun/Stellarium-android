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

MouseArea {
	id: root
	property string title: qsTr("Stellarium")
	property bool withMargins: false
	signal closed
	default property alias children: contentItem.data
	property Item childPage: null
	visible: opacity !== 0

	height: childrenRect.height

	drag.target: root
	drag.axis: Drag.XAxis
	drag.minimumX: -root.width
	drag.maximumX: 0
	drag.filterChildren: true

	drag.onActiveChanged: {
		if (drag.active) {
			root.state = "dragging"
			return;
		}
		if (root.x < -100) {
			close();
		} else {
			root.state = "visible"
		}
	}

	function open() {root.state = "visible"}
	function close(sendSignal) {
		if (sendSignal === undefined) sendSignal = true
		if (childPage) {
			childPage.close(false)
			childPage = null
		}
		root.state = "hidden";
		if (sendSignal === true)
			closed()
	}

	NumberAnimation {
		id: openAnimation
		target: root
		properties: "x"
		to: 0
   }

	Rectangle {
		id: header
		height: 50*stellarium.guiScaleFactor
		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
		}
		gradient: Gradient {
			GradientStop { position: 0.0; color: getColor(0, 1) }
			GradientStop { position: 1.0; color: getColor(0, 1.5) }
		}
		Text {
			anchors.centerIn: parent
			text: root.title
			color: "white"
			font.pixelSize: rootStyle.fontLargeSize
		}
		Image {
			anchors {
				left: parent.left
				leftMargin: rootStyle.margin
				verticalCenter: parent.verticalCenter
			}
			mirror: true
			source: "images/navigation_next_item.png"
            scale: rootStyle.scale*0.5
			opacity: mouseArea.pressed ? 0.5 : 1
		}
		MouseArea {
			id: mouseArea
			anchors.fill: parent
			onClicked: close()
		}
	}
	Rectangle {
		color: "black"
		opacity: 0.75
		anchors {
			fill: contentItem
			topMargin: root.withMargins ? -rootStyle.margin : 0
			bottomMargin: root.withMargins ? -rootStyle.margin : 0
		}
	}

	Item {
		id: contentItem
		height: childrenRect.height
		anchors {
			top: header.bottom
			left: parent.left
			right: parent.right
			topMargin: root.withMargins ? rootStyle.margin : 0
		}
		// HACK: I add this mouse area to prevent the drag events from passing through
		// the dialog and move the sky instead.  I am not sure why this is needed, since
		// the StelDialog element is itself a MouseArea, but without this it does not
		// behave correctly at least on some phones.
		MouseArea {
			anchors.fill: parent
		}
	}

	state: "hidden"
	states: [
		State {
			name: "hidden"
			PropertyChanges { target: root; x: -width }
			PropertyChanges { target: root; opacity: 0 }
		},
		State {
			name: "visible"
			PropertyChanges { target: root; x: 0 }
			PropertyChanges { target: root; opacity: 1 }
		},
		State {
			name: "dragging"
		}

	]
	transitions: [
		Transition {
			to: "visible,hidden"
			NumberAnimation { properties: "x"; easing.type: Easing.InOutQuad }
			NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
		}
	]

	function openPage(page) {
		root.state = "hidden"
		var component = Qt.createComponent(page)
		if (component.status === Component.Error) {
			console.log("Error loading component:", component.errorString())
			return
		}
		root.childPage = component.createObject(root.parent)
		root.childPage.closed.connect(root.open)
		root.childPage.open()
		return root.childPage
	}
}
