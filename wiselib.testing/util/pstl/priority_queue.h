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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_PRIORITY_QUEUE_H
#define __WISELIB_INTERNAL_INTERFACE_STL_PRIORITY_QUEUE_H

#include "util/pstl/iterator.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Value_P,
            int QUEUE_SIZE>
   class priority_queue
   {
   public:
      typedef Value_P value_type;
      typedef value_type* pointer;

      typedef typename OsModel_P::size_t size_type;
      // --------------------------------------------------------------------
      priority_queue()
      {
         start_ = &vec_[0];
         finish_ = start_;
         end_of_storage_ = start_ + QUEUE_SIZE;
      }
      // --------------------------------------------------------------------
      priority_queue( const priority_queue& pq )
      { *this = pq; }
      // --------------------------------------------------------------------
      ~priority_queue() {}
      // --------------------------------------------------------------------
      priority_queue& operator=( const priority_queue& pq )
      {
         memcpy( vec_, pq.vec_, sizeof(vec_) );
         start_ = &vec_[0];
         finish_ = start_ + (pq.finish_ - pq.start_);
         end_of_storage_ = start_ + QUEUE_SIZE;
         return *this;
      }
      // --------------------------------------------------------------------
      ///@name Capacity
      ///@{
      size_type size()
      { return size_type(finish_ - start_); }
      // --------------------------------------------------------------------
      size_type max_size()
      { return QUEUE_SIZE; }
      // --------------------------------------------------------------------
      size_type capacity()
      { return QUEUE_SIZE; }
      // --------------------------------------------------------------------
      bool empty()
      { return size() == 0; }
      // --------------------------------------------------------------------
      pointer data()
      { return pointer(this->start_); }
      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      value_type top()
      {
         return vec_[0];
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Modifiers
      ///@{
      void clear()
      {
         finish_ = start_;
      }
      // --------------------------------------------------------------------
      void push( const value_type& x )
      {
         int i = size();
         while ( i != 0 && x < vec_[i/2] )
         {
            vec_[i] = vec_[i/2];
            i = i/2;
         }
         vec_[i] = x;
         ++finish_;
      }
      // --------------------------------------------------------------------
      value_type pop()
      {
         int n = size() - 1;
         value_type e = vec_[0];
         value_type x = vec_[n];
         --finish_;
         int i = 0;
         int c = 1;
         while ( c <= n )
         {
            if ( c < n && vec_[c + 1] < vec_[c] )
               ++c;
            if ( !( vec_[c] < x ) )
               break;
            vec_[i] = vec_[c];
            i = c;
            c = 2 * i;
         }
         vec_[i] = x;
         return e;
      }
      ///@}

   protected:
      value_type vec_[QUEUE_SIZE];

      pointer start_, finish_, end_of_storage_;
   };

}

#endif
