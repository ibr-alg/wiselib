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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_UTIL_DEFUTILS_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_UTIL_DEFUTILS_H

#include "algorithms/localization/distance_based/math/vec.h"
#include <limits.h>
#include <float.h>
#include <math.h>

#ifndef M_PI
   #define M_PI 3.14159265358
#endif

namespace wiselib
{
#ifdef ARITHMATIC

   const fixed UNKNOWN_DISTANCE = -999999;
   const Vec<fixed> UNKNOWN_POSITION = Vec<fixed>( -999999, -999999, -999999 );
   const fixed UNKNOWN_ANGLE = -999999;
   const fixed UNKNOWN_AVG_HOP_DIST = 999999;
   const int UNKNOWN_HOP_CNT = INT_MAX;

   const char* SEL_NODE_TAG_NAME = "LOCALIZATION_SELECTED_NODE_TAG";
#else
   const double UNKNOWN_DISTANCE = DBL_MIN;
   const Vec<double> UNKNOWN_POSITION = Vec<double>( DBL_MIN, DBL_MIN, DBL_MIN );
   const double UNKNOWN_ANGLE = DBL_MIN;
   const double UNKNOWN_AVG_HOP_DIST = DBL_MAX;
   const int UNKNOWN_HOP_CNT = INT_MAX;

   const char* SEL_NODE_TAG_NAME = "LOCALIZATION_SELECTED_NODE_TAG";

//    typedef std::map<int, const LocalizationLocalCoordinateSystem*> LCSMap;
//    typedef LCSMap::iterator LCSMapIterator;
//    typedef LCSMap::const_iterator ConstLCSMapIterator;

//    typedef std::map<int, LocalizationNeighborInfo*> NeighborInfoMap;
//    typedef NeighborInfoMap::const_iterator ConstNeighborhoodIterator;
//    typedef NeighborInfoMap::iterator NeighborhoodIterator;
// 
//    typedef std::list<const LocalizationNeighborInfo*> NeighborInfoList;
//    typedef NeighborInfoList::const_iterator ConstNeighborInfoListIterator;
//    typedef NeighborInfoList::iterator NeighborInfoListIterator;
#endif
}// namespace wiselib
#endif
