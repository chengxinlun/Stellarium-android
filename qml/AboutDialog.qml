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
	title: qsTr("About")
	width: rootStyle.niceMenuWidth
	withMargins: true

	Column {
		id: column
		spacing: rootStyle.margin
		width: parent.width
		height: childrenRect.height

		Text {
			color: "white"
			font.pixelSize: rootStyle.fontSmallSize
			text: "<h2>Stellarium Mobile v" + stellarium.version + "<br></h2>Copyrights (C) 2014<br>Fabien &amp; Guillaume Chéreau<br>GNU GPL"
			width: parent.width
			horizontalAlignment: Text.AlignHCenter
			wrapMode: Text.Wrap
		}

		Image {
			source: "images/about-logo.png"
			width: parent.width * 0.6
			anchors.horizontalCenter: parent.horizontalCenter
			fillMode: Image.PreserveAspectFit
			smooth: true
		}
		
		Text {
			width: parent.width
			horizontalAlignment: Text.AlignHCenter
			color: "#2166cd"
			font.pixelSize: rootStyle.fontSmallSize
			text: "http://stellarium-mobile.org"
			MouseArea {
				anchors.fill: parent
				onClicked: {
					Qt.openUrlExternally("http://stellarium-mobile.org")
				}
			}
		}
		
		Text {
			color: "white"
			font.pixelSize: rootStyle.fontSmallSize
			text: "Stellarium: Copyrights (C) 2000-2014\nFabien Chéreau et al.\nGNU GPL\n\nwww.stellarium.org"
			width: parent.width
			horizontalAlignment: Text.AlignHCenter
			wrapMode: Text.Wrap
		}
	}
}
