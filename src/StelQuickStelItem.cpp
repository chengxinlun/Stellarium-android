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

#include "GPSMgr.hpp"
#include "StelQuickStelItem.hpp"
#include "StelObjectMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelMainView.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelIniParser.hpp"
#include "ConstellationMgr.hpp"
#include "SolarSystem.hpp"
#include "StelLocaleMgr.hpp"
#include "LandscapeMgr.hpp"
#include "StelUtils.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelLocationMgr.hpp"
#include "StelActionMgr.hpp"
#include "MilkyWay.hpp"

#ifdef Q_OS_ANDROID
#include "StelAndroid.hpp"
#endif

#include <QGuiApplication>
#include <QTimer>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

StelQuickStelItem::StelQuickStelItem()
{
	forwardClicks = false;
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->setInterval(100);
	timer->start();
	setAcceptHoverEvents(true);
	setAcceptedMouseButtons(Qt::AllButtons);

	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	connect(lmgr, SIGNAL(landscapeChanged(QString)), this, SIGNAL(currentLandscapeChanged()));
	StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
	connect(omgr, SIGNAL(selectedObjectChanged(StelModule::StelModuleSelectAction)), this, SIGNAL(selectedObjectChanged()));
	connect(omgr, SIGNAL(selectedObjectChanged(StelModule::StelModuleSelectAction)), this, SIGNAL(selectedObjectInfoChanged()));
	connect(omgr, SIGNAL(selectedObjectChanged(StelModule::StelModuleSelectAction)), this, SIGNAL(selectedObjectShortInfoChanged()));
	connect(StelApp::getInstance().getCore(), SIGNAL(locationChanged(StelLocation)), this, SIGNAL(positionChanged()));
	GPSMgr* gpsMgr = GETSTELMODULE(GPSMgr);
	connect(gpsMgr, SIGNAL(stateChanged(GPSMgr::State)), this, SIGNAL(gpsStateChanged()));

	QSettings* conf = StelApp::getInstance().getSettings();
	setAutoGotoNight(conf->value("gui/auto_goto_night", true).toBool());
	StelApp::getInstance().getStelActionManager()->addAction(
	            "actionAuto_Goto_Night", N_("Gui Options"), N_("Move to night at startup"), this, "autoGotoNight");
	
	QGuiApplication::instance()->installEventFilter(this);
	StelMainView::getInstance().installEventFilter(this);

	mainThreadProxy = new MainThreadProxy;
	mainThreadProxy->moveToThread(StelApp::getInstance().thread());
}

bool StelQuickStelItem::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
		case QEvent::ApplicationDeactivate:
			timer->stop();
			break;
		case QEvent::ApplicationActivate:
		case QEvent::TouchBegin:
			timer->start();
			break;
		default:
			break;
	}
	return QObject::eventFilter(obj, event);
}

double StelQuickStelItem::getJd() const
{
	StelCore* core = StelApp::getInstance().getCore();
	double jd = core->getJDay(); // (TT).
	// Convert to UT (Note: what we really want is UTC).
	jd -= StelApp::getInstance().getCore()->getDeltaT(jd) / 86400;
	return jd;
}

void StelQuickStelItem::setJd(double value)
{
	value += StelApp::getInstance().getCore()->getDeltaT(value) / 86400;
	StelApp::getInstance().getCore()->setJDay(value);
	emit timeChanged();
}

double StelQuickStelItem::getTimeRate() const
{
	return StelApp::getInstance().getCore()->getTimeRate();
}

void StelQuickStelItem::setTimeRate(double value)
{
	StelApp::getInstance().getCore()->setTimeRate(value);
}

bool StelQuickStelItem::getTracking() const
{
	return GETSTELMODULE(StelMovementMgr)->getFlagTracking();
}

void StelQuickStelItem::update()
{
	static double jd = 0;
	if (jd != getJd()) emit timeChanged();

	static bool flagTracking = false;
	const bool newFlagTracking = GETSTELMODULE(StelMovementMgr)->getFlagTracking();
	if (flagTracking != newFlagTracking)
	{
		flagTracking = newFlagTracking;
		emit trackingModeChanged();
	}

	static double timeRate = 0;
	if (getTimeRate() != timeRate) {
		timeRate = getTimeRate();
		emit timeRateChanged();
	}

	if (!GETSTELMODULE(StelObjectMgr)->getSelectedObject().isEmpty())
	{
		emit selectedObjectInfoChanged();
		emit selectedObjectShortInfoChanged();
	}
}

QString StelQuickStelItem::getSelectedObjectName() const
{
	StelObject::InfoStringGroup infoTextFilters = StelObject::InfoStringGroup(StelObject::PlainText | StelObject::Name);
	const QList<StelObjectP>& selected = GETSTELMODULE(StelObjectMgr)->getSelectedObject();
	if (selected.empty()) return "";
	StelObjectP object = selected[0];
	QString ret = object->getInfoString(StelApp::getInstance().getCore(), infoTextFilters).trimmed();
	if (ret.isEmpty()) { // Try at least to show the catalog number.
		StelObject::InfoStringGroup infoTextFilters = StelObject::InfoStringGroup(StelObject::PlainText | StelObject::Name | StelObject::CatalogNumber);
		ret = object->getInfoString(StelApp::getInstance().getCore(), infoTextFilters).trimmed();
	}
	return ret;
}

QString StelQuickStelItem::getSelectedObjectInfo() const
{
	StelObject::InfoStringGroup infoTextFilters = StelObject::InfoStringGroup(
				StelObject::PlainText | StelObject::Size | StelObject::Extra | StelObject::AltAzi | StelObject::RaDecOfDate |
				StelObject::CatalogNumber | StelObject::HourAngle);
	const QList<StelObjectP>& selected = GETSTELMODULE(StelObjectMgr)->getSelectedObject();
	if (selected.empty()) return "";
	StelCore* core = StelApp::getInstance().getCore();
	StelObjectP object = selected[0];
	// If the object name already tells the catalog number, no need to show it again in the infos.
	if (getSelectedObjectName().contains(
	            object->getInfoString(core, StelObject::PlainText | StelObject::CatalogNumber).trimmed()))
		infoTextFilters &= ~StelObject::CatalogNumber;
	return object->getInfoString(core, infoTextFilters);
}

QString StelQuickStelItem::getSelectedObjectShortInfo() const
{
	StelObject::InfoStringGroup infoTextFilters = StelObject::InfoStringGroup(StelObject::PlainText | StelObject::Magnitude | StelObject::Type | StelObject::Distance);
	const QList<StelObjectP>& selected = GETSTELMODULE(StelObjectMgr)->getSelectedObject();
	if (selected.empty()) return "";
	StelObjectP object = selected[0];
	return object->getInfoString(StelApp::getInstance().getCore(), infoTextFilters);
}

void StelQuickStelItem::unselectObject()
{
	GETSTELMODULE(StelObjectMgr)->unSelect();
	emit selectedObjectInfoChanged();
}

void StelQuickStelItem::zoom(int direction)
{
	StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
	switch (direction)
	{
	case (0):
		mvmgr->zoomIn(false);
		break;
	case (1):
		mvmgr->zoomIn(true);
		break;
	case (-1):
		mvmgr->zoomOut(true);
		break;
	default:
		Q_ASSERT(false);
		break;
	}
}

void StelQuickStelItem::pinch(float scale, bool started)
{
	StelApp::getInstance().handlePinch(scale, started);
}

void StelQuickStelItem::touch(int state, float x, float y)
{
	if (state == 0)
	{
		QMouseEvent event(QEvent::MouseButtonPress, QPoint(x, y), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
		StelApp::getInstance().handleClick(&event);
	}
	else if (state == 1)
	{
		QMouseEvent event(QEvent::MouseButtonRelease, QPoint(x, y), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
		StelApp::getInstance().handleClick(&event);
	}
	else if (state == 2)
	{
		StelApp::getInstance().handleMove(x, y, Qt::LeftButton);
	}
}

QString StelQuickStelItem::getPrintableTime() const
{
	double jd = StelApp::getInstance().getCore()->getJDay();
	// Convert from TT to UT1.
	// XXX: it would be better to merge the changes from trunk so that we can
	// use getJD (-> UT) and getJDE (-> TT).
	// Note: what we really want is UTC.
	jd -= StelApp::getInstance().getCore()->getDeltaT(jd) / 86400;
	QString time = StelApp::getInstance().getLocaleMgr().getPrintableDateLocal(jd) + " "
			+ StelApp::getInstance().getLocaleMgr().getPrintableTimeLocal(jd);
	return time.trimmed();
}

bool StelQuickStelItem::getDragTimeMode() const
{
	return GETSTELMODULE(StelMovementMgr)->getDragTimeMode();
}

void StelQuickStelItem::setDragTimeMode(bool value)
{
	GETSTELMODULE(StelMovementMgr)->setDragTimeMode(value);
	emit dragTimeModeChanged();
}

int StelQuickStelItem::getFps() const
{
	return StelApp::getInstance().getFps();
}

QStringList StelQuickStelItem::getLandscapeNames() const
{
	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	return lmgr->getAllLandscapeNames();
}

QString StelQuickStelItem::getCurrentLandscapeName() const
{
	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	return lmgr->getCurrentLandscapeName();
}

void StelQuickStelItem::setCurrentLandscapeName(const QString& value)
{
	// setCurrentLandscapeName need to be called in the main thread.
	QMetaObject::invokeMethod(mainThreadProxy, "setCurrentLandscapeName", Qt::AutoConnection, Q_ARG(QString, value));
}

void MainThreadProxy::setCurrentLandscapeID(const QString& value)
{
	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	lmgr->setCurrentLandscapeID(value);
	lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
}


void MainThreadProxy::setCurrentLandscapeName(const QString& value)
{
	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	lmgr->setCurrentLandscapeName(value);
	lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
}

void MainThreadProxy::setLocation(const QString locationId)
{
	StelLocation loc = StelApp::getInstance().getLocationMgr().locationForString(locationId);
	StelApp::getInstance().getCore()->moveObserverTo(loc, 0.);
	StelApp::getInstance().getCore()->setDefaultLocationID(locationId);
}

void MainThreadProxy::setManualPosition(double latitude, double longitude)
{
	StelLocation loc;
	loc.planetName = "Earth";
	loc.latitude = latitude;
	loc.longitude = longitude;
	StelApp::getInstance().getCore()->moveObserverTo(loc, 0.);
	StelApp::getInstance().getCore()->setDefaultLocationID(loc.getID());
}

QString StelQuickStelItem::getCurrentLandscapeHtmlDescription() const {
	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	return lmgr->getCurrentLandscapeHtmlDescription();
}

QStringList StelQuickStelItem::getSkyCultureListI18() const
{
	return StelApp::getInstance().getSkyCultureMgr().getSkyCultureListI18();
}

QString StelQuickStelItem::getCurrentSkyCultureI18() const
{
	return StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureNameI18();
}

void StelQuickStelItem::setCurrentSkyCultureI18(const QString& value)
{
	StelSkyCultureMgr* sc = &StelApp::getInstance().getSkyCultureMgr();
	sc->setCurrentSkyCultureNameI18(value);
	sc->setDefaultSkyCultureID(sc->getCurrentSkyCultureID());
	emit currentSkyCultureChanged();
	emit currentSkyCultureBaseUrlChanged();
}

QString StelQuickStelItem::getCurrentSkyCultureHtmlDescription() const
{
	QString descPath;
#ifdef ENABLE_NLS
	descPath = StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID() + "/description."+StelApp::getInstance().getLocaleMgr().getAppLanguage()+".utf8");
#endif
	if (descPath.isEmpty())
		descPath = StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID() + "/description.en.utf8");

	if (descPath.isEmpty())
	{
		return q_("No description");
	}
	else
	{
		QFile f(descPath);
		f.open(QIODevice::ReadOnly);
		return QString::fromUtf8(f.readAll());
	}
}

QString StelQuickStelItem::getCurrentSkyCultureBaseUrl() const
{
#ifdef Q_OS_ANDROID  // Puke
	QString result = StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());
#else
	QString result = StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID() + "/description.en.utf8");
#endif
	if (result.startsWith('.'))
	{
		const QFileInfo fileInfo(result);
		result = fileInfo.absoluteDir().absolutePath();
	}
	return result;
}

QStringList StelQuickStelItem::getCountryNames() const
{
	QSet<QString> done;
	QStringList ret;
	QList<StelLocation> allLocations = StelApp::getInstance().getLocationMgr().getAll();
	foreach(const StelLocation& loc, allLocations)
	{
		if (loc.planetName == "Earth" && !done.contains(loc.country))
		{
			ret << loc.country;
			done.insert(loc.country);
		}
	}
	ret.sort();
	return ret;
}

QStringList StelQuickStelItem::getCityNames(const QString& country) const
{
	QStringList ret;
	if (country.isEmpty()) return ret;
	QList<StelLocation> allLocations = StelApp::getInstance().getLocationMgr().getAll();
	foreach(const StelLocation& loc, allLocations)
	{
		if (loc.country == country)
			ret << loc.name;
	}
	ret.sort();
	return ret;
}

QString StelQuickStelItem::getLocation() const
{
	StelCore* core = StelApp::getInstance().getCore();
	const StelLocation& location = core->getCurrentLocation();
	if (location.name.isEmpty())
		return q_("Manual");
	return location.getID();
}

void StelQuickStelItem::setLocation(const QString locationId)
{
	QMetaObject::invokeMethod(mainThreadProxy, "setLocation", Qt::AutoConnection, Q_ARG(QString, locationId));
}

double StelQuickStelItem::getLatitude() const
{
	const StelLocation& location = StelApp::getInstance().getCore()->getCurrentLocation();
	return location.latitude;
}

void StelQuickStelItem::setManualPosition(double latitude, double longitude)
{
	QMetaObject::invokeMethod(mainThreadProxy, "setManualPosition", Qt::AutoConnection,
							  Q_ARG(double, latitude), Q_ARG(double, longitude));
}


double StelQuickStelItem::getLongitude() const
{
	const StelLocation& location = StelApp::getInstance().getCore()->getCurrentLocation();
	return location.longitude;
}

float StelQuickStelItem::getGuiScaleFactor() const
{
	return StelApp::getInstance().getGlobalScalingRatio();
}

float StelQuickStelItem::getFov() const
{
	return StelApp::getInstance().getCore()->getMovementMgr()->getCurrentFov();
}

void StelQuickStelItem::writeSetting(const QString& key, bool value)
{
	QSettings* conf = StelApp::getInstance().getSettings();
	if (conf->value(key).toBool() == value) return;
	conf->setValue(key, value);
}

QStringList StelQuickStelItem::search(const QString& text)
{
	QStringList ret;
	QString trimmedText = text.trimmed().toLower();
	if (trimmedText.isEmpty())
		return ret;
	StelObjectMgr* objectMgr = GETSTELMODULE(StelObjectMgr);
	ret += objectMgr->listMatchingObjectsI18n(trimmedText, 5);
	ret += objectMgr->listMatchingObjects(trimmedText, 5);
	ret.removeDuplicates();
	return ret;
}

void StelQuickStelItem::gotoObject(const QString& objectName)
{
	StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
	StelObjectMgr* objectMgr = GETSTELMODULE(StelObjectMgr);
	if (objectName.isEmpty()) return;
	if (!(objectMgr->findAndSelectI18n(objectName) || objectMgr->findAndSelect(objectName)))
		return;
	const QList<StelObjectP> newSelected = objectMgr->getSelectedObject();
	if (newSelected.empty())
		return;
	// Can't point to home planet
	if (newSelected[0]->getEnglishName() != StelApp::getInstance().getCore()->getCurrentLocation().planetName)
	{
		mvmgr->moveToJ2000(newSelected[0]->getEquinoxEquatorialPos(StelApp::getInstance().getCore()),mvmgr->getAutoMoveDuration());
		mvmgr->setFlagTracking(true);
	}
	else
	{
		GETSTELMODULE(StelObjectMgr)->unSelect();
	}
}

QString StelQuickStelItem::getModel() const
{
#ifdef Q_OS_ANDROID
	return StelAndroid::getModel();
#else
	return "";
#endif
}

bool StelQuickStelItem::isDay() const
{
	const Vec3d& sunPos = GETSTELMODULE(SolarSystem)->getSun()->getAltAzPosApparent(StelApp::getInstance().getCore());
	return sunPos[2] > -0.15;
}

bool StelQuickStelItem::isDesktop() const
{
#if defined Q_OS_ANDROID || defined Q_OS_IOS
	return false;
#else
	return true;
#endif
}

QString StelQuickStelItem::getGpsState() const
{
	switch (GETSTELMODULE(GPSMgr)->getState())
	{
		case GPSMgr::Disabled:
			return "Disabled";
		case GPSMgr::Searching:
			return "Searching";
		case GPSMgr::Found:
			return "Found";
		case GPSMgr::Unsupported:
			return "Unsupported";
		default:
			return "";
	}
}

int StelQuickStelItem::getLightPollution() const
{
	return StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale();
}

void StelQuickStelItem::setLightPollution(int value)
{
	LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
	lmgr->setAtmosphereBortleLightPollution(value);
	StelApp::getInstance().getCore()->getSkyDrawer()->setBortleScale(value);
	StelApp::getInstance().getSettings()->setValue("stars/init_bortle_scale", value);
	emit lightPollutionChanged();
}

int StelQuickStelItem::getMilkyWayBrightness() const
{
	MilkyWay* mw = GETSTELMODULE(MilkyWay);
	return mw->getIntensity();
}

void StelQuickStelItem::setMilkyWayBrightness(int value)
{
	MilkyWay* mw = GETSTELMODULE(MilkyWay);
	mw->setIntensity(value);
	StelApp::getInstance().getSettings()->setValue("astro/milky_way_intensity", value);
	emit milkyWayBrightnessChanged();
}

int StelQuickStelItem::getLinesThickness() const
{
	ConstellationMgr* mw = GETSTELMODULE(ConstellationMgr);
	return mw->getConstellationLineThickness();
}

void StelQuickStelItem::setLinesThickness(int value)
{
	ConstellationMgr* mw = GETSTELMODULE(ConstellationMgr);
	mw->setConstellationLineThickness(value);
	StelApp::getInstance().getSettings()->setValue("viewing/constellation_line_thickness", value);
	emit LinesThicknessChanged();
}

void StelQuickStelItem::resetSettings()
{
	QString defaultConfigFilePath = StelFileMgr::findFile("data/default_config.ini");
	QSettings conf(defaultConfigFilePath, StelIniFormat);
	setMilkyWayBrightness(conf.value("astro/milky_way_intensity",1.f).toFloat());
	setLightPollution(conf.value("stars/init_bortle_scale",2).toInt());
	setLinesThickness(conf.value("viewing/constellation_line_thickness",1).toInt());
	QMetaObject::invokeMethod(mainThreadProxy, "setCurrentLandscapeID", Qt::AutoConnection,
	                          Q_ARG(QString, conf.value("init_location/landscape_name").toString()));
	StelSkyCultureMgr* sc = &StelApp::getInstance().getSkyCultureMgr();
	sc->setCurrentSkyCultureID(conf.value("localization/sky_culture", "western").toString());
	sc->setDefaultSkyCultureID(sc->getCurrentSkyCultureID());

	const char *actions[][2] = {
		{"actionShow_Constellation_Lines", "viewing/flag_constellation_drawing"},
	    {"actionShow_Constellation_Labels", "viewing/flag_constellation_name"},
	    {"actionShow_Constellation_Art", "viewing/flag_constellation_art"},
	    {"actionShow_Equatorial_Grid", "viewing/flag_equatorial_grid"},
	    {"actionShow_Azimuthal_Grid", "viewing/flag_azimuthal_grid"},
	    {"actionShow_Ground", "landscape/flag_landscape"},
	    {"actionShow_Atmosphere", "landscape/flag_atmosphere"},
	    {"actionShow_Cardinal_Points", "viewing/flag_cardinal_points"},
	    {"actionShow_Nebulas", "astro/flag_nebula_name"},
	    {"actionShow_Satellite_hints", "Satellites/hints_visible"},
	    {"actionShow_Planets_Labels", "astro/flag_planets_labels"},
	    {"actionShow_Planets_Hints", "astro/flag_planets_hints"},
	    {"actionShow_Ecliptic_Line", "viewing/flag_ecliptic_line"},
	    {"actionShow_Meridian_Line", "viewing/flag_meridian_line"},
	    {"actionNight_Mode", "viewing/flag_night"},
	    {"actionShow_Constellation_Boundaries", "viewing/flag_constellation_boundaries"},
	};
	for (unsigned int i = 0; i < sizeof(actions) / sizeof(actions[0]); i++)
	{
		QVariant value = conf.value(actions[i][1]);
		if (value.isNull()) continue;
		StelAction* action = StelApp::getInstance().getStelActionManager()->findAction(actions[i][0]);
		Q_ASSERT(action);
		action->setChecked(value.toBool());
		writeSetting(actions[i][1], value.toBool());
	}
}
