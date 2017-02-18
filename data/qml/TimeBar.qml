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

Column {
    width: (48*rootStyle.scale)*4+rootStyle.margin

	Text {
        font.pixelSize: rootStyle.fontNormalSize
		color: "white"
        anchors.left: parent.left
        anchors.leftMargin: rootStyle.margin
		text: {
			var JD_SECOND = 0.000011574074074074074074
			var factor = stellarium.timeRate / JD_SECOND;
			if (factor === 1)
				return qsTr("Real time")
			if (factor === 0)
				return qsTr("Stopped")
			return "x" + factor.toFixed(0)
		}
	}

	Row {
		anchors.horizontalCenter: parent.horizontalCenter
		ImageButton {
			source: "images/timeRewind.png"
			action: "actionDecrease_Time_Speed"
		}

		ImageButton {
			source: "images/timeReal.png"
			action: "actionSet_Real_Time_Speed"
		}

		ImageButton {
			source: "images/timeForward.png"
			action: "actionIncrease_Time_Speed"
		}

		ImageButton {
			source: "images/timeNow.png"
			action: "actionReturn_To_Current_Time"
		}
	}
}
