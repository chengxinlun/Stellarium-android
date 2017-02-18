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
	title: qsTr("Starlore")
	width: rootStyle.niceMenuWidth

	property string current: stellarium.currentSkyCultureI18

	ListView {
		id: list
		width: 140*rootStyle.scale
		height: 400*rootStyle.scale
		delegate: StelListItem {
			text: modelData
			selected: root.current == modelData
			onClicked: stellarium.currentSkyCultureI18 = modelData
		}
		model: stellarium.getSkyCultureListI18()
		clip: true
	}

	Flickable {
		anchors {
			left: list.right
			top: parent.top
			right: parent.right
			bottom: parent.bottom
			margins: rootStyle.margin
		}
		clip: true
		contentWidth: width
		contentHeight: descriptionText.height
		flickableDirection: Flickable.VerticalFlick

		Text {
			y: -20*rootStyle.scale // compensate the <h2> top margin
			id: descriptionText
			width: parent.width
			textFormat: Text.StyledText
			linkColor: "#2166cd"
			onLinkActivated: Qt.openUrlExternally(link)
			text: stellarium.currentSkyCultureHtmlDescription
			wrapMode: Text.Wrap
			color: "white"
			font.pixelSize: rootStyle.fontSmallSize
			baseUrl: (Qt.platform.os === "android" ? "" : "file://") +stellarium.currentSkyCultureBaseUrl+'/'
		}
	}
}
