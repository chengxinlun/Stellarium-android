/***************************************************************************
 * Name: gSatWrapper.hpp
 *
 * Description: Wrapper over gSatTEME class.
 *              This class allow use Satellite orbit calculation module (gSAt) in
 *              Stellarium 'native' mode using Stellarium objects.
 *
 ***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2006 by J.L. Canales                                    *
 *   jlcanales.gasco@gmail.com                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.             *
 ***************************************************************************/



#include "gsatellite/stdsat.h"

#include "gSatWrapper.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelUtils.hpp"

#include "SolarSystem.hpp"
#include "StelModuleMgr.hpp"

#include <QDebug>
#include <QByteArray>


gSatWrapper::gSatWrapper(QString designation, QString tle1, QString tle2)
{
	// The TLE library actually modifies the TLE strings, which is annoying (because
	// when we get updates, we want to check if there has been a change by using ==
	// with the original.  Thus we make a copy to send to the TLE library.
	QByteArray t1(tle1.toLatin1().data()), t2(tle2.toLatin1().data());

	// Also, the TLE library expects no more than 130 characters length input.  We
	// shouldn't have sane input with a TLE longer than about 80, but just in case
	// we have a mal-formed input, we will truncate here to be safe
	t1.truncate(130);
	t2.truncate(130);

	pSatellite = new gSatTEME(designation.toLatin1().data(),
	                          t1.data(),
	                          t2.data());
	updateEpoch();
}


gSatWrapper::~gSatWrapper()
{
	if (pSatellite != NULL)
		delete pSatellite;
}


Vec3f gSatWrapper::getTEMEPos()
{
	Vec3f returnedVector;
	if (pSatellite != NULL)
	{
		const gVector& position = pSatellite->getPos();
		returnedVector.set(position[0], position[1], position[2]);
	}
	else
		qWarning() << "gSatWrapper::getTEMEPos Method called without pSatellite initialized";

	return returnedVector;
}


Vec3f gSatWrapper::getTEMEVel()
{
	Vec3f returnedVector;
	if (pSatellite != NULL)
	{
		gVector velocity;
		velocity = pSatellite->getVel();
		returnedVector.set(velocity[0], velocity[1], velocity[2]);
	}
	else
		qWarning() << "gSatWrapper::getTEMEVel Method called without pSatellite initialized";

	return returnedVector;
}


Vec3f gSatWrapper::getSubPoint()
{

	Vec3f returnedVector;
	if (pSatellite != NULL)
	{
		gVector satelliteSubPoint;
		satelliteSubPoint = pSatellite->getSubPoint();
		returnedVector.set(satelliteSubPoint[0], satelliteSubPoint[1], satelliteSubPoint[2]);
	}
	else
		qWarning() << "gSatWrapper::getTEMEVel Method called without pSatellite initialized";

	return returnedVector;
}


void gSatWrapper::updateEpoch()
{
	double jul_utc = StelApp::getInstance().getCore()->getJDay();
        epoch = jul_utc;

	if (pSatellite)
                pSatellite->setEpoch(epoch);
}

void gSatWrapper::setEpoch(double ai_julianDaysEpoch)
{
    epoch = ai_julianDaysEpoch;
    if (pSatellite)
		pSatellite->setEpoch(ai_julianDaysEpoch);
}


void gSatWrapper::calcObserverECIPosition(Vec3f& ao_position, Vec3f& ao_velocity)
{
	const StelLocation& loc   = StelApp::getInstance().getCore()->getCurrentLocation();

	float radLatitude = loc.latitude * KDEG2RAD;
	float theta       = epoch.toThetaLMST(loc.longitude * KDEG2RAD);

	/* Reference:  Explanatory supplement to the Astronomical Almanac, page 209-210. */
	/* Elipsoid earth model*/
	/* c = Nlat/a */
	const float sinradLatitude = std::sin(radLatitude);
	const float c = 1.f/std::sqrt(1.f + __f*(__f - 2.f)*sinradLatitude*sinradLatitude);
	const float sq = (1 - __f)*(1 - __f)*c;

	float r = (KEARTHRADIUS*c + loc.altitude/1000)*std::cos(radLatitude);
	ao_position[0] = r * std::cos(theta);/*kilometers*/
	ao_position[1] = r * std::sin(theta);
	ao_position[2] = (KEARTHRADIUS*sq + loc.altitude/1000)*sinradLatitude;
	ao_velocity[0] = -KMFACTOR*ao_position[1];/*kilometers/second*/
	ao_velocity[1] =  KMFACTOR*ao_position[0];
	ao_velocity[2] =  0;
}



Vec3f gSatWrapper::getAltAz()
{
	const StelLocation& loc   = StelApp::getInstance().getCore()->getCurrentLocation();
	Vec3f topoSatPos;
	Vec3f observerECIPos;
	Vec3f observerECIVel;
	calcObserverECIPosition(observerECIPos, observerECIVel);

	const Vec3f& satECIPos = getTEMEPos();
	Vec3f slantRange = satECIPos - observerECIPos;

	const float  radLatitude    = loc.latitude * KDEG2RAD;
	const float  theta          = epoch.toThetaLMST(loc.longitude * KDEG2RAD);

	const float sinradLatitude = std::sin(radLatitude);
	const float cosradLatitude = std::cos(radLatitude);
	const float sintheta = std::sin(theta);
	const float costheta = std::cos(theta);

	//top_s
	topoSatPos[0] = sinradLatitude * costheta*slantRange[0]
					 + sinradLatitude* sintheta*slantRange[1]
					 - cosradLatitude* slantRange[2];
	//top_e
	topoSatPos[1] = -1.0* sintheta*slantRange[0]
					 + costheta*slantRange[1];

	//top_z
	topoSatPos[2] = cosradLatitude * costheta*slantRange[0]
					 + cosradLatitude * sintheta*slantRange[1]
					 + sinradLatitude *slantRange[2];

	return topoSatPos;
}

void  gSatWrapper::getSlantRange(double &ao_slantRange, double &ao_slantRangeRate)
{

	Vec3f observerECIPos;
	Vec3f observerECIVel;

	calcObserverECIPosition(observerECIPos, observerECIVel);

	Vec3f satECIPos            = getTEMEPos();
	Vec3f satECIVel            = getTEMEVel();
	Vec3f slantRange           = satECIPos - observerECIPos;
	Vec3f slantRangeVelocity   = satECIVel - observerECIVel;

	ao_slantRange     = slantRange.length();
	ao_slantRangeRate = slantRange.dot(slantRangeVelocity)/ao_slantRange;
}

Vec3f gSatWrapper::getSunECIPos()
{
	// All positions in ECI system are positions referenced in a StelCore::EquinoxEq system centered in the earth centre
	Vec3f observerECIPos;
	Vec3f observerECIVel;
	calcObserverECIPosition(observerECIPos, observerECIVel);

	SolarSystem *solsystem = (SolarSystem*)StelApp::getInstance().getModuleMgr().getModule("SolarSystem");
	Vec3d sunEquinoxEqPos        = solsystem->getSun()->getEquinoxEquatorialPos(StelApp::getInstance().getCore());

	Vec3f sunECIPos(sunEquinoxEqPos[0]*AU, sunEquinoxEqPos[1]*AU, sunEquinoxEqPos[2]*AU);
	//sunEquinoxEqPos is measured in AU. we need meassure it in Km
	sunECIPos += observerECIPos; //Change ref system centre

	return sunECIPos;
}

// Operation getVisibilityPredict
// @brief This operation predicts the satellite visibility contidions.
int gSatWrapper::getVisibilityPredict()
{
	float sunSatAngle, Dist;
	int   visibility;

	const Vec3f satAltAzPos = getAltAz();
	if (satAltAzPos[2] > 0)
	{
		const Vec3f satECIPos = getTEMEPos();
		SolarSystem *solsystem = (SolarSystem*)StelApp::getInstance().getModuleMgr().getModule("SolarSystem");		
		Vec3d sunAltAzPos        = solsystem->getSun()->getAltAzPosGeometric(StelApp::getInstance().getCore());

		const Vec3f sunECIPos = getSunECIPos();

		if (sunAltAzPos[2] > 0.0)
		{
			visibility = RADAR_SUN;
		}
		else
		{
			sunSatAngle = sunECIPos.angle(satECIPos);
			Dist = satECIPos.length()*std::cos(sunSatAngle - (M_PI/2));

			if (Dist > KEARTHRADIUS)
			{
				visibility = VISIBLE;
			}
			else
			{
				visibility = RADAR_NIGHT;
			}
		}
	}
	else
		visibility = NOT_VISIBLE;

	return visibility; //TODO: put correct return
}

float gSatWrapper::getPhaseAngle()
{
	Vec3f sunECIPos = getSunECIPos();
	return sunECIPos.angle(getTEMEPos());
}




