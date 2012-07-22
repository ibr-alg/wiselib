/*
 * Simple Wiselib Example
 */
#include "external_interface/arduino/arduino_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "external_interface/arduino/arduino_debug.h"


typedef wiselib::ArduinoOsModel Os;

Os::Debug* debug;

class ExampleApplication
{
public:
   // --------------------------------------------------------------------
   void init()
   {
      debug = new Os::Debug();

      while ( 1 )
      {
         debug->debug( "Hello World from Example Application!\n" );

         if ( serialEventRun ) serialEventRun();
      }

   }
   // --------------------------------------------------------------------
   ~ExampleApplication()
   {
      delete debug;
   }

};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
int main( )
{
   init();

#if defined(USBCON)
   USBDevice.attach();
#endif

   example_app.init();
   return 0;
}

