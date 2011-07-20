/************************************************************************
 ** This file is part of the the iSense project.
 ** Copyright (C) 2006 coalesenses GmbH (http://www.coalesenses.com)
 ** ALL RIGHTS RESERVED.
 ************************************************************************/
#include <isense/application.h>
#include <isense/os.h>
#include <isense/task.h>
#include <isense/isense.h>
#include <isense/uart.h>
#include <isense/button_handler.h>
#include <isense/util/util.h>
#include <isense/modules/gateway_module/gateway_module.h>

#include "external_interface/isense/isense_os.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_clock.h"
#include "external_interface/isense/isense_timer.h"
#include "external_interface/isense/isense_debug.h"
#include "external_interface/isense/isense_com_uart.h"
#include "util/wisebed_node_api/virtual_radio.h"
#include "algorithms/metrics/one_hop_link/one_hop_link_metrics.h"
#include "util/pstl/vector_static.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MESSAGE_TYPE_VIRTUAL_RADIO_IN  (7)
#define MESSAGE_TYPE_VIRTUAL_RADIO_OUT (114)
#define MESSAGES              (10)
#define MESSAGE_SIZE          (10)
#define TRANSMISSION_INTERVAL (100)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
typedef wiselib::iSenseOsModel Os;

typedef wiselib::iSenseSerialComUartModel<Os> Uart;
typedef wiselib::VirtualRadioModel<Os, Os::Radio, Uart> VirtualRadio;

typedef wiselib::iSenseClockModel<Os> iSenseClock;

typedef wiselib::vector_static<Os, wiselib::OneHopLinkMetricsDataItem<VirtualRadio::node_id_t>, 10> MetricsDataItemContainer;

typedef wiselib::OneHopLinkMetrics<Os, VirtualRadio, Os::Timer, iSenseClock, MetricsDataItemContainer, Os::Debug> OneHopMetrics;
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 */
class iSenseDemoApplication :
   public isense::Application,
   public isense::Task,
   public isense::ButtonHandler
{
public:
   iSenseDemoApplication( isense::Os& os );

   virtual ~iSenseDemoApplication() ;

   ///From isense::Application
   virtual void boot (void) ;

   virtual void execute( void* userdata );

   ///From isense::ButtonHandler
   virtual void button_down( uint8 button );

private:
   void send_control_message( void );
   void evaluation( void );

   isense::GatewayModule gw_;
   bool control_message_;
   Uart uart_;
   Os::Radio radio_;
   Os::Timer timer_;
   iSenseClock clock_;
   Os::Debug debug_;
   VirtualRadio virtual_radio_;
   OneHopMetrics one_hop_metrics_;
};
//----------------------------------------------------------------------------
iSenseDemoApplication::
iSenseDemoApplication( isense::Os& os )
   : isense::Application ( os ),
      gw_                ( os ),
      control_message_   ( true ),
      uart_( os ),
      radio_( os ),
      timer_( os ),
      clock_( os ),
      debug_( os )
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
   os_.debug("WiselibMetrics::boot on %d", os_.id());

   virtual_radio_.init( radio_, uart_, debug_ );
   one_hop_metrics_.init( virtual_radio_, timer_, clock_, debug_ );

   gw_.set_button_handler( this );

   os_.allow_sleep( false );

   if ( os_.id() == 7 )
   {
      send_control_message();
      os_.add_task_in( isense::Time(10000), this, 0 );
   }
}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
execute( void* userdata )
{
   if( control_message_ )
   {
      send_control_message();
      control_message_ = false;
   }
   else
   {
      evaluation();
      control_message_ = true;
   }

//    send_control_message();
   os_.add_task_in( isense::Time(10000), this, 0 );
}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
button_down( uint8 button )
{
   os_.debug("WiselibMetrics::button %i down", button);

   if( button == 0 )
   {
      send_control_message();
   }
   else
   {
      evaluation();
   }
}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
send_control_message( void )
{
   os_.debug("Sending control packet");
   one_hop_metrics_.start( TRANSMISSION_INTERVAL, MESSAGES, MESSAGE_SIZE );
}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
evaluation( void )
{
   os_.debug("Packet Stats:");

   OneHopMetrics::MetricsData& md = one_hop_metrics_.metrics_data();

   for ( OneHopMetrics::MetricsDataContainerIterator
            it = md.link_metrics.begin();
            it != md.link_metrics.end();
            ++it )
   {
      os_.debug("Received %i of %i packets from %i",
         (*it).packets_received, MESSAGES, (*it).id );
   }

   int nodes = md.link_metrics.size();
   os_.debug("Total packets Received: %i of %i", md.total_received, nodes * MESSAGES );

   uint32 start_millis, stop_millis, total_millis;
   md.start_time.to_millis( start_millis );
   md.stop_time.to_millis( stop_millis );
   isense::Time total = md.stop_time - md.start_time;
   total.to_millis( total_millis );

   os_.debug("Start time: %i ms", start_millis );
   os_.debug("Stop time : %i ms", stop_millis );
   os_.debug("Total time: %i ms", total_millis );
}
//----------------------------------------------------------------------------
isense::Application* application_factory( isense::Os& os )
{
   return new iSenseDemoApplication(os);
}
