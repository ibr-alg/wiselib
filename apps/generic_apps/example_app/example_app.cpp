/*
 * Simple Wiselib Example
 */
#include "external_interface/external_interface.h"
//#include "integer.h"

typedef wiselib::OSMODEL Os;

class ExampleApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         //radio_->enable_radio();

         debug_->debug( "Hello World from Example Application!\n" );
  //       DWORD abc;
  //      abc = 5;
  //      debug_->debug("%x %d %lx %ld",abc, abc, abc, abc);

         //radio_->reg_recv_callback<ExampleApplication,
                                   //&ExampleApplication::receive_radio_message>( this );
         timer_->set_timer<ExampleApplication,
                           &ExampleApplication::start>( 5000, this, 0 );
      }
      // --------------------------------------------------------------------
      void start( void* )
      {
         debug_->debug( "broadcast message at %lu \n", (unsigned long)radio_->id() );
         Os::Radio::block_data_t message[] = "hello!\0";
         radio_->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );

         // following can be used for periodic messages to sink
         timer_->set_timer<ExampleApplication,
                          &ExampleApplication::start>( 5000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_radio_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "received msg at %lu from %lu", (unsigned long)radio_->id(), (unsigned long)from );
         debug_->debug( "  message is %s\n", buf );
      }
   private:
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
