/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_xbee_radio.h"
#include "external_interface/arduino/arduino_debug.h"

typedef wiselib::ArduinoOsModel Os;

Os::XBeeRadio radio;
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

   void onReceive(Os::XBeeRadio::node_id_t id, Os::XBeeRadio::size_t len,Os::XBeeRadio::block_data_t* data)
   {
     debug.debug((char*)data);
     debug.debug("From: %#lx",id);
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
 			
