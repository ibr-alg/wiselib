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
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "algorithms/routing/dsdv/dsdv_routing.h"
#include "algorithms/routing/dsr/dsr_routing.h"
#include "algorithms/routing/tree/tree_routing.h"

typedef wiselib::iSenseOsModel Os;
typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8, wiselib::DsdvRoutingTableValue<Os, Os::Radio> >
   DsdvRoutingTable;
typedef wiselib::DsdvRouting<Os, DsdvRoutingTable> dsdv_routing_t;

//typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 8, wiselib::DsrRoutingTableValue<Os::Radio, 8> >
//   DsrRoutingTable;
//typedef wiselib::DsrRouting<Os, DsrRoutingTable>
//   dsr_routing_t;

typedef wiselib::TreeRouting<Os, Os::Timer, Os::Radio> tree_routing_t;


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

   ///For registering in routing 
   virtual void receive_routing_message (uint16 src_addr, uint8 len, uint8 *buf) ;
   ///From isense::Task
   virtual void execute( void* userdata ) ;

  // --------------------------------------------------------------------
  virtual uint16 application_id( void )
  { return 0; }
  // --------------------------------------------------------------------
  virtual uint8 software_revision (void)
  { return 0; }

private:
   Os::Radio radio_;
   Os::Timer timer_;
   Os::Debug debug_;

//   tree_routing_t routing_;
//   dsr_routing_t routing_;
  dsdv_routing_t routing_;

  int channel_;
};

//----------------------------------------------------------------------------
iSenseDemoApplication::
iSenseDemoApplication(isense::Os& os)
   : isense::Application(os),
      radio_( os ),
      timer_( os ),
      debug_( os ),
      channel_(MIN_CHANNEL)
{
   routing_.init( radio_, timer_, debug_ );
}
//----------------------------------------------------------------------------
iSenseDemoApplication::
~iSenseDemoApplication()
{}
//----------------------------------------------------------------------------
void 
iSenseDemoApplication::
boot(void)
{
   os_.debug("WiselibExample::boot");
   routing_.enable_radio();
   routing_.reg_recv_callback<
      iSenseDemoApplication,
      &iSenseDemoApplication::receive_routing_message>(this);

   os_.allow_sleep(false);
   os_.add_task_in(isense::Time(MILLISECONDS), this, 0);
}
//----------------------------------------------------------------------------
void 
iSenseDemoApplication::
execute( void* userdata )
{
   os_.debug("exe");
   //os_.radio().send(1, 0, 0, 0, 0);
   routing_.send( 1, 0, 0 );
}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
receive_routing_message (uint16 src_addr, uint8 len, uint8 *buf)
{
   os_.debug("received routing message from %i", src_addr);
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
