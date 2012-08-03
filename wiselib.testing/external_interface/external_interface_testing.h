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
#ifndef __EXTERNAL_INTERFACE_FIRMWARE_TESTING_H__
#define __EXTERNAL_INTERFACE_FIRMWARE_TESTING_H__

#include "external_interface/facet_provider.h"
#include "external_interface/wiselib_application.h"

#ifdef ISENSE
#include "external_interface/isense/isense_os.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_extended_txradio.h"
#include "external_interface/isense/isense_debug.h"
#include "external_interface/isense/isense_extended_time.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_timer.h"
#include "external_interface/isense/isense_position.h"
#include "external_interface/isense/isense_clock.h"
#include "external_interface/isense/isense_com_uart.h"
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
#include "external_interface/shawn/shawn_clock.h"
#include "external_interface/shawn/shawn_rand.h"
#include "external_interface/shawn/shawn_position.h"
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

#ifdef PC
#include "external_interface/pc/com_isense_packet.h"
#include "external_interface/pc/com_isense_radio.h"
#include "external_interface/pc/pc_clock.h"
#include "external_interface/pc/pc_com_uart_file.h"
#include "external_interface/pc/pc_debug.h"
#include "external_interface/pc/pc_facet_provider.h"
#include "external_interface/pc/pc_os_model.h"
#include "external_interface/pc/pc_rand.h"
#include "external_interface/pc/pc_timer.h"
#include "external_interface/pc/pc_wiselib_application.h"
#endif

#ifdef TRISOS
#include "external_interface/trisos/trisos_os.h"
#include "external_interface/trisos/trisos_radio.h"
#include "external_interface/trisos/trisos_timer.h"
#include "external_interface/trisos/trisos_debug.h"
#include "external_interface/trisos/trisos_facet_provider.h"
#include "external_interface/trisos/trisos_clock.h"
#endif

#ifdef IOS
#include "external_interface/ios/com_testbed_radio.h"
#include "external_interface/ios/com_cocos_radio.h"
#include "external_interface/ios/ios_system.h"
#include "external_interface/ios/ios_model.h"
#include "external_interface/ios/ios_debug.h"
//#include "external_interface/ios/ios_radio.h"
#include "external_interface/ios/ios_facet_provider.h"
#include "external_interface/ios/ios_timer.h"
#endif

#ifdef ARDUINO
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#endif

#endif
