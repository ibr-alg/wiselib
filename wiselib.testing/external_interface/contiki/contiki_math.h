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
#ifndef CONNECTOR_CONTIKI_MATH_H
#define CONNECTOR_CONTIKI_MATH_H

#include "external_interface/contiki/contiki_os.h"
#include "util/standalone_math.h"
#include <stdint.h>
#include <float.h>

double sqrt(double value)
{ return wiselib::StandaloneMath<wiselib::ContikiOsModel>::sqrt(value); }

double fabs(double value)
{ return wiselib::StandaloneMath<wiselib::ContikiOsModel>::fabs(value); }

inline double sin(double value)
{ return wiselib::StandaloneMath<wiselib::ContikiOsModel>::sin(value); }

inline double cos(double value)
{ return wiselib::StandaloneMath<wiselib::ContikiOsModel>::cos(value); }

#endif
