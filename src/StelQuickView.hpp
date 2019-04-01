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

#include <QQuickView>
#include <QQuickItem>
#include <QOpenGLFramebufferObject>

class QSettings;
class StelApp;

class QmlGuiActionItem : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QString action READ getAction WRITE setAction)
	Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY changed)
	Q_PROPERTY(bool checkable READ isCheckable NOTIFY changed)
	Q_PROPERTY(QString text READ getText NOTIFY changed)

public:
	QmlGuiActionItem(): action(NULL) {;}
	Q_INVOKABLE void trigger();
	bool isChecked() const;
	void setChecked(bool value);
	bool isCheckable() const;
	QString getText() const;
	void setAction(QString value);
	QString getAction() const;
signals:
	void changed();
private:
	class StelAction* action;
};


class StelQuickView : public QQuickView
{
	Q_OBJECT
	Q_PROPERTY(bool nightMode READ getNightMode WRITE setNightMode NOTIFY nightModeChanged)
public:
	StelQuickView();
	void init(class QSettings* conf);

	//! Get the StelMainView singleton instance.
	static StelQuickView& getInstance() {Q_ASSERT(singleton); return *singleton;}
	bool getNightMode() const {return nightMode;}
	void setNightMode(bool value) {nightMode = value; emit nightModeChanged(value);}
signals:
	void initialized();
	void nightModeChanged(bool);
protected slots:
	void handleResize();
	void synchronize();
	void showGui();
protected:
	bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;
private:
	static StelQuickView* singleton;
	class QTimer* timer;
	StelApp* stelApp;
	float getScreenDensity() const;
	bool nightMode;
	bool quitRequested;
private slots:
	void beforeGlContextDestroyed();
	void requestQuit();
};
