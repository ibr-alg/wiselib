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

#ifndef PROJECT_CONF_H_INCLUDED__
	#define PROJECT_CONF_H_INCLUDED__

	#undef QUEUEBUF_CONF_NUM
	#define QUEUEBUF_CONF_NUM 1

	#undef QUEUEBUFRAM_NUM
	#define QUEUEBUFRAM_NUM 1

	#undef PROCESS_CONF_NO_PROCESS_NAMES
	#define PROCESS_CONF_NO_PROCESS_NAMES 1

	#undef UIP_CONF_IPV6_RPL
	#define UIP_CONF_IPV6_RPL 0

	#undef UIP_CONF_IPV6
	#define UIP_CONF_IPV6 0

	#undef UIP_CONF_TCP
	#define UIP_CONF_TCP 0

	#undef UIP_CONF_UDP
	#define UIP_CONF_UDP 0

	#undef UIP_CONF_BROADCAST
	#define UIP_CONF_BROADCAST 0

	#undef NETSTACK_CONF_MAC
	#define NETSTACK_CONF_MAC nullmac_driver
	#define NETSTACK_CONF_MAC csma_driver

	#undef NETSTACK_CONF_RDC
	#define NETSTACK_CONF_RDC nullrdc_driver
	//#define NETSTACK_CONF_RDC cxmac_driver
	//#define NETSTACK_CONF_RDC xmac_driver
	//#define NETSTACK_CONF_RDC contikimac_driver
	
	//#define NETSTACK_CONF_RDC lpp_driver

	#undef NETSTACK_RDC_CHANNEL_CHECK_RATE 
	#define NETSTACK_RDC_CHANNEL_CHECK_RATE 8
	

	
#endif // PROJECT_CONF_H

