/************************************************************************
 ** This file is part of the the iSense project.
 ** Copyright (C) 2006 coalesenses GmbH (http://www.coalesenses.com)
 ** ALL RIGHTS RESERVED.
 ************************************************************************/
#include <isense/application.h>
#include <isense/os.h>
#include <isense/dispatcher.h>
#include <isense/radio.h>
#include <isense/hardware_radio.h>
#include <isense/task.h>
#include <isense/timeout_handler.h>
#include <isense/isense.h>
#include <isense/uart.h>
#include <isense/dispatcher.h>
#include <isense/time.h>
#include <isense/button_handler.h>
#include <isense/sleep_handler.h>
#include <isense/modules/pacemate_module/pacemate_module.h>
#include <isense/util/util.h>

#include "external_interface/isense/isense_os.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_timer.h"
#include "external_interface/isense/isense_debug.h"
#include "external_interface/isense/isense_clock.h"
#include "external_interface/isense/isense_extended_time.h"


typedef wiselib::iSenseOsModel Os;
typedef wiselib::iSenseExtendedTime<Os> ExtendedTime;
typedef wiselib::iSenseClockModel<Os, ExtendedTime> Clock;


#define MILLISECONDS 10000
#define MIN_CHANNEL  11
#define MAX_CHANNEL  26
//----------------------------------------------------------------------------
/**
 */
class iSenseDemoApplication :
   public isense::Application,
   public isense::Task
{
public:
   iSenseDemoApplication(isense::Os& os);

   virtual ~iSenseDemoApplication() ;

   ///From isense::Application
   virtual void boot (void) ;

   ///From isense::Task
   virtual void execute( void* userdata ) ;

   void print_time( Clock::time_t& t );

private:
   Os::Radio radio_;
   Os::Timer timer_;
   Os::Debug debug_;
   Clock clock_;
};

//----------------------------------------------------------------------------
iSenseDemoApplication::
iSenseDemoApplication(isense::Os& os)
   : isense::Application(os),
      radio_( os ),
      timer_( os ),
      debug_( os ),
      clock_( os )
{}
//----------------------------------------------------------------------------
iSenseDemoApplication::
~iSenseDemoApplication()
{}
//----------------------------------------------------------------------------
void 
iSenseDemoApplication::
boot(void)
{
   os_.debug("ExtendedClock::boot");

   Clock::time_t t = clock_.time();
   print_time(t);
   t = clock_.time();
   print_time(t);
   os_.debug("+ 45" );
   t += 45;
   print_time(t);
   os_.debug("+ t" );
   t += t;
   print_time(t);
   os_.debug("t2 = t" );
   Clock::time_t t2(t);
   os_.debug("t2 == t? %d", t2 == t);
   os_.debug("t2 != t? %d", t2 != t);
   os_.debug("t2 >= t? %d", t2 >= t);
   os_.debug("t2 < t? %d", t2 < t);
   os_.debug("t2 -= 1" );
   t2 -= 1;
   os_.debug("t2 == t? %d", t2 == t);
   os_.debug("t2 != t? %d", t2 != t);
   os_.debug("t2 >= t? %d", t2 >= t);
   os_.debug("t2 < t? %d", t2 < t);
   os_.debug(" -------" );
   os_.debug("t /= 2" );
   t /= 2;
   print_time(t);
   os_.debug("t /= 2" );
   t /= 2;
   print_time(t);
   os_.debug("t *= 34" );
   t *= 34;
   print_time(t);

   os_.allow_sleep(false);
   os_.add_task_in(isense::Time(MILLISECONDS), this, 0);
}
//----------------------------------------------------------------------------
void 
iSenseDemoApplication::
execute( void* userdata )
{}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
print_time( Clock::time_t& t )
{
   os_.debug("Clock sec: %d ms: %d us: %d", clock_.seconds(t),
               clock_.milliseconds(t), clock_.microseconds(t) );
   os_.debug(" ---- " );
}
//----------------------------------------------------------------------------
/**
  */
isense::Application* application_factory(isense::Os& os)
{
   return new iSenseDemoApplication(os);
}


/*-----------------------------------------------------------------------
* Source  $Source: $
* Version $Revision: 1.24 $
* Date    $Date: 2006/10/19 12:37:49 $
*-----------------------------------------------------------------------
* $Log$
*-----------------------------------------------------------------------*/
