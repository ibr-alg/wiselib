/* 
 * File:   document_request.h
 * Author: maxpagel
 *
 * Created on 14. Mai 2012, 14:50
 */

#ifndef _DOCUMENT_REQUEST_H
#define	_DOCUMENT_REQUEST_H

#include "request_message.h"

namespace wiselib{
    template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
    class PayloadRequestMessage : public RequestMessage<OsModel_P,Radio_P>{

      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
    public:
	  
	  enum {
		  FLAG_COMPRESSED = 0x01
	  };
	  
      // -------------------------------------------------------------------
      inline uint16_t payload_length()
      { return read<OsModel, block_data_t, uint16_t>( this->buffer + PAYLOAD_SIZE_POS ); }
      // --------------------------------------------------------------------
      inline block_data_t* payload()
      { return this->buffer + PAYLOAD_POS; }
      // --------------------------------------------------------------------
      inline void set_payload( size_t len, block_data_t *buf )
      {
         set_payload_length( len );
         memcpy( this->buffer + PAYLOAD_POS, buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + payload_length(); }

      inline void set_payload_length( uint16_t len )
      { write<OsModel, block_data_t, uint16_t>( this->buffer + PAYLOAD_SIZE_POS, len ); }

	  uint8_t flags() { return read<OsModel, block_data_t, uint8_t>(this->buffer + PAYLOAD_FLAGS_POS); }
	  void set_flags(uint8_t f) {
		  write<OsModel, block_data_t, uint8_t>(this->buffer + PAYLOAD_FLAGS_POS, f);
	  }

private:
      enum positions
      {
		  PAYLOAD_FLAGS_POS = RequestMessage<OsModel_P,Radio_P>::REQUEST_TYPE + 1,
         PAYLOAD_SIZE_POS = RequestMessage<OsModel_P,Radio_P>::REQUEST_TYPE + 2,
         PAYLOAD_POS = PAYLOAD_SIZE_POS + 3
      };
    };
}

#endif	/* _DOCUMENT_REQUEST_H */

