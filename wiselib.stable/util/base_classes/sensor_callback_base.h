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
#ifndef __UTIL_BASECLASSES_SENSORCALLBACK_BASE_H__
#define __UTIL_BASECLASSES_SENSORCALLBACK_BASE_H__

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "config.h"

namespace wiselib
{

   /** \brief Base State Callback class
    *  \ingroup state_callback_concept
    *
    *  Basic state callback class that provides helpful methods like registration of
    *  callbacks.
    */
   template<typename OsModel_P,
            typename Value_P,
            int MAX_RECEIVERS = SENSOR_CALLBACK_BASE_MAX_RECEIVERS>
   class SensorCallbackBase
   {
   public:
      typedef OsModel_P OsModel;
      typedef Value_P value_t;

      typedef delegate1<void, value_t> sensor_callback_delegate_t;

      typedef vector_static<OsModel, sensor_callback_delegate_t, MAX_RECEIVERS> CallbackVector;
      typedef typename CallbackVector::iterator CallbackVectorIterator;
      // --------------------------------------------------------------------
      enum ReturnValues
      {
         SUCCESS = OsModel::SUCCESS
      };
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(value_t)>
      int register_sensor_callback( T *obj_pnt )
      {
         if ( callbacks_.empty() )
            callbacks_.assign( MAX_RECEIVERS, sensor_callback_delegate_t() );

         for ( unsigned int i = 0; i < callbacks_.size(); ++i )
         {
            if ( callbacks_.at(i) == sensor_callback_delegate_t() )
            {
               callbacks_.at(i) = sensor_callback_delegate_t::template from_method<T, TMethod>( obj_pnt );
               return i;
            }
         }

         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_read_callback( int idx )
      {
         callbacks_.at(idx) = sensor_callback_delegate_t();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void notify_receivers( value_t value )
      {
         for ( CallbackVectorIterator
                  it = callbacks_.begin();
                  it != callbacks_.end();
                  ++it )
         {
            if ( *it != sensor_callback_delegate_t() )
               (*it)( value );
         }
      }

   private:
      CallbackVector callbacks_;

   };

}
#endif
