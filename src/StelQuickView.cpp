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

#include "StelQuickView.hpp"
#include "StelApp.hpp"
#include "StelPainter.hpp"
#include "StelActionMgr.hpp"
#include "StelQuickStelItem.hpp"
#include "StelTranslator.hpp"

#include "LandscapeMgr.hpp"
#include "StelModuleMgr.hpp"
#include "ConstellationMgr.hpp"
#include "GridLinesMgr.hpp"
#include "NebulaMgr.hpp"
#include "SolarSystem.hpp"
#include "StelMovementMgr.hpp"
#include "StelObjectMgr.hpp"

#include <QSettings>
#include <QQuickItem>
#include <QDebug>
#include <QGuiApplication>
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QScreen>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include "StelAndroid.hpp"
#endif

StelQuickView* StelQuickView::singleton = NULL;
static QSettings* globalConf = NULL;


bool QmlGuiActionItem::isChecked() const
{
	return action && action->isChecked();
}

void QmlGuiActionItem::setChecked(bool value)
{
	Q_ASSERT(action);
	action->setChecked(value);
}

bool QmlGuiActionItem::isCheckable() const
{
	return action && action->isCheckable();
}

QString QmlGuiActionItem::getText() const
{
	return (action)? action->getText() : "";
}

QString QmlGuiActionItem::getAction() const
{
	return action == NULL ? "" : action->objectName();
}

void QmlGuiActionItem::setAction(QString value)
{
	Q_ASSERT(action == NULL);
	action = StelApp::getInstance().getStelActionManager()->findAction(value);
	if (!action) {
		qWarning() << "action" << value << "not found";
		return;
	}
	connect(action, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	emit changed();
}

void QmlGuiActionItem::trigger()
{
	if (action)
		QMetaObject::invokeMethod(action, "trigger", Qt::AutoConnection);
}


StelQuickView::StelQuickView() : stelApp(NULL), nightMode(false), quitRequested(false)
{
	singleton = this;
	timer = new QTimer(this);
	timer->setTimerType(Qt::PreciseTimer);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->setInterval(15);
	setSource(QUrl("qrc:/qml/Splash.qml"));
	connect(this, SIGNAL(widthChanged(int)), this, SLOT(handleResize()));
	connect(this, SIGNAL(heightChanged(int)), this, SLOT(handleResize()));
	connect(this, SIGNAL(beforeSynchronizing()), this, SLOT(synchronize()), Qt::DirectConnection);
	connect(this, &QQuickWindow::sceneGraphInvalidated, this, &StelQuickView::beforeGlContextDestroyed, Qt::DirectConnection);
	connect(this, SIGNAL(initialized()), this, SLOT(showGui()), Qt::QueuedConnection);
	connect((QObject*)engine(), SIGNAL(quit()), this, SLOT(requestQuit()));

	// We set the orientation mask here, otherwise we miss the initial orientation.
	QScreen* screen = qApp->primaryScreen();
	screen->setOrientationUpdateMask(Qt::PortraitOrientation | Qt::LandscapeOrientation |
									 Qt::InvertedPortraitOrientation | Qt::InvertedLandscapeOrientation);
}

void StelQuickView::init(QSettings* conf)
{
	globalConf = conf;
	setResizeMode(QQuickView::SizeRootObjectToView);
	setClearBeforeRendering(false);
	qmlRegisterType<StelQuickStelItem>("Stellarium", 1, 0, "Stellarium");
	qmlRegisterType<QmlGuiActionItem>("Stellarium", 1, 0, "StelAction");
	
#if defined(Q_OS_ANDROID)
	// Shouldn't we do showFullScreen on android as well?
	show();
#elif defined(Q_OS_IOS)
	showFullScreen();
#else
	int width = conf->value("video/screen_w", 480).toInt();
	int height = conf->value("video/screen_h", 700).toInt();
	resize(width, height);
	show();
#endif
	timer->start();
	QGuiApplication::instance()->installEventFilter(this);
}

void StelQuickView::beforeGlContextDestroyed()
{
	qWarning() << "StelQuickView::beforeGlContextDestroyed" << QOpenGLContext::currentContext();
	delete stelApp;
	stelApp = NULL;
	StelApp::deinitStatic();
	StelPainter::deinitGLShaders();
}

void StelQuickView::requestQuit()
{
	quitRequested = true;
}

void StelQuickView::handleResize()
{
	if (width() == 0 || height() == 0)
		return;
	if (stelApp==NULL)
		return;
	StelApp::getInstance().glWindowHasBeenResized(0, 0, width(), height());
}

float StelQuickView::getScreenDensity() const
{
#if defined(Q_OS_IOS)
	return screen()->physicalDotsPerInch()/160.f;
#elif defined(Q_OS_ANDROID)
	return StelAndroid::getScreenDensity()/160.f;
#else
	return screen()->physicalDotsPerInch()/96.f;
#endif
}

void StelQuickView::synchronize()
{
	const int splashTime = 2;
	static int initState = 0;
	if (initState < splashTime) // Give Qt the time to render the splash screen.
	{
		initState++;
		return;
	}
	if (initState == splashTime)
	{
		stelApp = new StelApp();
		StelApp::initStatic();
		stelApp->setGlobalScalingRatio(getScreenDensity());
		stelApp->init(globalConf);
		stelApp->getStelActionManager()->addAction("actionNight_Mode", N_("Display Options"), N_("Night mode"), this, "nightMode");
		StelPainter::initGLShaders();
		StelApp::getInstance().getStelObjectMgr().setObjectSearchRadius(25.f*stelApp->getGlobalScalingRatio());
		StelApp::getInstance().getStelObjectMgr().setDistanceWeight(0.2f);
		stelApp->glWindowHasBeenResized(0, 0, width(), height());
		setFlags(Qt::Window);
		setNightMode(globalConf->value("viewing/flag_night", false).toBool());
		initState++;

		emit initialized();
	}
	if (quitRequested)
	{
		// For the moment we just quit the app as fast as possible.
		QCoreApplication::instance()->quit();
	}
}

void StelQuickView::showGui()
{
	setSource(QUrl("qrc:/qml/main.qml"));
#ifdef Q_OS_ANDROID
	StelAndroid::setCanPause(true);
#endif
}

bool StelQuickView::eventFilter(QObject* obj, QEvent* event)
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
