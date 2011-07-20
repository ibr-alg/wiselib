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
