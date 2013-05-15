/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#include "algorithms/neighbor_discovery/arduino_zeroconf.h"
#include "external_interface/arduino/arduino_bluetooth_radio.h"

typedef wiselib::ArduinoOsModel Os;

Os::Debug debug;
Os::Clock clock;
Os::BluetoothRadio radio;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      Os::BluetoothRadio::block_data_t message[] = "Test\0";

      radio.enable_radio();

      wiselib::ArduinoZeroconf<Os, Os::BluetoothRadio, Os::Debug> mdns;
      mdns.init( radio, clock, debug );
      mdns.enable();

      debug.debug( "Hello World from Example Application!\n" );

      while ( 1 )
      {
         mdns.topology();
         radio.send( Os::BluetoothRadio::BROADCAST_ADDRESS, 4, message);

         if ( serialEventRun ) serialEventRun();
      }

   }
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
int main(Os::AppMainParameter& amp)
{
   init();
#if defined(USBCON)
   USBDevice.attach();
#endif
   example_app.init(amp);
   return 0;
}
			
