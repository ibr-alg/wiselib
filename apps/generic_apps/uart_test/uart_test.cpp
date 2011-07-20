#include "external_interface/external_interface.h"
#ifdef SHAWN
#include "external_interface/shawn/shawn_testbedservice_uart.h"
#endif
// --------------------------------------------------------------------------
typedef wiselib::OSMODEL Os;

#ifdef SHAWN
typedef wiselib::ShawnTestbedserviceUartModel<wiselib::OSMODEL> Uart;
#else
typedef Os::Uart Uart;
#endif

class TestUart
{
   typedef TestUart self_type;
   typedef Uart::block_data_t block_data_t;
   typedef Uart::size_t size_t;

public:
   void init( Os::AppMainParameter& value )
   {
      uart_ = &wiselib::FacetProvider<Os, Uart>::get_facet( value );
      debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
      timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );

      enable();
      timer_->set_timer<self_type, &self_type::start_test>( 2000, this, 0 );
   }
   // -----------------------------------------------------------------------
   void enable()
   {
      debug_->debug( "TestUart::enable()\n" );

      uart_->enable_serial_comm();

      Uart::block_data_t x[] = "123456789:\n";
      uart_->write( sizeof(x) - 1, x );
      uart_->reg_read_callback<self_type, &self_type::receive_packet>( this );
   }
   // -----------------------------------------------------------------------
   void start_test( void* )
   {
      debug_->debug( "TestUart::start_test()\n" );

      Uart::block_data_t x[] = " Testmessage printed via uart_->write!\n";
      uart_->write( sizeof(x), x );

      timer_->set_timer<self_type, &self_type::start_test>( 10000, this, 0 );
   }
   // -----------------------------------------------------------------------
   void receive_packet( size_t len, block_data_t *buf )
   {
      debug_->debug( "TestUart::received len %i, message is '%s'\n", len, buf );
      uart_->write( len, (Uart::block_data_t*)buf );
   }

private:
   Uart::self_pointer_t uart_;
   Os::Debug::self_pointer_t debug_;
   Os::Timer::self_pointer_t timer_;
};
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

wiselib::WiselibApplication<Os, TestUart> uart_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
   uart_app.init( value );
}
