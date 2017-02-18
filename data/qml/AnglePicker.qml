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

StelDialog {
	id: root
	title: type == "LAT" ? qsTr("Latitude") : qsTr("Longitude")
	property string type // LAT | LON
	property int max: (type === "LAT") ? 90 : 180
	property real value: 0

	property bool disableChange: false
	property alias degree: degreeInput.value
	property alias minute: minuteInput.value
	property alias second: secondInput.value
	property alias direction: directionInput.value // N: 0, S: 1

	width: 320*rootStyle.scale
	height: childrenRect.height

	Row {
		width: parent.width
		height: childrenRect.height
		StelSpinBox {
			id: degreeInput
			text: "degree"
			min: 0
			max: root.max
			valueText: "%1°".arg(value)
			width: parent.width / 4
		}
		StelSpinBox {
			id: minuteInput
			text: "minute"
			min: 0
			max: 60
			valueText: "%1'".arg(value)
			width: parent.width / 4
		}
		StelSpinBox {
			id: secondInput
			text: "second"
			min: 0
			max: 60
			valueText: "%1\"".arg(value)
			width: parent.width / 4
		}
		StelSpinBox {
			id: directionInput
			text: "direction"
			min: 0
			max: 2
			valueText: type == "LAT" ? (value === 0 ? "N" : "S") :
									   (value === 0 ? "E" : "W")
			width: parent.width / 4
		}
	}

	onDegreeChanged: recomputeValue()
	onMinuteChanged: recomputeValue()
	onSecondChanged: recomputeValue()
	onDirectionChanged: recomputeValue()

	onValueChanged: {
		if (root.disableChange) return
		var tmp = root.value
		var sign = tmp >= 0
		tmp = Math.abs(tmp)
		var degree = Math.floor(tmp)
		tmp = (tmp - degree) * 60
		var minute = Math.floor(tmp)
		tmp = (tmp - minute) * 60
		var second = Math.floor(tmp)
		root.degree = degree
		root.minute = minute
		root.second = second
		root.direction = sign ? 0 : 1
	}

	function recomputeValue() {
		root.disableChange = true
		var value = degree + minute / 60 + second / 3600
		if (direction == 1)
			value = -value
		root.value = value
		root.disableChange = false
	}
}
