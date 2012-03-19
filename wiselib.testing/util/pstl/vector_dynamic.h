// vim: set ts=3 sw=3 expandtab:
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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_VECTOR_DYNAMIC_H
#define __WISELIB_INTERNAL_INTERFACE_STL_VECTOR_DYNAMIC_H

#include "util/pstl/iterator.h"
#include <string.h>

#define VECTOR_DYNAMIC_MIN_SIZE 4

namespace wiselib
{

   template<typename OsModel_P,
            typename Value_P,
            typename Allocator_P>
   class vector_dynamic
   {
   public:
      typedef Value_P value_type;
      typedef Allocator_P Allocator;
      typedef value_type* pointer;
      typedef value_type& reference;

      typedef vector_dynamic<OsModel_P, value_type, Allocator_P> vector_type;
      typedef vector_dynamic<OsModel_P, value_type, Allocator_P> self_type;
      typedef typename Allocator::template pointer_t<self_type> self_pointer_t;

      typedef normal_iterator<OsModel_P, pointer, vector_type> iterator;

      typedef typename OsModel_P::size_t size_type;
      typedef typename Allocator::template array_pointer_t<value_type> buffer_pointer_t;
      // --------------------------------------------------------------------
      vector_dynamic() :  size_(0), capacity_(0), buffer_(0), allocator_(0)
      {
      }
      // --------------------------------------------------------------------
      vector_dynamic(typename Allocator::self_pointer_t alloc) :  size_(0), capacity_(0), buffer_(0),allocator_(alloc)
      {
      }
      // --------------------------------------------------------------------
      vector_dynamic( const vector_dynamic& vec )
      {
         *this = vec;
      }
      // --------------------------------------------------------------------
      ~vector_dynamic() {
         if(buffer_) {
            allocator_->free_array(buffer_);
         }
      }
      
		/**
		 * Detach from internal buffer.
		 * This will detach from the internal data structures such that
		 * destroying this object wont free them anymore. This is useful, when
		 * you have a bitstring_static_view on the same structure and want to
		 * only use that
		 */
		void detach() {
         size_ = 0;
         buffer_ = 0;
		}
      
      void attach(buffer_pointer_t buffer, size_type size) {
         if(buffer_) {
            allocator_->free_array(buffer_);
         }
         buffer_ = buffer;
         size_ = size;
      }
      
      vector_dynamic& operator=( const vector_dynamic& vec )
      {
         allocator_ = vec.allocator_;
          //if(buffer_!= buffer_pointer_t()){
              clear();
              resize(vec.capacity_);
          //}
          for(size_t i = 0;i < vec.size_;++i){
              buffer_[size_++] = vec[i];
          }
          /*
          for(int i=0; i<vec.size(); i++) {
             assert(buffer_[i] == vec[i]);
          }
          */
          return *this;
      }
      /*
      vector_dynamic& operator=( vector_dynamic& vec )
      {
          if(buffer_!= buffer_pointer_t()){
              resize(0);
              resize(vec.size_);
          }
          for(size_t i = 0;i < vec.size_;++i){
              buffer_[size_++] = vec[i];
          }
         return *this;
      }
      */
      // --------------------------------------------------------------------
      void set_allocator(typename Allocator::self_pointer_t alloc) { allocator_ = alloc; }
      ///@name Iterators
      ///@{
      iterator begin()
      { return iterator( buffer_.raw() ); }
      // --------------------------------------------------------------------
      iterator end()
      { return iterator( buffer_.raw() + size_ ); }
      ///@}
      // --------------------------------------------------------------------
      ///@name Capacity
      ///@{
      size_type size() const
      { return size_; }
      // --------------------------------------------------------------------
      size_type max_size() const
      { return capacity_; }
      // --------------------------------------------------------------------
      size_type capacity() const
      { return capacity_; }
      // --------------------------------------------------------------------
      bool empty() const
      { return size() == 0; }
      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      const reference operator[](size_type n) const
      {
     //    assert(n < size_);
         return *(this->buffer_ + n);
      }
      reference operator[](size_type n)
      {
       //  assert(n < size_);
         return *(this->buffer_ + n);
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
      reference back()
      {
         return *(end() - 1);
      }
      // --------------------------------------------------------------------
      pointer data()
      { return pointer(&*buffer_); }
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
         if(size_ >= capacity_) {
            grow();
         }
            
         buffer_[size_++] = x;
         
         //printf("v: %d %d\n", buffer_[0], buffer_[1]);
//         assert(buffer_[size_ - 1] == x);
      }
      // --------------------------------------------------------------------
      void pop_back()
      {
         if ( size_ > 0 )
            --size_;
         
         if(size_ < (capacity_ / 4)) {
            shrink();
         }
      }
      // --------------------------------------------------------------------
      iterator insert( iterator position, const value_type& x )
      {
         // no more elements can be inserted because vector is full
         if ( size() == max_size() )
            return iterator(buffer_ + size_);

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

         size_ = &(*first) - buffer_;
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
         size_ = 0;
         resize(0);
      }
      ///@}
      
      void grow() {
         if(capacity_ < VECTOR_DYNAMIC_MIN_SIZE) {
            resize(VECTOR_DYNAMIC_MIN_SIZE);
         }
         else {
            resize(capacity_ * 2);
         }
      }
      void shrink() { resize(capacity_ / 2); }
      
      void pack() { resize(size_); }
      
      void resize(size_t n) {
         //assert(allocator_!=0);
         //assert(n >= size_);
         buffer_pointer_t new_buffer(0);
         if(n != 0) {
            new_buffer = allocator_->template allocate_array<value_type>(n);
         }
         
         if(buffer_) {
            for(size_type i=0; i<size_; i++) {
               new_buffer[i] = buffer_[i];
            }
            
            allocator_->free_array(buffer_);
         }
         buffer_ = new_buffer;
         capacity_ = n;
         
      }
      
  // protected:
     // value_type vec_[VECTOR_SIZE];

      size_type size_, capacity_;
      buffer_pointer_t buffer_;
      typename Allocator::self_pointer_t allocator_;
      
      //friend class bitstring_static_view<OsModel;

   } __attribute__((__packed__));

}

#endif
