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
#ifndef __UTIL_BASECLASSES_EXTENDED_RADIO_BASE_H__
#define __UTIL_BASECLASSES_EXTENDED_RADIO_BASE_H__

#include "util/base_classes/radio_base.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/base_classes/base_extended_data.h"
#include "config.h"

namespace wiselib
{

   /** \brief Base extended radio class
    *  \ingroup routing_concept
    *
    *  Basic extended radio class that provides helpful methods like registration of
    *  callbacks.
    */
   template<typename OsModel_P,
            typename NodeId_P,
            typename Size_P,
            typename BlockData_P,
            int MAX_RECEIVERS = RADIO_BASE_MAX_RECEIVERS,
            typename ExtendedData_P = BaseExtendedData<OsModel_P>
            >
   class ExtendedRadioBase : public RadioBase< OsModel_P, NodeId_P, Size_P, BlockData_P, MAX_RECEIVERS>
   {
   public:
	  typedef ExtendedData_P ExtendedData;

      typedef OsModel_P OsModel;

      typedef NodeId_P node_id_t;
      typedef Size_P size_t;
      typedef BlockData_P block_data_t;

      typedef delegate3<void, node_id_t, size_t, block_data_t*> radio_delegate_t;
      typedef delegate4<void, node_id_t, size_t, block_data_t*, const ExtendedData&> extended_radio_delegate_t;

      typedef vector_static<OsModel, radio_delegate_t, MAX_RECEIVERS> CallbackVector;
      typedef typename CallbackVector::iterator CallbackVectorIterator;

      typedef vector_static<OsModel, extended_radio_delegate_t, MAX_RECEIVERS> ExtendedCallbackVector;
      typedef typename ExtendedCallbackVector::iterator ExtendedCallbackVectorIterator;

      // --------------------------------------------------------------------
      enum ReturnValues
      {
         SUCCESS = OsModel::SUCCESS
      };
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
    	  return RadioBase<OsModel_P, NodeId_P, Size_P, BlockData_P, MAX_RECEIVERS>::template reg_recv_callback<T, TMethod>( obj_pnt );
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*, const ExtendedData&)>
      int reg_recv_callback( T *obj_pnt )
      {
         if ( extended_callbacks_.empty() )
        	 extended_callbacks_.assign( MAX_RECEIVERS, extended_radio_delegate_t() );

         for ( unsigned int i = 0; i < extended_callbacks_.size(); ++i )
         {
            if ( extended_callbacks_.at(i) == extended_radio_delegate_t() )
            {
               extended_callbacks_.at(i) = extended_radio_delegate_t::template from_method<T, TMethod>( obj_pnt );
               return MAX_RECEIVERS+i;
            }
         }

         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      {
    	  if( idx < MAX_RECEIVERS )
    		  return RadioBase<OsModel_P, NodeId_P, Size_P, BlockData_P, MAX_RECEIVERS>::unreg_recv_callback( idx );
    	  else
    		  extended_callbacks_.at( idx - MAX_RECEIVERS ) = extended_radio_delegate_t();

         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void notify_receivers( node_id_t from, size_t len, block_data_t *data )
      {
    	  RadioBase<OsModel_P, NodeId_P, Size_P, BlockData_P, MAX_RECEIVERS>::notify_receivers( from, len, data );
      }
      // --------------------------------------------------------------------
      void notify_receivers( node_id_t from, size_t len, block_data_t *data, const ExtendedData& ext_data )
      {
    	 notify_receivers( from, len, data );

         for ( ExtendedCallbackVectorIterator
                  it = extended_callbacks_.begin();
                  it != extended_callbacks_.end();
                  ++it )
         {
            if ( *it != extended_radio_delegate_t() )
               (*it)( from, len, data, ext_data );
         }
      }

   private:
      ExtendedCallbackVector extended_callbacks_;
   };

}
#endif
