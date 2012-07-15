
#include "external_interface/external_interface.h"
#include "algorithms/6lowpan/ipv6_address.h"

#include "algorithms/6lowpan/ipv6_stack.h"

#ifdef SHAWN
#include "external_interface/shawn/shawn_testbedservice_uart.h"
#endif

typedef wiselib::OSMODEL Os;
typedef Os::Radio Radio;
typedef Os::Radio::node_id_t node_id_t;

#ifdef SHAWN
typedef wiselib::ShawnTestbedserviceUartModel<wiselib::OSMODEL> Uart;
#else
typedef Os::B_Uart Uart;
#endif


typedef wiselib::IPv6Stack<Os, Radio, Os::Debug, Os::Timer, Uart> IPv6_stack_t;

typedef wiselib::IPv6Address<Radio, Os::Debug> IPv6Address_t;


class lowpanApp
{
  public:
    typedef Radio::block_data_t block_data_t;
    typedef Radio::node_id_t node_id_t;
    typedef Radio::size_t size_t;
    typedef Radio::message_id_t message_id_t;
    
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
	 uart_ = &wiselib::FacetProvider<Os, Uart>::get_facet( value );

         debug_->debug( "Booting with ID: %x\n", radio_->id());
	 
	 ipv6_stack_.init(*radio_, *debug_, *timer_, *uart_);
	 
	 
	 callback_id = ipv6_stack_.icmpv6.reg_recv_callback<lowpanApp,&lowpanApp::receive_echo_reply>( this );
	 callback_id = ipv6_stack_.udp.reg_recv_callback<lowpanApp,&lowpanApp::receive_radio_message>( this );
	 
	 /*
	 //HACK
	 //It will have to come from an advertisement!
	 uint8_t my_prefix[8];
	 my_prefix[0] = 0x12;
	 my_prefix[1] = 0x1F;
	 my_prefix[2] = 0x1A;
	 my_prefix[3] = 0x12;
	 my_prefix[4] = 0x1B;
	 my_prefix[5] = 0x1A;
	 my_prefix[6] = 0xF2;
	 my_prefix[7] = 0x1D;
	 //HACK
	 ipv6_stack_.interface_manager.set_prefix_for_interface( my_prefix, ipv6_stack_.interface_manager.INTERFACE_RADIO , 64 );
	 */
	 //NOTE Test IP packet
	 IPv6Address_t sourceaddr;
	 IPv6Address_t destinationaddr;
	 
	 destinationaddr.set_debug( *debug_ );
	 
	 
	 destinationaddr.make_it_link_local();
	 
	 
	 node_id_t ll_id = 0x2110;
	 destinationaddr.set_long_iid(&ll_id, false);
	 
	 //uint8_t mypayload[] = "hello :) This is a long test message from node 0 to test the fragmentation. Google will pre-publish the evaluation questions for both students and mentors. Mentors will fill out mid-term and final evaluations for their students via the Google Summer of Code 2012 site. These evaluations will be visible in the system to the mentor and the mentoring organization s administrator(s). Students will fill out a mid-term and final evaluation of their mentors online as well, and their evaluations will only be visible in the system to the mentoring organization s administrator(s). Program administrators from Google will have access to all evaluation data. Any student who does not submit an evaluation by the evaluation deadline will fail that evaluation, regardless of the grade the mentor gives the student. If a student submits his or her evaluation on time but the mentor does not, then the student is  in an undecided state until the program administrators can speak to the mentor and determine the student s grade. Students who fail the mid-term are immediately removed from the program: it s not possible to fail the mid-term, stay in the program, and then have a final evaluation. In almost all cases, students will never see their mentor s evaluation of their progress, nor will a mentor see a student s evaluation of her/his mentorship.";
	 
	 /*
	 UDP
	 */
	/* if( radio_->id() == 1 )
	 {
	 	int my_number = ipv6_stack_.udp.add_socket( 10, 10, destinationaddr, callback_id );
	 	//ipv6_stack_.udp.print_sockets();
	 	ipv6_stack_.udp.send(my_number,sizeof(mypayload),mypayload);
	 }
	 if( radio_->id() == 0 )
	 {
	 	node_id_t ll_id = 1;
	 	destinationaddr.set_long_iid(&ll_id, false);
	 	ipv6_stack_.udp.add_socket( 10, 10, destinationaddr, callback_id );
	 }*/
	 
	 /*ICMPv6 Ping test*/
	 // 0 ---> 1 <---- 2
	 //if( radio_->id() == 0x2110 )
	 //{
	 	debug_->debug("Application layer: sending echo request");
		ipv6_stack_.icmpv6.ping(destinationaddr);
	 	//ipv6_stack_.icmpv6.ping(ipv6_stack_.ipv6.BROADCAST_ADDRESS);
	 //}
	 
	 
         //timer_->set_timer<lowpanApp,&lowpanApp::broadcast_loop>( 3000, this, 0 );
      }
      // --------------------------------------------------------------------
      void broadcast_loop( void* )
      {
	 /*debug_->debug( "broadcasting message at %x\n", radio_->id() );
         Os::Radio::block_data_t message[] = "ID-collision test\0";
         radio_->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );*/

	 //timer_->set_timer<lowpanApp,&lowpanApp::broadcast_loop>( 3000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_radio_message( IPv6Address_t from, uint16_t len, Os::Radio::block_data_t *buf )
      {

         debug_->debug( "Application layer received msg at %x from ", radio_->id() );
	 from.set_debug( *debug_ );
	 from.print_address();
	 debug_->debug( " Size: %i Content: ", len);
	 //for( int i = 0; i < len; i++ )
	 	//debug_->debug( "%c", buf[i] );
	 debug_->debug( "\n" );

      }
      // --------------------------------------------------------------------
      void receive_echo_reply( IPv6Address_t from, size_t len, Os::Radio::block_data_t *buf )
      {
       if( len == 1 && buf[0] == ipv6_stack_.icmpv6.ECHO_REPLY )
       {
       		debug_->debug( "Application layer received echo reply at %x from ", radio_->id() );
       		from.set_debug( *debug_ );
       		from.print_address();
       }
       
      }
   private:
      int callback_id;

      IPv6_stack_t ipv6_stack_;
      Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
      Uart::self_pointer_t uart_;
      //Os::Rand::self_pointer_t rand_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, lowpanApp> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
