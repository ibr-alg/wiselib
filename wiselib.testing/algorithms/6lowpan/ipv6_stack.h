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

#include "algorithms/6lowpan/icmpv6.h"
#include "algorithms/6lowpan/udp.h"
#include "algorithms/6lowpan/ipv6.h"
#include "algorithms/6lowpan/lowpan.h"

namespace wiselib
{

	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	class IPv6Stack
	{
	public:
	
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	
	typedef wiselib::LoWPAN<OsModel, Radio, Debug> LoWPAN_t;
	typedef wiselib::IPv6<OsModel, LoWPAN_t, Debug> IPv6_t;
	typedef wiselib::UDP<OsModel, IPv6_t, LoWPAN_t, Debug> UDP_t;
	typedef wiselib::ICMPv6<OsModel, IPv6_t, LoWPAN_t, Debug> ICMPv6_t;
	
	void init( Radio& radio, Debug& debug )
	{
		radio_ = &radio;
		debug_ = &debug;
		
		debug_->debug( "IPv6 stack init: %d\n", radio_->id());
	
		//Init LoWPAN
		lowpan.init(*radio_, *debug_);
	 
		//Init IPv6
		ipv6.init( lowpan, *debug_);
		
		/*//Init UDP
		udp.init( ipv6, *debug_);
		udp.enable_radio();*/
		
		//Init ICMPv6
		icmpv6.init( ipv6, *debug_);
		icmpv6.enable_radio();
	}
	
	ICMPv6_t icmpv6; 
	UDP_t udp;
	IPv6_t ipv6;
	LoWPAN_t lowpan;
	
	
	private:
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	};
}
#endif