/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2010 Bogdan Marinov
 * Copyright (C) 2011 Alexander Wolf
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

#include "SolarSystem.hpp"
#include "StelTexture.hpp"
#include "stellplanet.h"
#include "Orbit.hpp"

#include "StelProjector.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelTextureMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelIniParser.hpp"
#include "Planet.hpp"
#include "MinorPlanet.hpp"
#include "Comet.hpp"

#include "StelSkyDrawer.hpp"
#include "StelUtils.hpp"
#include "StelPainter.hpp"
#include "TrailGroup.hpp"
#include "RefractionExtinction.hpp"

#include <functional>
#include <algorithm>

#include <QTextStream>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QMultiMap>
#include <QMapIterator>
#include <QDebug>
#include <QDir>

SolarSystem::SolarSystem()
	: moonScale(1.),
	  flagOrbits(false),
	  flagLightTravelTime(false),
	  allTrails(NULL)
{
	planetNameFont.setPixelSize(StelApp::getInstance().getSettings()->value("gui/base_font_size", 13).toInt());
	setObjectName("SolarSystem");
}

void SolarSystem::setFontSize(float newFontSize)
{
	planetNameFont.setPixelSize(newFontSize);
}

SolarSystem::~SolarSystem()
{
	// release selected:
	selected.clear();
	foreach (Orbit* orb, orbits)
	{
		delete orb;
		orb = NULL;
	}
	sun.clear();
	moon.clear();
	earth.clear();
	Planet::hintCircleTex.clear();
	Planet::texEarthShadow.clear();

	delete allTrails;
	allTrails = NULL;

	// Get rid of circular reference between the shared pointers which prevent proper destruction of the Planet objects.
	foreach (PlanetP p, systemPlanets)
	{
		p->satellites.clear();
	}
}

/*************************************************************************
 Reimplementation of the getCallOrder method
*************************************************************************/
double SolarSystem::getCallOrder(StelModuleActionName actionName) const
{
	if (actionName==StelModule::ActionDraw)
		return StelApp::getInstance().getModuleMgr().getModule("StarMgr")->getCallOrder(actionName)+10;
	return 0;
}

// Init and load the solar system data
void SolarSystem::init()
{
	QSettings* conf = StelApp::getInstance().getSettings();
	Q_ASSERT(conf);

	loadPlanets();	// Load planets data

	// Compute position and matrix of sun and all the satellites (ie planets)
	// for the first initialization Q_ASSERT that center is sun center (only impacts on light speed correction)	
	computePositions(StelUtils::getJDFromSystem());

	setSelected("");	// Fix a bug on macosX! Thanks Fumio!
	setFlagMoonScale(conf->value("viewing/flag_moon_scaled", conf->value("viewing/flag_init_moon_scaled", "false").toBool()).toBool());  // name change
	setMoonScale(conf->value("viewing/moon_scale", 5.0).toFloat());
	setFlagPlanets(conf->value("astro/flag_planets").toBool());
	setFlagHints(conf->value("astro/flag_planets_hints").toBool());
	setFlagLabels(conf->value("astro/flag_planets_labels", true).toBool());
	setLabelsAmount(conf->value("astro/labels_amount", 3.).toFloat());
	setFlagOrbits(conf->value("astro/flag_planets_orbits").toBool());
	setFlagLightTravelTime(conf->value("astro/flag_light_travel_time", false).toBool());
	setFlagMarkers(conf->value("astro/flag_planets_markers", true).toBool());

	recreateTrails();

	setFlagTrails(conf->value("astro/flag_object_trails", false).toBool());

	StelObjectMgr *objectManager = GETSTELMODULE(StelObjectMgr);
	objectManager->registerStelObjectMgr(this);
	connect(objectManager, SIGNAL(selectedObjectChanged(StelModule::StelModuleSelectAction)),
			this, SLOT(selectedObjectChange(StelModule::StelModuleSelectAction)));

	texPointer = StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/pointeur4.png");
	Planet::hintCircleTex = StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/planet-indicator.png");

	StelApp *app = &StelApp::getInstance();
	connect(app, SIGNAL(languageChanged()), this, SLOT(updateI18n()));
	connect(app, SIGNAL(colorSchemeChanged(const QString&)), this, SLOT(setStelStyle(const QString&)));

	QString displayGroup = N_("Display Options");
	addAction("actionShow_Planets_Labels", displayGroup, N_("Planet labels"), "labelsDisplayed", "P");
	addAction("actionShow_Planets_Orbits", displayGroup, N_("Planet orbits"), "orbitsDisplayed", "O");
	addAction("actionShow_Planets_Trails", displayGroup, N_("Planet trails"), "trailsDisplayed", "Shift+T");
	addAction("actionShow_Planets_Hints", displayGroup, N_("Planet markers"), "hintsDisplayed");
}

void SolarSystem::recreateTrails()
{
	// Create a trail group containing all the planets orbiting the sun (not including satellites)
	if (allTrails!=NULL)
		delete allTrails;
	allTrails = new TrailGroup(365.f);
	foreach (const PlanetP& p, getSun()->satellites)
	{
		allTrails->addObject((QSharedPointer<StelObject>)p, &trailColor);
	}
}

void SolarSystem::drawPointer(const StelCore* core)
{
	const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);

	const QList<StelObjectP> newSelected = GETSTELMODULE(StelObjectMgr)->getSelectedObject("Planet");
	if (!newSelected.empty())
	{
		const StelObjectP obj = newSelected[0];
		Vec3d pos=obj->getJ2000EquatorialPos(core);

		Vec3d screenpos;
		// Compute 2D pos and return if outside screen
		if (!prj->project(pos, screenpos))
			return;


		StelPainter sPainter(prj);
		Vec3f color = getPointersColor();
		sPainter.setColor(color[0],color[1],color[2]);

		float size = obj->getAngularSize(core)*M_PI/180.*prj->getPixelPerRadAtCenter()*2.;
		
		const float scale = prj->getDevicePixelsPerPixel()*StelApp::getInstance().getGlobalScalingRatio();
		size+= scale * (45.f + 10.f*std::sin(2.f * StelApp::getInstance().getTotalRunTime()));

		texPointer->bind();

		sPainter.enableTexture2d(true);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

		size*=0.5;
		const float angleBase = StelApp::getInstance().getTotalRunTime() * 10;
		// We draw 4 instances of the sprite at the corners of the pointer
		for (int i = 0; i < 4; ++i)
		{
			const float angle = angleBase + i * 90;
			const double x = screenpos[0] + size * cos(angle / 180 * M_PI);
			const double y = screenpos[1] + size * sin(angle / 180 * M_PI);
			sPainter.drawSprite2dMode(x, y, 10, angle);
		}
	}
}

void ellipticalOrbitPosFunc(double jd,double xyz[3], void* userDataPtr)
{
	static_cast<EllipticalOrbit*>(userDataPtr)->positionAtTimevInVSOP87Coordinates(jd, xyz);
}
void cometOrbitPosFunc(double jd,double xyz[3], void* userDataPtr)
{
	static_cast<CometOrbit*>(userDataPtr)->positionAtTimevInVSOP87Coordinates(jd, xyz);
}

// Init and load the solar system data
void SolarSystem::loadPlanets()
{
	qDebug() << "Loading Solar System data (1: planets and moons) ...";
	QString solarSystemFile = StelFileMgr::findFile("data/ssystem_major.ini");
	if (solarSystemFile.isEmpty())
	{
		qWarning() << "ERROR while loading ssystem_major.ini (unable to find data/ssystem_major.ini): " << endl;
		return;
	}

	if (!loadPlanets(solarSystemFile))
	{
		qWarning() << "ERROR while loading ssystem_major.ini: " << endl;
		return;
	}

	qDebug() << "Loading Solar System data (2: minor bodies)...";
	QStringList solarSystemFiles = StelFileMgr::findFileInAllPaths("data/ssystem_minor.ini");
	if (solarSystemFiles.isEmpty())
	{
		qWarning() << "ERROR while loading ssystem_minor.ini (unable to find data/ssystem_minor.ini): " << endl;
		return;
	}
	foreach (const QString& solarSystemFile, solarSystemFiles)
	{
		if (loadPlanets(solarSystemFile))
		{
			qDebug() << "File ssystem_minor.ini is loaded successfully...";
			break;
		}
		else
		{
//			sun.clear();
//			moon.clear();
//			earth.clear();
			//qCritical() << "We should not be here!";

			qDebug() << "Removing minor bodies";
			foreach (PlanetP p, systemPlanets)
			{
				p->satellites.clear();
				p.clear();
			}
			systemPlanets.clear();
			//Memory leak? What's the proper way of cleaning shared pointers?

			// TODO: 0.16pre what about the orbits list?

			//If the file is in the user data directory, rename it:
			if (solarSystemFile.contains(StelFileMgr::getUserDir()))
			{
				QString newName = QString("%1/data/ssystem-%2.ini").arg(StelFileMgr::getUserDir()).arg(QDateTime::currentDateTime().toString("yyyyMMddThhmmss"));
				if (QFile::rename(solarSystemFile, newName))
					qWarning() << "Invalid Solar System file" << QDir::toNativeSeparators(solarSystemFile) << "has been renamed to" << QDir::toNativeSeparators(newName);
				else
				{
					qWarning() << "Invalid Solar System file" << QDir::toNativeSeparators(solarSystemFile) << "cannot be removed!";
					qWarning() << "Please either delete it, rename it or move it elsewhere.";
				}
			}
		}
	}

    // Modified: load 1000 comets file (Cheng Xinlun, Jun 23 2020)
    qDebug() << "Loading Solar System data (3: 1000 comets)";
    QStringList solarSystemCometFiles = StelFileMgr::findFileInAllPaths("data/ssystem_1000comets.ini");
    if (solarSystemCometFiles.isEmpty())
    {
        qWarning() << "ERROR while loading ssystem_1000comets.ini (unable to find data/ssystem_1000comets.ini):" << Qt::endl;
        return;
    }
    foreach (const QString& solarSystemFile, solarSystemCometFiles)
    {
        if (loadPlanets(solarSystemFile))
        {
            qDebug() << "File ssystem_1000comets.ini is loaded successfully...";
            break;
        }
        else
        {
            qDebug() << "Removing comets";
            foreach (PlanetP p, systemPlanets)
            {
                p->satellites.clear();
                p.clear();
            }
            systemPlanets.clear();
            if (solarSystemFile.contains(StelFileMgr::getUserDir()))
            {
                QString newName = QString("%1/data/ssystem-%2.ini").arg(StelFileMgr::getUserDir()).arg(QDateTime::currentDateTime().toString("yyyyMMddThhmmss"));
                if (QFile::rename(solarSystemFile, newName))
                    qWarning() << "Invalid Solar System file" << QDir::toNativeSeparators(solarSystemFile) << "has been renamed to" << QDir::toNativeSeparators(newName);
                else
                {
                    qWarning() << "Invalid Solar System file" << QDir::toNativeSeparators(solarSystemFile) << "cannot be removed!";
                    qWarning() << "Please either delete it, rename it or move it elsewhere.";
                }
            }
        }
    }

    // Modified: load user solar system object file (Cheng Xinlun, Jun 23 2020)
    qDebug() << "Loading Solar System data (4: user-defined solar system objects)";
    QStringList solarSystemUserFiles = StelFileMgr::findFileInAllPaths("data/ssystem_user.ini", StelFileMgr::Flags(StelFileMgr::Writable|StelFileMgr::File));
    if (solarSystemUserFiles.isEmpty())
    {
        qWarning() << "ERROR while loading ssystem_user.ini (unable to find data/ssystem_user.ini):" << Qt::endl;
        return;
    }
    foreach (const QString& solarSystemFile, solarSystemUserFiles)
    {
        if (loadPlanets(solarSystemFile))
        {
            qDebug() << "File ssystem_user.ini is loaded successfully...";
            break;
        }
        else
        {
            qDebug() << "Removing user-defined bodies";
            foreach (PlanetP p, systemPlanets)
            {
                p->satellites.clear();
                p.clear();
            }
            systemPlanets.clear();
        }
    }

    shadowPlanetCount = 0;

    foreach (const PlanetP& planet, systemPlanets)
        if(planet->parent != sun || !planet->satellites.isEmpty())
            shadowPlanetCount++;
}

bool SolarSystem::loadPlanets(const QString& filePath)
{
	QSettings pd(filePath, StelIniFormat);
	if (pd.status() != QSettings::NoError)
	{
		qWarning() << "ERROR while parsing" << QDir::toNativeSeparators(filePath);
		return false;
	}

	// QSettings does not allow us to say that the sections of the file
	// will be listed in the same order  as in the file like the old
	// InitParser used to so we can no longer assume that.
	//
	// This means we must first decide what order to read the sections
	// of the file in (each section contains one planet) to avoid setting
	// the parent Planet* to one which has not yet been created.
	//
	// Stage 1: Make a map of body names back to the section names
	// which they come from. Also make a map of body name to parent body
	// name. These two maps can be made in a single pass through the
	// sections of the file.
	//
	// Stage 2: Make an ordered list of section names such that each
	// item is only ever dependent on items which appear earlier in the
	// list.
	// 2a: Make a QMultiMap relating the number of levels of dependency
	//     to the body name, i.e.
	//     0 -> Sun
	//     1 -> Mercury
	//     1 -> Venus
	//     1 -> Earth
	//     2 -> Moon
	//     etc.
	// 2b: Populate an ordered list of section names by iterating over
	//     the QMultiMap.  This type of contains is always sorted on the
	//     key, so it's easy.
	//     i.e. [sol, earth, moon] is fine, but not [sol, moon, earth]
	//
	// Stage 3: iterate over the ordered sections decided in stage 2,
	// creating the planet objects from the QSettings data.

	// Stage 1 (as described above).
	QMap<QString, QString> secNameMap;
	QMap<QString, QString> parentMap;
	QStringList sections = pd.childGroups();
	for (int i=0; i<sections.size(); ++i)
	{
		const QString secname = sections.at(i);
		const QString englishName = pd.value(secname+"/name").toString();
		const QString strParent = pd.value(secname+"/parent", "Sun").toString();
		secNameMap[englishName] = secname;
		if (strParent!="none" && !strParent.isEmpty() && !englishName.isEmpty())
			parentMap[englishName] = strParent;
	}

	// Stage 2a (as described above).
	QMultiMap<int, QString> depLevelMap;
	for (int i=0; i<sections.size(); ++i)
	{
		const QString englishName = pd.value(sections.at(i)+"/name").toString();

		// follow dependencies, incrementing level when we have one
		// till we run out.
		QString p=englishName;
		int level = 0;
		while(parentMap.contains(p) && parentMap[p]!="none")
		{
			level++;
			p = parentMap[p];
		}

		depLevelMap.insert(level, secNameMap[englishName]);
	}

	// Stage 2b (as described above).
	QStringList orderedSections;
	QMapIterator<int, QString> levelMapIt(depLevelMap);
	while(levelMapIt.hasNext())
	{
		levelMapIt.next();
		orderedSections << levelMapIt.value();
	}

	// Stage 3 (as described above).
	int readOk=0;
	int totalPlanets=0;
	for (int i = 0;i<orderedSections.size();++i)
	{
		totalPlanets++;
		const QString secname = orderedSections.at(i);
		const QString englishName = pd.value(secname+"/name").toString();
		const QString strParent = pd.value(secname+"/parent", "Sun").toString(); // Obvious default, keep file entries simple.
		PlanetP parent;
		if (strParent!="none")
		{
			// Look in the other planets the one named with strParent
			foreach (const PlanetP& p, systemPlanets)
			{
				if (p->getEnglishName()==strParent)
				{
					parent = p;
					break;
				}
			}
			if (parent.isNull())
			{
				qWarning() << "ERROR : can't find parent solar system body for " << englishName;
				//abort();
				continue;
			}
		}

		const QString funcName = pd.value(secname+"/coord_func").toString();
		posFuncType posfunc=NULL;
		void* userDataPtr=NULL;
		OsculatingFunctType *osculatingFunc = 0;
		bool closeOrbit = pd.value(secname+"/closeOrbit", true).toBool();

		if (funcName=="ell_orbit")
		{
			// Read the orbital elements
			const double epoch = pd.value(secname+"/orbit_Epoch",J2000).toDouble();
			const double eccentricity = pd.value(secname+"/orbit_Eccentricity").toDouble();
			if (eccentricity >= 1.0) closeOrbit = false;
			double pericenterDistance = pd.value(secname+"/orbit_PericenterDistance",-1e100).toDouble();
			double semi_major_axis;
			if (pericenterDistance <= 0.0) {
				semi_major_axis = pd.value(secname+"/orbit_SemiMajorAxis",-1e100).toDouble();
				if (semi_major_axis <= -1e100) {
					qDebug() << "ERROR: " << englishName
						<< ": you must provide orbit_PericenterDistance or orbit_SemiMajorAxis";
					//abort();
					continue;
				} else {
					semi_major_axis /= AU;
					Q_ASSERT(eccentricity != 1.0); // parabolic orbits have no semi_major_axis
					pericenterDistance = semi_major_axis * (1.0-eccentricity);
				}
			} else {
				pericenterDistance /= AU;
				semi_major_axis = (eccentricity == 1.0)
								? 0.0 // parabolic orbits have no semi_major_axis
								: pericenterDistance / (1.0-eccentricity);
			}
			double meanMotion = pd.value(secname+"/orbit_MeanMotion",-1e100).toDouble();
			double period;
			if (meanMotion <= -1e100) {
				period = pd.value(secname+"/orbit_Period",-1e100).toDouble();
				if (period <= -1e100) {
					meanMotion = (eccentricity == 1.0)
								? 0.01720209895 * (1.5/pericenterDistance) * sqrt(0.5/pericenterDistance)
								: (semi_major_axis > 0.0)
								? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
								: 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));
					period = 2.0*M_PI/meanMotion;
				} else {
					meanMotion = 2.0*M_PI/period;
				}
			} else {
				period = 2.0*M_PI/meanMotion;
			}
			const double inclination = pd.value(secname+"/orbit_Inclination").toDouble()*(M_PI/180.0);
			const double ascending_node = pd.value(secname+"/orbit_AscendingNode").toDouble()*(M_PI/180.0);
			double arg_of_pericenter = pd.value(secname+"/orbit_ArgOfPericenter",-1e100).toDouble();
			double long_of_pericenter;
			if (arg_of_pericenter <= -1e100) {
				long_of_pericenter = pd.value(secname+"/orbit_LongOfPericenter").toDouble()*(M_PI/180.0);
				arg_of_pericenter = long_of_pericenter - ascending_node;
			} else {
				arg_of_pericenter *= (M_PI/180.0);
				long_of_pericenter = arg_of_pericenter + ascending_node;
			}
			double mean_anomaly = pd.value(secname+"/orbit_MeanAnomaly",-1e100).toDouble();
			double mean_longitude;
			if (mean_anomaly <= -1e100) {
				mean_longitude = pd.value(secname+"/orbit_MeanLongitude").toDouble()*(M_PI/180.0);
				mean_anomaly = mean_longitude - long_of_pericenter;
			} else {
				mean_anomaly *= (M_PI/180.0);
				mean_longitude = mean_anomaly + long_of_pericenter;
			}

			// when the parent is the sun use ecliptic rather than sun equator:
			const double parentRotObliquity = parent->getParent()
											  ? parent->getRotObliquity()
											  : 0.0;
			const double parent_rot_asc_node = parent->getParent()
											  ? parent->getRotAscendingnode()
											  : 0.0;
			double parent_rot_j2000_longitude = 0.0;
			if (parent->getParent()) {
				const double c_obl = cos(parentRotObliquity);
				const double s_obl = sin(parentRotObliquity);
				const double c_nod = cos(parent_rot_asc_node);
				const double s_nod = sin(parent_rot_asc_node);
				const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
				const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
				const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
				const Vec3d J2000Pole(StelCore::matJ2000ToVsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
				Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
				J2000NodeOrigin.normalize();
				parent_rot_j2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
			}

			// Create an elliptical orbit
			EllipticalOrbit *orb = new EllipticalOrbit(pericenterDistance,
								   eccentricity,
								   inclination,
								   ascending_node,
								   arg_of_pericenter,
								   mean_anomaly,
								   period,
								   epoch,
								   parentRotObliquity,
								   parent_rot_asc_node,
								   parent_rot_j2000_longitude);
			orbits.push_back(orb);

			userDataPtr = orb;
			posfunc = &ellipticalOrbitPosFunc;
		}
		else if (funcName=="comet_orbit")
		{
			// Read the orbital elements
			// orbit_PericenterDistance,orbit_SemiMajorAxis: given in AU
			// orbit_MeanMotion: given in degrees/day
			// orbit_Period: given in days
			// orbit_TimeAtPericenter,orbit_Epoch: JD
			// orbit_MeanAnomaly,orbit_Inclination,orbit_ArgOfPericenter,orbit_AscendingNode: given in degrees
			const double eccentricity = pd.value(secname+"/orbit_Eccentricity",0.0).toDouble();
			if (eccentricity >= 1.0) closeOrbit = false;
			double pericenterDistance = pd.value(secname+"/orbit_PericenterDistance",-1e100).toDouble();
			double semi_major_axis;
			if (pericenterDistance <= 0.0) {
				semi_major_axis = pd.value(secname+"/orbit_SemiMajorAxis",-1e100).toDouble();
				if (semi_major_axis <= -1e100) {
					qWarning() << "ERROR: " << englishName
						<< ": you must provide orbit_PericenterDistance or orbit_SemiMajorAxis";
					//abort();
					continue;
				} else {
					Q_ASSERT(eccentricity != 1.0); // parabolic orbits have no semi_major_axis
					pericenterDistance = semi_major_axis * (1.0-eccentricity);
				}
			} else {
				semi_major_axis = (eccentricity == 1.0)
								? 0.0 // parabolic orbits have no semi_major_axis
								: pericenterDistance / (1.0-eccentricity);
			}
			double meanMotion = pd.value(secname+"/orbit_MeanMotion",-1e100).toDouble();
			if (meanMotion <= -1e100) {
				const double period = pd.value(secname+"/orbit_Period",-1e100).toDouble();
				if (period <= -1e100) {
					if (parent->getParent()) {
						qWarning() << "ERROR: " << englishName
							<< ": when the parent body is not the sun, you must provide "
							<< "either orbit_MeanMotion or orbit_Period";
					} else {
						// in case of parent=sun: use Gaussian gravitational constant
						// for calculating meanMotion:
						meanMotion = (eccentricity >= 0.9999 && eccentricity <= 1.0)
									? 0.01720209895 * (1.5/pericenterDistance) * sqrt(0.5/pericenterDistance)
									: (semi_major_axis > 0.0)
									? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
									: 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));
					}
				} else {
					meanMotion = 2.0*M_PI/period;
				}
			} else {
				meanMotion *= (M_PI/180.0);
			}
			double time_at_pericenter = pd.value(secname+"/orbit_TimeAtPericenter",-1e100).toDouble();
			if (time_at_pericenter <= -1e100) {
				const double epoch = pd.value(secname+"/orbit_Epoch",-1e100).toDouble();
				double mean_anomaly = pd.value(secname+"/orbit_MeanAnomaly",-1e100).toDouble();
				if (epoch <= -1e100 || mean_anomaly <= -1e100) {
					qWarning() << "ERROR: " << englishName
						<< ": when you do not provide orbit_TimeAtPericenter, you must provide both "
						<< "orbit_Epoch and orbit_MeanAnomaly";
					//abort();
					continue;
				} else {
					mean_anomaly *= (M_PI/180.0);
					time_at_pericenter = epoch - mean_anomaly / meanMotion;
				}
			}
			const double orbitGoodDays=pd.value(secname+"/orbit_good", 1000).toDouble();
			const double inclination = pd.value(secname+"/orbit_Inclination").toDouble()*(M_PI/180.0);
			const double arg_of_pericenter = pd.value(secname+"/orbit_ArgOfPericenter").toDouble()*(M_PI/180.0);
			const double ascending_node = pd.value(secname+"/orbit_AscendingNode").toDouble()*(M_PI/180.0);
			const double parentRotObliquity = parent->getParent() ? parent->getRotObliquity() : 0.0;
			const double parent_rot_asc_node = parent->getParent() ? parent->getRotAscendingnode() : 0.0;
			double parent_rot_j2000_longitude = 0.0;
						if (parent->getParent()) {
							const double c_obl = cos(parentRotObliquity);
							const double s_obl = sin(parentRotObliquity);
							const double c_nod = cos(parent_rot_asc_node);
							const double s_nod = sin(parent_rot_asc_node);
							const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
							const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
							const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
							const Vec3d J2000Pole(StelCore::matJ2000ToVsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
							Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
							J2000NodeOrigin.normalize();
							parent_rot_j2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
						}
			CometOrbit *orb = new CometOrbit(pericenterDistance,
							 eccentricity,
							 inclination,
							 ascending_node,
							 arg_of_pericenter,
							 time_at_pericenter,
							 orbitGoodDays,
							 meanMotion,
							 parentRotObliquity,
							 parent_rot_asc_node,
							 parent_rot_j2000_longitude);
			orbits.push_back(orb);
			userDataPtr = orb;
			posfunc = &cometOrbitPosFunc;
		}

		if (funcName=="sun_special")
			posfunc = &get_sun_helio_coordsv;

		if (funcName=="mercury_special") {
			posfunc = &get_mercury_helio_coordsv;
			osculatingFunc = &get_mercury_helio_osculating_coords;
		}

		if (funcName=="venus_special") {
			posfunc = &get_venus_helio_coordsv;
			osculatingFunc = &get_venus_helio_osculating_coords;
		}

		if (funcName=="earth_special") {
			posfunc = &get_earth_helio_coordsv;
			osculatingFunc = &get_earth_helio_osculating_coords;
		}

		if (funcName=="lunar_special")
			posfunc = &get_lunar_parent_coordsv;

		if (funcName=="mars_special") {
			posfunc = &get_mars_helio_coordsv;
			osculatingFunc = &get_mars_helio_osculating_coords;
		}

		if (funcName=="phobos_special")
			posfunc = posFuncType(get_phobos_parent_coordsv);

		if (funcName=="deimos_special")
			posfunc = &get_deimos_parent_coordsv;

		if (funcName=="jupiter_special") {
			posfunc = &get_jupiter_helio_coordsv;
			osculatingFunc = &get_jupiter_helio_osculating_coords;
		}

		if (funcName=="europa_special")
			posfunc = &get_europa_parent_coordsv;

		if (funcName=="calisto_special")
			posfunc = &get_callisto_parent_coordsv;

		if (funcName=="io_special")
			posfunc = &get_io_parent_coordsv;

		if (funcName=="ganymede_special")
			posfunc = &get_ganymede_parent_coordsv;

		if (funcName=="saturn_special") {
			posfunc = &get_saturn_helio_coordsv;
			osculatingFunc = &get_saturn_helio_osculating_coords;
		}

		if (funcName=="mimas_special")
			posfunc = &get_mimas_parent_coordsv;

		if (funcName=="enceladus_special")
			posfunc = &get_enceladus_parent_coordsv;

		if (funcName=="tethys_special")
			posfunc = &get_tethys_parent_coordsv;

		if (funcName=="dione_special")
			posfunc = &get_dione_parent_coordsv;

		if (funcName=="rhea_special")
			posfunc = &get_rhea_parent_coordsv;

		if (funcName=="titan_special")
			posfunc = &get_titan_parent_coordsv;

		if (funcName=="iapetus_special")
			posfunc = &get_iapetus_parent_coordsv;

		if (funcName=="hyperion_special")
			posfunc = &get_hyperion_parent_coordsv;

		if (funcName=="uranus_special") {
			posfunc = &get_uranus_helio_coordsv;
			osculatingFunc = &get_uranus_helio_osculating_coords;
		}

		if (funcName=="miranda_special")
			posfunc = &get_miranda_parent_coordsv;

		if (funcName=="ariel_special")
			posfunc = &get_ariel_parent_coordsv;

		if (funcName=="umbriel_special")
			posfunc = &get_umbriel_parent_coordsv;

		if (funcName=="titania_special")
			posfunc = &get_titania_parent_coordsv;

		if (funcName=="oberon_special")
			posfunc = &get_oberon_parent_coordsv;

		if (funcName=="neptune_special") {
			posfunc = posFuncType(get_neptune_helio_coordsv);
			osculatingFunc = &get_neptune_helio_osculating_coords;
		}

		if (funcName=="pluto_special")
			posfunc = &get_pluto_helio_coordsv;


		if (posfunc==NULL)
		{
			qWarning() << "ERROR : can't find posfunc " << funcName << " for " << englishName;
			exit(-1);
		}

		// Create the Solar System body and add it to the list
		QString type = pd.value(secname+"/type").toString();
		PlanetP p;
		// New class objects, named "plutoid", has properties similar asteroids and we should calculate their
		// positions like for asteroids. Plutoids having one exception - Pluto - we should use special
		// function for calculation of orbit of Pluto.
		if ((type == "asteroid" || type == "plutoid") && !englishName.contains("Pluto"))
		{
			p = PlanetP(new MinorPlanet(englishName,
						    pd.value(secname+"/lighting", true).toBool(),
						    pd.value(secname+"/radius").toDouble()/AU,
						    pd.value(secname+"/oblateness", 0.0).toDouble(),
						    StelUtils::strToVec3f(pd.value(secname+"/color", "1.0,1.0,1.0").toString()),
						    pd.value(secname+"/albedo").toFloat(),
						    pd.value(secname+"/tex_map").toString(),
						    posfunc,
						    userDataPtr,
						    osculatingFunc,
						    closeOrbit,
						    pd.value(secname+"/hidden", false).toBool(),
						    type));

			QSharedPointer<MinorPlanet> mp =  p.dynamicCast<MinorPlanet>();

			//Number
			int minorPlanetNumber = pd.value(secname+"/minor_planet_number", 0).toInt();
			if (minorPlanetNumber)
			{

				mp->setMinorPlanetNumber(minorPlanetNumber);
			}

			//Provisional designation
			QString provisionalDesignation = pd.value(secname+"/provisional_designation").toString();
			if (!provisionalDesignation.isEmpty())
			{
				mp->setProvisionalDesignation(provisionalDesignation);
			}

			//H-G magnitude system
			double magnitude = pd.value(secname+"/absolute_magnitude", -99).toDouble();
			double slope = pd.value(secname+"/slope_parameter", 0.15).toDouble();
			if (magnitude > -99)
			{
				if (slope >= 0 && slope <= 1)
				{
					mp->setAbsoluteMagnitudeAndSlope(magnitude, slope);
				}
				else
				{
					mp->setAbsoluteMagnitudeAndSlope(magnitude, 0.15);
				}
			}

			mp->setSemiMajorAxis(pd.value(secname+"/orbit_SemiMajorAxis", 0).toDouble());

		}
		else if (type == "comet")
		{
			p = PlanetP(new Comet(englishName,
			               pd.value(secname+"/lighting", true).toBool(),
			               pd.value(secname+"/radius").toDouble()/AU,
			               pd.value(secname+"/oblateness", 0.0).toDouble(),
			               StelUtils::strToVec3f(pd.value(secname+"/color", "1.0,1.0,1.0").toString()),
			               pd.value(secname+"/albedo").toFloat(),
			               pd.value(secname+"/tex_map").toString(),
			               posfunc,
			               userDataPtr,
			               osculatingFunc,
			               closeOrbit,
				       pd.value(secname+"/hidden", false).toBool(),
				       type));

			QSharedPointer<Comet> mp =  p.dynamicCast<Comet>();

			//g,k magnitude system
			double magnitude = pd.value(secname+"/absolute_magnitude", -99).toDouble();
			double slope = pd.value(secname+"/slope_parameter", 4.0).toDouble();
			if (magnitude > -99)
			{
				if (slope >= 0 && slope <= 20)
				{
					mp->setAbsoluteMagnitudeAndSlope(magnitude, slope);
				}
				else
				{
					mp->setAbsoluteMagnitudeAndSlope(magnitude, 4.0);
				}
			}

			mp->setSemiMajorAxis(pd.value(secname+"/orbit_SemiMajorAxis", 0).toDouble());

		}
		else
		{
			bool lighting = pd.value(secname+"/lighting", true).toBool();
			if (englishName == "Sun") lighting = false;
			p = PlanetP(new Planet(englishName,
					       lighting,
					       pd.value(secname+"/radius").toDouble()/AU,
					       pd.value(secname+"/oblateness", 0.0).toDouble(),
					       StelUtils::strToVec3f(pd.value(secname+"/color", "1.0,1.0,1.0").toString()),
					       pd.value(secname+"/albedo").toFloat(),
					       pd.value(secname+"/tex_map").toString(),
					       posfunc,
					       userDataPtr,
					       osculatingFunc,
					       closeOrbit,
					       pd.value(secname+"/hidden", false).toBool(),
					       pd.value(secname+"/atmosphere", false).toBool(),
					       type));
		}


		if (!parent.isNull())
		{
			parent->satellites.append(p);
			p->parent = parent;
		}
		if (secname=="earth") earth = p;
		if (secname=="sun") sun = p;
		if (secname=="moon") moon = p;

		double rotObliquity = pd.value(secname+"/rot_obliquity",0.).toDouble()*(M_PI/180.0);
		double rotAscNode = pd.value(secname+"/rot_equator_ascending_node",0.).toDouble()*(M_PI/180.0);

		// Use more common planet North pole data if available
		// NB: N pole as defined by IAU (NOT right hand rotation rule)
		// NB: J2000 epoch
		double J2000NPoleRA = pd.value(secname+"/rot_pole_ra", 0.).toDouble()*M_PI/180.;
		double J2000NPoleDE = pd.value(secname+"/rot_pole_de", 0.).toDouble()*M_PI/180.;

		// Guillaume: disabled for earth for the moment in the mobile version,
		// because it leads to incorrect results.  For some reason we need to
		// use an obliquity of -23° (instead of 23°)!
		if ((J2000NPoleRA || J2000NPoleDE) && englishName != "Earth")
		{
			Vec3d J2000NPole;
			StelUtils::spheToRect(J2000NPoleRA,J2000NPoleDE,J2000NPole);

			Vec3d vsop87Pole(StelCore::matJ2000ToVsop87.multiplyWithoutTranslation(J2000NPole));

			double ra, de;
			StelUtils::rectToSphe(&ra, &de, vsop87Pole);

			rotObliquity = (M_PI_2 - de);
			rotAscNode = (ra + M_PI_2);
			// qDebug() << "\tCalculated rotational obliquity: " << rotObliquity*180./M_PI << endl;
			// qDebug() << "\tCalculated rotational ascending node: " << rotAscNode*180./M_PI << endl;
		}
		// Guillaume: force the rot_precession_rate value for earth because
		// it has been removed in the new data, but the current mobile code
		// still need it!
		double precession_rate = pd.value(secname+"/rot_precession_rate",0.).toDouble();
		if (englishName == "Earth") precession_rate = 1.39639;

		p->setRotationElements(
			pd.value(secname+"/rot_periode", pd.value(secname+"/orbit_Period", 1.).toDouble()*24.).toDouble()/24.,
			pd.value(secname+"/rot_rotation_offset",0.).toDouble(),
			pd.value(secname+"/rot_epoch", J2000).toDouble(),
			rotObliquity,
			rotAscNode,
			precession_rate*M_PI/(180*36525),
			pd.value(secname+"/orbit_visualization_period", fabs(pd.value(secname+"/orbit_Period", 1.).toDouble())).toDouble()); // this is given in days...


		if (pd.value(secname+"/rings", 0).toBool()) {
			const double rMin = pd.value(secname+"/ring_inner_size").toDouble()/AU;
			const double rMax = pd.value(secname+"/ring_outer_size").toDouble()/AU;
			Ring *r = new Ring(rMin,rMax,pd.value(secname+"/tex_ring").toString());
			p->setRings(r);
		}

		systemPlanets.push_back(p);
		readOk++;
	}

	if (systemPlanets.isEmpty())
	{
		qWarning() << "No Solar System objects loaded from" << QDir::toNativeSeparators(filePath);
		return false;
	}

	// special case: load earth shadow texture
	Planet::texEarthShadow = StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/earth-shadow.png");

	return true;
}

// Compute the position for every elements of the solar system.
// The order is not important since the position is computed relatively to the mother body
void SolarSystem::computePositions(double date, const Vec3d& observerPos)
{
	if (flagLightTravelTime)
	{
		foreach (PlanetP p, systemPlanets)
		{
			p->computePositionWithoutOrbits(date);
		}
		foreach (PlanetP p, systemPlanets)
		{
			const double light_speed_correction = (p->getHeliocentricEclipticPos()-observerPos).length() * (AU / (SPEED_OF_LIGHT * 86400));
			p->computePosition(date-light_speed_correction);
		}
	}
	else
	{
		foreach (PlanetP p, systemPlanets)
		{
			p->computePosition(date);
		}
	}
	computeTransMatrices(date, observerPos);
}

// Compute the transformation matrix for every elements of the solar system.
// The elements have to be ordered hierarchically, eg. it's important to compute earth before moon.
void SolarSystem::computeTransMatrices(double date, const Vec3d& observerPos)
{
	if (flagLightTravelTime)
	{
		foreach (PlanetP p, systemPlanets)
		{
			const double light_speed_correction = (p->getHeliocentricEclipticPos()-observerPos).length() * (AU / (SPEED_OF_LIGHT * 86400));
			p->computeTransMatrix(date-light_speed_correction);
		}
	}
	else
	{
		foreach (PlanetP p, systemPlanets)
		{
			p->computeTransMatrix(date);
		}
	}
}

// And sort them from the furthest to the closest to the observer
struct biggerDistance : public std::binary_function<PlanetP, PlanetP, bool>
{
	bool operator()(PlanetP p1, PlanetP p2)
	{
		return p1->getDistance() > p2->getDistance();
	}
};

// Draw all the elements of the solar system
// We are supposed to be in heliocentric coordinate
void SolarSystem::draw(StelCore* core)
{
	if (!flagShow)
		return;

	// Compute each Planet distance to the observer
	Vec3d obsHelioPos = core->getObserverHeliocentricEclipticPos();

	foreach (PlanetP p, systemPlanets)
	{
		p->computeDistance(obsHelioPos);
	}

	// And sort them from the furthest to the closest
	sort(systemPlanets.begin(),systemPlanets.end(),biggerDistance());

	if (trailFader.getInterstate()>0.0000001f)
	{
		StelPainter* sPainter = new StelPainter(core->getProjection2d());
		allTrails->setOpacity(trailFader.getInterstate());
		allTrails->draw(core, sPainter);
		delete sPainter;
	}

	// Make some voodoo to determine when labels should be displayed
	float maxMagLabel = (core->getSkyDrawer()->getLimitMagnitude()<5.f ? core->getSkyDrawer()->getLimitMagnitude() :
			5.f+(core->getSkyDrawer()->getLimitMagnitude()-5.f)*1.2f) +(labelsAmount-3.f)*1.2f;

	// Draw the elements
	foreach (const PlanetP& p, systemPlanets)
	{
		p->draw(core, maxMagLabel, planetNameFont);
	}

	if (GETSTELMODULE(StelObjectMgr)->getFlagSelectedObjectPointer() && getFlagMarkers())
		drawPointer(core);
}

void SolarSystem::setStelStyle(const QString& section)
{
	// Load colors from config file
	QSettings* conf = StelApp::getInstance().getSettings();
	QString defaultColor = conf->value(section+"/default_color").toString();
	setLabelsColor(StelUtils::strToVec3f(conf->value(section+"/planet_names_color", defaultColor).toString()));
	setOrbitsColor(StelUtils::strToVec3f(conf->value(section+"/planet_orbits_color", defaultColor).toString()));
	setTrailsColor(StelUtils::strToVec3f(conf->value(section+"/object_trails_color", defaultColor).toString()));
	setPointersColor(StelUtils::strToVec3f(conf->value(section+"/planet_pointers_color", "1.0,0.3,0.3").toString()));

	// Recreate the trails to apply new colors
	recreateTrails();
}

PlanetP SolarSystem::searchByEnglishName(QString planetEnglishName) const
{
	foreach (const PlanetP& p, systemPlanets)
	{
		if (p->getEnglishName() == planetEnglishName)
			return p;
	}
	return PlanetP();
}

StelObjectP SolarSystem::searchByNameI18n(const QString& planetNameI18) const
{
	foreach (const PlanetP& p, systemPlanets)
	{
		if (p->getNameI18n() == planetNameI18)
			return qSharedPointerCast<StelObject>(p);
	}
	return StelObjectP();
}


StelObjectP SolarSystem::searchByName(const QString& name) const
{
	foreach (const PlanetP& p, systemPlanets)
	{
		if (p->getEnglishName() == name)
			return qSharedPointerCast<StelObject>(p);
	}
	return StelObjectP();
}

float SolarSystem::getPlanetVMagnitude(QString planetName) const
{
	PlanetP p = searchByEnglishName(planetName);
	float r = 0.f;
	r = p->getVMagnitude(StelApp::getInstance().getCore());
	return r;
}

double SolarSystem::getDistanceToPlanet(QString planetName) const
{
	PlanetP p = searchByEnglishName(planetName);
	double r = 0.f;
	r = p->getDistance();
	return r;
}

double SolarSystem::getElongationForPlanet(QString planetName) const
{
	PlanetP p = searchByEnglishName(planetName);
	double r = 0.f;
	r = p->getElongation(StelApp::getInstance().getCore()->getObserverHeliocentricEclipticPos());
	return r;
}

double SolarSystem::getPhaseAngleForPlanet(QString planetName) const
{
	PlanetP p = searchByEnglishName(planetName);
	double r = 0.f;
	r = p->getPhaseAngle(StelApp::getInstance().getCore()->getObserverHeliocentricEclipticPos());
	return r;
}

float SolarSystem::getPhaseForPlanet(QString planetName) const
{
	PlanetP p = searchByEnglishName(planetName);
	float r = 0.f;
	r = p->getPhase(StelApp::getInstance().getCore()->getObserverHeliocentricEclipticPos());
	return r;
}

// Search if any Planet is close to position given in earth equatorial position and return the distance
StelObjectP SolarSystem::search(Vec3d pos, const StelCore* core) const
{
	pos.normalize();
	PlanetP closest;
	double cos_angle_closest = 0.;
	Vec3d equPos;

	foreach (const PlanetP& p, systemPlanets)
	{
		equPos = p->getEquinoxEquatorialPos(core);
		equPos.normalize();
		double cos_ang_dist = equPos*pos;
		if (cos_ang_dist>cos_angle_closest)
		{
			closest = p;
			cos_angle_closest = cos_ang_dist;
		}
	}

	if (cos_angle_closest>0.999)
	{
		return qSharedPointerCast<StelObject>(closest);
	}
	else return StelObjectP();
}

// Return a stl vector containing the planets located inside the limFov circle around position v
QList<StelObjectP> SolarSystem::searchAround(const Vec3d& vv, double limitFov, const StelCore* core) const
{
	QList<StelObjectP> result;
	if (!getFlagPlanets())
		return result;

	Vec3d v = core->j2000ToEquinoxEqu(vv);
	v.normalize();
	double cosLimFov = std::cos(limitFov * M_PI/180.);
	Vec3d equPos;
	double cosAngularSize;

	foreach (const PlanetP& p, systemPlanets)
	{
		equPos = p->getEquinoxEquatorialPos(core);
		equPos.normalize();

		cosAngularSize = std::cos(p->getSpheroidAngularSize(core) * M_PI/180.);

		if (equPos*v>=std::min(cosLimFov, cosAngularSize))
		{
			result.append(qSharedPointerCast<StelObject>(p));
		}
	}
	return result;
}

// Update i18 names from english names according to current translator
void SolarSystem::updateI18n()
{
	const StelTranslator& trans = StelApp::getInstance().getLocaleMgr().getAppStelTranslator();
	foreach (PlanetP p, systemPlanets)
		p->translateName(trans);
}

QString SolarSystem::getPlanetHashString(void)
{
	QString str;
	QTextStream oss(&str);
	foreach (const PlanetP& p, systemPlanets)
	{
		if (!p->getParent().isNull() && p->getParent()->getEnglishName() != "Sun")
		{
			oss << p->getParent()->getEnglishName() << " : ";
		}
		oss << p->getEnglishName() << endl;
		oss << p->getEnglishName() << endl;
	}
	return str;
}

void SolarSystem::setFlagTrails(bool b)
{
	trailFader = b;
	if (b)
		allTrails->reset();
}

bool SolarSystem::getFlagTrails() const
{
	return (bool)trailFader;
}

void SolarSystem::setFlagHints(bool b)
{
	foreach (PlanetP p, systemPlanets)
		p->setFlagHints(b);
}

bool SolarSystem::getFlagHints(void) const
{
	foreach (const PlanetP& p, systemPlanets)
	{
		if (p->getFlagHints())
			return true;
	}
	return false;
}

void SolarSystem::setFlagLabels(bool b)
{
	foreach (PlanetP p, systemPlanets)
		p->setFlagLabels(b);
}

bool SolarSystem::getFlagLabels() const
{
	foreach (const PlanetP& p, systemPlanets)
	{
		if (p->getFlagLabels())
			return true;
	}
	return false;
}

void SolarSystem::setFlagOrbits(bool b)
{
	flagOrbits = b;
	if (!b || !selected || selected==sun)
	{
		foreach (PlanetP p, systemPlanets)
			p->setFlagOrbits(b);
	}
	else
	{
		// If a Planet is selected and orbits are on, fade out non-selected ones
		foreach (PlanetP p, systemPlanets)
		{
			if (selected == p)
				p->setFlagOrbits(b);
			else
				p->setFlagOrbits(false);
		}
	}
}

void SolarSystem::setFlagLightTravelTime(bool b)
{
	flagLightTravelTime = b;
}

void SolarSystem::setSelected(PlanetP obj)
{
	if (obj && obj->getType() == "Planet")
		selected = obj;
	else
		selected.clear();;
	// Undraw other objects hints, orbit, trails etc..
	setFlagHints(getFlagHints());
	setFlagOrbits(getFlagOrbits());
}


void SolarSystem::update(double deltaTime)
{
	trailFader.update(deltaTime*1000);
	if (trailFader.getInterstate()>0.f)
	{
		allTrails->update();
	}

	foreach (PlanetP p, systemPlanets)
	{
		p->update((int)(deltaTime*1000));
	}
}


// is a lunar eclipse close at hand?
bool SolarSystem::nearLunarEclipse()
{
	// TODO: could replace with simpler test

	Vec3d e = getEarth()->getEclipticPos();
	Vec3d m = getMoon()->getEclipticPos();  // relative to earth
	Vec3d mh = getMoon()->getHeliocentricEclipticPos();  // relative to sun

	// shadow location at earth + moon distance along earth vector from sun
	Vec3d en = e;
	en.normalize();
	Vec3d shadow = en * (e.length() + m.length());

	// find shadow radii in AU
	double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;

	// modify shadow location for scaled moon
	Vec3d mdist = shadow - mh;
	if(mdist.length() > r_penumbra + 2000/AU) return 0;   // not visible so don't bother drawing

	return 1;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
QStringList SolarSystem::listMatchingObjectsI18n(const QString& objPrefix, int maxNbItem, bool useStartOfWords) const
{
	QStringList result;
	if (maxNbItem==0)
		return result;

	QString sson;
	bool find;
	foreach (const PlanetP& p, systemPlanets)
	{
		sson = p->getNameI18n();
		find = false;
		if (useStartOfWords)
		{
			QString constw = sson.mid(0, objPrefix.size()).toUpper();
			if (constw==objPrefix.toUpper())
				find = true;
		}
		else
		{
			if (sson.contains(objPrefix, Qt::CaseInsensitive))
				find = true;

		}
		if (find)
		{
			result << sson;
			if (result.size()==maxNbItem)
				return result;
		}

	}
	return result;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object English name
QStringList SolarSystem::listMatchingObjects(const QString& objPrefix, int maxNbItem, bool useStartOfWords) const
{
	QStringList result;
	if (maxNbItem==0)
		return result;

	QString sson;
	bool find;
	foreach (const PlanetP& p, systemPlanets)
	{
		sson = p->getEnglishName();
		find = false;
		if (useStartOfWords)
		{
			QString constw = sson.mid(0, objPrefix.size()).toUpper();
			if (constw==objPrefix.toUpper())
				find = true;
		}
		else
		{
			if (sson.contains(objPrefix, Qt::CaseInsensitive))
				find = true;

		}
		if (find)
		{
			result << sson;
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}

QStringList SolarSystem::listAllObjects(bool inEnglish) const
{
	QStringList result;
	if (inEnglish)
	{
		foreach(const PlanetP& p, systemPlanets)
		{
			result << p->getEnglishName();
		}
	}
	else
	{
		foreach(const PlanetP& p, systemPlanets)
		{
			result << p->getNameI18n();
		}
	}
	return result;
}

void SolarSystem::selectedObjectChange(StelModule::StelModuleSelectAction)
{
	const QList<StelObjectP> newSelected = GETSTELMODULE(StelObjectMgr)->getSelectedObject("Planet");
	if (!newSelected.empty())
		setSelected(qSharedPointerCast<Planet>(newSelected[0]));
}

// Activate/Deactivate planets display
void SolarSystem::setFlagPlanets(bool b)
{
	flagShow=b;
}

bool SolarSystem::getFlagPlanets(void) const {return flagShow;}

// Set/Get planets names color
void SolarSystem::setLabelsColor(const Vec3f& c) {Planet::setLabelColor(c);}
const Vec3f& SolarSystem::getLabelsColor(void) const {return Planet::getLabelColor();}

// Set/Get orbits lines color
void SolarSystem::setOrbitsColor(const Vec3f& c) {Planet::setOrbitColor(c);}
Vec3f SolarSystem::getOrbitsColor(void) const {return Planet::getOrbitColor();}

// Set/Get if Moon display is scaled
void SolarSystem::setFlagMoonScale(bool b)
{
	if (!b) getMoon()->setSphereScale(1);
	else getMoon()->setSphereScale(moonScale);
	flagMoonScale = b;
}

// Set/Get Moon display scaling factor
void SolarSystem::setMoonScale(float f)
{
	moonScale = f;
	if (flagMoonScale)
		getMoon()->setSphereScale(moonScale);
}

// Set selected planets by englishName
void SolarSystem::setSelected(const QString& englishName)
{
	setSelected(searchByEnglishName(englishName));
}

// Get the list of all the planet english names
QStringList SolarSystem::getAllPlanetEnglishNames() const
{
	QStringList res;
	foreach (const PlanetP& p, systemPlanets)
		res.append(p->englishName);
	return res;
}

QStringList SolarSystem::getAllPlanetLocalizedNames() const
{
	QStringList res;
	foreach (const PlanetP& p, systemPlanets)
		res.append(p->nameI18);
	return res;
}

void SolarSystem::reloadPlanets()
{
	// Save flag states
	bool flagScaleMoon = getFlagMoonScale();
	float moonScale = getMoonScale();
	bool flagPlanets = getFlagPlanets();
	bool flagHints = getFlagHints();
	bool flagLabels = getFlagLabels();
	bool flagOrbits = getFlagOrbits();
	
	// Save observer location (fix for LP bug # 969211)
	// TODO: This can probably be done better with a better understanding of StelObserver --BM
	StelCore* core = StelApp::getInstance().getCore();
	StelLocation loc = core->getCurrentLocation();

	// Unload all Solar System objects
	selected.clear();//Release the selected one
	foreach (Orbit* orb, orbits)
	{
		delete orb;
		orb = NULL;
	}
	orbits.clear();

	sun.clear();
	moon.clear();
	earth.clear();
	Planet::texEarthShadow.clear(); //Loaded in loadPlanets()

	delete allTrails;
	allTrails = NULL;

	foreach (PlanetP p, systemPlanets)
	{
		p->satellites.clear();
		p.clear();
	}
	systemPlanets.clear();
	// Memory leak? What's the proper way of cleaning shared pointers?

	// Re-load the ssystem.ini file
	loadPlanets();	
	computePositions(StelUtils::getJDFromSystem());
	setSelected("");
	recreateTrails();
	
	// Restore observer location
	core->moveObserverTo(loc, 0., 0.);

	// Restore flag states
	setFlagMoonScale(flagScaleMoon);
	setMoonScale(moonScale);
	setFlagPlanets(flagPlanets);
	setFlagHints(flagHints);
	setFlagLabels(flagLabels);
	setFlagOrbits(flagOrbits);

	// Restore translations
	updateI18n();
}
