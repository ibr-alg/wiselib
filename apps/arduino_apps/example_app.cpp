/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#include "external_interface/arduino/arduino_ethernet_radio.h"

typedef wiselib::ArduinoOsModel Os;

Os::Debug* debug;
Os::EthernetRadio* radio;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init()
   {
      debug = new Os::Debug();
      radio = new Os::EthernetRadio();

      Os::EthernetRadio::block_data_t message[] = "Test\0";

      radio->enable_radio();

      while ( 1 )
      {
         debug->debug( "Hello World from Example Application!\n" );

         radio->send( Os::EthernetRadio::BROADCAST_ADDRESS, 4, message );

         if ( serialEventRun ) serialEventRun();
      }

   }
   // --------------------------------------------------------------------
   ~ExampleApplication()
   {
      delete debug;
      delete radio;
   }

};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
int main( )
{
   init();

#if defined(USBCON)
   USBDevice.attach();
#endif

   example_app.init();
   return 0;
}

