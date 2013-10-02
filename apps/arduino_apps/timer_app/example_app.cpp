/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#include "external_interface/arduino/arduino_timer.h"

#include <avr/sleep.h>
#include <avr/wdt.h>

typedef wiselib::ArduinoOsModel Os;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   ExampleApplication()
   {
      timer = new Os::Timer(); 
//       debug.debug("H!");
      timer->set_timer<ExampleApplication, &ExampleApplication::on_time>( 1000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_third_time>( 3000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_fifth_time>( 5000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_seventh_time>( 7000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_second_time>( 2000, this, (void* )timer);
      timer->set_timer<ExampleApplication, &ExampleApplication::on_fourth_time>( 4000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_sixth_time>( 6000, this, ( void* )timer );
      timer->set_timer<ExampleApplication, &ExampleApplication::on_eigth_time>( 8000, this, ( void* )timer );
   }
 
   void on_time(void*)
   {
     ::Serial.write("1");
      timer->set_timer<ExampleApplication, &ExampleApplication::on_ninth_time>( 9000, this, ( void* )timer );
   }

   void on_second_time(void*)
   {
      ::Serial.write("2");
   }

   void on_third_time(void*)
   {
      ::Serial.write("3");
   }

   void on_fourth_time(void*)
   {
      ::Serial.write("4");
   }

   void on_fifth_time(void*)
   {
      ::Serial.write("5");
   }

   void on_sixth_time(void*)
   {
      ::Serial.write("6");
   }

   void on_seventh_time(void*)
   {
      ::Serial.write("7");
   }

   void on_eigth_time(void*)
   {
      ::Serial.write("8");
   }

   void on_ninth_time(void*)
   {
      ::Serial.write("9");
   }

private:
   Os::Timer* timer;
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
   ::Serial.begin(9600);
   for(;;)
   {
     if ( serialEventRun ) serialEventRun();
     while(true)
     {
       cli();
       if(wiselib::ArduinoTask::tasks_.empty())
       {
//          #if ARDUINO_ALLOW_SLEEP
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
//          #endif
	sei();
	delay(10);
       }
       else
       {
	 sei();
	 break;
       }
     }
     wiselib::ArduinoTask t = wiselib::ArduinoTask::tasks_.front();
     wiselib::ArduinoTask::tasks_.pop();
     t.callback_(t.userdata_);
     delay(10);
   }
    return 0;
}


