
TARGET = stellarium
VERSION = 0.12.3
MOBILE_VERSION = 1.29.6_m_1
INCLUDEPATH += \
	. src/ src/core src/core/modules src/core/external \
	src/core/external/glues_stel/source src/core/external/kfilter \
	src/core/external/gsatellite \
	src/core/external/glues_stel/source src/core/external/qtcompress \
        src/core/planetsephems src/scripting src/core/external/GeographicLib/include

TEMPLATE = app
QT += network gui sensors qml quick positioning
android {
	QT += androidextras
}
CONFIG += qt thread
#CONFIG += release

LIBS += -lz

QMAKE_CFLAGS += -Wno-unused-parameter

CONFIG(debug, debug|release) {

} else {
  DEFINES += NDEBUG QT_NO_DEBUG
  message(Building release version)
}

RESOURCES += data/mainRes.qrc

QMAKE_CXXFLAGS_RELEASE += -Ofast

android {
	QT += androidextras
	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
	ANDROID_PACKAGE = com.noctuasoftware.stellarium
        ANDROID_MINIMUM_VERSION = 16
        ANDROID_TARGET_VERSION = 26
	ANDROID_APP_NAME = Stellarium Mobile

	data_dir.source = mobileData/data
	data_dir.path = data
	landscapes_dir.source = mobileData/landscapes
	landscapes_dir.path = landscapes
	nebulae_dir.source = mobileData/nebulae
	nebulae_dir.path = nebulae
	skycultures_dir.source = mobileData/skycultures
	skycultures_dir.path = skycultures
	stars_dir.source = mobileData/stars
	stars_dir.path = stars
	textures_dir.source = mobileData/textures
	textures_dir.path = textures
	translations_dir.source = mobileData/translations
	translations_dir.path = translations
	DEPLOYMENTFOLDERS = data_dir textures_dir landscapes_dir nebulae_dir skycultures_dir stars_dir translations_dir

	include(deployment.pri)
}

ios {
	data_dir.files = $$PWD/mobileData/data
	landscapes_dir.files = $$PWD/mobileData/landscapes
	nebulae_dir.files = $$PWD/mobileData/nebulae
	skycultures_dir.files = $$PWD/mobileData/skycultures
	stars_dir.files = $$PWD/mobileData/stars
	textures_dir.files = $$PWD/mobileData/textures
	translations_dir.files = $$PWD/mobileData/translations
	QMAKE_BUNDLE_DATA += data_dir landscapes_dir nebulae_dir skycultures_dir stars_dir textures_dir translations_dir
	QMAKE_INFO_PLIST = ios/Info.plist
	QTPLUGIN += qtsensors_ios
	OBJECTIVE_SOURCES += src/core/modules/SensorsMgr.mm
	ios_icon.files = $$files($$PWD/ios/AppIcon*.png)
	ios_launch_images.files = $$PWD/ios/Launch.xib $$files($$PWD/ios/LaunchImage*.png)
	ios_itunes_icon.files = $$files($$PWD/ios/iTunesArtwork*)
	QMAKE_BUNDLE_DATA += ios_icon ios_launch_images ios_itunes_icon
}

HEADERS += \
	src/config.h \
	src/translations.h \
	src/CLIProcessor.hpp \
	src/StelAndroid.hpp \
	src/StelLogger.hpp \
	src/StelMainView.hpp \


SOURCES += \
	src/CLIProcessor.cpp \
	src/main.cpp \
	src/StelLogger.cpp \
	src/StelMainView.cpp \


android {
	HEADERS += \
		src/StelAndroid.hpp
	SOURCES += \
		src/StelAndroid.cpp
}

DEFINES += DISABLE_SCRIPTING
DEFINES += ENABLE_NLS
DEFINES += PACKAGE_VERSION_NOSTR=$${VERSION}
DEFINES += MOBILE_GUI_VERSION_NOSTR=$${MOBILE_VERSION}
DEFINES += INSTALL_DATADIR_NOSTR=

#QMAKE_CFLAGS += -include src/config.h
#QMAKE_CXXFLAGS += -include src/config.h

contains(QT, quick) {
	DEFINES += USE_QUICKVIEW
	SOURCES += src/StelQuickView.cpp src/StelQuickStelItem.cpp
	HEADERS += src/StelQuickView.hpp src/StelQuickStelItem.hpp
}

# Core files
HEADERS += \
	src/CLIProcessor.hpp \
	src/StelLogger.hpp \
	src/core/MultiLevelJsonBase.hpp \
	src/core/OctahedronPolygon.hpp \
	src/core/RefractionExtinction.hpp \
	src/core/SimbadSearcher.hpp \
	src/core/SphericMirrorCalculator.hpp \
	src/core/StelActionMgr.hpp \
	src/core/StelApp.hpp \
	src/core/StelAudioMgr.hpp \
	src/core/StelCore.hpp \
	src/core/StelFader.hpp \
	src/core/StelFileMgr.hpp \
	src/core/StelGeodesicGrid.hpp \
	src/core/StelGuiBase.hpp \
	src/core/StelIniParser.hpp \
	src/core/StelJsonParser.hpp \
	src/core/StelLocaleMgr.hpp \
	src/core/StelLocation.hpp \
	src/core/StelLocationMgr.hpp \
	src/core/StelModule.hpp \
	src/core/StelModuleMgr.hpp \
	src/core/StelMovementMgr.hpp \
	src/core/StelObject.hpp \
	src/core/StelObjectMgr.hpp \
	src/core/StelObjectModule.hpp \
	src/core/StelObjectType.hpp \
	src/core/StelObserver.hpp \
	src/core/StelPainter.hpp \
	src/core/StelPluginInterface.hpp \
	src/core/StelProjectorClasses.hpp \
	src/core/StelProjector.hpp \
	src/core/StelProjectorType.hpp \
	src/core/StelProgressController.hpp \
	src/core/StelRegionObject.hpp \
	src/core/StelSkyCultureMgr.hpp \
	src/core/StelSkyDrawer.hpp \
	src/core/StelSkyImageTile.hpp \
	src/core/StelSkyLayer.hpp \
	src/core/StelSkyLayerMgr.hpp \
	src/core/StelSkyPolygon.hpp \
	src/core/StelSphereGeometry.hpp \
	src/core/StelSphericalIndex.hpp \
	src/core/StelTexture.hpp \
	src/core/StelTextureMgr.hpp \
	src/core/StelTextureTypes.hpp \
	src/core/StelToneReproducer.hpp \
	src/core/StelTranslator.hpp \
	src/core/StelUtils.hpp \
	src/core/StelVertexArray.hpp \
	src/core/StelVideoMgr.hpp \
	src/core/StelViewportEffect.hpp \
	src/core/TrailGroup.hpp \
	src/core/VecMath.hpp \
	src/core/modules/Atmosphere.hpp \
	src/core/modules/Comet.hpp \
	src/core/modules/Constellation.hpp \
	src/core/modules/ConstellationMgr.hpp \
	src/core/modules/GPSMgr.hpp \
	src/core/modules/GridLinesMgr.hpp \
	src/core/modules/LabelMgr.hpp \
	src/core/modules/Landscape.hpp \
	src/core/modules/LandscapeMgr.hpp \
	src/core/modules/Meteor.hpp \
	src/core/modules/MeteorMgr.hpp \
	src/core/modules/MilkyWay.hpp \
	src/core/modules/MinorPlanet.hpp \
	src/core/modules/Nebula.hpp \
	src/core/modules/NebulaMgr.hpp \
	src/core/modules/Orbit.hpp \
	src/core/modules/Planet.hpp \
	src/core/modules/Satellites.hpp \
	src/core/modules/Satellite.hpp \
	src/core/modules/Skybright.hpp \
	src/core/modules/Skylight.hpp \
	src/core/modules/SensorsMgr.hpp \
	src/core/modules/SolarSystem.hpp \
	src/core/modules/Solve.hpp \
	src/core/modules/Star.hpp \
	src/core/modules/StarMgr.hpp \
	src/core/modules/StarWrapper.hpp \
	src/core/modules/ZoneArray.hpp \
	src/core/modules/ZoneData.hpp \
	src/core/external/glues_stel/source/glues_error.h \
	src/core/external/glues_stel/source/glues.h \
	src/core/external/glues_stel/source/libtess/dict.h \
	src/core/external/glues_stel/source/libtess/dict-list.h \
	src/core/external/glues_stel/source/libtess/geom.h \
	src/core/external/glues_stel/source/libtess/memalloc.h \
	src/core/external/glues_stel/source/libtess/mesh.h \
	src/core/external/glues_stel/source/libtess/normal.h \
	src/core/external/glues_stel/source/libtess/priorityq.h \
	src/core/external/glues_stel/source/libtess/priorityq-heap.h \
	src/core/external/glues_stel/source/libtess/priorityq-sort.h \
	src/core/external/glues_stel/source/libtess/render.h \
	src/core/external/glues_stel/source/libtess/sweep.h \
	src/core/external/glues_stel/source/libtess/tess.h \
	src/core/external/glues_stel/source/libtess/tessmono.h \
	src/core/external/qtcompress/qzipreader.h \
	src/core/external/qtcompress/qzipwriter.h \
	src/core/planetsephems/calc_interpolated_elements.h \
	src/core/planetsephems/elliptic_to_rectangular.h \
	src/core/planetsephems/elp82b.h \
	src/core/planetsephems/gust86.h \
	src/core/planetsephems/l1.h \
	src/core/planetsephems/marssat.h \
	src/core/planetsephems/pluto.h \
	src/core/planetsephems/sideral_time.h \
	src/core/planetsephems/stellplanet.h \
	src/core/planetsephems/tass17.h \
        src/core/planetsephems/vsop87.h \
        src/core/external/gsatellite/gException.hpp \
        src/core/external/gsatellite/gSatTEME.hpp \
        src/core/external/gsatellite/gTime.hpp \
        src/core/external/gsatellite/gVector.hpp \
        src/core/external/gsatellite/gVectorTempl.hpp \
        src/core/external/gsatellite/sgp4ext.h \
        src/core/external/gsatellite/sgp4io.h \
        src/core/external/gsatellite/sgp4unit.h \
        src/core/external/gsatellite/stdsat.h \
        src/core/external/GeographicLib/include/GeographicLib/Accumulator.hpp \
    src/core/external/GeographicLib/include/GeographicLib/AlbersEqualArea.hpp \
    src/core/external/GeographicLib/include/GeographicLib/AzimuthalEquidistant.hpp \
    src/core/external/GeographicLib/include/GeographicLib/CassiniSoldner.hpp \
    src/core/external/GeographicLib/include/GeographicLib/CircularEngine.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Constants.hpp \
    src/core/external/GeographicLib/include/GeographicLib/DMS.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Ellipsoid.hpp \
    src/core/external/GeographicLib/include/GeographicLib/EllipticFunction.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GARS.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Geocentric.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GeoCoords.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Geodesic.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GeodesicExact.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GeodesicLine.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GeodesicLineExact.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Geohash.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Geoid.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Georef.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Gnomonic.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GravityCircle.hpp \
    src/core/external/GeographicLib/include/GeographicLib/GravityModel.hpp \
    src/core/external/GeographicLib/include/GeographicLib/LambertConformalConic.hpp \
    src/core/external/GeographicLib/include/GeographicLib/LocalCartesian.hpp \
    src/core/external/GeographicLib/include/GeographicLib/MagneticCircle.hpp \
    src/core/external/GeographicLib/include/GeographicLib/MagneticModel.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Math.hpp \
    src/core/external/GeographicLib/include/GeographicLib/MGRS.hpp \
    src/core/external/GeographicLib/include/GeographicLib/NearestNeighbor.hpp \
    src/core/external/GeographicLib/include/GeographicLib/NormalGravity.hpp \
    src/core/external/GeographicLib/include/GeographicLib/OSGB.hpp \
    src/core/external/GeographicLib/include/GeographicLib/PolarStereographic.hpp \
    src/core/external/GeographicLib/include/GeographicLib/PolygonArea.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Rhumb.hpp \
    src/core/external/GeographicLib/include/GeographicLib/SphericalEngine.hpp \
    src/core/external/GeographicLib/include/GeographicLib/SphericalHarmonic.hpp \
    src/core/external/GeographicLib/include/GeographicLib/SphericalHarmonic1.hpp \
    src/core/external/GeographicLib/include/GeographicLib/SphericalHarmonic2.hpp \
    src/core/external/GeographicLib/include/GeographicLib/TransverseMercator.hpp \
    src/core/external/GeographicLib/include/GeographicLib/TransverseMercatorExact.hpp \
    src/core/external/GeographicLib/include/GeographicLib/Utility.hpp \
    src/core/external/GeographicLib/include/GeographicLib/UTMUPS.hpp

SOURCES += \
	src/core/MultiLevelJsonBase.cpp \
	src/core/OctahedronPolygon.cpp \
	src/core/RefractionExtinction.cpp \
	src/core/SimbadSearcher.cpp \
	src/core/SphericMirrorCalculator.cpp \
	src/core/StelActionMgr.cpp \
	src/core/StelApp.cpp \
	src/core/StelAudioMgr.cpp \
	src/core/StelCore.cpp \
	src/core/StelFileMgr.cpp \
	src/core/StelGeodesicGrid.cpp \
	src/core/StelGuiBase.cpp \
	src/core/StelIniParser.cpp \
	src/core/StelJsonParser.cpp \
	src/core/StelLocaleMgr.cpp \
	src/core/StelLocation.cpp \
	src/core/StelLocationMgr.cpp \
	src/core/StelModule.cpp \
	src/core/StelModuleMgr.cpp \
	src/core/StelMovementMgr.cpp \
	src/core/StelObject.cpp \
	src/core/StelObjectMgr.cpp \
	src/core/StelObjectModule.cpp \
	src/core/StelObserver.cpp \
	src/core/StelPainter.cpp \
	src/core/StelProjectorClasses.cpp \
	src/core/StelProjector.cpp \
	src/core/StelSkyCultureMgr.cpp \
	src/core/StelSkyDrawer.cpp \
	src/core/StelSkyImageTile.cpp \
	src/core/StelSkyLayer.cpp \
	src/core/StelSkyLayerMgr.cpp \
	src/core/StelSkyPolygon.cpp \
	src/core/StelSphereGeometry.cpp \
	src/core/StelSphericalIndex.cpp \
	src/core/StelTexture.cpp \
	src/core/StelTextureMgr.cpp \
	src/core/StelToneReproducer.cpp \
	src/core/StelTranslator.cpp \
	src/core/StelUtils.cpp \
	src/core/StelVertexArray.cpp \
	src/core/StelVideoMgr.cpp \
	src/core/StelViewportEffect.cpp \
	src/core/TrailGroup.cpp \
        src/core/modules/gSatWrapper.cpp \
	src/core/modules/Atmosphere.cpp \
	src/core/modules/Comet.cpp \
	src/core/modules/Constellation.cpp \
	src/core/modules/ConstellationMgr.cpp \
	src/core/modules/GPSMgr.cpp \
	src/core/modules/GridLinesMgr.cpp \
	src/core/modules/LabelMgr.cpp \
	src/core/modules/Landscape.cpp \
	src/core/modules/LandscapeMgr.cpp \
	src/core/modules/Meteor.cpp \
	src/core/modules/MeteorMgr.cpp \
	src/core/modules/MilkyWay.cpp \
	src/core/modules/MinorPlanet.cpp \
	src/core/modules/Nebula.cpp \
	src/core/modules/NebulaMgr.cpp \
	src/core/modules/Orbit.cpp \
	src/core/modules/Planet.cpp \
	src/core/modules/SensorsMgr.cpp \
	src/core/modules/Satellite.cpp \
	src/core/modules/Satellites.cpp \
	src/core/modules/Skybright.cpp \
	src/core/modules/Skylight.cpp \
	src/core/modules/SolarSystem.cpp \
	src/core/modules/Star.cpp \
	src/core/modules/StarMgr.cpp \
	src/core/modules/StarWrapper.cpp \
	src/core/modules/ZoneArray.cpp \
	src/core/external/glues_stel/source/glues_error.c \
	src/core/external/glues_stel/source/libtess/dict.c \
	src/core/external/glues_stel/source/libtess/geom.c \
	src/core/external/glues_stel/source/libtess/memalloc.c \
	src/core/external/glues_stel/source/libtess/mesh.c \
	src/core/external/glues_stel/source/libtess/normal.c \
	src/core/external/glues_stel/source/libtess/priorityq.c \
	src/core/external/glues_stel/source/libtess/render.c \
	src/core/external/glues_stel/source/libtess/sweep.c \
	src/core/external/glues_stel/source/libtess/tess.c \
	src/core/external/glues_stel/source/libtess/tessmono.c \
        src/core/external/gsatellite/gSatTEME.cpp \
        src/core/external/gsatellite/gTime.cpp \
        src/core/external/gsatellite/gTimeSpan.cpp \
        src/core/external/gsatellite/gVector.cpp \
        src/core/external/gsatellite/sgp4ext.cpp \
        src/core/external/gsatellite/sgp4io.cpp \
        src/core/external/gsatellite/sgp4unit.cpp \
	src/core/external/qtcompress/qzip.cpp \
	src/core/planetsephems/calc_interpolated_elements.c \
	src/core/planetsephems/elliptic_to_rectangular.c \
	src/core/planetsephems/elp82b.c \
	src/core/planetsephems/gust86.c \
	src/core/planetsephems/l1.c \
	src/core/planetsephems/marssat.c \
	src/core/planetsephems/pluto.c \
	src/core/planetsephems/sideral_time.c \
	src/core/planetsephems/stellplanet.c \
	src/core/planetsephems/tass17.c \
        src/core/planetsephems/vsop87.c \
    src/core/external/GeographicLib/src/Accumulator.cpp \
    src/core/external/GeographicLib/src/AlbersEqualArea.cpp \
    src/core/external/GeographicLib/src/AzimuthalEquidistant.cpp \
    src/core/external/GeographicLib/src/CassiniSoldner.cpp \
    src/core/external/GeographicLib/src/CircularEngine.cpp \
    src/core/external/GeographicLib/src/DMS.cpp \
    src/core/external/GeographicLib/src/Ellipsoid.cpp \
    src/core/external/GeographicLib/src/EllipticFunction.cpp \
    src/core/external/GeographicLib/src/GARS.cpp \
    src/core/external/GeographicLib/src/Geocentric.cpp \
    src/core/external/GeographicLib/src/GeoCoords.cpp \
    src/core/external/GeographicLib/src/Geodesic.cpp \
    src/core/external/GeographicLib/src/GeodesicExact.cpp \
    src/core/external/GeographicLib/src/GeodesicExactC4.cpp \
    src/core/external/GeographicLib/src/GeodesicLine.cpp \
    src/core/external/GeographicLib/src/GeodesicLineExact.cpp \
    src/core/external/GeographicLib/src/Geohash.cpp \
    src/core/external/GeographicLib/src/Geoid.cpp \
    src/core/external/GeographicLib/src/Georef.cpp \
    src/core/external/GeographicLib/src/Gnomonic.cpp \
    src/core/external/GeographicLib/src/GravityCircle.cpp \
    src/core/external/GeographicLib/src/GravityModel.cpp \
    src/core/external/GeographicLib/src/LambertConformalConic.cpp \
    src/core/external/GeographicLib/src/LocalCartesian.cpp \
    src/core/external/GeographicLib/src/MagneticCircle.cpp \
    src/core/external/GeographicLib/src/MagneticModel.cpp \
    src/core/external/GeographicLib/src/Math.cpp \
    src/core/external/GeographicLib/src/MGRS.cpp \
    src/core/external/GeographicLib/src/NormalGravity.cpp \
    src/core/external/GeographicLib/src/OSGB.cpp \
    src/core/external/GeographicLib/src/PolarStereographic.cpp \
    src/core/external/GeographicLib/src/PolygonArea.cpp \
    src/core/external/GeographicLib/src/Rhumb.cpp \
    src/core/external/GeographicLib/src/SphericalEngine.cpp \
    src/core/external/GeographicLib/src/TransverseMercator.cpp \
    src/core/external/GeographicLib/src/TransverseMercatorExact.cpp \
    src/core/external/GeographicLib/src/Utility.cpp \
    src/core/external/GeographicLib/src/UTMUPS.cpp

OTHER_FILES += \
	data/qml/AboutDialog.qml \
	data/qml/ImageButton.qml \
	data/qml/InfoPanel.qml \
	data/qml/LandscapesDialog.qml \
	data/qml/LocationCityPicker.qml \
	data/qml/LocationDialog.qml \
	data/qml/LocationMap.qml \
	data/qml/main.qml \
	data/qml/QuickBar.qml \
	data/qml/SearchInput.qml \
	data/qml/Splash.qml \
	data/qml/StarloreDialog.qml \
	data/qml/SettingsPanel.qml \
	data/qml/StelButton.qml \
	data/qml/StelCheckBox.qml \
	data/qml/StelDialog.qml \
	data/qml/StelListItem.qml \
	data/qml/StelMessage.qml \
	data/qml/StelSpinBox.qml \
	data/qml/StelTimeSpinBox.qml \
	data/qml/TimeBar.qml \
	data/qml/TimeDialog.qml \
	data/qml/TouchPinchArea.qml \
	data/qml/AdvancedDialog.qml \
	data/qml/AnglePicker.qml \
	data/qml/ValuePicker.qml \

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../openssl-1.1.0h/libcrypto.so \
        $$PWD/../openssl-1.1.0h/libssl.so
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew \
    android/gradlew.bat \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/drawable-xhdpi/icon.png \
    android/res/drawable/icon.png \
    android/res/values/libs.xml \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/res/values/strings.xml \
    android/src/com/noctuasoftware/stellarium/Stellarium.java

android: include($${ANDROID_SDK_ROOT}/android_openssl/openssl.pri)

ANDROID_EXTRA_LIBS = \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/arm/libcrypto_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/arm/libssl_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/arm64/libcrypto_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/arm64/libssl_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/x86/libcrypto_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/x86/libssl_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/x86_64/libcrypto_1_1.so \
$${ANDROID_SDK_ROOT}/android_openssl/no-asm/latest/x86_64/libssl_1_1.so

