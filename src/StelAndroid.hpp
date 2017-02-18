/*
 * Stellarium
 * Copyright (C) 2013 Guillaume Chereau
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

#include <QString>
#include <QFileInfo>

class StelAndroid
{
public:
	static float getScreenDensity();
	static bool GPSSupported();
	static void setGPSCallback(void (*callback)(double, double, double, double));
	static void (*GPSCallback)(double, double, double, double);
	static int getOrientation();
	static void setCanPause(bool value);
	static QString getModel();
private:
	static class QAndroidJniObject* getStellarium();
};
