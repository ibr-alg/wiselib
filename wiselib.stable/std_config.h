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
#ifndef __CONFIG_H
#define __CONFIG_H

// --------------------------------------------------------------------------
// --------------------- Algorithms -----------------------------------------
// --------------------------------------------------------------------------

// ---------------- Routing -------------------------------------------------
// #define ROUTING_DSDV_DEBUG
// #define ROUTING_DSR_DEBUG
// #define ROUTING_TREE_DEBUG

// --------------------------------------------------------------------------
// --------------------- Util -----------------------------------------------
// --------------------------------------------------------------------------

// ---------------- Base Classes --------------------------------------------
#define RADIO_BASE_MAX_RECEIVERS 10
#define UART_BASE_MAX_RECEIVERS 10
#define STATE_CALLBACK_BASE_MAX_RECEIVERS 10
#define SENSOR_CALLBACK_BASE_MAX_RECEIVERS 10

// ---------------- pSTL ----------------------------------------------------

#endif
