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

