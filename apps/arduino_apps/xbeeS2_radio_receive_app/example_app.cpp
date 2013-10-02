/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_xbeeS2_radio.h"
#include "external_interface/arduino/arduino_debug.h"
typedef wiselib::ArduinoOsModel Os;

Os::XBeeS2Radio radio;
Os::Debug debug;

class ExampleApplication
{public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      radio.enable_radio();
      debug.debug("Hello !");
      radio.reg_recv_callback<ExampleApplication, &ExampleApplication::onReceive>(this);
      while ( 1 )
      {
	if ( serialEventRun ) serialEventRun();
	if(wiselib::ArduinoTask::tasks_.empty());
	else
	{
	  wiselib::ArduinoTask t = wiselib::ArduinoTask::tasks_.front();
	  wiselib::ArduinoTask::tasks_.pop();
	  t.callback_(t.userdata_);
	  delay(10);
	}
      }
   }

   void onReceive(Os::XBeeS2Radio::node_id_t id, Os::XBeeS2Radio::size_t len,Os::XBeeS2Radio::block_data_t* data)
   {
     debug.debug((char*)data);
     uint32_t lsb = (uint32_t)id;
     id &= 0xFFFFFFFF00000000;
     uint32_t msb = (uint32_t)(id>>32);
     debug.debug("Received from: 0x%lx %lx",msb,lsb);
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
 			
