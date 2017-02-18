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

#include "StelModule.hpp"

class SensorsMgr : public StelModule
{
	Q_OBJECT
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
public:
	SensorsMgr();
	virtual ~SensorsMgr();
	virtual void init() Q_DECL_OVERRIDE;
	virtual void update(double deltaTime) Q_DECL_OVERRIDE;
	bool isEnabled() const {return enabled;}
	void setEnabled(bool value);
signals:
	void enabledChanged(bool);
private:
	void applyOrientation(float* x, float *y, float* z);
	bool enabled;
	class QAccelerometer* accelerometerSensor;
	class QMagnetometer* magnetometerSensor;
	qreal sensorX, sensorY, sensorZ;
	qreal magnetX, magnetY, magnetZ;
	qreal sensorAzimuth;
	bool firstMeasure;
};
