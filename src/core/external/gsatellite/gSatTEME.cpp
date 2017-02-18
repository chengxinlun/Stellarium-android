/***************************************************************************
 * Name: gSatTEME.cpp
 *
 * Description: gSatTEME class implementation.
 *              This class abstract all the SGP4 complexity. It uses the
 *              David. A. Vallado code for SGP4 Calculation.
 *
 * Reference:
 *              Revisiting Spacetrack Report #3 AIAA 2006-6753
 *              Vallado, David A., Paul Crawford, Richard Hujsak, and T.S.
 *              Kelso, "Revisiting Spacetrack Report #3,"
 *              presented at the AIAA/AAS Astrodynamics Specialist
 *              Conference, Keystone, CO, 2006 August 21–24.
 *              http://celestrak.com/publications/AIAA/2006-6753/
 ***************************************************************************/

/***************************************************************************
 *   Copyright (C) 2004 by J.L. Canales                                    *
 *   ph03696@homeserver                                                    *
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

// GKepFile
#include "gSatTEME.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

#include "stdsat.h"

#include "sgp4io.h"

#define CONSTANTS_SET wgs72
#define TYPERUN_SET   'c'
#define OPSMODE_SET   'i'
#define TYPEINPUT_SET 'm'

// Constructors
gSatTEME::gSatTEME(const char *pstrName, char *pstrTleLine1, char *pstrTleLine2)
{
	double startmfe, stopmfe, deltamin;
	double ro[3] = {NAN, NAN, NAN};
	double vo[3] = {NAN, NAN, NAN};

	m_Position.resize(3);
	m_Vel.resize(3);
	m_SubPoint.resize(3);

	m_SatName = pstrName;

	//set gravitational constants
	getgravconst(CONSTANTS_SET, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2);

	//Parsing TLE_Files and sat variables setting
	twoline2rv(pstrTleLine1, pstrTleLine2, TYPERUN_SET, TYPEINPUT_SET, OPSMODE_SET, CONSTANTS_SET,
	           startmfe, stopmfe, deltamin, satrec);

	// call the propagator to get the initial state vector value
	sgp4(CONSTANTS_SET, satrec,  0.0, ro,  vo);

	m_Position[ 0]= ro[ 0];
	m_Position[ 1]= ro[ 1];
	m_Position[ 2]= ro[ 2];
	m_Vel[ 0]     = vo[ 0];
	m_Vel[ 1]     = vo[ 1];
	m_Vel[ 2]     = vo[ 2];
}

void gSatTEME::setEpoch(gTime ai_time)
{
	gTime     kepEpoch(satrec.jdsatepoch);
	gTimeSpan tSince = ai_time - kepEpoch;

	double ro[3] = {NAN, NAN, NAN};
	double vo[3] = {NAN, NAN, NAN};
	double dtsince = tSince.getDblSeconds()/KSEC_PER_MIN;
	// call the propagator to get the initial state vector value
	sgp4(CONSTANTS_SET, satrec,  dtsince, ro,  vo);

	m_Position[ 0]= ro[ 0];
	m_Position[ 1]= ro[ 1];
	m_Position[ 2]= ro[ 2];
	m_Vel[ 0]     = vo[ 0];
	m_Vel[ 1]     = vo[ 1];
	m_Vel[ 2]     = vo[ 2];
	computeSubPoint(ai_time, &m_SubPoint);
}

void gSatTEME::setMinSinceKepEpoch(double ai_minSinceKepEpoch)
{
	double ro[3] = {NAN, NAN, NAN};
	double vo[3] = {NAN, NAN, NAN};
	gTimeSpan tSince( ai_minSinceKepEpoch/KMIN_PER_DAY);
	gTime     Epoch(satrec.jdsatepoch);
	Epoch += tSince;
	// call the propagator to get the initial state vector value
	sgp4(CONSTANTS_SET, satrec,  ai_minSinceKepEpoch, ro,  vo);

	m_Position[ 0]= ro[ 0];
	m_Position[ 1]= ro[ 1];
	m_Position[ 2]= ro[ 2];
	m_Vel[ 0]     = vo[ 0];
	m_Vel[ 1]     = vo[ 1];
	m_Vel[ 2]     = vo[ 2];
	computeSubPoint(Epoch, &m_SubPoint);
}

void gSatTEME::computeSubPoint(gTime ai_Time, gVector* res)
{
	float theta = std::atan2(m_Position[1], m_Position[0]); // radians
	float lon = fmod((theta - ai_Time.toThetaGMST()), K2PI);  //radians

	float r = std::sqrt(m_Position[0]*m_Position[0] + m_Position[1]*m_Position[1]);
	static const float e2 = __f*(2 - __f);
	float lat = std::atan2(m_Position[2],r); /*radians*/

	float phi, c;
	do
	{
		phi = lat;
		const float sinphi = std::sin(phi);
		c = 1/std::sqrt(1.f - e2*sinphi*sinphi);
		lat = std::atan2(m_Position[2] + KEARTHRADIUS*c*e2*sinphi,r);
	}
	while(std::fabs(lat - phi) >= 1E-4);

	float alt = r/std::cos(lat) - KEARTHRADIUS*c;/*kilometers*/

	if(lat > KPI/2.f) lat -= K2PI;

	lat  = lat/KDEG2RAD;
	lon = lon/KDEG2RAD;
	if(lon < -180.f) lon += 360.f;
	else if(lon > 180.f) lon -= 360;

	// (0) Latitude, (1) Longitude, (2) altitude
	(*res)[0] = lat;
	(*res)[1] = lon;
	(*res)[2] = alt;
}
