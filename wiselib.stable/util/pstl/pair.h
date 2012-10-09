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
#ifndef WISELIB_DATA_STRUCTURES_MISC_PAIR
#define WISELIB_DATA_STRUCTURES_MISC_PAIR

namespace wiselib
{

   template <typename First, typename Second>
   struct pair
   {
      typedef First first_type;
      typedef Second second_type;

      First first;
      Second second;

      pair()
         : first  ( First() ),
            second ( Second() )
      {}

      pair( const First& a, const Second& b )
         : first  ( a ),
         second ( b )
      {}

      template<typename First2, typename Second2>
      pair( const pair<First2, Second2>& pair2 )
         : first  ( pair2.first ),
         second ( pair2.second )
      {}

   //    const pair<First, Second>& operator=( const pair<First, Second>& pair2 )
   //    {
   //       *this.first = pair2.first;
   //       *this.second = pair2.second;
   //
   //       return *this;
   //    }

   };
   
   template <class First, class Second>
   inline bool operator < ( const pair<First, Second>& p1, const pair<First, Second>& p2)
   {
      return p1.first < p2.first ||
      ( !( p2.first < p1.first ) && p1.second < p2.second ); 
   }
   
   template <class First, class Second>
   inline bool operator==(const pair<First, Second>& p1, const pair<First, Second>& p2)
   {
      return p1.first == p2.first && p1.second == p2.second;
   }

}

#endif
