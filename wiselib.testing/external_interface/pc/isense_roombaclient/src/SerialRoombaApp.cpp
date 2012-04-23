// vim: set expandtab ts=3 sw=3:

/******************************************************************************/
#undef DEBUG
//#define DEBUG
/******************************************************************************/

#include "SerialRoombaApp.h"
#include <isense/radio.h>
#include <isense/platforms/jennic/jennic_radio.h>
#include <isense/util/util.h>

SerialRoombaApp::SerialRoombaApp( Os& os ) :
   Application( os ), uart_( os.uart( 0 ) )
{
}

void SerialRoombaApp::boot()
{
   os().debug( "SerialRoombaApp up and running" );
   while( os().allow_sleep( false ) ) ;
   while( os().allow_doze( false ) ) ;
   os().dispatcher().add_receiver( this );

   uart_.set_control( 8, 'N', 1 );
   if ( uart_.set_packet_handler( isense::Uart::MESSAGE_TYPE_CUSTOM_IN_1,
      this ) )
   {
#ifdef DEBUG
      os_.debug( "set packet handler successful" );
#endif // DEBUG
   }
   else
   {
      os_.fatal( "err: set packet handler failed!" );
   }
   uart_.enable_interrupt( true );

   // send node ID on boot
   // FIXME still only works with 16-bit addresses
   uint8 buf[4];
   buf[0] = 'N';
   buf[1] = 0;
   buf[2] = os().id() >> 8;
   buf[3] = os().id();
   uart_.write_packet( isense::Uart::MESSAGE_TYPE_CUSTOM_OUT, (char *) buf,
      sizeof(buf) );
}

void SerialRoombaApp::receive( uint8 len, const uint8 * msg,
   ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr,
   uint16 signal_strength, uint16 signal_quality, uint8 sequence_no,
   uint8 interface, Time rx_time )
{
   if ( msg[0] == Uart::MESSAGE_TYPE_LOG ) // log messages
   {
      char log[266]; /* len <= 255 incl. \0, +11 in snprintf */
      int l = snprintf( log, sizeof(log), "[debug %d] %s", src_addr, msg + 1 );
      if ( l < 0 || l >= (int) sizeof(log) - 1 )
      {
         os().fatal( "snprintf error" );
         return;
      }
      // forward over serial interface
      os().debug( log );
   }

   // everything else, relay over serial interface
   else
   {
      /*
       * FIXME this does currently only work with 16-bit adresses
       * Format of Relay In Packets (Air --> iSense --> Serial --> Java):
       * Byte order: network byte order (high byte first)
       * Byte 0:           'I'
       * Byte 1:           0x0 (padding byte)
       * Bytes 2-3:        Address of the source node (uint16)
       * Bytes 4-5:        Address of the destination node (uint16)
       *                   (in fact, this is our own or the broadcast ID ;-))
       * Bytes 6-7:        Signal strength (a.k.a. LQI or RSSI, uint16)
       * Bytes 8-9:        Signal quality (a.k.a. MSQ, uint16)
       * Byte 10:          ID of the interface that received the packet (uint8)
       * Byte 11:          Sequence number of the received packet (uint8)
       * Bytes 12-15:      Local time in milliseconds when the packet was
       *                   received (uint32)
       */
      uint32 time;
      rx_time.to_millis( time );

      uint8 buf[255 + 16]; /* at least sizeof(uint8) + 16 header bytes */
      buf[0] = 'I';
      buf[1] = 0;
      buf[2] = (char) (src_addr >> 8);
      buf[3] = (char) src_addr;
      buf[4] = (char) (dest_addr >> 8);
      buf[5] = (char) dest_addr;
      buf[6] = (char) (signal_strength >> 8);
      buf[7] = (char) signal_strength;
      buf[8] = (char) (signal_quality >> 8);
      buf[9] = (char) signal_quality;
      buf[10] = (char) interface;
      buf[11] = (char) sequence_no;
      buf[12] = (char) (time >> 24);
      buf[13] = (char) (time >> 16);
      buf[14] = (char) (time >> 8);
      buf[15] = (char) time;
      // copy original message
      for ( size_t i = 0; i < len; i++ )
      {
         buf[16 + i] = msg[i];
      }
      uart_.write_packet( Uart::MESSAGE_TYPE_CUSTOM_OUT, (char *) buf, len
         + 16 );
   }
}

void SerialRoombaApp::handle_uart_packet( uint8 type, uint8 * buf,
   uint8 length )
{
   /*
    * FIXME this does currently only work with 16-bit adresses
    * Format of Relay Out Packets:  (Java --> Serial --> iSense --> Air)
    * Byte order: network byte order (high byte first)
    * Byte 0:           'O'
    * Byte 1:           transfer power (valid values (for Jennic) are: -30, -24, -18, -12, -6, 0
    * Bytes 2-3:        UID of the destination node (uint16)
    * Bytes 4-length:   Message to be relayed to the destination node
    */
   if ( type == Uart::MESSAGE_TYPE_CUSTOM_IN_1 && length > 4 && buf[0] == 'O' )
   {
      // send everything to the UID given
	  int8 tx_power = (int8)buf[1];
      uint16 uid = (buf[2] << 8) | buf[3];

      if( tx_power != 1 )
    	  os().radio().hardware_radio().set_power( -tx_power );//-tx_power is sent because of conversion problems

      os().radio().send( uid, length - 4, buf + 4, 0, this );
   }
   
   if ( type == Uart::MESSAGE_TYPE_CUSTOM_IN_1 && length == 2 && buf[0] == 'T' )
   {
      // sets the transmission power to the given value
	  int8 tx_power = (int8)buf[1]; //-tx_power is sent because of conversion problems
	  tx_power *= -1;

	  os().radio().hardware_radio().set_power( tx_power );
	  os().debug( "Changed the transmission power to %i.", tx_power );
   }

   if ( type == Uart::MESSAGE_TYPE_CUSTOM_IN_1 && length == 2 && buf[0] == 't' )
   {
      // sends the current transmission power of the radio over the uart
	  int8 tx_power = os().radio().hardware_radio().power();

	  uint8 buf[2];
	  buf[0] = 'P';
	  buf[1] = -tx_power; //We send -tx_power because of conversion problems

      uart_.write_packet( isense::Uart::MESSAGE_TYPE_CUSTOM_OUT, (char *) buf,
            sizeof(buf) );
   }

   if ( type == Uart::MESSAGE_TYPE_CUSTOM_IN_1 && length > 0 && buf[0] == 'a' )
   {
      uint8 buf[4];
      buf[0] = 'N';
      buf[1] = 0x00;
      buf[2] = os().id() << 8;
      buf[3] = os().id() & 0xff;

      uart_.write_packet( isense::Uart::MESSAGE_TYPE_CUSTOM_OUT, (char *) buf,
            sizeof(buf) );
   }
}

Application* application_factory( Os& os )
{
   return new SerialRoombaApp( os );
}
