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
 * File:   rest_response.h
 * Author: maxpagel
 *
 * Created on 27. Dezember 2010, 16:03
 */

#ifndef _REST_RESPONSE_H
#define	_REST_RESPONSE_H

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
   class SwapResponseMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      // --------------------------------------------------------------------
      inline uint8_t command_type()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }
      // --------------------------------------------------------------------
      inline void set_command_type( uint8_t type )
      { write<OsModel, block_data_t, uint8_t>( buffer, type ); }
      // --------------------------------------------------------------------
      inline uint8_t request_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer + REQUEST_ID_POS ); }
      // --------------------------------------------------------------------
      inline void set_request_id( uint8_t type )
      { write<OsModel, block_data_t, uint8_t>( buffer + REQUEST_ID_POS, type ); }
      // --------------------------------------------------------------------
      inline uint8_t payload_length()
      { return read<OsModel, block_data_t, uint8_t>( buffer + PAYLOAD_SIZE_POS ); }
      // --------------------------------------------------------------------
      inline uint8_t resonse_code()
      { return read<OsModel, block_data_t, uint8_t>( buffer + RESPONSE_CODE_POS ); }
      // --------------------------------------------------------------------
      inline void set_response_code( uint8_t dst )
      { write<OsModel, block_data_t, uint8_t>( buffer + RESPONSE_CODE_POS, dst ); }
      // --------------------------------------------------------------------
      inline block_data_t* payload()
      { return buffer + PAYLOAD_POS; }
      // --------------------------------------------------------------------
      inline void set_payload( size_t len, block_data_t *buf )
      {
         set_payload_length( len );
         memcpy( buffer + PAYLOAD_POS, buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + payload_length(); }


   private:

      inline void set_payload_length( uint8_t len )
      { write<OsModel, block_data_t, uint8_t>( buffer + PAYLOAD_SIZE_POS, len ); }
      // --------------------------------------------------------------------
      enum data_in_positions
      {
         COMMAND_TYPE     = 0,
         REQUEST_ID_POS   = 1,
         RESPONSE_CODE_POS= 2,
         PAYLOAD_SIZE_POS = 3,
         PAYLOAD_POS      = 4
      };
      // --------------------------------------------------------------------
      block_data_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
   };

}

#endif	/* _REST_RESPONSE_H */

