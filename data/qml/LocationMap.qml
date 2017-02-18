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
	property real latitude
	property real longitude
	property bool enabled: true
	signal moved(real newLatitude, real newLongitude)

	width: parent.width
	height: childrenRect.height

	// This should be done simply using a binding but it seems to be buggy with current Qt version.
	onLatitudeChanged: updatePointer()
	onLongitudeChanged: updatePointer()
	Component.onCompleted: updatePointer()
	function updatePointer()
	{
		pointer.x = image.width / 2 + root.longitude * image.width / 360 - pointer.width / 2
		pointer.y = image.height / 2 - root.latitude * image.height / 180 - pointer.height / 2
	}

	Image {
		id: image
		anchors.top: parent.top
		anchors.left: parent.left
		source: "images/world.png"
		width: parent.width
		fillMode: Image.PreserveAspectFit

		Image {
			id: pointer
			source: "images/map-pointer.png"
		}
		MouseArea {
			anchors.fill: parent
			onClicked: {
				if (root.enabled)
					image.setValue(mouse.x, mouse.y)
			}
		}
		function setValue(x, y) {
			var newLatitude = -(y / image.height * 180 - 90)
			var newLongitude = x / image.width * 360 - 180
			root.moved(newLatitude, newLongitude)
		}
	}
}
