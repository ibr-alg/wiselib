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
#ifndef __ALGORITHMS_6LOWPAN_IPV6_STACK_H__
#define __ALGORITHMS_6LOWPAN_IPV6_STACK_H__

#include "algorithms/6lowpan/uart_radio.h"
#include "algorithms/6lowpan/lowpan.h"
#include "algorithms/6lowpan/interface_manager.h"
#include "algorithms/6lowpan/ipv6.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/icmpv6.h"
#include "algorithms/6lowpan/udp.h"


namespace wiselib
{

	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_P>
	class IPv6Stack
	{
	public:
	
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	typedef Timer_P Timer;
	typedef Uart_P Uart;
	
	typedef IPv6Stack<OsModel, Radio, Debug, Timer, Uart> self_type;
	
	typedef wiselib::UartRadio<OsModel, Radio, Debug, Timer, Uart> UartRadio_t;
	typedef wiselib::LoWPAN<OsModel, Radio, Debug, Timer, UartRadio_t> LoWPAN_t;
	
	typedef wiselib::InterfaceManager<OsModel, LoWPAN_t, Radio, Debug, Timer, UartRadio_t> InterfaceManager_t;
	
	typedef wiselib::IPv6<OsModel, LoWPAN_t, Radio, Debug, Timer, InterfaceManager_t> IPv6_t;

	typedef wiselib::UDP<OsModel, IPv6_t, Radio, Debug> UDP_t;
	typedef wiselib::ICMPv6<OsModel, IPv6_t, Radio, Debug> ICMPv6_t;
	typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
	
	
	void init( Radio& radio, Debug& debug, Timer& timer, Uart& uart)
	{
		radio_ = &radio;
		debug_ = &debug;
		timer_ = &timer;
		uart_ = &uart;
		
		//debug_->debug( "IPv6 stack init: %x\n", radio_->id());
		
		packet_pool_mgr.init( *debug_ );
		
		
		//Init LoWPAN
		lowpan.init(*radio_, *debug_, &packet_pool_mgr, *timer_ );
		
		//Init Uart Radio
		uart_radio.init( *uart_, *radio_, *debug_, &packet_pool_mgr, *timer_ );
	 
		interface_manager.init( &lowpan, *debug_, &uart_radio );
		
		//Init IPv6
		ipv6.init( *radio_, *debug_, &packet_pool_mgr, *timer_, &interface_manager );
		//IPv6 will enable lower level radios
		ipv6.enable_radio();
		
		
		//Init UDP
		udp.init( ipv6, *debug_, &packet_pool_mgr);
		//Just register callback, not enable IP radio
		udp.enable_radio();
		
		//Init ICMPv6
		icmpv6.init( ipv6, *debug_, &packet_pool_mgr);
		//Just register callback, not enable IP radio
		icmpv6.enable_radio();
	}
	
	ICMPv6_t icmpv6; 
	UDP_t udp;
	IPv6_t ipv6;
	LoWPAN_t lowpan;
	UartRadio_t uart_radio;
	Packet_Pool_Mgr_t packet_pool_mgr;
	InterfaceManager_t interface_manager;
	
	
	private:
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	typename Timer::self_pointer_t timer_;
	typename Uart::self_pointer_t uart_;
	};
}
#endif
