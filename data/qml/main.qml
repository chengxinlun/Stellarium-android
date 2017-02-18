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
	id: root
	property string mode: "DEFAULT" // DEFAULT | TIME | QUICKBAR | DIALOGS | SETTINGS
	property bool isKindle : stellarium.model.indexOf("Amazon") !== -1

	function jdToDate(jd) {
		var unix_time = (jd - 2440587.5) * 86400
		var ret = new Date()
		ret.setTime(unix_time * 1000.0)
		return ret
	}

	function dateToJd(date) {
		var unix_time = date.getTime() / 1000.0
		return unix_time / 86400.0 + 2440587.5
	}

	// Show equatorial grid in time mode when we drag.
	property bool savedGridFlag;
	StelAction {
		id: showEquatorGrid
		action: "actionShow_Equatorial_Grid"
	}
	property alias dragging: touchPinchArea.dragging
	onDraggingChanged: {
		if (root.mode !== "TIME") return
		if (dragging) {
			savedGridFlag = showEquatorGrid.checked
			showEquatorGrid.checked = true
		} else {
			showEquatorGrid.checked = savedGridFlag
		}
	}

	// XXX: this is a bit messy.
	onModeChanged: {
		if (mode === "SETTINGS") {
			settingsPanel.state = "visible"
		} else if (mode !== "DIALOGS") {
			settingsPanel.close(false)
		}
		if (mode == "DEFAULT") { // The focus could have been grabed by the search box.
			root.focus = true
			stellarium.dragTimeMode = false
		}
	}

	Stellarium {
		id: stellarium
		anchors.fill: parent
		forwardClicks: mode !== "DEFAULT"
		property bool hasSelectedObject: stellarium.selectedObjectInfo !== ""

		// Trick to deselect current object when we click on it again.
		property string lastSelectedObject
		onSelectedObjectChanged: {
			if (selectedObjectName && selectedObjectName == lastSelectedObject) {
				lastSelectedObject = ""
				unselectObject()
				return
			}
			lastSelectedObject = selectedObjectName
			root.mode = "DEFAULT"
		}

		TouchPinchArea {
			id: touchPinchArea
			anchors.fill: parent
			useMouseArea: stellarium.desktop
			onPressed: stellarium.touch(0, x, height - 1 - y)
			onReleased: {
				if (root.mode === "DEFAULT")
					stellarium.touch(1, x, height - 1 - y)
				stellarium.dragTimeMode = false
				root.mode = "DEFAULT"
				Qt.inputMethod.hide()
			}
			onMoved: stellarium.touch(2, x, height - 1 - y)
			onMoveFinished: stellarium.touch(1, x, height - 1 - y)
			onPinchStarted: stellarium.pinch(1, true)
			onPinchUpdated: stellarium.pinch(pinch, false)
		}

		onDragTimeModeChanged: {
			if (dragTimeMode) {
				root.mode = "TIME"
			} else {
				root.mode = "DEFAULT"
			}
		}

		Component.onCompleted: {
			if (autoGotoNight) {
				goToNight.start();
			}
		}

		Timer {
			id: goToNight
			property int state: 0
			interval: 1000
			running: false
			repeat: true
			triggeredOnStart: true
			onTriggered: {
				// Wait one frame to make sure the location is properly set.
				if (state === 0) {
					state = 1;
				} else if (state === 1) {
					if (stellarium.isDay()) {
						rootMessage.show(qsTr("It is daytime, fast forward time until..."))
						state = 2
					} else {
						stop();
					}
				} else if (state === 2) {
					stellarium.timeRate = 2 / 24;
					interval = 100
					state = 3
				} else if (state === 3 && !stellarium.isDay()) {
					stellarium.timeRate = 1 / 24. / 60. / 60.
					stop()
					var lines = stellarium.printableTime.split(" ")
					rootMessage.show(lines[1], 3000)
				}
			}
		}
	}

	// Handle android buttons.
	focus: true
	Keys.onReleased: {
		if (event.key === Qt.Key_Menu) {
			root.mode = "QUICKBAR"
		}
		if (event.key === Qt.Key_Search) {
			root.mode = "QUICKBAR"
			searchInput.takeFocus(true)
			Qt.inputMethod.show()
		}
		if (event.key === Qt.Key_Back) {
			if (mode === "DEFAULT") {
				event.accepted = true
				Qt.quit()
				return
			}
			event.accepted = true
			mode = "DEFAULT"
		}
	}

	StelMessage {
		id: rootMessage
	}

	SettingsPanel {
		id: settingsPanel
		onClosed: root.mode = "QUICKBAR"
	}

	SearchInput {
		id: searchInput
		visible: root.mode === "QUICKBAR"
		onReturnPressed: root.mode = "DEFAULT"
	}

	InfoPanel {
		id: infoPanel
		anchors {
			left: parent.left
			top: parent.top
			margins: 0
		}
		opacity: (root.mode === "DEFAULT" && stellarium.hasSelectedObject)? 1 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
	}

	ImageButton {
		opacity: (root.mode === "QUICKBAR")? 1 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
		
		anchors {
			left: parent.left
			bottom: quickBar.top
			bottomMargin: 20*rootStyle.scale
		}
		
		source: "images/settings.png"
		helpMessage: qsTr("Settings")
		onClicked: {
			root.mode = "SETTINGS"
		}
	}

	ImageButton {
		id: openQuickBarButton
		source: "images/openQuickBar.png"
		opacity: (root.mode === "DEFAULT")? 1 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
		helpMessage: qsTr("Open Quick Bar")
		anchors {
            left: parent.left
			bottom: parent.bottom
		}
		onClicked: {
			root.mode = "QUICKBAR"
		}
	}

	QuickBar {
		id: quickBar
		opacity: (root.mode === "QUICKBAR")? 1 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			margins: 5
		}
	}

	Text {
		id: locationText
		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: quickBar.top
			margins: rootStyle.margin
		}
		visible: root.mode === "QUICKBAR"
		color: "white"
		font.pixelSize: rootStyle.fontNormalSize
		text: {
			switch (stellarium.gpsState) {
			case "Searching":
				return qsTr("GPS...")
			case "Found":
				return qsTr("GPS")
			default:
				return stellarium.location
			}
		}
	}

	// Time.
	Item {
		id: timeBarFrame
        property bool fullWithDate: stellarium.dragTimeMode
		height: openQuickBarButton.height
        width: hourText.width+rootStyle.margin
		anchors {
            right: parent.right
			bottom: parent.bottom
			margins: 0
		}
        opacity: (root.mode === "DEFAULT" || root.mode === "TIME")? 1 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
		
		Text {
			id: hourText
			anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom:  timeBarFrame.fullWithDate ? dateText.top : parent.bottom
            anchors.bottomMargin: 5*rootStyle.scale
			color: "white"
            font.pixelSize: rootStyle.fontLargeSize
            font.bold: true
			text: {
				var lines = stellarium.printableTime.split(" ")
                if (timeBarFrame.fullWithDate == true)
                    return lines[1];
                else
                    return lines[1].substring(0, 5);
			}
		}
        Text {
			id: dateText
			anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom:  parent.bottom
            anchors.bottomMargin: 5*rootStyle.scale
			color: "white"
			font.pixelSize: rootStyle.fontSmallSize
			text: {
				var lines = stellarium.printableTime.split(" ")
				return lines[0];
			}
            visible: timeBarFrame.fullWithDate
        }
		MouseArea {
			anchors.fill: parent
			onClicked: {
				stellarium.dragTimeMode = true
			}
		}
	}

	TimeBar {
		id: timeBar
		opacity: (root.mode === "TIME")? 1 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
		anchors {
            left: parent.left
			bottom: parent.bottom
            margins: 10
		}
		onVisibleChanged: {
			if (visible)
                rootMessage.show(qsTr("Drag sky to move in time"), 3000)
			else
				rootMessage.hide()
		}
	}

	// The zoom buttons.
	Item {
		id: zoomButtons
		opacity: (root.mode === "DEFAULT")? 0.4 : 0
		visible: opacity > 0
		Behavior on opacity { PropertyAnimation {} }
		anchors {
			bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
		}
		height: childrenRect.height

		Row {
			height: childrenRect.height
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: root.isKindle ? 30*rootStyle.scale : 0
			ImageButton {
				source: "images/autoZoomIn.png"
				onPressed: stellarium.zoom(1)
				onReleased: stellarium.zoom(0)
			}
			ImageButton {
				source: "images/autoZoomOut.png"
				onPressed: stellarium.zoom(-1)
				onReleased: stellarium.zoom(0)
			}
		}
	}

	// The fov
	Text {
		id : fov
		property double fade: 0
		opacity: fade * ((root.mode === "DEFAULT")? 0.75 : 0)
		visible: opacity > 0
		anchors {
			bottom: zoomButtons.top
			horizontalCenter: zoomButtons.horizontalCenter
		}
		text: "FOV " + stellarium.fov.toFixed(1) + '°'
		horizontalAlignment: Text.AlignHCenter
		font.pixelSize: rootStyle.fontNormalSize
		color: "white"
		onTextChanged: showAnimation.restart()
		SequentialAnimation {
			id: showAnimation
			NumberAnimation {target: fov; properties: "fade"; to: 1}
			PauseAnimation { duration: 1000 }
			NumberAnimation {target: fov; properties: "fade"; to: 0}
		}
	}

	// Show FPS
	/*
	Text {
		anchors {
			top: parent.top
			right: parent.right
		}
		color: "white"
		text: "fps:" + stellarium.fps
	}
	*/

	Item {
		id: rootStyle
		property double scale: stellarium.guiScaleFactor
		property int fontReferenceSize: 16*scale
		property int fontNormalSize: 1.2*fontReferenceSize
		property int fontSmallSize: 0.9*fontReferenceSize
		property int fontLargeSize: 1.5*fontReferenceSize
		property int fontExtraLargeSize: 2*fontReferenceSize
		property int margin: 8*scale
		// A snapped menu width to avoid having menu taking almost the screen width but not quite (which looks bad)
		property int niceMenuWidth: root.width-(400.*stellarium.guiScaleFactor)<80*stellarium.guiScaleFactor ? root.width : 400.*stellarium.guiScaleFactor
		property int maxMenuHeight: root.height - 50 * stellarium.guiScaleFactor
		property color color0: "#01162d"
		property color color1: "#012d1b"
	}

	// Create a color based on the style colors
	function getColor(index, darker, pressed)
	{
		var c = [rootStyle.color0, rootStyle.color1][index];
		if (darker)
			c = Qt.darker(c, darker)
		if (pressed)
			c = Qt.lighter(c, 1.5);
		return c;
	}
}
