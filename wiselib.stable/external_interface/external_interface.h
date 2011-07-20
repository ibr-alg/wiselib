/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/
#ifndef __EXTERNAL_INTERFACE_FIRMWARE_H__
#define __EXTERNAL_INTERFACE_FIRMWARE_H__


#ifndef WISELIB_BUILD_ONLY_STABLE
#include "external_interface/external_interface_testing.h"
#endif


#include "external_interface/facet_provider.h"
#include "external_interface/wiselib_application.h"

#ifdef ISENSE
#include "external_interface/isense/isense_os.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_timer.h"
#include "external_interface/isense/isense_debug.h"
#include "external_interface/isense/isense_types.h"
#include "external_interface/isense/isense_facet_provider.h"
#include "external_interface/isense/isense_wiselib_application.h"
#endif

#ifdef __SCATTERWEB__
#include "external_interface/scw/scw_os.h"
#include "external_interface/scw/scw_radio.h"
#include "external_interface/scw/scw_timer.h"
#include "external_interface/scw/scw_debug.h"
#endif

#ifdef SHAWN
#include "external_interface/shawn/shawn_os.h"
#include "external_interface/shawn/shawn_radio.h"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_debug.h"
#include "external_interface/shawn/shawn_types.h"
#include "external_interface/shawn/shawn_facet_provider.h"
#include "external_interface/shawn/shawn_wiselib_application.h"
#endif

#ifdef CONTIKI
#include "external_interface/contiki/contiki_os.h"
#include "external_interface/contiki/contiki_radio.h"
#include "external_interface/contiki/contiki_timer.h"
#include "external_interface/contiki/contiki_debug.h"
#include "external_interface/contiki/contiki_facet_provider.h"
#endif

#ifdef WISELIB_OSA
#include "external_interface/osa/osa_os.h"
#include "external_interface/osa/osa_radio.h"
#include "external_interface/osa/osa_timer.h"
#include "external_interface/osa/osa_debug.h"
#endif

#ifdef LORIEN
#include "external_interface/lorien/lorien_os.h"
#include "external_interface/lorien/lorien_radio.h"
#include "external_interface/lorien/lorien_timer.h"
#include "external_interface/lorien/lorien_debug.h"
#include "external_interface/lorien/lorien_facet_provider.h"
#include "external_interface/lorien/lorien_wiselib_application.h"
#endif

#ifdef LORIEN_COMPONENT_CONSTRUCTION
#include "external_interface/lorien/lorien_os_component.h"
#endif

#ifdef TINYOS
#include "external_interface/tinyos/tinyos_os.h"
#include "external_interface/tinyos/tinyos_radio.h"
#include "external_interface/tinyos/tinyos_timer.h"
#include "external_interface/tinyos/tinyos_debug.h"
#include "external_interface/tinyos/tinyos_facet_provider.h"
#include "external_interface/tinyos/tinyos_wiselib_application.h"
#endif

#ifdef FEUERWARE
#include "external_interface/feuerwhere/feuerwhere_os.h"
#include "external_interface/feuerwhere/feuerwhere_timer.h"
#include "external_interface/feuerwhere/feuerwhere_cc1100_radio.h"
#include "external_interface/feuerwhere/feuerwhere_debug.h"
#include "external_interface/feuerwhere/feuerwhere_types.h"
#endif

#endif
