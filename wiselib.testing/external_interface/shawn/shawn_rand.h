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
#ifndef CONNECTOR_SHAWN_RANDMODEL_H
#define CONNECTOR_SHAWN_RANDMODEL_H

#include <cstdlib>

namespace wiselib
{
   template<typename OsModel_P>
   class ShawnRandModel
   {
   public:
      typedef OsModel_P OsModel;
      typedef int rand_t;
      typedef ShawnRandModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef int value_t;
      // --------------------------------------------------------------------
      enum { RANDOM_MAX = RAND_MAX };
      // --------------------------------------------------------------------
      ShawnRandModel( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      ShawnRandModel( ShawnOs& os, value_t seed )
         : os_( os )
      {
         std::srand( seed );
      }
      // --------------------------------------------------------------------
      void srand( value_t seed )
      {
         std::srand( seed );
      }
      // --------------------------------------------------------------------
      value_t operator()()
      {
         return std::rand();
      }
      // --------------------------------------------------------------------
      value_t operator()(value_t max) {
         return std::rand()%max;
      }
   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };

}

#endif
