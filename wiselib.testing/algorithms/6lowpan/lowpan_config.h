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
#ifndef __LOWPAN_CONFIG_H
#define __LOWPAN_CONFIG_H

//6LoWPAN IP packet max size
#define LOWPAN_IP_PACKET_BUFFER_MAX_SIZE 1500

//The Contexts number, it should be 16
#define LOWPAN_CONTEXTS_NUMBER 16

//Number of neighbors in the neighbor cache
#define LOWPAN_MAX_OF_NEIGHBORS 10

//Number of routers in the default routers' list
#define LOWPAN_MAX_OF_ROUTERS 5

//Number of prefixes per interface
#define LOWPAN_MAX_PREFIXES 2

//Timeout in ms for a packet via the Radio (handle lost fragments)
#define LOWPAN_REASSEMBLING_TIMEOUT 250

//The maximum of stored mesh broadcast sequence numbers
#define MAX_BROADCAST_SEQUENCE_NUMBERS 15

//IP packet store size
#define IP_PACKET_POOL_SIZE 2

//Forwarding table size in the IPv6 layer
#define FORWARDING_TABLE_SIZE 8

//Minimum: 1, the index starts from 0 at the get_interface function!
#define NUMBER_OF_INTERFACES 2

//Number of UDP sockets in the UDP layer
#define NUMBER_OF_UDP_SOCKETS 4

//Enable the Router Solicitation messages on the UART interface
// #define IPv6_SLIP

//Select routing method
#define LOWPAN_ROUTE_OVER
//#define LOWPAN_MESH_UNDER

#endif
