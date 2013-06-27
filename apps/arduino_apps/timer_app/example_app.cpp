/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#include "external_interface/arduino/arduino_timer.h"

typedef wiselib::ArduinoOsModel Os;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   ExampleApplication()
   {
      timer = new Os::Timer(); 
      debug = new Os::Debug();
      debug->debug("Timer Event Test App");
      timer->set_timer<ExampleApplication, &ExampleApplication::on_time>( 3000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_second_time>( 1000, this, (void* )timer);
      while (true)
      {
         timer->run_arduino_timer();
         //if ( serialEventRun ) serialEventRun();
      }

   }
 
   void on_time(void*)
   {
      debug->debug("Three second timer event");
   }

   void on_second_time(void*)
   {
      debug->debug("One second timer event");
   }

private:
   Os::Timer* timer;
   Os::Debug* debug;

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
   return 0;
}
			
