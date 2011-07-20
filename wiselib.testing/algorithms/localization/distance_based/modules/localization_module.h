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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_MODULE_H

#include "algorithms/localization/distance_based/neighborhood/localization_neighborhood.h"
#include "algorithms/localization/distance_based/neighborhood/localization_local_coordinate_system.h"
#include "algorithms/localization/distance_based/util/localization_shared_data.h"

namespace wiselib
{

   /// Basic localization module to act with LocalizationProcessor.
   /** This is the basic localization module which has to be implemented if
    *  someone wants to add a new algorithm.
    *
    *  Functionality is like a Processor. There are the same methods boot(),
    *  process_message() and work(), which are called by the
    *  LocalizationProcessor. Each LocalizationProcessor owns two
    *  LocalizationModules; one for distance phase (get distance to anchors)
    *  and one for refinement phase (calibrating the estimated position).
    *  Moreover the modules are able to send messages by the method send().
    *
    *  Particularly a distance module has to work on given neighborhood(),
    *  because the LocalizationProcessor uses it to estimate it's position.
    *
    *  If the distance phase finished, method finished() has to return
    *  \c true, so that the LocalizationProcessor is able to estimate it's
    *  position. Same with a refinement module to make the
    *  LocalizationProcessor able to deactivate itself.

         // Boot method which is called once on startup.
         void enable( void )

         // This method is called if the LocalizationProcessor receives a
         // message and forward it to the modules.
         void receive( node_id_t from, size_t len, block_data_t *data )

         // This method is called once each simulation round as long as the
         //  module is not finished.
         //
         //  Moreover a refinement module's method is not called until the
         //  distance module is finished.
         void work( void )

         // This method is called if there is a rollback_period given and
         //  reached. This is done in most cases, because the nodes are mobile,
         //  so that they change their position.
         //
         //  The easiest way of implementation would be to just reset all
         //  information.
         void rollback( void )

         returns \c true, if module is finished. \c false otherwise
         bool finished( void )

         // This method sets the shared data object, which can be accessed by
         // all available modules.
         void set_shared_data( SharedData& shared_data )
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename SharedData_P>
   class LocalizationModule
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef SharedData_P SharedData;

      typedef LocalizationModule<OsModel, Radio, SharedData> self_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename SharedData::Neighborhood Neighborhood;
      typedef typename SharedData::LocalCoordinateSystem LocalCoordinateSystem;

      ///@name basic access to owner, observer, neighborhood and local coordinate system
      ///@{
      /** This method sets the shared data object, which can be accessed by
       *  all available modules.
       *
       *  \param LocalizationSharedData Shared data between all modules.
       */
      void set_shared_data( SharedData& shared_data )
      { shared_data_ = &shared_data; }
      /** \return LocalizationSharedData shared by all modules.
       */
//       const SharedData& shared_data( void ) const
//       { return *shared_data_; }
      /** \return writable LocalizationSharedData shared by all modules.
       */
      SharedData& shared_data( void )
      { return *shared_data_; }
      /** \return Writable shared LocalizationNeighborhood.
       */
      Neighborhood& neighborhood( void )
      { return shared_data_->neighborhood(); }
      /** \return Writable shared LocalizationLocalCoordinateSystem.
       */
      LocalCoordinateSystem& local_coord_sys( void )
      { return shared_data_->local_coord_sys(); }
      ///@}


   private:

      SharedData* shared_data_;
   };

}// namespace wiselib
#endif
