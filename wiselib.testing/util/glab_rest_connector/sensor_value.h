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
 * File:   sensor_value.h
 * Author: maxpagel
 *
 * Created on 5. Januar 2011, 11:46
 */

#ifndef _SENSOR_VALUE_H
#define	_SENSOR_VALUE_H
#include "util/serialization/simple_types.h"
namespace wiselib
{
template<typename OsModel_P,typename Encoding_P,
        typename Radio_P = typename OsModel_P::Radio>
   class SensorValue
   {
   public:
      typedef OsModel_P OsModel;
      typedef Encoding_P Encoding;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      // --------------------------------------------------------------------
      inline uint8_t encoding()
      { return read<OsModel, block_data_t, uint8_t >( buffer ); }
      // --------------------------------------------------------------------
      inline void set_encoding( uint8_t encoding )
      { write<OsModel, block_data_t, uint8_t >( buffer, encoding ); }
      // --------------------------------------------------------------------
      inline void set_value(Encoding value )
      {
         set_length( sizeof( Encoding) );
         write<OsModel, block_data_t, Encoding >( buffer + VALUE_POS, value  );
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return VALUE_POS + value_length(); }

      inline uint8_t value_length()
      {
        return read<OsModel,block_data_t,uint8_t >(buffer + VALUE_LEN_POS);
      }
   private:

      inline void set_length( uint8_t len )
      { write<OsModel, block_data_t, uint8_t >( buffer + VALUE_LEN_POS, len ); }
      // --------------------------------------------------------------------
      enum data_in_positions
      {
         ENCODING     = 0,
         VALUE_LEN_POS   = 1,
         VALUE_POS   = 2
      };
      // --------------------------------------------------------------------
      block_data_t buffer[3];
   };
}
#endif	/* _SENSOR_VALUE_H */

