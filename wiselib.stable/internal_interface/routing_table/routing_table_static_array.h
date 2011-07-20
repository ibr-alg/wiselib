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
#ifndef __INTERNAL_INTERFACE_ROUTING_TABLE_STATIC_ARRAY__
#define __INTERNAL_INTERFACE_ROUTING_TABLE_STATIC_ARRAY__

#include "util/pstl/iterator.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "util/serialization/pstl_pair.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            unsigned int TABLE_SIZE,
            typename Value_P = typename Radio_P::node_id_t>
   class StaticArrayRoutingTable
      : public vector_static<OsModel_P, pair<typename Radio_P::node_id_t, Value_P>, TABLE_SIZE>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;

      typedef StaticArrayRoutingTable<OsModel, Radio, TABLE_SIZE, Value_P> map_type;
      typedef typename map_type::vector_type vector_type;

      typedef typename map_type::iterator iterator;
      typedef typename map_type::size_type size_type;

      typedef typename map_type::value_type value_type;
      typedef typename Radio::node_id_t key_type;
      typedef Value_P mapped_type;
      typedef typename map_type::pointer pointer;
      typedef typename map_type::reference reference;
      // --------------------------------------------------------------------
      StaticArrayRoutingTable()
         : vector_type()
      {}
      // --------------------------------------------------------------------
      StaticArrayRoutingTable(StaticArrayRoutingTable& rt)
         : vector_type( rt )
      {}
      // --------------------------------------------------------------------
      ~StaticArrayRoutingTable()
      {}
      // --------------------------------------------------------------------
      StaticArrayRoutingTable& operator=(StaticArrayRoutingTable& rt)
      {
         *this = rt;
      }
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
         return dummy_;
      }
      ///@}
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
         return ret;
      }
      // --------------------------------------------------------------------
      //TODO template <class InputIterator>
      //     void insert ( InputIterator first, InputIterator last );
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
      ///@}

   private:
      mapped_type dummy_;
   };

}

#endif
