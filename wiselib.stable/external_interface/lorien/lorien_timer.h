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
#ifndef CONNECTOR_LORIEN_TIMER_H
#define CONNECTOR_LORIEN_TIMER_H

#include "external_interface/lorien/lorien_types.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
extern "C" {
#include "lorien.h"
}

namespace wiselib
{

   namespace lorien
   {
      typedef delegate1<void, void*> timer_delegate_t;

      struct TimerItem
      {
         timer_delegate_t cb;
         void *userdata;
      };
   }

   /** \brief Lorien Implementation of \ref timer_concept "Timer Concept".
    *
    *  \ingroup timer_concept
    *  \ingroup lorien_facets
    *
    *  Lorien implementation of the \ref timer_concept "Timer Concept" ...
    */
   template<typename OsModel_P>
   class LorienTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef LorienTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef vector_static<OsModel, lorien::TimerItem, 10> TimerItemVector;
      typedef typename TimerItemVector::iterator TimerItemVectorIterator;

      typedef uint32_t millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // -----------------------------------------------------------------
      LorienTimerModel()
      {
      }
      // -----------------------------------------------------------------
      void init( Component *comp )
      {
         comp_ = comp;
         ((LXState*) comp_ -> state) -> lorien_timer = this;

         if ( timer_items_.empty() )
            timer_items_.assign( 10, lorien::TimerItem() );
      }
      // --------------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer(millis_t millis, T *obj_pnt, void *userdata )
      {
         lorien::TimerItem *item = free_timer_item();
         if (!item)
         	{
            return ERR_UNSPEC;
            }

         item->cb = lorien::timer_delegate_t::from_method<T, TMethod>( obj_pnt );
         item->userdata = userdata;

         ((ITimer*) ((LXState*) comp_ -> state) -> timer -> userRecp) -> setTimer(((LXState*) comp_ -> state) -> timer -> ifHostComponent, comp_, millis, item);
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void timer_elapsed( void *ptr )
      {
         lorien::TimerItem *item = (lorien::TimerItem*)ptr;
         item->cb( item->userdata );

         item->cb = lorien::timer_delegate_t();
      }

   private:
      lorien::TimerItem* free_timer_item()
      {
         for ( TimerItemVectorIterator
                  it = timer_items_.begin();
                  it != timer_items_.end();
                  ++it )
         {
            if ( !(it->cb) )
               return &(*it);
         }

         return 0;
      }

      Component *comp_;
      TimerItemVector timer_items_;
   };
}

#endif
