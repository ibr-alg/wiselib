/* 
 * File:   rest_request.h
 * Author: maxpagel
 *
 * Created on 27. Dezember 2010, 16:02
 */

#ifndef _REST_REQUEST_H
#define	_REST_REQUEST_H
#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
   class SwapRequestMessage
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
      inline void set_request_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer + REQUEST_ID_POS, id ); }
      // --------------------------------------------------------------------
      inline uint8_t request_opts_length()
      { return read<OsModel, block_data_t, uint8_t>( buffer + REQUEST_OPTS_SIZE_POS ); }
      // --------------------------------------------------------------------
      inline uint8_t request_type()
      { return read<OsModel, block_data_t, uint8_t>( buffer + REQUEST_TYPE ); }
      // --------------------------------------------------------------------
      inline void set_request_type( uint8_t dst )
      { write<OsModel, block_data_t, uint8_t>( buffer + REQUEST_TYPE, dst ); }
      // --------------------------------------------------------------------
      inline block_data_t* request_opts()
      { return buffer + REQUEST_OPTS_POS; }
      // --------------------------------------------------------------------
      inline void set_request_opts( size_t len, block_data_t *buf )
      {
         set_request_opts_length( len );
         memcpy( buffer + REQUEST_OPTS_POS, buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return REQUEST_OPTS_POS + request_opts_length(); }


   private:

      inline void set_request_opts_length( uint8_t len )
      { write<OsModel, block_data_t, uint8_t>( buffer + REQUEST_OPTS_SIZE_POS, len ); }
      // --------------------------------------------------------------------
      enum data_in_positions
      {
         COMMAND_TYPE     = 0,
         REQUEST_ID_POS   = 1,
         REQUEST_TYPE     = 2,
         REQUEST_OPTS_SIZE_POS = 3,
         REQUEST_OPTS_POS      = 4
      };
      // --------------------------------------------------------------------
      block_data_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
   };

}


#endif	/* _REST_REQUEST_H */

