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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_MIMMAX_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_MIMMAX_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "algorithms/localization/distance_based/math/localization_triangulation.h"
#include "algorithms/localization/distance_based/math/vec.h"
#include "config_testing.h"

namespace wiselib
{

   ///
   /**
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P >
   class LocalizationMinMaxModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;
      typedef Arithmatic_P Arithmatic;

      typedef LocalizationMinMaxModule<OsModel, Radio, Debug, SharedData, Arithmatic> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename SharedData::NeighborInfoList NeighborInfoList;
      typedef typename SharedData::Neighborhood Neighborhood;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationMinMaxModule();
      ///
      ~LocalizationMinMaxModule();
      ///@}

      ///@name standard methods startup/simulation steps
      ///@{
      /** Dummy - not used.
       */
      void receive( node_id_t from, size_t len, block_data_t *data )
      {}
      /**
       */
      void work( void );
      ///@}

      ///@name module status info
      ///@{
      /** \return \c true
       */
      bool finished( void );
      ///@}
      void rollback( void );

      void init( Radio& radio, Debug& debug, SharedData& shared_data )
      {
         radio_ = &radio;
         debug_ = &debug;
         this->set_shared_data( shared_data );
      }

   private:

      enum MinMaxState
      {
         minmax_wait,
         minmax_work,
         minmax_finished
      };

      MinMaxState state_;

      Radio* radio_;
      Debug* debug_;
      //SharedData* shared_data_;
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   LocalizationMinMaxModule<OsModel_P, Radio_P, Debug_P, SharedData_P, Arithmatic_P>::
   LocalizationMinMaxModule()
      : state_( minmax_wait )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   LocalizationMinMaxModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   ~LocalizationMinMaxModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationMinMaxModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   work( void )
   {

#ifdef LOCALIZATION_DISTANCEBASED_MINMAX_DEBUG
            debug_->debug( "MINMAX working" );
#endif
      // if anchor, do not change position
      if ( this->shared_data().is_anchor() )
         state_ = minmax_finished;

      if ( state_ == minmax_finished )
         return;



      if ( state_ == minmax_wait )
         state_ = minmax_work;

      Vec<Arithmatic> est_pos;
      NeighborInfoList neighbors;
      collect_neighbors<OsModel, Neighborhood, NeighborInfoList, Arithmatic>(
         this->neighborhood(), lat_anchors, neighbors );

  /*    if ( neighbors.empty() )
       {
 //#ifdef LOCALIZATION_DISTANCEBASED_LATERATION_DEBUG
           debug_->debug(  "LocalizationMINMAXModule: No neighbors found\n" );
 //#endif
           return;
       }*/

      if ( neighbors.empty() )
            {
      //#ifdef LOCALIZATION_DISTANCEBASED_LATERATION_DEBUG
          	  if((radio_->id()==0x9999)||(radio_->id()==0x0c))
                debug_->debug(  "LocalizationMINMAXModule: No neighbors found\n" );
      //#endif
                return;
            }
      //#ifdef LOCALIZATION_DISTANCEBASED_LATERATION_DEBUG
      if((radio_->id()==0x9999)||(radio_->id()==0x0c))
      		  debug_->debug(  "LocalizationMINMAXModule: %d neigbous found\n",neighbors.size() );

      //#endif

      if ( est_pos_min_max<OsModel, NeighborInfoList, Arithmatic>( neighbors, est_pos ) )
      {
         if ( !this->shared_data().check_residue() ||
               check_residue<OsModel, NeighborInfoList, Arithmatic>(
                                neighbors, est_pos, lat_anchors,
                                this->shared_data().communication_range() ) )
         {
            this->shared_data().set_position( est_pos );
#ifdef LOCALIZATION_DISTANCEBASED_MINMAX_DEBUG
            debug_->debug( "set pos to (%d, %d)\n", (uint8_t)est_pos.x(), (uint8_t)est_pos.y() );
#endif
         }
      }

      state_ = minmax_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   bool
   LocalizationMinMaxModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   finished( void )
   {
      return state_ == minmax_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationMinMaxModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   rollback( void )
   {
      state_ = minmax_wait;
      //shared_data_->reset_neighborhood_();
   }

}// namespace wiselib
#endif
