/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "algorithms/neighbor_discovery/arduino_zeroconf.h"
#include "external_interface/arduino/arduino_xbeeS2_radio.h"
#include "external_interface/arduino/arduino_debug.h"
typedef wiselib::ArduinoOsModel Os;

Os::XBeeS2Radio radio;
Os::Debug debug;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      radio.enable_radio();
      radio.reg_recv_callback<ExampleApplication, &ExampleApplication::onReceive>(this);
      while ( 1 )
      {
	if ( serialEventRun )
	{
	  serialEventRun();
	}
      }
   }

   void onReceive(Os::XBeeS2Radio::node_id_t id, Os::XBeeS2Radio::size_t len,Os::XBeeS2Radio::block_data_t* data)
   {
     debug.debug((char*)data);
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
 			
