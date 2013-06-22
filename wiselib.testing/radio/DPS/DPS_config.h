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
#ifndef __DPS_CONFIG_H__
#define __DPS_CONFIG_H__

#define DPS_MAX_PROTOCOLS 1

#define DPS_MAX_CONNECTIONS 1

#define DPS_MAX_BUFFER_LIST 1

//The footer is not used
// 0 - unused 
// 1 - XOR based 
// 2 - AES* based
#define DPS_FOOTER 1

#define DPS_TIMER_DISCOVERY_MIN 3 //Not used at the moment
#define DPS_ACK_MAX_RETRIES 3
	
#ifdef SHAWN
	#define DPS_TIMER_DISCOVERY_FREQUENCY 1000 //30
	#define DPS_GENERAL_TIMER_FREQUENCY 1000
	#define DPS_HEARTBEAT_THRESHOLD 4000 //~800
	#define DPS_DELETE_CONNECTION_THRESHOLD 6000 //1000
	#define DPS_CONNECT_TIMEOUT 3000
	#define DPS_ACK_TIMEOUT 3000
	#define DPS_FRAGMENT_COLLECTION_TIMEOUT 6000
	#define DPS_WAIT_FOR_MORE_ADVERTISEMENTS_TIMEOUT 500
#else
	#define DPS_TIMER_DISCOVERY_FREQUENCY 30
	#define DPS_GENERAL_TIMER_FREQUENCY 100
	#define DPS_HEARTBEAT_THRESHOLD 4000 //~800
	#define DPS_DELETE_CONNECTION_THRESHOLD 6000 //1000
	#define DPS_CONNECT_TIMEOUT 3000
	#define DPS_ACK_TIMEOUT 100
	#define DPS_FRAGMENT_COLLECTION_TIMEOUT 2000
	#define DPS_WAIT_FOR_MORE_ADVERTISEMENTS_TIMEOUT 500
#endif

#define DPS_IPv6
// #define DPS_IPv6_STUB //Client
// #define DPS_IPv6_SKELETON //Server

#endif
