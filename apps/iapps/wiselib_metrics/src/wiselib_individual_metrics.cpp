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
#include "algorithms/metrics/individual_link/individual_link_metrics.h"
#include "util/pstl/vector_static.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MESSAGE_TYPE_VIRTUAL_RADIO_IN (7)
#define MESSAGE_TYPE_VIRTUAL_RADIO_OUT (114)
#define MESSAGES              (100)
#define TRANSMISSION_INTERVAL (200)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
typedef wiselib::iSenseOsModel Os;

typedef wiselib::iSenseSerialComUartModel<Os> Uart;
typedef wiselib::VirtualRadioModel<Os, Os::Radio, Uart> VirtualRadio;

typedef wiselib::iSenseClockModel<Os> Clock;
typedef wiselib::vector_static<Os, Clock::time_t, 100> MetricsDataItemContainer;
typedef wiselib::IndividualLinkMetrics<Os, VirtualRadio, Os::Timer, Clock, Os::Debug, MetricsDataItemContainer> IndividualLinkMetrics;
//typedef wiselib::IndividualLinkMetrics<Os, Os::Radio, Os::Timer,
//                                       Clock, Os::Debug, MetricsDataItemContainer>
//   IndividualLinkMetrics;
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 */
class iSenseDemoApplication :
   public isense::Application,
   public isense::ButtonHandler
{
public:
   iSenseDemoApplication( isense::Os& os );

   virtual ~iSenseDemoApplication() ;

   ///From isense::Application
   virtual void boot (void) ;

   ///From isense::ButtonHandler
   virtual void button_down( uint8 button );

private:
   isense::GatewayModule gw_;
   Uart uart_;
   Os::Radio radio_;
   Os::Timer timer_;
   Clock clock_;
   Os::Debug debug_;
   VirtualRadio virtual_radio_;
   IndividualLinkMetrics individual_metrics_;
};
//----------------------------------------------------------------------------
iSenseDemoApplication::
iSenseDemoApplication( isense::Os& os )
   : isense::Application ( os ),
      gw_                ( os ),
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
   os_.debug("WiselibMetrics::boot");
   virtual_radio_.init( radio_, uart_, debug_ );
	virtual_radio_.enable_radio();
   individual_metrics_.init( virtual_radio_, timer_, clock_, debug_ );

   gw_.set_button_handler( this );

   os_.allow_sleep( false );
}
//----------------------------------------------------------------------------
void
iSenseDemoApplication::
button_down( uint8 button )
{
   os_.debug("WiselibMetrics::button %i down", button);

   if( button == 0 )
   {
//       uint16 receiver = 38032;
//       if ( os_.id() == receiver )
//          receiver = 1544;
      uint16 receiver = 2;

      os_.debug("Start link evaluation with %i ", receiver );
      individual_metrics_.start( receiver, TRANSMISSION_INTERVAL, MESSAGES );
   }
   else
   {
      os_.debug("Individual Link Stats:");

      IndividualLinkMetrics::MetricsData& md = individual_metrics_.metrics_data();

      os_.debug("  Data Items  :");
      uint32 min = 0xffffffff, max = 0, avg = 0;
      int cnt = 0;
      for ( IndividualLinkMetrics::DataItemContainerIterator
               it = md.link_times.begin();
               it != md.link_times.end();
               ++it )
      {
         uint32 tmp;
         (*it).to_millis( tmp );

         os_.debug("    -> Item %i: %i", cnt++, tmp );

         if ( tmp > max )
            max = tmp;
         if ( tmp < min )
            min = tmp;
         avg += tmp;
      }
      avg /= md.link_times.size();

      os_.debug("  Total sent  : %i", md.total_sent );
      os_.debug("  Lost        : %i", md.lost );
      os_.debug("  Min duration: %i ms", min );
      os_.debug("  Max duration: %i ms", max );
      os_.debug("  Avg duration: %i ms", avg );
   }
}
//----------------------------------------------------------------------------
isense::Application* application_factory( isense::Os& os )
{
   return new iSenseDemoApplication(os);
}
