#include "external_interface/external_interface.h"

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
typedef Os::Uart Uart;
#endif

typedef wiselib::IPv6Address<Radio, Os::Debug> IPv6Address_t;
typedef wiselib::UDPSocket<IPv6Address_t> UDPSocket_t;
typedef wiselib::IPv6Stack<Os, Radio, Os::Debug, Os::Timer, Uart/*, Os::Clock*/> IPv6_stack_t;

class RPLTest
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
		//clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
		debug_->debug( "Booting with ID: %x\n", radio_->id());
		
		ipv6_stack_.init(*radio_, *debug_, *timer_, *uart_/*, *clock_*/);

		send_count = 0;

		callback_id = ipv6_stack_.udp.reg_recv_callback<RPLTest,&RPLTest::receive_radio_message>( this );

		// --------------------------------------------------------------------
		//TESTING PART

		node_id_t root;
		#ifdef SHAWN
		root = 0x0;
		#else
		root = 0x2110;
		#endif
		
		if( radio_->id() == root )
			ipv6_stack_.rpl.set_dodag_root(true);
		else
			ipv6_stack_.rpl.set_dodag_root(false);
		
		debug_->debug("\nBEFORE RPL INIT, MEM: %u", mem->mem_free());
		ipv6_stack_.rpl.start();

		//EXPERIMENT
		again = true;

		#ifdef SHAWN
		source = 0x1;
		source2 = 0x2;
		source3 = 0x3;
		source4 = 0x4;
		source5 = 0x0;

		dest =  0x5;
		dest2 = 0x6;
		dest3 = 0x7;
		dest4 =  0x8;
		dest5 =  0x9;
		
		#else
		source = 0x2120;
		source2 = 0x212c;
		source3 = 0x2100;
		source4 = 0x2114;
		source5 = 0x2118;

		dest =  0x212c;
		dest2 = 0x2138;
		dest3 = 0x2110;
		dest4 =  0x2134;
		dest5 =  0x2104;
		#endif

		uint8_t global_prefix[8];
		global_prefix[0]=0xAA;
		global_prefix[1]=0xAA;
		memset(&(global_prefix[2]),0, 6);

		destination.set_prefix(global_prefix);
		destination.prefix_length = 64;
		destination.set_long_iid( &dest, true );

		destination2.set_prefix(global_prefix);
		destination2.prefix_length = 64;
		destination2.set_long_iid( &dest2, true );

		destination3.set_prefix(global_prefix);
		destination3.prefix_length = 64;
		destination3.set_long_iid( &dest3, true );

		destination4.set_prefix(global_prefix);
		destination4.prefix_length = 64;
		destination4.set_long_iid( &dest4, true );

		destination5.set_prefix(global_prefix);
		destination5.prefix_length = 64;
		destination5.set_long_iid( &dest5, true );

		if( radio_->id() == source )
			my_socket = UDPSocket_t( 61616, 61617, destination );
		else if( radio_->id() == source2 )
			my_socket = UDPSocket_t( 61616, 61617, destination2 );
		else if( radio_->id() == source3 )
			my_socket = UDPSocket_t( 61616, 61617, destination3 );
		else if( radio_->id() == source4 )
			my_socket = UDPSocket_t( 61616, 61617, destination4 );
		else if( radio_->id() == source5 )
			my_socket = UDPSocket_t( 61616, 61617, destination5 );

		else if( radio_->id() == dest )
			ipv6_stack_.udp.listen( 61617, callback_id );
		else if( radio_->id() == dest2 )
			ipv6_stack_.udp.listen( 61617, callback_id );
		else if( radio_->id() == dest3 )
			ipv6_stack_.udp.listen( 61617, callback_id );
		else if( radio_->id() == dest4 )
			ipv6_stack_.udp.listen( 61617, callback_id );
		else if( radio_->id() == dest5 )
			ipv6_stack_.udp.listen( 61617, callback_id );

		timer_->set_timer<RPLTest, &RPLTest::send_data>( 90000, this, 0 );

	}

	void receive_radio_message( UDPSocket_t socket, uint16_t len, Os::Radio::block_data_t *buf )
	{
		//Drop the messages from this node --> only the IP addresses are compared
		if( socket != ipv6_stack_.udp.id() )
		{
			char str[43];
			debug_->debug( "Application layer received msg at %llx from %s", (long long unsigned)(radio_->id()), socket.remote_host.get_address(str) );
			debug_->debug( "    Size: %i Content: %s ", len, buf);
		}
	}

	void send_data( void* )
	{

		if(send_count == 0)
			debug_->debug( "\nSTART SENDING" );

		uint8_t mypayload[] = "hello :) This is a test message.";

		int result;

		if( radio_->id() == source )
			result = ipv6_stack_.udp.send(my_socket,sizeof(mypayload),mypayload);

		else if( radio_->id() == source2 )
			result =ipv6_stack_.udp.send(my_socket,sizeof(mypayload),mypayload);

		else if( radio_->id() == source3 )
			result = ipv6_stack_.udp.send(my_socket,sizeof(mypayload),mypayload);

		else if( radio_->id() == source4 )
			result = ipv6_stack_.udp.send(my_socket,sizeof(mypayload),mypayload);

		else if( radio_->id() == source5 )
			result = ipv6_stack_.udp.send(my_socket,sizeof(mypayload),mypayload);

		debug_->debug( "\nRESULT IS: %i", result );

		send_count = send_count + 1;

		if(send_count <200)
		{
			timer_->set_timer<RPLTest, &RPLTest::send_data>( 2000, this, 0 );
		}

	}
	

private:
	int callback_id;
	bool again;

	uint16_t send_count;

	UDPSocket_t my_socket;

	node_id_t source;
	node_id_t source2;
	node_id_t source3;
	node_id_t source4;
	node_id_t source5;

	node_id_t dest;
	node_id_t dest2;
	node_id_t dest3;
	node_id_t dest4;
	node_id_t dest5;

	IPv6Address_t destination;
	IPv6Address_t destination2;
	IPv6Address_t destination3;
	IPv6Address_t destination4;
	IPv6Address_t destination5;

	IPv6_stack_t ipv6_stack_;
	Radio::self_pointer_t radio_;
	Os::Timer::self_pointer_t timer_;
	Os::Debug::self_pointer_t debug_;
	Uart::self_pointer_t uart_;
	//Os::Clock::self_pointer_t clock_;

   
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, RPLTest> rpl_test;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  rpl_test.init( value );
}
