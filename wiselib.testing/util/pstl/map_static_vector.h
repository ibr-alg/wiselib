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
#ifndef __UTIL_PSTL_MAP_STATIC_VECTOR__
#define __UTIL_PSTL_MAP_STATIC_VECTOR__

#include <util/pstl/iterator.h>
#include <util/pstl/vector_static.h>
#include <util/pstl/pair.h>
#include <util/serialization/pstl_pair.h>

namespace wiselib
{

   template<typename OsModel_P,
            typename Key_P,
            typename Value_P,
            unsigned int TABLE_SIZE>
   class MapStaticVector
      : public vector_static<OsModel_P, pair<Key_P, Value_P>, TABLE_SIZE>
   {
   public:
      typedef OsModel_P OsModel;

      typedef MapStaticVector<OsModel, Key_P, Value_P, TABLE_SIZE> map_type;
      typedef typename map_type::vector_type vector_type;

      typedef typename map_type::iterator iterator;
      typedef typename map_type::size_type size_type;

      typedef typename map_type::value_type value_type;
      typedef Key_P key_type;
      typedef Value_P mapped_type;
      typedef typename map_type::pointer pointer;
      typedef typename map_type::reference reference;
      // --------------------------------------------------------------------
      MapStaticVector()
         : vector_type()
      {}
      // --------------------------------------------------------------------
      MapStaticVector( const MapStaticVector& map )
         : vector_type( map )
      {}
      // --------------------------------------------------------------------
      ~MapStaticVector()
      {}
      // --------------------------------------------------------------------
      MapStaticVector& operator=( MapStaticVector& map )
      {
         vector_type::clear();
         for ( iterator it = map.begin(); it != map.end(); ++it )
            vector_type::insert( this->end(), *it );

         return *this;
      }
      // --------------------------------------------------------------------
      template <class InputIterator>
      MapStaticVector( InputIterator f, InputIterator l )
      {
         for ( InputIterator it = f; it != l; ++it )
            insert( *it );
      }
      // --------------------------------------------------------------------
      void swap( map_type& m )
      {
         map_type tmp = *this;
         *this = m;
         m = tmp;
      }
      // --------------------------------------------------------------------
      ///@name Modifiers
      ///@{
      pair<iterator, bool> insert( const value_type& x )
      {
         pair<iterator, bool> ret;
         ret.first = find(x.first);

         if ( ret.first != this->end() )
         {
            ret.second = false;
            return ret;
         }

         ret.first = vector_type::insert( this->end(), x );
         ret.second = true;
         return ret;
      }
      // --------------------------------------------------------------------
      template <class InputIterator>
      void insert ( InputIterator first, InputIterator last )
      {
         for ( InputIterator it = first; it != last; ++it )
            insert( *it );
      }
      // --------------------------------------------------------------------
      size_type erase( const key_type& k )
      {
         size_type count = 0;
         iterator it;
         while ( (it = find(k)) != this->end() )
         {
            vector_type::erase( it );
            count++;
         }
         return count;
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Operations
      ///@{
      iterator find( const key_type& k )
      {
         for ( iterator it = this->begin(); it != this->end(); ++it )
         {
            if ( it->first == k )
               return it;
         }
         return this->end();
      }
      // --------------------------------------------------------------------
      size_type count(const key_type& k)
      {
         size_type count = 0;

         for ( iterator it = this->begin(); it != this->end(); ++it )
         {
            if ( it->first == k )
               count++;
         }

         return count;
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      mapped_type& operator[]( const key_type& k )
      {
         iterator it = find(k);
         if ( it != this->end() )
            return it->second;

         value_type val;
         val.first = k;
         this->push_back( val );

         it = find(k);
         if ( it != this->end() )
            return it->second;

         // return dummy value that can be written to; this dummy value is
         // *only* returned if the static vector is full and can not hold
         // new components
         // TODO: Print Error message since this case should not happen!
		 //return dummy_;
		 assert(false);
		 return *(mapped_type*)0;
      }
      ///@}
      
      
      bool contains (const key_type& k ){
      iterator it = find(k);
         if ( it != this->end() )
         return true;
         else return false;
      }

   private:
      mapped_type dummy_;
   };

}

#endif
