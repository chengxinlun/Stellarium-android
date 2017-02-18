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
	title: qsTr("Date and Time")
	width: rootStyle.niceMenuWidth
	height: childrenRect.height

	Column {
		width: parent.width
		height: childrenRect.height
		StelListItem {
			text: qsTr("Advance to night at startup");
			action: "actionAuto_Goto_Night"
			checkbox: true
			setting: "gui/auto_goto_night"
		}

		StelButton {
			text: qsTr("Set time to now")
			action: "actionReturn_To_Current_Time"
			anchors.margins: rootStyle.margin
		}

		Grid {
			id: time
			columns: 3
			width: parent.width
			height: childrenRect.height
			Repeater {
				model: ["year", "month", "day", "hour", "minute", "second"]
				Item {
					width: parent.width / 3
					height: childrenRect.height
					StelTimeSpinBox {
						id: spinBox
						large: true
						anchors.top: text.bottom
						width: parent.width
						type: modelData;
						time: stellarium.jd;
						onValueManuallyChanged: {
							stellarium.setJd(newTime)
						}
					}
					Text {
						anchors.horizontalCenter: spinBox.right
						anchors.verticalCenter: spinBox.verticalCenter
						color: "white"
						font.pixelSize: rootStyle.fontExtraLargeSize
						font.bold: true
						text: index >= 3 ? ":" : "-"
						visible: (index % 3) !== 2
					}
				}
			}
		}
	}
}
