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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_UTIL_SHARED_DATA_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_UTIL_SHARED_DATA_H

#include "algorithms/localization/distance_based/math/vec.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include <float.h>

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Neighborhood_P,
            typename NeighborInfoList_P,
            typename NodeSet_P,
            typename NodeList_P,
            typename DistanceMap_P,
            typename LocationMap_P,
            typename Arithmatic_P >
   class LocalizationSharedData
   {
      public:
         typedef OsModel_P OsModel;
         typedef Radio_P Radio;
         typedef Clock_P Clock;

         typedef Neighborhood_P Neighborhood;
         typedef typename Neighborhood::NeighborInfo NeighborInfo;

         typedef NeighborInfoList_P NeighborInfoList;
         typedef typename NeighborInfoList::iterator NeighborInfoListIterator;

         typedef NodeList_P NodeList;
         typedef NodeSet_P NodeSet;
         typedef DistanceMap_P DistanceMap;
         typedef LocationMap_P LocationMap;
         typedef Arithmatic_P Arithmatic;

         typedef LocalizationSharedData<OsModel, Radio, Clock, NeighborInfo, NeighborInfoList, NodeSet, NodeList, DistanceMap, LocationMap, Arithmatic> self_type;

         typedef typename Radio::node_id_t node_id_t;
         typedef typename Radio::size_t size_t;
         typedef typename Radio::block_data_t block_data_t;

         typedef typename Clock_P::time_t time_t;

         typedef LocalizationLocalCoordinateSystem<OsModel, node_id_t, Neighborhood, LocationMap, Arithmatic> LocalCoordinateSystem;
         // -----------------------------------------------------------------
         LocalizationSharedData()
            : anchor_      ( false ),
               confidence_ ( 0.1 ),
               position_   ( UNKNOWN_POSITION ),
               floodlimit_ ( 5 ),
               communication_range_ ( 15 ),
               check_residue_ ( true )
         {}
         // -----------------------------------------------------------------
         void set_position( const Vec<Arithmatic_P>& position )
         { position_ = position; }
         // -----------------------------------------------------------------
         const Vec<Arithmatic_P>& position( void )
         { return position_; }
         // -----------------------------------------------------------------
         void set_anchor( bool anchor )
         { anchor_ = anchor; }
         // -----------------------------------------------------------------
         bool is_anchor( void )
         { return anchor_; }
         // -----------------------------------------------------------------
         void set_confidence( Arithmatic confidence )
         { confidence_ = confidence; }
         // -----------------------------------------------------------------
         Arithmatic confidence( void )
         { return confidence_; }
         // -----------------------------------------------------------------
         void set_idle_time( time_t idle_time )
         { idle_time_ = idle_time; }
         // -----------------------------------------------------------------
         time_t idle_time( void )
         { return idle_time_; }
         // -----------------------------------------------------------------
         void set_floodlimit( unsigned int floodlimit )
         { floodlimit_ = floodlimit; }
         // -----------------------------------------------------------------
         unsigned int floodlimit( void )
         { return floodlimit_; }
         // -----------------------------------------------------------------
         void set_communication_range( int communication_range )
         { communication_range_ = communication_range; }
         // -----------------------------------------------------------------
         int communication_range( void )
         { return communication_range_; }
         // --------------------------------------------------------------------
         void set_check_residue( bool check_residue )
         { check_residue_ = check_residue; }
         // -----------------------------------------------------------------
         bool check_residue( void )
         { return check_residue_; }
         // -----------------------------------------------------------------
         Neighborhood& neighborhood( void )
         { return neighborhood_; }
         // -----------------------------------------------------------------
         LocalCoordinateSystem& local_coord_sys( void )
         { return local_coord_sys_; }

         void reset_neighborhood_( void )
         { neighborhood_.get_neighborhood().clear(); }

      private:
         bool anchor_;
         Arithmatic confidence_;
         Vec<Arithmatic_P> position_;
         unsigned int floodlimit_;
         int communication_range_;
         bool check_residue_;
         time_t idle_time_;
         Neighborhood neighborhood_;
         LocalCoordinateSystem local_coord_sys_;
   };

}// namespace wiselib
#endif
