/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#include "external_interface/arduino/arduino_bluetooth_radio.h"

#include <avr/sleep.h>
#include <avr/wdt.h>

typedef wiselib::ArduinoOsModel Os;

Os::Debug debug_;
Os::Clock clock;
Os::Timer* timer;
Os::BluetoothRadio radio;

Os::BluetoothRadio::block_data_t message[] = "Test\0";
class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init(Os::AppMainParameter& amp)
   {
      debug_.debug( "Hello!" );
      radio.get_slave_name(";BTSlave");
      radio.enable_radio();
      radio.connect_radio(1);
      timer->set_timer<ExampleApplication , &ExampleApplication::send_packet> ( 50, this , ( void* )timer );

      for(;;)
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
   
   void onReceive(Os::BluetoothRadio::node_id_t id, Os::BluetoothRadio::size_t len,Os::BluetoothRadio::block_data_t* data)
   {
     //your wish, my "command"
   }

   void send_packet(void*)
   {
	  radio.send( Os::BluetoothRadio::BROADCAST_ADDRESS, 5, message);
	  debug_.debug((char*)message);
	  timer->set_timer<ExampleApplication , &ExampleApplication::send_packet> ( 50, this , ( void* )timer );
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

