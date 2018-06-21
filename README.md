# Stellarium-android
A version of Stellarium for android

Original readme document has been renamed to README

Original source code: http://stelladata.noctua-software.com/stellarium-mobile/stellarium-mobile-1.24.tar.gz 
(This link is dead now...)

Assets from https://code.launchpad.net/stellarium/

Copyright: Original Stellarium dev team

## Prerequisites
Qt-for-android (preferably qt 5.6 or higher)

Since qt does not provide a redistributable version of qt-for-android, you
may have to build it for yourself. The building procedure is already 
detaily documented in the document of qt. Please refer to qt.io for info.

Python PIL

If you want to create low-resolution resources, please run 
create-mobile-data.sh. It will call PIL so you might want to have it installed 
firsthand.

## Installation
The original INSTALL file nearly states nothing useful, but some links are 
still provided within that document. Simply download and build. I have 
included all assets in this respository as I might modify them in the 
future.

## Change Log
### June 21, 2018
Android 8.0 support
Runtime permission request
### May 4, 2017
New feature: magnetic declination correction
### February 20, 2017
Minor bug fix
