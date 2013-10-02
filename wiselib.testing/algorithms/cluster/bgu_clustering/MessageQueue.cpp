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

#include "algorithms/bgu_clustering/MessageQueue.h"

using namespace wiselib;

wiselib::MessageQueue::MessageQueue() {
	// TODO Auto-generated constructor stub
}


wiselib::MessageQueue::~MessageQueue() {
	// TODO Auto-generated destructor stub
}

void
wiselib::MessageQueue::receiveBuffer( Radio::node_id_t from, Radio::size_t len, Radio::block_data_t *buf )
{
	uint8_t *byte_buffer = reinterpret_cast<uint8_t*>(buf); //we don't know the size of block_data_t
	size_t nbytes = len;// * sizeof(Os::Radio::block_data_t);
	if (len > 0){
	  switch (byte_buffer[0])
	  {
		  case BGU_TOPOLOGY_MESSAGE_ID:
		  {
		    // printf("we just got: %d, %d from %d\n", byte_buffer[15], byte_buffer[7], from);
			  _mqueue.push(new TopologyMessage(byte_buffer, nbytes));
			  //			  TopologyMessage temp(byte_buffer, nbytes);
		  
		  }break;
		  default:
		  {
	//		  assert(false);
		  }
	  }
	}
}

Message*
wiselib::MessageQueue::nextMessage()
{
	Message* ret = NULL;

	if (_mqueue.empty())
	{
		return NULL;
	}

	ret = _mqueue.front();
	_mqueue.pop();
	return ret;
}
