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


#include <QDateTime>
#include <QQuickItem>

// Special object that is just there so we can invoke some methods in the main thread.
class MainThreadProxy : public QObject
{
	Q_OBJECT
public:
	Q_INVOKABLE void setCurrentLandscapeID(const QString& value);
	Q_INVOKABLE void setCurrentLandscapeName(const QString& value);
	Q_INVOKABLE void setLocation(const QString locationId);
	Q_INVOKABLE void setManualPosition(double latitude, double longitude);
};

class StelQuickStelItem : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(bool forwardClicks READ getForwardClicks WRITE setForwardClicks)
	Q_PROPERTY(QString selectedObjectName READ getSelectedObjectName NOTIFY selectedObjectChanged)
	Q_PROPERTY(QString selectedObjectShortInfo READ getSelectedObjectShortInfo NOTIFY selectedObjectShortInfoChanged)
	Q_PROPERTY(QString selectedObjectInfo READ getSelectedObjectInfo NOTIFY selectedObjectInfoChanged)
	Q_PROPERTY(bool tracking READ getTracking NOTIFY trackingModeChanged)
	Q_PROPERTY(double jd READ getJd WRITE setJd NOTIFY timeChanged)
	Q_PROPERTY(bool dragTimeMode READ getDragTimeMode WRITE setDragTimeMode NOTIFY dragTimeModeChanged)
	Q_PROPERTY(double timeRate READ getTimeRate WRITE setTimeRate NOTIFY timeRateChanged)
	Q_PROPERTY(QString printableTime READ getPrintableTime NOTIFY timeChanged)
	Q_PROPERTY(int fps READ getFps NOTIFY timeChanged)
	Q_PROPERTY(QString currentLandscapeName READ getCurrentLandscapeName WRITE setCurrentLandscapeName NOTIFY currentLandscapeChanged)
	Q_PROPERTY(QString currentLandscapeHtmlDescription READ getCurrentLandscapeHtmlDescription NOTIFY currentLandscapeChanged)
	Q_PROPERTY(QString currentSkyCultureI18 READ getCurrentSkyCultureI18 WRITE setCurrentSkyCultureI18 NOTIFY currentSkyCultureChanged)
	Q_PROPERTY(QString currentSkyCultureHtmlDescription READ getCurrentSkyCultureHtmlDescription NOTIFY currentSkyCultureChanged)
	Q_PROPERTY(QString currentSkyCultureBaseUrl READ getCurrentSkyCultureBaseUrl NOTIFY currentSkyCultureBaseUrlChanged)
	Q_PROPERTY(QString location READ getLocation WRITE setLocation NOTIFY positionChanged)
	Q_PROPERTY(double latitude READ getLatitude NOTIFY positionChanged)
	Q_PROPERTY(double longitude READ getLongitude NOTIFY positionChanged)
	Q_PROPERTY(float guiScaleFactor READ getGuiScaleFactor NOTIFY guiScaleFactorChanged)
	Q_PROPERTY(float fov READ getFov NOTIFY timeChanged)
	Q_PROPERTY(QString version READ getVersion CONSTANT)
	Q_PROPERTY(QString model READ getModel CONSTANT)
	Q_PROPERTY(bool autoGotoNight READ getAutoGotoNight WRITE setAutoGotoNight)
	Q_PROPERTY(bool desktop READ isDesktop CONSTANT)
	Q_PROPERTY(QString gpsState READ getGpsState NOTIFY gpsStateChanged)
	Q_PROPERTY(int lightPollution READ getLightPollution WRITE setLightPollution NOTIFY lightPollutionChanged)
	Q_PROPERTY(int milkyWayBrightness READ getMilkyWayBrightness WRITE setMilkyWayBrightness NOTIFY milkyWayBrightnessChanged)
	Q_PROPERTY(int linesThickness READ getLinesThickness WRITE setLinesThickness NOTIFY LinesThicknessChanged)
public:
	StelQuickStelItem();
	QString getSelectedObjectName() const;
	QString getSelectedObjectInfo() const;
	QString getSelectedObjectShortInfo() const;
	Q_INVOKABLE void unselectObject();
	void setForwardClicks(bool value) {forwardClicks = value;}
	bool getForwardClicks() const {return forwardClicks;}
	bool getTracking() const;
	bool getDragTimeMode() const;
	void setDragTimeMode(bool value);
	Q_INVOKABLE void zoom(int direction);
	Q_INVOKABLE void pinch(float scale, bool started);
	Q_INVOKABLE void touch(int state, float x, float y);
	double getJd() const;
	Q_INVOKABLE void setJd(double value);
	double getTimeRate() const;
	void setTimeRate(double value);
	QString getPrintableTime() const;
	int getFps() const;
	Q_INVOKABLE QStringList getLandscapeNames() const;
	QString getCurrentLandscapeName() const;
	void setCurrentLandscapeName(const QString& value);
	QString getCurrentLandscapeHtmlDescription() const;
	Q_INVOKABLE QStringList getSkyCultureListI18() const;
	QString getCurrentSkyCultureI18() const;
	void setCurrentSkyCultureI18(const QString& value);
	QString getCurrentSkyCultureHtmlDescription() const;
	QString getCurrentSkyCultureBaseUrl() const;
	Q_INVOKABLE QStringList getCountryNames() const;
	Q_INVOKABLE QStringList getCityNames(const QString& country) const;
	QString getLocation() const;
	void setLocation(const QString locationId);
	double getLatitude() const;
	double getLongitude() const;
	Q_INVOKABLE void setManualPosition(double latitude, double longitude);
	float getGuiScaleFactor() const;
	float getFov() const;
	Q_INVOKABLE void writeSetting(const QString& key, bool value);
	Q_INVOKABLE QStringList search(const QString& name);
	Q_INVOKABLE void gotoObject(const QString& objectName);
	QString getVersion() const {return MOBILE_GUI_VERSION;}
	QString getModel() const;
	Q_INVOKABLE bool isDay() const;
	bool getAutoGotoNight() const {return autoGotoNight;}
	void setAutoGotoNight(bool value) {autoGotoNight = value;}
	bool isDesktop() const;
	QString getGpsState() const;
	int getLightPollution() const;
	void setLightPollution(int value);
	int getMilkyWayBrightness() const;
	void setMilkyWayBrightness(int value);
	int getLinesThickness() const;
	void setLinesThickness(int value);
	Q_INVOKABLE void resetSettings();

protected:
	bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

signals:
	void clicked();
	void timeChanged();
	void timeRateChanged();
	void selectedObjectChanged();
	void selectedObjectInfoChanged();
	void selectedObjectShortInfoChanged();
	void trackingModeChanged();
	void dragTimeModeChanged();
	void draggingChanged();
	void currentLandscapeChanged();
	void currentSkyCultureChanged();
	void currentSkyCultureBaseUrlChanged();
	void positionChanged();
	void guiScaleFactorChanged();
	void gpsStateChanged();
	void lightPollutionChanged();
	void milkyWayBrightnessChanged();
	void LinesThicknessChanged();

private slots:
	void update();

private:
	class QTimer* timer;
	bool forwardClicks;
	bool autoGotoNight;
	MainThreadProxy* mainThreadProxy;
};
