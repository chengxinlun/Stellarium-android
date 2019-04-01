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
#include "StelTranslator.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelModuleMgr.hpp"
#include "StelQuickView.hpp"
#include <QDebug>
#include <QSettings>
#include <QThread>
#include <QDateTime>

GPSMgr* GPSMgr::singleton = NULL;

GPSMgr::GPSMgr() :
	state(GPSMgr::Disabled),
	source(NULL)
{
	singleton = this;
	setObjectName("GPSMgr");
	// Move to thread disabled for the moment since we probably don't
	// need it now that qml doesn't use multi threads.
	// moveToThread(StelQuickView::getInstance().thread());
}

GPSMgr::~GPSMgr()
{
}

void GPSMgr::init()
{
	qRegisterMetaType<GPSMgr::State>("GPSMgr::State");
	addAction("actionGPS", N_("Movement and Selection"), N_("GPS"), "enabled");
	
	// We start the GPS if the configured location is GPS, or if there is no configured
	// location and the GPS is available.
	QSettings* conf = StelApp::getInstance().getSettings();
	bool enable = (StelApp::getInstance().getCore()->getCurrentLocation().name == "GPS") ||
	        (conf->value("init_location/location").isNull() && state != Unsupported);
	if (enable)
	{
		setEnabled(true);
	}
}

void GPSMgr::positionUpdated(const QGeoPositionInfo &info)
{
	if (!info.isValid()) return;
	StelLocation loc;
	loc.planetName = "Earth";
	loc.latitude = info.coordinate().latitude();
	loc.longitude = info.coordinate().longitude();
	loc.altitude = info.coordinate().altitude();
	if (info.coordinate().altitude() == qQNaN())
		loc.altitude = 0;
	loc.name = "GPS";
	StelApp::getInstance().getCore()->moveObserverTo(loc, 0.);
	StelApp::getInstance().getCore()->setDefaultLocationID(loc.getID());
	// Stop GPS when accuracy is < 500 m.
	if (info.timestamp().isValid() &&
			info.timestamp().secsTo(QDateTime::currentDateTime()) < 60 * 60 && // Less than one hour ago.
			info.attribute(QGeoPositionInfo::HorizontalAccuracy) < 500)
	{
		source->stopUpdates();
		state = Found;
		emit stateChanged(state);
	}
}

void GPSMgr::onError(QGeoPositionInfoSource::Error positioningError)
{
	qDebug() << "GPS Error" << positioningError;
}

void GPSMgr::setEnabled(bool value)
{
	if (state == Unsupported)
		return;
	if (isEnabled() == value)
		return;
	// We can only call this from the main thread.  Otherwise it does not work on iOS.
	if (this->thread() != QThread::currentThread())
	{
		QMetaObject::invokeMethod(this, "setEnabled", Qt::AutoConnection, Q_ARG(bool, value));
		return;
	}
	state = value ? Searching : Disabled;

	if (value && !source)
	{
		source = QGeoPositionInfoSource::createDefaultSource(this);
		if (source)
		{
			connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)),
					this, SLOT(positionUpdated(QGeoPositionInfo)));
			connect(source, SIGNAL(error(QGeoPositionInfoSource::Error)),
					this, SLOT(onError(QGeoPositionInfoSource::Error)));
		}
		else
		{
			qDebug() << "GPS: cannot create source";
			state = Unsupported;
			return;
		}
	}
	
	value ? source->startUpdates() : source->stopUpdates();
	emit stateChanged(state);
	emit enabledChanged(value);
	if (value && source->lastKnownPosition().isValid())
		positionUpdated(source->lastKnownPosition());
}
