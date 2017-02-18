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

StelDialog {
	id: root
	title: qsTr("Location")
	width: rootStyle.niceMenuWidth
	
	StelAction {
		id: gpsAction
		action: "actionGPS"
	}

	Column {
		width: parent.width
		StelListItem {
			checkbox: true
			text: qsTr("Use GPS") + (stellarium.gpsState === "Searching" ? qsTr(" (searching)") : "")
			action: "actionGPS"
			enabled: stellarium.gpsState !== "Unsupported"
		}
		StelListItem {
			text: "Latitude"
			withArrow: true
			enabled: !gpsAction.checked
			rightText: latitudeRepr(stellarium.latitude)
			onClicked: {
				var picker = root.openPage("AnglePicker.qml")
				picker.type = "LAT"
				picker.value = stellarium.latitude
				function onValueChanged() {
					stellarium.setManualPosition(picker.value, stellarium.longitude)
				}
				picker.valueChanged.connect(onValueChanged)
			}
		}
		StelListItem {
			text: "Longitude"
			withArrow: true
			enabled: !gpsAction.checked
			rightText: longitudeRepr(stellarium.longitude)
			onClicked: {
				var picker = root.openPage("AnglePicker.qml")
				picker.type = "LON"
				picker.value = stellarium.longitude
				function onValueChanged() {
					stellarium.setManualPosition(stellarium.latitude, picker.value)
				}
				picker.valueChanged.connect(onValueChanged)
			}
		}
		StelListItem {
			text: qsTr("Name/City:")  // XXX: just so that we get the translation from desktop version
			withArrow: true
			enabled: !gpsAction.checked
			rightText: stellarium.location
			onClicked: root.openPage("LocationCityPicker.qml")
		}
		LocationMap {
			latitude: stellarium.latitude
			longitude: stellarium.longitude
			enabled: !gpsAction.checked
			onMoved: {
				stellarium.setManualPosition(newLatitude, newLongitude)
			}
		}
	}

	function latitudeRepr(latitude) {
		var sign = latitude >= 0
		latitude = Math.abs(latitude)
		var degree = Math.floor(latitude)
		latitude = (latitude - degree) * 60
		var minutes = Math.floor(latitude)
		latitude = (latitude - minutes) * 60
		var seconds = Math.floor(latitude)
		return "%1° %2' %3\" %4".arg(degree).arg(minutes).arg(seconds).arg(sign ? "N" : "S")
	}

	function longitudeRepr(longitude) {
		var sign = longitude >= 0
		longitude = Math.abs(longitude)
		var degree = Math.floor(longitude)
		longitude = (longitude - degree) * 60
		var minutes = Math.floor(longitude)
		longitude = (longitude - minutes) * 60
		var seconds = Math.floor(longitude)
		return "%1° %2' %3\" %4".arg(degree).arg(minutes).arg(seconds).arg(sign ? "E" : "W")
	}
	
}
