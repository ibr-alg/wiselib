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
 **                                                                       **
 ** Author: Christoph Knecht, University of Bern 2010                     **
 ***************************************************************************/
#ifndef DIFFIE_HELLMAN_CONFIG_H
#define DIFFIE_HELLMAN_CONFIG_H

#include "config.h"

// defines the diffie hellman key length
#define KEY_LENGTH 4096
// amount of Neighbours (sets the size of the neighbours array)
#define NEIGHBOUR_COUNT 8
// identifier list is broadcasted in the time interval [MIN_WORK_TIME, MIN_WORK_TIME+60] seconds
#define MIN_WORK_TIME 60
// define total amount of nodes
#define TOTAL_NODES RADIO_BASE_MAX_RECEIVERS

#endif
