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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_REVERSE_ITERATOR_H_
#define __WISELIB_INTERNAL_INTERFACE_STL_REVERSE_ITERATOR_H_

namespace wiselib {

   /**
    * allows to step backwards
    */
   template<typename Iterator_P>
   class reverse_iterator
   {
   public:
      // --------------------------------------------------------------------
      typedef reverse_iterator<Iterator_P> iterator_type;
typedef      typename Iterator_P::reference reference;
      typedef typename Iterator_P::pointer pointer;
      // --------------------------------------------------------------------
      /// default constructor
      reverse_iterator( Iterator_P iterator ) :
      inner_iterator_( iterator )
      {

      }
      // --------------------------------------------------------------------
      reference operator*() const
      {
         return *inner_iterator_;
      }
      // --------------------------------------------------------------------
      pointer operator->() const
      {
         // TODO: test
         return &inner_iterator_;
      }
      // --------------------------------------------------------------------
      iterator_type& operator++()
      {
         --inner_iterator_;
         return *this;
      }
      // --------------------------------------------------------------------
      iterator_type operator++( int )
      {
         iterator_type tmp = *this;
         --inner_iterator_;
         return tmp;
      }
      // --------------------------------------------------------------------
      iterator_type& operator--()
      {
         ++inner_iterator_;
         return *this;
      }
      // --------------------------------------------------------------------
      iterator_type operator--( int )
      {
         iterator_type tmp = *this;
         ++inner_iterator_;
         return tmp;
      }
      // --------------------------------------------------------------------
      bool operator==( const reverse_iterator& x ) const
      {
         return inner_iterator_ == x.inner_iterator_;
      }
      // --------------------------------------------------------------------
      bool operator!=( const reverse_iterator& x ) const
      {
         return inner_iterator_ != x.inner_iterator_;
      }
      // --------------------------------------------------------------------
   private:
      Iterator_P inner_iterator_;
   };
}

#endif /* __WISELIB_INTERNAL_INTERFACE_STL_REVERSE_ITERATOR_H_ */
