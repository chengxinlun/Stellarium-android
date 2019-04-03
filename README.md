# Stellarium-android
A modded version of Stellarium-Mobile

Stellarium kernel version 0.12.3

Mobile platform specific source code http://stelladata.noctua-software.com/stellarium-mobile/stellarium-mobile-1.29.tar.gz 
Assets from https://code.launchpad.net/stellarium/
Note that changes have been made to the source code and assets (mainly use uncompressed PC version assets instead of compressed ones)

Copyright: Original Stellarium dev team

## Features
* Extend default star catalog limiting magnitude to 13.5
* Allow custom configuration files and catalogs (Put them in ```/sdcard/stellarium```. Same file structure to ```mobileData/data```)
* Tweak UI size for better handling on mobile
* Correct for magnetic declination angle (Magnetic North is not real North)
* Higher quality texture compared to official Stellarium-Mobile

## Prerequisites
* Qt-for-android (Known to work on qt-5.11 and 5.12)

Please note that for version ealier than 5.10, API for permission models for Android 6.0+ were not correctly implemented in Qt.
Notably, Qt did not provide any API for requesting ```read/write to external storage``` permission. This permission is essential
to the application.

## Building APK
Set up android SDK and NDK, and environment in QtCreator.
Open ```stellarium.pro``` in QtCreator.
Push the Build button.

If the building process runs into problems while compiling ```dummy.cpp```, simply copy the file to build directory and rename it ```moc_predefs.h```.

## Localization guide
TODO

## Change Log
### April 3, 2019
Update to mobile source code to 1.29.6
Clean up unused files
### June 21, 2018
Android 8.0 support
Runtime permission request
### May 4, 2017
New feature: magnetic declination correction
### February 20, 2017
Minor bug fix
