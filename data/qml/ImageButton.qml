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

Item
{
	id: root

	width: 48*rootStyle.scale
	height: 48*rootStyle.scale

	property alias action: stelAction.action
	property string setting
	property alias source: buttonImg.source
	property alias mirror: buttonImg.mirror
	property string helpMessage: stelAction.text
	
	signal clicked
	signal pressed
	signal released

	StelAction {
		id: stelAction
		onCheckedChanged: {
			if (root.setting)
				stellarium.writeSetting(root.setting, stelAction.checked)
		}
	}

	states: [
		State {
			name: "OFF"
			when: stelAction.checkable && !stelAction.checked
			PropertyChanges {target: root; opacity: 0.5}
		},
		State {
			name: "PRESSED"
			PropertyChanges {target: halo; opacity: 1}
		}

	]

	// We use a timer to show the help message so that if we do a short click on
	// A uncheked button it doesn't show the help message.
	Timer {
		id: showMessageTimer
		interval: 200
		onTriggered: {
			rootMessage.show(root.helpMessage)
		}
	}

	MouseArea {
		anchors.fill: parent
		onClicked: {
			stelAction.trigger();
			root.clicked();
			/*if (stelAction.checked)
				rootMessage.show(root.helpMessage)
			else
				rootMessage.hide()*/
		}

		onPressed: {
			halo.state = "ON"
			showMessageTimer.start()
			root.pressed()
		}

		onReleased: {
			rootMessage.hide()
			halo.state = "OFF"
			showMessageTimer.stop()
			root.released()
		}

		onPressAndHold: {}
	}

	Image {
		id: buttonImg
		anchors.fill: parent

		Image {
			width: parent.width*1.2
			height: parent.height*1.2
			anchors.centerIn: parent
			states: [
				State {
					name: "OFF"
					PropertyChanges {target: halo; opacity: 0}
				},
				State {
					name: "ON"
					PropertyChanges {target: halo; opacity: 1}
				}
			]
			id: halo
			source: "images/hoverHalo.png"
			opacity: 0
			transitions: [
				Transition {
					from: "OFF"; to: "ON"
					PropertyAnimation { target: halo; property: "opacity"; duration: 10 }
				},
				Transition {
					from: "ON"; to: "OFF"
					PropertyAnimation { target: halo; property: "opacity"; duration: 500 }
				}
			]
		}
	}
}
