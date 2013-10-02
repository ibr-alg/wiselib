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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <external_interface/external_interface_testing.h>

#define ASSERT(_X) if (!(_X)) {std::cout<<"*** ASSERTION ("<<#_X<<") FAILED AT "<<__FILE__<<":"<<__LINE__<<" ***"<<std::endl; std::cin.get();}

typedef wiselib::OSMODEL Os;
typedef Os::Timer Timer;

#ifdef SHAWN
typedef Os::TxRadio Radio;
#else
typedef Os::ExtendedRadio Radio;
#endif

enum error_code_t
{
	ecSuccess,
	ecBufferTooShort
};

typedef Radio::node_id_t nodeid_t;

#define ARRSIZE(_a) (sizeof(_a)/sizeof((_a)[0]))

//This struct's size must be a multiple of 4 in order for serialization to work.
struct topology_record_t
{
	nodeid_t nodeid;
	uint8_t	distance;
	bool is_leader;
	uint8_t will_be_leader;
	nodeid_t leader;
	nodeid_t parent;
};

#endif /* UTILS_H_ */
