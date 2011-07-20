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
#ifndef __INTERNAL_INTERFACE_ISENSE_DISTANCE__
#define __INTERNAL_INTERFACE_ISENSE_DISTANCE__

#include "internal_interface/routing_table/routing_table_static_array.h"
#include "external_interface/isense/isense_os.h"

#include "external_interface/isense/isense_types.h"
//#include "util/delegates/delegate.hpp"
//#include <isense/os.h>
//#include <isense/radio.h>
//#include <isense/dispatcher.h>

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::TxRadio,
            int TABLE_SIZE = 20>
   class iSenseDistanceModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef isense::Os iSenseOs;

      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::ExtendedData ExtendedData;

      typedef iSenseDistanceModel<OsModel, Radio, TABLE_SIZE> self_type;
      typedef self_type* self_pointer_t;

      typedef double distance_t;
      typedef StaticArrayRoutingTable<OsModel, Radio, TABLE_SIZE, distance_t> DistanceMap;
      typedef typename DistanceMap::iterator DistanceMapIterator;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      iSenseDistanceModel( iSenseOs& os )
         : os_( os )
      {}
      // --------------------------------------------------------------------
      int init( Radio& radio)
      {
         radio_ = &radio;
         radio_->template reg_recv_callback<self_type, self_type::receive>( this );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      distance_t distance( node_id_t to )
      {
         distance_t distance = -1.0;

         DistanceMapIterator it = distances_.find( to );
         if ( it != distances_.end() )
            distance = it->second;

         return distance;
      };

   private:
      void receive( node_id_t id, size_t len, block_data_t* data, const ExtendedData& exdata )
           {
              // TODO: Why does the lqi-radio returns (255 - lqi)? hence, we transform it back here...
              uint16 lqi = 255 - exdata.link_metric();
              distance_t distance = 0;

              if ( lqi >= 118 )
                 distance = 1.0;
              else
                 distance = (-0.2 * lqi) + 25;
     //          else if ( lqi < 118 && lqi >= 110 )
     //             distance = 2.0;
     //          else if ( lqi < 110 && lqi >= 104 )
     //             distance = 3.0;
     //          else if ( lqi < 104 && lqi >= 100 )
     //             distance = 4.0;
     //          else if ( lqi < 100 && lqi >= 94 )
     //             distance = 5.0;
     //          else if ( lqi < 94 && lqi >= 88 )
     //             distance = 6.0;
     //          else if ( lqi < 88 && lqi >= 83 )
     //             distance = 7.0;
     //          else if ( lqi < 83 && lqi >= 78 )
     //             distance = 8.0;
     //          else if ( lqi < 78 && lqi >= 73 )
     //             distance = 9.0;
     //          else
     //             distance = 10.0;

              distances_[id] = distance;
           }


      iSenseOs& os()
      { return os_; }

      iSenseOs& os_;

      DistanceMap distances_;
      Radio *radio_;
   };

}
#endif
