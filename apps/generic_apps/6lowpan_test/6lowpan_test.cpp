/*
* File: 6lowpan_test.cpp
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*
* Test application for the IPv6 stack
* 
* Serial Line IP PC side: https://github.com/GDanii/Wiselib-TunSLIP6
*
*/


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
#elif defined ( ISENSE )
//In case of iSense use the large packet uart --> uint16 for packet length
#include "external_interface/isense/isense_com_uart_largepackets.h"
typedef wiselib::iSenseSerialComUartModelLargePackets<Os> Uart;
#else
typedef Os::Uart Uart;
#endif

typedef wiselib::IPv6Stack<Os, Radio, Os::Debug, Os::Timer, Uart> IPv6_stack_t;

typedef wiselib::IPv6Address<Radio, Os::Debug> IPv6Address_t;
typedef wiselib::UDPSocket<IPv6Address_t> UDPSocket_t;

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
		
	#ifdef ISENSE

		uint8_t uart_size = 8;
		//For the SLIP support the extended addressing is needed on the UART
		#ifdef ISENSE_ENABLE_UART_16BIT
			uart_size = 16;
		#endif
	// --> Set the Radio Chanel in this line
		debug_->debug( "Booting with ID: %llx, radio channel: %i, %i-bit radio & %i-bit UART addresses\n", (long long unsigned)(radio_->id()), radio_->set_channel( 18 ), 8*sizeof(radio_->id()), uart_size);
	#else
		debug_->debug( "Booting with ID: %llx\n", (long long unsigned)(radio_->id()));
	#endif
		
		ipv6_stack_.init(*radio_, *debug_, *timer_, *uart_);
		
		callback_id = ipv6_stack_.icmpv6.reg_recv_callback<lowpanApp,&lowpanApp::receive_echo_reply>( this );
		callback_id = ipv6_stack_.udp.reg_recv_callback<lowpanApp,&lowpanApp::receive_radio_message>( this );
		
// --> The Neighbor Discovery can be started with this line, it MUST be enabled for the SLIP communication
		//ipv6_stack_.icmpv6.ND_timeout_manager_function( NULL );
		
// --> for the border router the IPv6_SLIP MUST be defined in the lowpan_config.h
		
		//--------------------------------------------------------------------------------------
		//					Testing part
		//--------------------------------------------------------------------------------------
		
		/*
			Test destination
		*/
				
		IPv6Address_t destinationaddr;
		
		//Host ID of the destination
		node_id_t ll_id = 0x2140;
		
		//Global addressing
		/*
		uint8_t my_prefix[8];
		my_prefix[0] = 0x20;
		my_prefix[1] = 0x01;
		my_prefix[2] = 0x63;
		my_prefix[3] = 0x80;
		my_prefix[4] = 0x70;
		my_prefix[5] = 0xA0;
		my_prefix[6] = 0xB0;
		my_prefix[7] = 0x69;
		
		destinationaddr.set_prefix( my_prefix, 64 );
		destinationaddr.set_long_iid(&ll_id, true);
		*/
		
		//Link-local addressing
		destinationaddr.make_it_link_local();
		destinationaddr.set_long_iid(&ll_id, true);
		
		/*
			UDP test
			
			For the best compression use ports between 61616 and 61631
		*/
		
		if( radio_->id() == 0x2144 )
		{
			uint8_t mypayload[] = "hello :) This is a test message.";
			
			//Set IPv6 header fields
// 			ipv6_stack_.ipv6.set_traffic_class_flow_label( 0, 42 );
			
			// local_port, remote_port, remote_host
			UDPSocket_t my_socket = UDPSocket_t( 61616, 61617, destinationaddr );
			ipv6_stack_.udp.send(my_socket,sizeof(mypayload),mypayload);
		}
		if( radio_->id() == 0x2140 )
		{
			//local_port, registered UDP callback ID, [remote_port], [remote_host]
			ipv6_stack_.udp.listen( 61617, callback_id );
			//ipv6_stack_.udp.print_sockets();
		}
		
		
		/*
			ICMPv6 Ping test
		*/
		
		/*
		if( radio_->id() == 0x0 )
		{
			debug_->debug("Application layer: sending echo request");
			ipv6_stack_.icmpv6.ping(destinationaddr);
			//ipv6_stack_.icmpv6.ping(ipv6_stack_.ipv6.BROADCAST_ADDRESS);
		}
		*/
		
	}

	// --------------------------------------------------------------------
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

	// --------------------------------------------------------------------
	void receive_echo_reply( IPv6Address_t from, uint16_t len, Os::Radio::block_data_t *buf )
	{
		if( len == 1 && buf[0] == ipv6_stack_.icmpv6.ECHO_REPLY )
		{
			char str[43];
			debug_->debug( "Application layer received echo reply at %llx from %s", (long long unsigned)(radio_->id()), from.get_address(str) );
		}
	}

private:
	int callback_id;

	IPv6_stack_t ipv6_stack_;
	Radio::self_pointer_t radio_;
	Os::Timer::self_pointer_t timer_;
	Os::Debug::self_pointer_t debug_;
	Uart::self_pointer_t uart_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, lowpanApp> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
	example_app.init( value );
}
