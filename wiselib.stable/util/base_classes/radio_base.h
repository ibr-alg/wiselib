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
#ifndef __UTIL_BASECLASSES_RADIO_BASE_H__
#define __UTIL_BASECLASSES_RADIO_BASE_H__

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "config.h"

namespace wiselib
{

   /** \brief Base radio class
    * 
    *  \ingroup radio_concept
    *
    *  Basic radio class that provides helpful methods like registration of
    *  callbacks.
    */
   template<typename OsModel_P,
            typename NodeId_P,
            typename Size_P,
            typename BlockData_P,
            int MAX_RECEIVERS = RADIO_BASE_MAX_RECEIVERS>
   class RadioBase
   {
   public:
      typedef OsModel_P OsModel;

      typedef NodeId_P node_id_t;
      typedef Size_P size_t;
      typedef BlockData_P block_data_t;

      typedef delegate3<void, node_id_t, size_t, block_data_t*> radio_delegate_t;

      typedef vector_static<OsModel, radio_delegate_t, MAX_RECEIVERS> CallbackVector;
      typedef typename CallbackVector::iterator CallbackVectorIterator;
      // --------------------------------------------------------------------
      enum ReturnValues
      {
         SUCCESS = OsModel::SUCCESS
      };
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
         if ( callbacks_.empty() )
            callbacks_.assign( MAX_RECEIVERS, radio_delegate_t() );

         for ( unsigned int i = 0; i < callbacks_.size(); ++i )
         {
            if ( callbacks_.at(i) == radio_delegate_t() )
            {
               callbacks_.at(i) = radio_delegate_t::template from_method<T, TMethod>( obj_pnt );
               return i;
            }
         }

         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      {
         callbacks_.at(idx) = radio_delegate_t();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void notify_receivers( node_id_t from, size_t len, block_data_t *data )
      {
         for ( CallbackVectorIterator
                  it = callbacks_.begin();
                  it != callbacks_.end();
                  ++it )
         {
            if ( *it != radio_delegate_t() )
               (*it)( from, len, data );
         }
      }

   private:
      CallbackVector callbacks_;

   };

}
#endif
