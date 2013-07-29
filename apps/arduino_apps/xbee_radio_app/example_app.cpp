/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "algorithms/neighbor_discovery/arduino_zeroconf.h"
#include "external_interface/arduino/arduino_xbee_radio.h"

typedef wiselib::ArduinoOsModel Os;

Os::XBeeRadio radio;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      Os::XBeeRadio::block_data_t message[] = "Hello World";
      radio.enable_radio();

      while ( 1 )
      {
	radio.send( Os::XBeeRadio::BROADCAST_ADDRESS, 11, message);
	if ( serialEventRun )
	{
	  serialEventRun();
	}
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
 			
