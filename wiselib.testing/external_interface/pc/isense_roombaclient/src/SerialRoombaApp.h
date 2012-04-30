#ifndef SERIAL_ROOMBA_APP_H
#define SERIAL_ROOMBA_APP_H

#include <isense/isense.h>
#include <isense/application.h>
#include <isense/dispatcher.h>
#include <isense/os.h>
#include <isense/radio.h>
#include <isense/uart.h>
#include <isense/data_handlers.h>
//#include <isense/task.h>

using namespace isense;

/**
 * This application has to be flashed
 */
class SerialRoombaApp:
   public Application,
   public Receiver,
   public Sender,
   public UartPacketHandler
{
private:
   Uart& uart_;

public:
   SerialRoombaApp( Os& os );
   virtual ~SerialRoombaApp()
   {
   }

   virtual void boot( void );
   virtual void receive( uint8 len, const uint8 * msg,
      ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr,
      uint16 signal_strength, uint16 signal_quality, uint8 sequence_no,
      uint8 interface, Time rx_time );
   virtual void confirm( uint8 type, uint8 tries, Time time )
   {
   }
   virtual void handle_uart_packet( uint8 type, uint8 * buf, uint8 length );

};

Application* application_factory( Os& );

#endif // SERIAL_ROOMBA_APP_H
