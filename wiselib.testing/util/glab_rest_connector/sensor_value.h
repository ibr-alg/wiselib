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

