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

// This file is automatically added before any other included files
// by the -include compilation flags.

// Create strings macros from the "NOSTR" versions.
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#ifdef PACKAGE_VERSION_NOSTR
    #define PACKAGE_VERSION STRINGIFY(PACKAGE_VERSION_NOSTR)
#endif

#ifdef MOBILE_GUI_VERSION_NOSTR
    #define MOBILE_GUI_VERSION STRINGIFY(MOBILE_GUI_VERSION_NOSTR)
#endif

#ifdef INSTALL_DATADIR_NOSTR
    #define INSTALL_DATADIR STRINGIFY(INSTALL_DATADIR_NOSTR)
#endif

// On ios GL_DOUBLE may not be defined.
#ifndef GL_DOUBLE
	#define GL_DOUBLE 0x140A
#endif

#define GL_GLEXT_PROTOTYPES
#include <qopengl.h>
