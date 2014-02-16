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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_VECTOR_STATIC_H
#define __WISELIB_INTERNAL_INTERFACE_STL_VECTOR_STATIC_H

#include "util/pstl/iterator.h"
#include <string.h>

   template<typename OsModel_P,
            typename Value_P,
            int VECTOR_SIZE,
            bool Outsource_P = false>
   class vector_static;

#include <util/meta.h>

#if CONTIKI
//#include <contiki.h>
#include <stdio.h>
#endif

namespace wiselib
{

   template<typename OsModel_P,
            typename Value_P,
            int VECTOR_SIZE,
            bool Outsource_P = false>
   class vector_static
   {
   public:
      typedef OsModel_P OsModel;
      typedef Value_P value_type;
      typedef value_type* pointer;
      typedef const value_type* const_pointer;
      typedef value_type& reference;
      typedef const value_type& const_reference;

      typedef vector_static<OsModel_P, value_type, VECTOR_SIZE> vector_type;

      typedef normal_iterator<OsModel_P, pointer, vector_type> iterator;
      // FIXME: implement proper const_iterator
      typedef normal_iterator<OsModel_P, pointer, vector_type> const_iterator;

      typedef typename iterator::difference_type difference_type;
      typedef typename OsModel_P::size_t size_type;


   //#if VECTOR_STATIC_OUTSOURCE
      template<typename T>
      void set_data(T* v) {
         value_type *vec = reinterpret_cast<value_type*>(v);
         //vec_ = vec;
         finish_ = vec + (finish_ - start_);
         start_ = vec;
         end_of_storage_ = start_ + VECTOR_SIZE;
      }

      void set_size(size_type sz) {
         finish_ = start_ + sz;
      }
   //#endif

      // --------------------------------------------------------------------
      vector_static()
      {
         start_ = vec_; // &vec_[0];
         finish_ = start_;
         end_of_storage_ = start_ + VECTOR_SIZE;
      }
      // --------------------------------------------------------------------
      vector_static( const vector_static& vec )
      { *this = vec; }
      // --------------------------------------------------------------------
      vector_static( size_type n, const value_type& value = value_type() )
      {
         n = VECTOR_SIZE < n ? VECTOR_SIZE : n;
         for ( unsigned int i = 0; i < n; ++i )
            push_back( value );
      }
      template <class InputIterator>
      vector_static( InputIterator first, InputIterator last )
      {
         for ( unsigned int i = 0; i < VECTOR_SIZE && first != last; ++i, ++first )
            push_back( *first++ );
      }
      // --------------------------------------------------------------------
      ~vector_static() {}
      // --------------------------------------------------------------------
      vector_static& operator=( const vector_static& vec )
      {
         memcpy( start_, vec.start_, sizeof(value_type) * vec.size() );
         //start_ = &vec_[0];
         finish_ = start_ + (vec.finish_ - vec.start_);
         end_of_storage_ = start_ + VECTOR_SIZE;
         return *this;
      }
      // --------------------------------------------------------------------
      ///@name Iterators
      ///@{
      iterator begin()
      { return iterator( start_ ); }
      // --------------------------------------------------------------------
      iterator end()
      { return iterator( finish_ ); }
      // --------------------------------------------------------------------
      const_iterator begin() const
      { return const_iterator( start_ ); }
      // --------------------------------------------------------------------
      const_iterator end() const
      { return const_iterator( finish_ ); }
      ///@}
      // --------------------------------------------------------------------
      ///@name Capacity
      ///@{
      size_type size() const
      { return size_type(finish_ - start_); }
      // --------------------------------------------------------------------
      size_type max_size() const
      { return VECTOR_SIZE; }
      // --------------------------------------------------------------------
      size_type capacity() const
      { return VECTOR_SIZE; }
      // --------------------------------------------------------------------
      bool empty() const
      { return size() == 0; }
      // --------------------------------------------------------------------
      bool full() const
      { return size() == max_size(); }
      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      reference operator[](size_type n)
      {
         return *(this->start_ + n);
      }
      // --------------------------------------------------------------------
      reference at(size_type n)
      {
         return (*this)[n];
      }
      // --------------------------------------------------------------------
      reference front()
      {
         return *begin();
      }
      // --------------------------------------------------------------------
      const_reference front() const
      {
         return *begin();
      }
      // --------------------------------------------------------------------
      reference back()
      {
         return *(end() - 1);
      }
      // --------------------------------------------------------------------
      const_reference back() const
      {
         return *(end() - 1);
      }
      // --------------------------------------------------------------------
      pointer data()
      { return pointer(this->start_); }
      ///@}
      // --------------------------------------------------------------------
      ///@name Modifiers
      ///@{
      template <class InputIterator>
      void assign ( InputIterator first, InputIterator last )
      {
         clear();
         for ( InputIterator it = first; it != last; ++it )
            push_back( *it );
      }
      // --------------------------------------------------------------------
      void assign( size_type n, const value_type& u )
      {
         clear();
         for ( unsigned int i = 0; i < n; ++i )
            push_back( u );
      }
      // --------------------------------------------------------------------
      void push_back( const value_type& x )
      {
         if ( finish_ != end_of_storage_ )
         {
            *finish_ = x;
            ++finish_;
         }
         #if CONTIKI
         else {
            printf("vec ful");
         }
         #endif
      }
      // --------------------------------------------------------------------
      void pop_back()
      {
         if ( finish_ != start_ )
            --finish_;
      }
      // --------------------------------------------------------------------
      iterator insert( const value_type& x )
      {
         return insert( end(), x );
      }
      // --------------------------------------------------------------------
      iterator insert( iterator position, const value_type& x )
      {
         // no more elements can be inserted because vector is full
         if ( size() == max_size() )
            return iterator(finish_);

         value_type cur = x, temp;
         for ( iterator it = position; it != end(); ++it )
         {
            temp = *it;
            *it = cur;
            cur = temp;
         }
         push_back( cur );

         return position;
      }
      // --------------------------------------------------------------------
      void insert( iterator position, size_type n, const value_type& x )
      {
         for ( int i = 0; i < n; ++i )
            insert( position, x );
      }
      // --------------------------------------------------------------------
      template <class InputIterator>
      void insert( iterator position, InputIterator first, InputIterator last )
      {
         for ( ; first != last; ++first )
            insert( position, *first );
      }
      // --------------------------------------------------------------------
      iterator erase( iterator position )
      {
         if ( position == end() )
            return end();

         for ( iterator cur = position; cur != end(); cur++ )
            *cur = *(cur + 1);

         pop_back();

         return position;
      }
      // --------------------------------------------------------------------
      iterator erase( iterator first, iterator last )
      {
         if ( first == end() || first == last )
            return first;

         iterator ret = first;

         while ( last != end() )
         {
            *first = *last;
            first++;
            last++;
         }

         finish_ = &(*first);
         return ret;
      }
      // --------------------------------------------------------------------
      void swap( vector_type& vec )
      {
         vector_type tmp = *this;
         *this = vec;
         vec = tmp;
      }
      // --------------------------------------------------------------------
      void clear()
      {
         finish_ = start_;
      }
      // --------------------------------------------------------------------
      iterator find( const value_type& x )
      {
         for(iterator it = begin(); it != end(); ++it) {
            if(*it == x) { return it; }
         }
         return end();
      }

      ///@}

   protected:
//#if VECTOR_STATIC_OUTSOURCE
      //value_type *vec_;
//#else
      //value_type vec_[VECTOR_SIZE];
//#endif
      //typedef ENABLE_IF(Outsource_P, value_type*, vec_);
      //typedef ENABLE_IF(!Outsource_P, value_type X[VECTOR_SIZE]);

      value_type vec_[VECTOR_SIZE * (1 - Outsource_P)];

      pointer start_, finish_, end_of_storage_;
   };

}

#endif
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
