/***************************************************************************
** This file is part of the generic algorithm library Wiselib.           **
** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
**                                                                       **
** The Wiselib is free software: you can redistribute it and/or modify   **
** it under the terms of the GNU Lesser General Public License as        **
** published by the Free Software Foundation, either version 3 of the    **
** License, or (at your option) any later version.                       **
**                                                                       **
** The Wiselib is distributed in the hope that it will be useful,        **
** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
** GNU Lesser General Public License for more details.                   **
**                                                                       **
** You should have received a copy of the GNU Lesser General Public      **
** License along with the Wiselib.                                       **
** If not, see <http://www.gnu.org/licenses/>.                           **
***************************************************************************/

/*
 * File: ipv6_stack.h
 * Class(es): IPv6Stack
 * Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
 */

#ifndef __ALGORITHMS_6LOWPAN_IPV6_STACK_H__
#define __ALGORITHMS_6LOWPAN_IPV6_STACK_H__

#include "config_testing.h"
#include "algorithms/6lowpan/lowpan_config.h"

#include "radio/DPS/DPS_radio.h"

#ifdef DPS_IPv6_SKELETON
	#include "algorithms/6lowpan/uart_radio.h"
	#include "algorithms/6lowpan/lowpan.h"
#endif


#include "radio/DPS/IPv6/DPS_interface_manager.h"

#include "radio/DPS/IPv6/ipv6.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
// #include "algorithms/6lowpan/icmpv6.h"
#include "algorithms/6lowpan/udp.h"


namespace wiselib
{
	/** \brief IP stack structure builder class, main class of the IPv6 implementation
	* 
	* This class builds the IP stack, it deals with the templating of the layers and managers.
	* This class has to be included into the target application. The init function must be called
	* with the instances of the template parameters. After it packets can be sent by UDP, and
	* an echo request could be sent by the ICMPv6.
	*
	* The Stack:
	* ------------------------
	* |   UDP   |   ICMPv6   |
	* ------------------------
	* |        IPv6          |
	* ------------------------
	* [   InterfaceManager   ]
	* ------------------------
	* | 6LoWPAN |  UartRadio |
	* ------------------------
	* |  Radio  |    Uart    |
	* ------------------------
	*
	* InterfaceManager: This manager separates the interfaces, at the moment there are two interfaces
	*			in the stack, but it can be extended.
	* PacketPoolManager: This manager stores the IP packets in the system, because dynamic memory allocation
	*			is not allowed.
	*/
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
#ifdef DPS_IPv6_SKELETON
		typename Uart_P,
#endif
		typename Rand_P>
	class DPS_IPv6Stack
	{
	public:
		
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Rand_P Rand;
		
		typedef wiselib::DPS_Radio<OsModel, Radio, Debug, Timer, Rand> DPS_Radio_t;
		
#ifdef DPS_IPv6_SKELETON
		typedef Uart_P Uart;
		typedef DPS_IPv6Stack<OsModel, Radio, Debug, Timer, Uart, Rand> self_type;
		typedef wiselib::UartRadio<OsModel, Radio, Debug, Timer, Uart> UartRadio_t;
		typedef wiselib::LoWPAN<OsModel, Radio, Debug, Timer, UartRadio_t> LoWPAN_t;
		typedef wiselib::InterfaceManager<OsModel, LoWPAN_t, Radio, Debug, Timer, UartRadio_t, DPS_Radio_t> InterfaceManager_t;
#elif defined DPS_IPv6_STUB
		typedef DPS_IPv6Stack<OsModel, Radio, Debug, Timer, Rand> self_type;
		typedef wiselib::InterfaceManager<OsModel, Radio, Debug, Timer, DPS_Radio_t> InterfaceManager_t;
#endif
		
		typedef wiselib::IPv6<OsModel, Radio, Debug, Timer, InterfaceManager_t> IPv6_t;
		
		typedef wiselib::UDP<OsModel, IPv6_t, Radio, Debug> UDP_t;
// 		typedef wiselib::ICMPv6<OsModel, IPv6_t, Radio, Debug, Timer> ICMPv6_t;
		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		
		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
			ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
		};
		
		/** \brief Initialization of the stack
		*
		* This function initializes the - layers of the stack: 6LoWPAN, UartRadio, IPv6, UDP, ICMPv6
		*				- managers of the stack: PacketPoolManager, InterfaceManager
		*/
#ifdef DPS_IPv6_SKELETON
		void init( Radio& radio, Debug& debug, Timer& timer, Uart& uart, Rand& rand)
#elif defined DPS_IPv6_STUB
		void init( Radio& radio, Debug& debug, Timer& timer, Rand& rand)
#endif
		{
			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			rand_ = &rand;
			
			debug_->debug( "IPv6 stack init: %llx", (long long unsigned)(radio_->id()));
			
			packet_pool_mgr.init( *debug_ );
			
			dps_radio.init(*radio_, *debug_, *timer_, *rand_);
			dps_radio.enable_radio();
			
#ifdef DPS_IPv6_SKELETON
			uart_ = &uart;
			
			//Init LoWPAN
			lowpan.init(*radio_, *debug_, &packet_pool_mgr, *timer_ );
			
			//Init Uart Radio
			uart_radio.init( *uart_, *radio_, *debug_, &packet_pool_mgr, *timer_ );
		
			interface_manager.init( &dps_radio, &lowpan, *debug_, &uart_radio, &packet_pool_mgr );
			
			lowpan.nd_storage_.set_debug( *debug_ );
#elif defined DPS_IPv6_STUB
			interface_manager.init( &dps_radio, *debug_, &packet_pool_mgr );
#endif
// 			
// 			
			
// 			
// 			//Init IPv6
			ipv6.init( *radio_, *debug_, &packet_pool_mgr, *timer_, &interface_manager );
// 			//IPv6 will enable lower level radios
			if( SUCCESS != ipv6.enable_radio() )
				debug_->debug( "DPS enIP failed" );
// 			
// 			
// 			//Init UDP
			udp.init( ipv6, *debug_, &packet_pool_mgr);
// 			//Just register callback, not enable IP radio
			if( SUCCESS != udp.enable_radio() )
				debug_->debug( "DPS enUDP failed" );
			
// 			//Init ICMPv6
// 			icmpv6.init( ipv6, *debug_, *timer_, &packet_pool_mgr);
// 			//Just register callback, not enable IP radio
// 			if( SUCCESS != icmpv6.enable_radio() )
// 				debug_->debug( "Fatal error: ICMPv6 layer enabling failed! " );
			
			
// 			if( !lowpan.nd_storage_.is_router )
// 				icmpv6.ND_timeout_manager_function( NULL );
		}
		
// 		ICMPv6_t icmpv6; 
		UDP_t udp;
		IPv6_t ipv6;
		InterfaceManager_t interface_manager;
		DPS_Radio_t dps_radio;
		
	private:
		typename Radio::self_pointer_t radio_;
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		typename Rand::self_pointer_t rand_;
		
#ifdef DPS_IPv6_SKELETON
		typename Uart::self_pointer_t uart_;
		LoWPAN_t lowpan;
		UartRadio_t uart_radio;
#endif
		Packet_Pool_Mgr_t packet_pool_mgr;
	};
}
#endif
