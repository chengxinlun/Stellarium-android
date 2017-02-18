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
	title: qsTr("Advanced")
	width: rootStyle.niceMenuWidth

	// XXX: I guess we could move the flickable + column into a custom item.
	Flickable {
		width: root.width
		height: Math.min(column.height, rootStyle.maxMenuHeight)
		clip: true
		contentWidth: column.width
		contentHeight: column.height
		flickableDirection: Flickable.VerticalFlick

		Column {
			id: column
			width: root.width
			height: childrenRect.height

			StelListItem {
				checkbox: true
				action: "actionShow_Planets_Hints"
				setting: "astro/flag_planets_hints"
				text: qsTr("Show planet markers")
			}

			StelListItem {
				checkbox: true
				action: "actionShow_Ecliptic_Line"
				setting: "viewing/flag_ecliptic_line"
				text: qsTr("Show ecliptic line")
			}

			StelListItem {
				checkbox: true
				action: "actionShow_Meridian_Line"
				setting: "viewing/flag_meridian_line"
				text: qsTr("Show meridian line")
			}

			StelListItem {
				checkbox: true
				action: "actionShow_Constellation_Boundaries"
				setting: "viewing/flag_constellation_boundaries"
				text: qsTr("Constellation boundaries")
			}

			StelListItem {
				text: qsTr("Light pollution")
				rightText: stellarium.lightPollution
				withArrow: true
				onClicked: {
					var picker = root.openPage("ValuePicker.qml")
					picker.title = qsTr("Light pollution")
					picker.min = 1
					picker.max = 9
					picker.value = stellarium.lightPollution
					function onValueChanged() {
						stellarium.lightPollution = picker.value
					}
					picker.valueChanged.connect(onValueChanged)
				}
			}

			StelListItem {
				text: qsTr("Milky Way brightness")
				rightText: stellarium.milkyWayBrightness
				withArrow: true
				onClicked: {
					var picker = root.openPage("ValuePicker.qml")
					picker.title = qsTr("Milky Way brightness")
					picker.min = 0
					picker.max = 10
					picker.value = stellarium.milkyWayBrightness
					function onValueChanged() {
						stellarium.milkyWayBrightness = picker.value
					}
					picker.valueChanged.connect(onValueChanged)
				}
			}

			StelListItem {
				text: qsTr("Thickness of lines")
				rightText: stellarium.linesThickness
				withArrow: true
				onClicked: {
					var picker = root.openPage("ValuePicker.qml")
					picker.title = qsTr("Thickness of lines")
					picker.min = 0
					picker.max = 8
					picker.value = stellarium.linesThickness
					function onValueChanged() {
						stellarium.linesThickness = picker.value
					}
					picker.valueChanged.connect(onValueChanged)
				}
			}
			StelButton {
				text: qsTr("Restore default settings")
				anchors.margins: rootStyle.margin
				onClicked: stellarium.resetSettings()
			}
		}
	}
}
