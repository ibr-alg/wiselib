
#include <iostream>

#include "external_interface/pc/pc_os_model.h"
#include "external_interface/pc/pc_timer.h"
#include "external_interface/pc/pc_clock.h"

using namespace wiselib;

typedef PCOsModel Os;

class MyClass
{
public:
   MyClass()
   {
      timer = new Os::Timer();
      timer->set_timer<MyClass, &MyClass::on_time>( 3000, this, ( void* )timer );
   }

   ~MyClass()
   {
      delete timer;
      delete clock;
   }

   void on_time( void* )
   {
      clock = new Os::Clock();
      Os::Clock::time_t t = clock->time();
      std::cout << "Seconds: " << clock->seconds ( t ) << " ";
      std::cout << "Millis: " << clock->milliseconds ( t ) << " ";
      std::cout << "Micros: " << clock->microseconds ( t ) << std::endl;
      std::cout << "It's time!" << std::endl;
      timer->set_timer<MyClass, &MyClass::on_time>( 1000, this, ( void* )timer );
      delete clock;
   }

private:
   Os::Timer* timer;
   Os::Clock* clock;
};


int main( int argc, char** argv )
{

   MyClass c;

   // Loop forever in a resource-efficient way
   // so timer events will actually occur
   while( true ) pause();

   return 0;
}


