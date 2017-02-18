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
#include <QGeoPositionInfoSource>

class GPSMgr : public StelModule
{
	Q_OBJECT
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(State state READ getState NOTIFY stateChanged)
public:
	enum State {Disabled, Searching, Found, Unsupported};
	GPSMgr();
	virtual ~GPSMgr();
	virtual void init() Q_DECL_OVERRIDE;
	virtual void update(double deltaTime) Q_DECL_OVERRIDE {Q_UNUSED(deltaTime)}
	bool isEnabled() const {return state == Searching || state == Found;}
	Q_INVOKABLE void setEnabled(bool value);
	State getState() const {return state;}
	static GPSMgr *singleton;
signals:
	void stateChanged(GPSMgr::State value);
	void enabledChanged(bool value);
private:
	State state;
	QGeoPositionInfoSource* source;
private slots:
	void positionUpdated(const QGeoPositionInfo &info);
	void onError(QGeoPositionInfoSource::Error positioningError);
};

Q_DECLARE_METATYPE(GPSMgr::State)
