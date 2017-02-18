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

StelSpinBox {
	id: root
	property string type // year | month | day | hour | minute | second
	property double time
	property double newTime // Se use it to avoid nasty loops with stellarium time!
	text: type
	min: -10000
	max: 10000
	valueText: (type !== "year" && value < 10) ? "0%1".arg(value) : "%1".arg(value)

	onTimeChanged: setValue()

	function setValue() {
		var date = jdToDate(time)
		if (type == "year")
			value = date.getFullYear()
		if (type == "month")
			value = date.getMonth() + 1
		if (type == "day")
			value = date.getDate()
		if (type == "hour")
			value = date.getHours()
		if (type == "minute")
			value = date.getMinutes()
		if (type == "second")
			value = date.getSeconds()
	}

	onValueChanged: {
		var date = jdToDate(root.time)
		if (type == "year")
			date.setFullYear(value)
		if (type == "month")
			date.setMonth(value - 1)
		if (type == "day")
			date.setDate(value)
		if (type == "hour")
			date.setHours(value)
		if (type == "minute")
			date.setMinutes(value)
		if (type == "second")
			date.setSeconds(value)
		newTime = dateToJd(date)
	}
}
