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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_NOP_MODULE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_NOP_MODULE_H

#include "algorithms/localization/distance_based/modules/localization_module.h"

namespace wiselib
{

   /// Module that does nothing
   /** This module is just a place holder, that has no special task. It just
    *  returns, that it is finished, and is used, e.g., if there is no
    *  refinement algorithm selected.
    */
   template<typename OsModel_P,
            typename Radio_P,
            typename SharedData_P>
   class LocalizationNopModule
      : public LocalizationModule<OsModel_P, Radio_P, SharedData_P>
   {

   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef SharedData_P SharedData;

      typedef LocalizationNopModule<OsModel, Radio, SharedData> self_type;

      typedef typename Radio::size_t size_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;

      ///@name construction / destruction
      ///@{
      ///
      LocalizationNopModule()
      {}
      ///
      ~LocalizationNopModule()
      {}
      ///@}

      ///@name dummy methods - do nothing in nop module
      ///@{
      void enable( void )
      {}
      void receive( node_id_t from, size_t len, block_data_t *data )
      {}
      void work( void )
      {}
      void rollback( void )
      {}
      ///@}

      ///@name module status info
      ///@{
      /** \return \c true
       */
      bool finished( void )
      { return true; }
      ///@}

   };

}// namespace wiselib
#endif
