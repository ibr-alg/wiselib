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

/*
 * File:   configuration.h
 * Author: Oikonomou, Amaxilatis
 *
 *
 */


#define MAX_PG_PAYLOAD 30

#define IMIN 1000 //change based on ND_RELAX_MILLIS
#define IMAX 3000
#define PERIOD 1000
#define I_STEP 100


#ifndef SHAWN        
typedef uint8_t lqi_t;
#else
typedef uint16_t lqi_t;
#endif


//#define FIX_K

#define P -1
 
