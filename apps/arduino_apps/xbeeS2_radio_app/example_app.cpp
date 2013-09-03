/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "algorithms/neighbor_discovery/arduino_zeroconf.h"
#include "external_interface/arduino/arduino_xbeeS2_radio.h"

typedef wiselib::ArduinoOsModel Os;

Os::XBeeS2Radio radio;
Os::Debug debug;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      Os::XBeeS2Radio::block_data_t message[] = {'h','i'};
      if(radio.enable_radio()==-1)
	debug.debug("Error!");
      else
	debug.debug("Success");
      Os::XBeeS2Radio::node_id_t dest;
      dest[0] = 0x0013a200;
      dest[1] = 0x40a11e78;
      radio.send( dest, 2, message);
      while ( 1 )
      {
	radio.send( dest, 2, message);
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
 			
