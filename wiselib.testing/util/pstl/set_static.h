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

#ifndef SET_STATIC_H
#define SET_STATIC_H

#include "util/pstl/iterator.h"
#include <string.h>

namespace wiselib
{

   template<typename OsModel_P,
            typename Value_P,
            int SET_SIZE>
   class set_static
   {
   public:
      typedef Value_P value_type;
      typedef value_type* pointer;
      typedef value_type& reference;
      typedef const value_type& const_reference;

      typedef set_static<OsModel_P, value_type, SET_SIZE> set_type;

      typedef normal_iterator<OsModel_P, pointer, set_type> iterator;

      typedef typename OsModel_P::size_t size_type;
      // --------------------------------------------------------------------
      set_static()
      {
         finish_ = set_;
         end_of_storage_ = set_ + SET_SIZE;
      }
      // --------------------------------------------------------------------
      set_static( const set_static& set )
      { *this = set; }
      // --------------------------------------------------------------------
      ~set_static() {}
      // --------------------------------------------------------------------
      set_static& operator=( const set_static& set )
      {
         memcpy( set_, set.set_, sizeof(set_) );
         finish_ = set_ + (set.finish_ - set.set_);
         end_of_storage_ = set_ + SET_SIZE;
         return *this;
      }
      // --------------------------------------------------------------------
      ///@name Iterators
      ///@{
      iterator begin()
      { return iterator( set_ ); }
      // --------------------------------------------------------------------
      iterator end()
      { return iterator( finish_ ); }
      // --------------------------------------------------------------------
      //TODO iterator rbegin()
      //TODO iterator rend()
      ///@}
      // --------------------------------------------------------------------
      ///@name Capacity
      ///@{
      size_type size() const
      { return size_type(finish_ - set_); }
      // --------------------------------------------------------------------
      size_type max_size() const
      { return SET_SIZE; }
      // --------------------------------------------------------------------
      size_type capacity() const
      { return SET_SIZE; }
      // --------------------------------------------------------------------
      bool empty() const
      { return size() == 0; }

      /*/ --------------------------------------------------------------------

      // --------------------------------------------------------------------
       *
       */
      iterator insert( iterator position , const value_type& x )
      {
         // no more elements can be inserted because vector is full
         if ( size() == max_size() )
            return iterator(finish_);

         value_type cur = x, temp;

         for ( iterator it = position; it != end(); ++it )
         {
        	if(cur == *it)
        		return iterator(finish_);
            temp = *it;
            *it = cur;
            cur = temp;
         }
         push_back( cur );
         return position;
      }
      // --------------------------------------------------------------------
      inline iterator insert(const value_type& x )
      { return insert(this->begin(), x); }
      // --------------------------------------------------------------------
      void insert( iterator position, size_type n, const value_type& x )
      {
         for ( int i = 0; i < n; ++i )
            insert( position, x );
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
      // -------------------------------------------------------------------

      // --------------------------------------------------------------------
      // --------------------------------------------------------------------
      size_type erase( const value_type& x )
      {
         size_type count = 0;
         iterator it;
         while ( (it = find(x)) != this->end() )
         {
            erase( it);
            count++;
         }
         return count;
      }
      // --------------------------------------------------------------------
      ///@name Operations
      ///@{
      iterator find( const value_type& x )
      {
         for ( iterator it = this->begin(); it != this->end(); ++it )
         {
        	iterator cur = it;
            if ( *cur.base() == x )
               return cur;
         }
         return this->end();
      }
	  bool contains(const value_type& x) { return find(x) != end(); }
      // --------------------------------------------------------------------
      size_type count(const value_type& x)
      {
         size_type count = 0;

         for ( iterator it = this->begin(); it != this->end(); ++it )
         {
        	 iterator cur = it;
            if ( cur.base() == x )
               count++;
         }

         return count;
      }
      ///@}
      // --------------------------------------------------------------------
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
      void swap( set_type& set )
      {
         set_type tmp = *this;
         *this = set;
         set = tmp;
      }
      // --------------------------------------------------------------------
      void clear()
      {
         finish_ = set_;
      }
      ///@}

   protected:
      value_type set_[SET_SIZE];

      pointer finish_, end_of_storage_;


   private:

      /*
      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      reference operator[](size_type n)
      {
         return *(this->set_ + n);
      }
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      const_reference operator[](size_type n) const
      {
         return *(this->set_ + n);
      }
      // --------------------------------------------------------------------
      reference at(size_type n)
      {
         return (*this)[n];
      }
      // --------------------------------------------------------------------
      const_reference at(size_type n) const
      {
         return (*this)[n];
      }
      // --------------------------------------------------------------------
       */
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
      { return pointer(this->set_); }
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
         for ( int i = 0; i < n; ++i )
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
      }
      // --------------------------------------------------------------------
      void pop_back()
            {
               if ( finish_ != set_ )
                  --finish_;
            }

   };

}

#endif // SET_STATIC_H

