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

Text {
	id: root
	anchors {
		left: parent.left
		right: parent.right
		verticalCenter: parent.verticalCenter
		margins: 10
	}
	horizontalAlignment: Text.AlignHCenter
	wrapMode: Text.Wrap
	color: "white"
	font.pixelSize: rootStyle.fontLargeSize
	opacity: 0

	property int duration
	SequentialAnimation {
		id: showAnimation
		NumberAnimation {
			target: root
			properties: "opacity"
			to: 1
		}
		PauseAnimation { duration: root.duration }
		ScriptAction { script: root.hide(); }
	}

	NumberAnimation {
		id: hideAnimation
		target: root
		properties: "opacity"
		to: 0
	}

	function show(msg, duration) {
		root.duration = duration ? duration : 1000
		root.text = msg
		hideAnimation.stop()
		showAnimation.start()
	}

	function hide() {
		showAnimation.stop()
		hideAnimation.start()
	}
}
