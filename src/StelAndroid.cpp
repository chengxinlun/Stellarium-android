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

#include "StelAndroid.hpp"

#include <jni.h>
#include <QDebug>
#include <QStringList>
#include <QRegExp>
#include <QDir>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>

QAndroidJniObject* StelAndroid::getStellarium()
{
	static QAndroidJniObject* stellarium = NULL;
	if (!stellarium)
	{
		QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
		            "org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
		stellarium = new QAndroidJniObject("com/noctuasoftware/stellarium/Stellarium", "(Landroid/app/Activity;)V",
		                                   activity.object<jobject>());
	}
	return stellarium;
}

float StelAndroid::getScreenDensity()
{
	return getStellarium()->callMethod<float>("getScreenDensity", "()F");
}

int StelAndroid::getOrientation()
{
	return getStellarium()->callMethod<int>("getRotation", "()I");
}

void StelAndroid::setCanPause(bool value)
{
	getStellarium()->callMethod<void>("setCanPause", "(Z)V", value);
}

QString StelAndroid::getModel()
{
	return getStellarium()->callObjectMethod<jstring>("getModel").toString();
}
