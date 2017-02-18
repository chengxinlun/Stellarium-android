/*
 * Stellarium
 * Copyright (C) 2015 Guillaume Chereau
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

#import <CoreLocation/CoreLocation.h>
#import <CoreMotion/CoreMotion.h>

#include "SensorsMgr.hpp"
#include "StelApp.hpp"
#include "StelTranslator.hpp"
#include "StelCore.hpp"
#include "StelModuleMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelUtils.hpp"

#include <QGuiApplication>
#include <QScreen>

static CMMotionManager *manager = NULL;

SensorsMgr::SensorsMgr() :
	enabled(false),
	accelerometerSensor(NULL),
	magnetometerSensor(NULL),
	sensorX(0), sensorY(0), sensorZ(0),
	magnetX(0), magnetY(0), magnetZ(0),
	sensorAzimuth(0),
	firstMeasure(true)
{
	setObjectName("SensorsMgr");
}

SensorsMgr::~SensorsMgr()
{
}

void SensorsMgr::init()
{
	addAction("actionSensorsControl", N_("Movement and Selection"), N_("Sensors"), "enabled");
}

void SensorsMgr::setEnabled(bool value)
{
	if (value == enabled)
		return;
	enabled = value;
	firstMeasure = true;
	if (!enabled)
	{
		Vec3d up(0,0,1);
		StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
		mmgr->setViewUpVectorJ2000(StelApp::getInstance().getCore()->altAzToJ2000(up));
		StelApp::getInstance().getCore()->setDefautAngleForGravityText(0);
	}

	if (enabled)
	{
		Q_ASSERT(manager == NULL);
		manager = [[CMMotionManager alloc] init];
		manager.deviceMotionUpdateInterval = 1 / 60.;
		manager.showsDeviceMovementDisplay = YES;
		[manager startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXTrueNorthZVertical];
	}
	else
	{
		[manager stopDeviceMotionUpdates];
		[manager release];
		manager = nil;
	}
	emit enabledChanged(enabled);
}

void SensorsMgr::applyOrientation(float* x, float* y, float* z)
{
	Q_UNUSED(z);
	const float xx = *x, yy = *y;
	QScreen* screen = qApp->primaryScreen();
	switch (screen->orientation())
	{
	case Qt::PortraitOrientation:
		break;
	case Qt::LandscapeOrientation:
		*x = -yy;
		*y = xx;
		break;
	case Qt::InvertedPortraitOrientation:
		*x = -xx;
		*y = -yy;
		break;
	case Qt::InvertedLandscapeOrientation:
		*x = yy;
		*y = -xx;
		break;
	default:
		break;
	}
}

void SensorsMgr::update(double deltaTime)
{
	Q_UNUSED(deltaTime);
	if (!enabled)
		return;
	Q_ASSERT(manager);
	CMDeviceMotion *motion = manager.deviceMotion;
	if (motion == nil)
		return;

	float fov=qMin(130.f, StelApp::getInstance().getCore()->getProjection(StelCore::FrameJ2000)->getFov());
	fov = qMax(fov, 5.f);
	float averagingCoef = (firstMeasure)? 1 : 0.05f+(fov-5.f)*0.15f/125.f;
	firstMeasure = false;
	
	sensorX += (motion.gravity.x - sensorX) * averagingCoef;
	sensorY += (motion.gravity.y - sensorY) * averagingCoef;
	sensorZ += (motion.gravity.z - sensorZ) * averagingCoef;

	float x = sensorX;
	float y = sensorY;
	float z = sensorZ;
	applyOrientation(&x, &y, &z);

	float roll = atan2(x, -y);
	float pitch = std::asin(z);

	StelApp::getInstance().getCore()->setDefautAngleForGravityText(roll*180./M_PI);
	StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
	Vec3d viewDirection = StelApp::getInstance().getCore()->j2000ToAltAz(mmgr->getViewDirectionJ2000());

	float lng, lat;
	StelUtils::rectToSphe(&lng, &lat, viewDirection);
	StelUtils::spheToRect(lng, pitch, viewDirection);
	mmgr->setViewDirectionJ2000(StelApp::getInstance().getCore()->altAzToJ2000(viewDirection));

	Vec3f viewHoriz;
	StelUtils::spheToRect(lng, 0, viewHoriz);
	Mat4f rot=Mat4f::rotation(viewHoriz, roll);
	Vec3f up(0,0,1);
	up.transfo4d(rot);
	Vec3d tmp(up[0],up[1],up[2]);
	mmgr->setViewUpVectorJ2000(StelApp::getInstance().getCore()->altAzToJ2000(tmp));
	
	CMRotationMatrix m = motion.attitude.rotationMatrix;
	Mat4d mat(m.m11, m.m12, m.m13, 0,
			  m.m21, m.m22, m.m23, 0,
			  m.m31, m.m32, m.m33, 0,
			  0, 0, 0, 1);
	Vec3d newViewDirection(0, 0, -1);
	newViewDirection.transfo4d(mat);
	newViewDirection.normalize();
	float az, dumb;
	StelUtils::rectToSphe(&dumb, &lat, viewDirection);
	StelUtils::rectToSphe(&az, &dumb, newViewDirection);
	az = -az * 180.f/M_PI;
	if (fabs(az - sensorAzimuth) > 180)
			sensorAzimuth += (sensorAzimuth < az)? 360 : -360;
	sensorAzimuth += (az-sensorAzimuth)*averagingCoef;
	lng = -sensorAzimuth*M_PI/180 + M_PI;
	StelUtils::spheToRect(lng, lat, viewDirection);
	mmgr->setViewDirectionJ2000(StelApp::getInstance().getCore()->altAzToJ2000(viewDirection));
}
