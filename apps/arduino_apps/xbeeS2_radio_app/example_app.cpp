/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
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
      Os::XBeeS2Radio::block_data_t message[] = {'h','i','\0'};
      //if(radio.enable_radio()==-1)
	//debug.debug("Error!");
      //else
	//debug.debug("Success");
      radio.enable_radio();
      debug.debug("Hello !");
      Os::XBeeS2Radio::node_id_t dest;
      uint32_t MSB = 0x0013a200;	//Enter the destination SH
      uint32_t LSB = 0x40a11ed4;	//Enter the destination SL
      dest = (uint64_t)MSB;
      dest = (dest<<32);
      dest |= (uint64_t)LSB;
      radio.send( dest, 2, message);
      while ( 1 )
      {
	radio.send( dest, 2, message);
	debug.debug((char*)message);
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
};
// --------------------------------------------------------------------------
//wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
int main(Os::AppMainParameter& amp)
{
    init();
#if defined(USBCON)
    USBDevice.attach();
#endif
   ExampleApplication example_app;
   example_app.init(amp);
   return 0;
}
 			
