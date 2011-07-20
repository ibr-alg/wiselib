#include "external_interface/external_interface.h"
#include "util/wisebed_node_api/virtual_radio.h"

#ifdef CONTIKI
#include "util/wisebed_node_api/uart_packet_extractor.h"
#endif

typedef wiselib::OSMODEL Os;
typedef Os::Uart uart_t;

#ifndef SHAWN

   #ifdef CONTIKI
      typedef wiselib::UartPacketExtractor<Os, uart_t> uart_extractor_t;
      typedef wiselib::VirtualRadioModel<Os, Os::Radio, uart_extractor_t> virtual_radio_t;
   #else
      typedef wiselib::VirtualRadioModel<Os, Os::Radio, uart_t> virtual_radio_t;
   #endif
#endif

class VirtualRadioTestApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         uart_ = &wiselib::FacetProvider<Os, uart_t>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

#ifdef ISENSE
         radio_->set_channel(17);
#endif

#ifndef SHAWN

   #ifdef CONTIKI
         uart_extractor_.init( *uart_ );
         virtual_radio_.init( *radio_, uart_extractor_, *debug_ );
   #else
         virtual_radio_.init( *radio_, *uart_, *debug_ );
   #endif
         virtual_radio_.enable_radio();
         virtual_radio_.reg_recv_callback<VirtualRadioTestApplication, &VirtualRadioTestApplication::receive_message>( this );
#else
         radio_->reg_recv_callback<VirtualRadioTestApplication, &VirtualRadioTestApplication::receive_message>( this );
#endif

         timer_->set_timer<VirtualRadioTestApplication,
                         &VirtualRadioTestApplication::start>( 500, this, 0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
#ifndef SHAWN
         virtual_radio_t::block_data_t message[] = "test vlinks\0";
         virtual_radio_.send( virtual_radio_t::BROADCAST_ADDRESS, sizeof(message), message );
#else
         Os::Radio::block_data_t message[] = "test vlinks\0";
         radio_->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );
#endif
         debug_->debug("broadcasted message at %u: '%s'\n", radio_->id(), message);

         timer_->set_timer<VirtualRadioTestApplication,
                         &VirtualRadioTestApplication::start>( 5000, this, 0 );
      }
      // --------------------------------------------------------------------
  void receive_message( Os::Radio::node_id_t from,
         Os::Radio::size_t len,
         Os::Radio::block_data_t *buf )
      {
         debug_->debug("received message at %u '%s' from node %d\n", radio_->id(), buf, from);
      }

   private:
#ifndef SHAWN
      virtual_radio_t virtual_radio_;
#endif
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
      uart_t::self_pointer_t uart_;
#ifdef CONTIKI
      uart_extractor_t uart_extractor_;
#endif
};
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, VirtualRadioTestApplication> vtest_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
   vtest_app.init( value );
}
