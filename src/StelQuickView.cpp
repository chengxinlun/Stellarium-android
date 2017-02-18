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

static int nextPowerOf2(int x)
{
	return pow(2, ceil(log(x) / log(2)));
}


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
	nightViewShader.prog = NULL;
	blitShader.prog = NULL;
	setSource(QUrl("qrc:/qml/Splash.qml"));
	connect(this, SIGNAL(widthChanged(int)), this, SLOT(handleResize()));
	connect(this, SIGNAL(heightChanged(int)), this, SLOT(handleResize()));
	connect(this, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
	connect(this, SIGNAL(afterRendering()), this, SLOT(afterRendering()), Qt::DirectConnection);
	connect(this, SIGNAL(beforeSynchronizing()), this, SLOT(synchronize()), Qt::DirectConnection);
	connect(this, SIGNAL(initialized()), this, SLOT(showGui()));
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
	
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	show();
#else
	int width = conf->value("video/screen_w", 480).toInt();
	int height = conf->value("video/screen_h", 700).toInt();
	resize(width, height);
	show();
#endif
	timer->start();
	QGuiApplication::instance()->installEventFilter(this);
}

void StelQuickView::deinit()
{
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

void StelQuickView::createNightViewShader()
{
	QOpenGLShader vshader(QOpenGLShader::Vertex);
	const char *vsrc =
		"attribute mediump vec2 pos;\n"
		"attribute mediump vec2 texCoord;\n"
		"varying mediump vec2 texc;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = vec4(pos.x, pos.y, 0, 1);\n"
		"    texc = texCoord;\n"
		"}\n";
	if (!vshader.compileSourceCode(vsrc))
	{
		qWarning() << __func__ << ": Warnings while compiling vshader: " << vshader.log();
	}
	
	QOpenGLShader fshader(QOpenGLShader::Fragment);
	const char *fsrc =
	        "varying mediump vec2 texc;\n"
	        "uniform sampler2D tex;\n"
	        "void main(void)\n"
	        "{\n"
	        "	mediump vec3 color = texture2D(tex, texc).rgb;\n"
	        "	mediump float luminance = max(max(color.r, color.g), color.b);\n"
	        "	gl_FragColor = vec4(luminance, 0, 0, 1);\n"
	        "}\n";
	if (!fshader.compileSourceCode(fsrc))
	{
		qWarning() << __func__ << ": Warnings while compiling fshader: " << fshader.log();
	}
	nightViewShader.prog = new QOpenGLShaderProgram(QOpenGLContext::currentContext());
	nightViewShader.prog->addShader(&vshader);
	nightViewShader.prog->addShader(&fshader);
	if (!nightViewShader.prog->link())
	{
		qWarning() << __func__ << ": Warnings while linking night view: " << nightViewShader.prog->log();
	}
	
	nightViewShader.texCoord = nightViewShader.prog->attributeLocation("texCoord");
	nightViewShader.pos = nightViewShader.prog->attributeLocation("pos");
	nightViewShader.texture = nightViewShader.prog->uniformLocation("tex");
}

void StelQuickView::createBlitShader()
{
	QOpenGLShader vshader(QOpenGLShader::Vertex);
	const char *vsrc =
		"attribute mediump vec2 pos;\n"
		"attribute mediump vec2 texCoord;\n"
		"varying mediump vec2 texc;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = vec4(pos.x, pos.y, 0, 1);\n"
		"    texc = texCoord;\n"
		"}\n";
	if (!vshader.compileSourceCode(vsrc))
	{
		qWarning() << __func__ << ": Warnings while compiling vshader: " << vshader.log();
	}
	
	QOpenGLShader fshader(QOpenGLShader::Fragment);
	const char *fsrc =
	        "varying mediump vec2 texc;\n"
	        "uniform sampler2D tex;\n"
	        "void main(void)\n"
	        "{\n"
	        "	gl_FragColor = texture2D(tex, texc);\n"
	        "}\n";
	if (!fshader.compileSourceCode(fsrc))
	{
		qWarning() << __func__ << ": Warnings while compiling fshader: " << fshader.log();
	}
	blitShader.prog = new QOpenGLShaderProgram(QOpenGLContext::currentContext());
	blitShader.prog->addShader(&vshader);
	blitShader.prog->addShader(&fshader);
	if (!blitShader.prog->link())
	{
		qWarning() << __func__ << ": Warnings while linking night view: " << nightViewShader.prog->log();
	}
	
	blitShader.texCoord = blitShader.prog->attributeLocation("texCoord");
	blitShader.pos = blitShader.prog->attributeLocation("pos");
	blitShader.texture = blitShader.prog->uniformLocation("tex");
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
		createBlitShader();
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
		// deinit();
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

QSize StelQuickView::surfaceSize() const
{
	float ratio = devicePixelRatio();
	return QSize(width() * ratio, height() * ratio);
}

QOpenGLFramebufferObject* StelQuickView::createFbo()
{
	QOpenGLFunctions funcs(QOpenGLContext::currentContext());
	bool npot = funcs.hasOpenGLFeature(QOpenGLFunctions::NPOTTextures);
	QSize fboSize = npot ? size() : QSize(nextPowerOf2(surfaceSize().width()), nextPowerOf2(surfaceSize().height()));
	return new QOpenGLFramebufferObject(fboSize, QOpenGLFramebufferObject::CombinedDepthStencil);
}

void StelQuickView::paint()
{
	if (stelApp==NULL)
		return;
	static double lastPaint = -1.;
	double newTime = StelApp::getTotalRunTime();
	if (lastPaint<0)
		lastPaint = newTime-0.01;
	stelApp->update(newTime-lastPaint);
	lastPaint = newTime;

	if (nightMode && !finalFbo)
	{
		if (!nightViewShader.prog) createNightViewShader();
		finalFbo.reset(createFbo());
		setRenderTarget(finalFbo.data()->handle(), surfaceSize());
	}
	if (finalFbo && !nightMode)
	{
		finalFbo.reset();
		setRenderTarget(NULL);
	}
	if (!nightMode && !finalFbo && renderTarget()!=0)
	{
		setRenderTarget(NULL);
	}

	if (finalFbo) finalFbo->bind();
	stelApp->draw();
	// On my iPhone 5, with Qt 5.5, stellarium crashes when I rotate the
	// screen.  Adding a glFush fixes the problem.  I need to test again
	// when there is a new version of Qt.
#ifdef Q_OS_IOS
	glFlush();
#endif
}

void StelQuickView::blit(QOpenGLFramebufferObject* fbo, Shader* shader)
{
	glViewport(0, 0, surfaceSize().width(), surfaceSize().height());
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	float texSize[2] = {
	    (float)surfaceSize().width() / fbo->size().width(),
	    (float)surfaceSize().height() / fbo->size().height()
	};
	const GLfloat vertices[][4] = {
	    {-1, -1, 0, 0},
	    {+1, -1, texSize[0], 0},
	    {-1, +1, 0, texSize[1]},
	    {+1, +1, texSize[0], texSize[1]}
	};
	const int stride = sizeof(*vertices);
	shader->prog->bind();
	shader->prog->setAttributeArray(shader->pos, GL_FLOAT, &vertices[0][0], 2, stride);
	shader->prog->enableAttributeArray(shader->pos);
	shader->prog->setAttributeArray(shader->texCoord, GL_FLOAT, &vertices[0][2], 2, stride);
	shader->prog->enableAttributeArray(shader->texCoord);
	shader->prog->setUniformValue(shader->texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo->texture());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	shader->prog->disableAttributeArray(shader->pos);
	shader->prog->disableAttributeArray(shader->texCoord);
	shader->prog->release();
}

void StelQuickView::afterRendering()
{
	if (finalFbo)
	{
		finalFbo->release();
		blit(finalFbo.data(), &nightViewShader);
	}
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

void StelQuickView::resizeEvent(QResizeEvent* event)
{
	// Ensure that the buffer is discarded.
	if (finalFbo)
	{
		finalFbo.reset();
	}
	
	QQuickView::resizeEvent(event);
}

