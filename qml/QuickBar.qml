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

Grid {
	id: buttonGrid
	width: childrenRect.width
	height: childrenRect.height
	spacing: 5 * rootStyle.scale
	property real parentWidth: parent.width

	// Compute the optimal number of columns so that the buttons are evently distributed.
	onParentWidthChanged: {
		function sizeForColumns(n) { return 48 * rootStyle.scale * n + buttonGrid.spacing * ( n - 1); }
		var choices = [12, 6];
		for (var i = 0; i < choices.length; i++)
		if (parentWidth >= sizeForColumns(choices[i])) {
			columns = choices[i];
			return;
		}
		// default
		columns = 6;
	}

	ImageButton {
		source: "images/constellationLines.png"
		action: "actionShow_Constellation_Lines"
		setting: "viewing/flag_constellation_drawing"
	}

	ImageButton {
		source: "images/constellationLabels.png"
		action: "actionShow_Constellation_Labels"
		setting: "viewing/flag_constellation_name"
	}

	ImageButton {
		source: "images/starlore.png"
		action: "actionShow_Constellation_Art"
		setting: "viewing/flag_constellation_art"
	}

	ImageButton {
		source: "images/equatorialGrid.png"
		action: "actionShow_Equatorial_Grid"
		setting: "viewing/flag_equatorial_grid"
	}

	ImageButton {
		source: "images/azimuthalGrid.png"
		action: "actionShow_Azimuthal_Grid"
		setting: "viewing/flag_azimuthal_grid"
	}

	ImageButton {
		source: "images/landscape.png"
		action: "actionShow_Ground"
		setting: "landscape/flag_landscape"
	}

	ImageButton {
		source: "images/atmosphere.png"
		action: "actionShow_Atmosphere"
		setting: "landscape/flag_atmosphere"
	}

	ImageButton {
		source: "images/location.png"
		action: "actionShow_Cardinal_Points"
		setting: "viewing/flag_cardinal_points"
	}

	ImageButton {
		source: "images/nebulas.png"
		action: "actionShow_Nebulas"
		setting: "astro/flag_nebula_name"
	}

	ImageButton {
		source: "images/satellites.png"
		action: "actionShow_Satellite_hints"
		setting: "Satellites/hints_visible"
	}

	ImageButton {
		source: "images/sensors.png"
		action: "actionSensorsControl"
	}

	ImageButton {
		source: "images/nightView.png"
		action: "actionNight_Mode"
		setting: "viewing/flag_night"
	}
}
