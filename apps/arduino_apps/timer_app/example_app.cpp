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
      timer->set_timer<ExampleApplication, &ExampleApplication::on_time>( 1000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_third_time>( 3000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_fifth_time>( 5000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_seventh_time>( 7000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_ninth_time>( 9000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_second_time>( 2000, this, (void* )timer);
      timer->set_timer<ExampleApplication, &ExampleApplication::on_fourth_time>( 4000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_sixth_time>( 6000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_eigth_time>( 8000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_tenth_time>( 10000, this, ( void* )timer );
   }

   void on_time(void*)
   {
      debug->debug("One second timer event");
      timer->set_timer<ExampleApplication, &ExampleApplication::on_eleventh_time>( 11000, this, ( void* )timer );
   }

   void on_second_time(void*)
   {
      debug->debug("Two second timer event");
   }

   void on_third_time(void*)
   {
      debug->debug("Three second timer event");
   }

   void on_fourth_time(void*)
   {
      debug->debug("Four second timer event");
   }

   void on_fifth_time(void*)
   {
      debug->debug("Five second timer event");
   }

   void on_sixth_time(void*)
   {
      debug->debug("Six second timer event");
   }

   void on_seventh_time(void*)
   {
      debug->debug("Seven second timer event");
   }

   void on_eigth_time(void*)
   {
      debug->debug("Eight second timer event");
   }

   void on_ninth_time(void*)
   {
      debug->debug("Nine second timer event");
   }

   void on_tenth_time(void*)
   {
      debug->debug("Ten second timer event");
   }

   void on_eleventh_time(void*)
   {
      debug->debug("Eleven second timer event");
   }

private:
   Os::Timer* timer;
   Os::Debug* debug;
};
// --------------------------------------------------------------------------
//wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
int main()
{
   init();

#if defined(USBCON)
   USBDevice.attach();
#endif
   ExampleApplication Example;

   while(1)
   {
      if ( serialEventRun ) serialEventRun();
   }
   return 0;
}


