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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_LATERATION_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_LATERATION_MODULE_H

#include "config_testing.h"
#include "algorithms/localization/distance_based/modules/localization_module.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"
#include "algorithms/localization/distance_based/math/localization_triangulation.h"
#include "algorithms/localization/distance_based/math/vec.h"

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
   class LocalizationLaterationModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef SharedData_P SharedData;
      typedef Arithmatic_P Arithmatic;


      typedef LocalizationLaterationModule<OsModel, Radio, Debug, SharedData, Arithmatic> self_type;
      typedef LocalizationModule<OsModel, Radio, SharedData> base_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename SharedData::Neighborhood Neighborhood;
      typedef typename SharedData::NeighborInfoList NeighborInfoList;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationLaterationModule();
      ///
      ~LocalizationLaterationModule();
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
      void rollback(void );

      void init( Radio& radio, Debug& debug, SharedData& shared_data ) {
         radio_ = &radio;

         debug_ = &debug;
         //shared_data_ = &shared_data;
         this->shared_data().is_anchor();
      }


   private:

      enum LaterationState
      {
         lat_wait,
         lat_work,
         lat_finished
      };

      LaterationState state_;
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
   LocalizationLaterationModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   LocalizationLaterationModule()
      : state_( lat_wait )
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   LocalizationLaterationModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   ~LocalizationLaterationModule()
   {}
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationLaterationModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   work( void )
   {


      // if anchor, do not change position
      if ( this->shared_data().is_anchor() )
         state_ = lat_finished;

      if ( state_ == lat_finished )
         return;

      if ( state_ == lat_wait )
         state_ = lat_work;

      Vec<Arithmatic> est_pos;
      NeighborInfoList neighbors;

      collect_neighbors<OsModel, Neighborhood, NeighborInfoList, Arithmatic>( this->neighborhood(), lat_anchors, neighbors );
      if ( neighbors.empty() )
      {
//#ifdef LOCALIZATION_DISTANCEBASED_LATERATION_DEBUG
    	  if(radio_->id()==0x1c72)
          debug_->debug(  "LocalizationLaterationModule: No neighbors found\n" );
//#endif
          return;
      }
//#ifdef LOCALIZATION_DISTANCEBASED_LATERATION_DEBUG
	  if(radio_->id()==0x1c72)
		  debug_->debug(  "LocalizationLaterationModule: %d neigbous found\n",neighbors.size() );

//#endif


      // Execute lateration two times. First, to simply get the
      // estimated position. Second, use the estimated position
      // in the lateration phase as new parameter in least squares
      // approach. Particularly if you know only 3 anchors, the new
      // parameter in the second pass is very useful.
      if ( est_pos_lateration<OsModel, NeighborInfoList, Arithmatic>( neighbors, est_pos, lat_anchors, false ) &&
           est_pos_lateration<OsModel, NeighborInfoList, Arithmatic>( neighbors, est_pos, lat_anchors, true ) )
      {
         if ( !this->shared_data().check_residue() ||
               check_residue<OsModel, NeighborInfoList, Arithmatic>( neighbors, est_pos, lat_anchors,
                              this->shared_data().communication_range() ) )
            this->shared_data().set_position( est_pos );
      }

      #ifdef LOCALIZATION_DISTANCEBASED_LATERATION_DEBUG
      	  if(radio_->id()==0x9999)
      		  debug_->debug(  "LocalizationLaterationModule:after caluculting ");

      #endif

      state_ = lat_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   bool
   LocalizationLaterationModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   finished( void )
   {
      return state_ == lat_finished;
   }
   // ----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename SharedData_P,
            typename Arithmatic_P>
   void
   LocalizationLaterationModule<OsModel_P, Radio_P, Debug_P, SharedData_P,Arithmatic_P>::
   rollback(void)
   {
      state_ = lat_wait;
     // shared_data_->reset_neighborhood_();
   }

}// namespace wiselib
#endif
