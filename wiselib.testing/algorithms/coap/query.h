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

/*
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef QUERY_H
#define  QUERY_H

namespace wiselib
{
   template < typename OsModel_P, typename String_P = StaticString >
   class Queries
   {
      public:
         typedef OsModel_P OsModel;
         typedef String_P String;

         struct query {
            String name;
            String value;
         };

         typedef struct query query_t;
         typedef wiselib::vector_static<OsModel, query_t, CONF_MAX_QUERIES> queries_vector_t;
         typedef typename queries_vector_t::iterator queries_iterator_t;

         void add_query( char* my_query, uint16_t query_len )
         {
            uint16_t split_position = 0;
            for( int i = 0; i < query_len; i++ )
            {
               if( my_query[i] == 61 ) // ascii of "="
               {
                  split_position = i;
                  break;
               }
            }
            String name( my_query, split_position );
            name.append( "\0" );
            String value( my_query + split_position + 1, query_len - split_position - 1 );
            value.append( "\0" );
            query_t new_query;
            new_query.name = name;
            new_query.value = value;
            queries_.push_back( new_query );
         }

         char* value_of( String name )
         {
            for( queries_iterator_t it = queries_.begin(); it != queries_.end(); it++ )
            {
               if ( it->name == name )
               {
                  return it->value.c_str();
               }
            }
         }
      private:
         queries_vector_t queries_;
   };
}
#endif
