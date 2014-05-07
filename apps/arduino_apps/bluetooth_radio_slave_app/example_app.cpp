/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_bluetooth_radio.h"

typedef wiselib::ArduinoOsModel Os;

Os::Debug debug_;
Os::BluetoothRadio radio;
class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      debug_.debug( "Hello!\n" );
      radio.enable_radio();
      radio.connect_radio(0);
      radio.reg_recv_callback<ExampleApplication, &ExampleApplication::onReceive>(this);
      while ( 1 )
      {
	if ( serialEventRun )
	  serialEventRun();
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
   
   void onReceive(Os::BluetoothRadio::node_id_t id, Os::BluetoothRadio::size_t len,Os::BluetoothRadio::block_data_t* data)
   {
     wiselib::ArduinoDebug<wiselib::ArduinoOsModel>(true).debug("Len:%d",len);
     wiselib::ArduinoDebug<wiselib::ArduinoOsModel>(true).debug("s: %s",data);
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
