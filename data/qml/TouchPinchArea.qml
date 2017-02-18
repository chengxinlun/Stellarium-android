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
	property bool useMouseArea: false
	signal pressed(real x, real y)
	signal released(real x, real y)
	signal moved(real x, real y)
	signal moveFinished(real x, real y)
	signal pinchStarted()
	signal pinchUpdated(real pinch)
	property bool dragging: area.mode === "MOVE"
	property real moveDist: rootStyle.scale * 10

	function getDist(x1, y1, x2, y2) {
		var x = x1 - x2
		var y = y1 - y2
		return Math.sqrt(x * x + y * y)
	}

	MultiPointTouchArea {
		id: area
		enabled: !root.useMouseArea
		anchors.fill: parent
	
		minimumTouchPoints: 1
		maximumTouchPoints: 2
		touchPoints: [
			TouchPoint { id: point1 },
			TouchPoint { id: point2 }
		]
		property string mode: "" // "PINCH", "MOVE"
		property real pinchStartDist
		property real startX
		property real startY
	
		onPressed: {
			if (point2.pressed)
				mode = "PINCH"
			if (mode === "") {
				root.pressed(point1.x, point1.y)
				startX = point1.x
				startY = point1.y
			}
			if (mode === "PINCH"){
				pinchStartDist = root.getDist(point1.x, point1.y, point2.x, point2.y)
				pinchStartDist = Math.max(1, pinchStartDist)
				root.pinchStarted()
			}
		}
		onReleased: {
			if (mode === "") {
				root.released(point1.x, point1.y)
			}
			if (mode === "MOVE")
				root.moveFinished(point1.x, point1.y)
			if (!point1.pressed && !point2.pressed)
				mode = ""
		}
		onUpdated: {
			var dist
			if (mode === "") {
				dist = root.getDist(startX, startY, point1.x, point1.y)
				if (dist >= root.moveDist)
					mode = "MOVE"
			}
			if (mode === "PINCH") {
				dist = root.getDist(point1.x, point1.y, point2.x, point2.y)
				root.pinchUpdated(dist / pinchStartDist)
			}
			if (mode === "MOVE") {
				root.moved(point1.x, point1.y)
			}
		}
	}

	// Alternative implementation using a mouse area.
	// Used for desktop.
	MouseArea {
		id: mouseArea
		anchors.fill: parent
		enabled: root.useMouseArea
		property string mode: "" // "", "MOVE"
		property real startX
		property real startY
		onPressed: {
			startX = mouseX
			startY = mouseY
			root.pressed(mouseX, mouseY)
		}
		onReleased: {
			if (mode === "")
				root.released(mouseX, mouseY)
			if (mode == "MOVE")
				root.moveFinished(mouseX, mouseY)
			mode = ""
		}
		onPositionChanged: {
			var dist
			dist = root.getDist(startX, startY, mouseX, mouseY)
			if (dist >= root.moveDist)
				mode = "MOVE"
			if (mode === "MOVE") {
				root.moved(mouseX, mouseY)
			}
		} 
	}
}
