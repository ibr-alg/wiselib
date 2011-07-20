/*
 * TopologyMessage.cpp
 *
 *  Created on: Nov 15, 2010
 *      Author: wiselib
 */

#include "algorithms/bgu_clustering/TopologyMessage.h"
#include "algorithms/bgu_clustering/MessageDestination.h"
#include "algorithms/bgu_clustering/MessageSerialization.h"
#include "util/serialization/simple_types.h"

using namespace wiselib;




// FIXME: XXX_serialize no longer needed
// #ifdef ISENSE
// template<typename T>
// static void
// do_serialize(wiselib::uint32_t* out_ptr, T* data)
// {
// 	wiselib::uint32_t* in_ptr = (wiselib::uint32_t*)data;
// 
// 	for (unsigned int i=0; i < sizeof(T)/sizeof(wiselib::uint32_t); ++i)
// 	{
// 		*(out_ptr++)=*(in_ptr++);
// 	}
// }
// 
// template<typename T>
// static void
// do_deserialize(wiselib::uint32_t* in_ptr, T* data)
// {
// 	wiselib::uint32_t* out_ptr = (wiselib::uint32_t*)data;
// 
// 	for (unsigned int i=0; i < sizeof(T)/sizeof(wiselib::uint32_t); ++i)
// 	{
// 		*(out_ptr++)=*(in_ptr++);
// 	}
// }
// #else
// template<typename T>
// static void
// do_serialize(uint32_t* out_ptr, T* data)
// {
// 	uint32_t* in_ptr = (uint32_t*)data;
// 
// 	for (unsigned int i=0; i < sizeof(T)/sizeof(uint32_t); ++i)
// 	{
// 		*(out_ptr++)=*(in_ptr++);
// 	}
// }
// 
// template<typename T>
// static void
// do_deserialize(uint32_t* in_ptr, T* data)
// {
// 	uint32_t* out_ptr = (uint32_t*)data;
// 
// 	for (unsigned int i=0; i < sizeof(T)/sizeof(uint32_t); ++i)
// 	{
// 		*(out_ptr++)=*(in_ptr++);
// 	}
// }
// #endif

TopologyMessage::TopologyMessage(nodeid_t sender)
:Message(BGU_TOPOLOGY_MESSAGE_ID), _sender(sender)
{

}

TopologyMessage::TopologyMessage(uint8_t* buffer, uint32_t buffer_length)
:Message(buffer[0])
{
        //this is a c'tor. we can't throw an exception, and we certainly don't want to silently fail.
         /**************************  isense problam  *****************************************************/
	
	//ASSERT(buffer_length >= sizeof(topology_message_header_t));
	//ASSERT(buffer_length <= 255);
	
        /*************************************************************************************************/
// 	topology_message_header_t header;
// 
// 	uint32_t* in_ptr = reinterpret_cast<uint32_t*>(&buffer[0]);
// 
// 	//	std::cout<<in_ptr<<","<<*in_ptr<<std::endl;
// 	
// // 	read<Os>(in_ptr, header);
// 	do_deserialize(in_ptr, &header);
// 	in_ptr += sizeof(header)/sizeof(*in_ptr);
//         /**************************  isense problam  *****************************************************/
// 	//ASSERT(header.msgid ==  BGU_TOPOLOGY_MESSAGE_ID);
// 	//ASSERT(buffer_length >= sizeof(header)+header.num_records*sizeof(serializable_topology_record_t));
//         /*************************************************************************************************/
// 	_sender = header.sender;
// 	
// 	//	std::cout<<" received: msgid="<<(int)header.msgid<<", nrecs="<<(int)header.num_records<<", sender="<<(int)header.sender<<std::endl;
// 
// 	for (int i=0; i<header.num_records; ++i)
// 	{
// 	  serializable_topology_record_t in_record;
// 	  topology_record_t record;
// 	  
// // 	  read<Os>(in_ptr, in_record);
// 	  do_deserialize(in_ptr, &in_record);
// 	  in_ptr += sizeof(in_record)/sizeof(*in_ptr);
// 	  
// 	  convert(in_record, &record);
// 
// 	  //	  printf("---> %d", record.nodeid);
// 
// 	  _topology.push_back(record);
// 	}

   topology_message_header_t header;
   read<Os, uint8_t, topology_message_header_t>(buffer, header);
   _sender = header.sender;

   // start reading first record after header
   int idx = topology_message_header_t::RECORD_SIZE;

   for (int i=0; i<header.num_records; ++i)
   {
     serializable_topology_record_t in_record;
     topology_record_t record;

     read<Os, uint8_t, serializable_topology_record_t>(buffer + idx, in_record);

     convert(in_record, &record);

     //    printf("---> %d", record.nodeid);

     _topology.push_back(record);
     idx += serializable_topology_record_t::RECORD_SIZE;
   }
}

TopologyMessage::~TopologyMessage()
{
}

error_code_t
TopologyMessage::serialize(uint8_t *buffer, uint32_t buffer_size)
{
	if (buffer_size < serializationBufferSize())
	{
		return ecBufferTooShort;
	}

	topology_message_header_t header;
   
	header.msgid = BGU_TOPOLOGY_MESSAGE_ID;
	header.num_records = _topology.size();
	header.sender = _sender;

	uint32_t* out_ptr = reinterpret_cast<uint32_t*>(&buffer[0]);

   write<Os, uint8_t, topology_message_header_t>(buffer, header);
// 	do_serialize(out_ptr, &header);

   // start reading first record after header
   int idx = topology_message_header_t::RECORD_SIZE;

// 	memcpy(out_ptr, &header, sizeof(header));
	out_ptr += sizeof(header)/sizeof(*out_ptr);

	for (iterator it = _topology.begin(); it != _topology.end(); ++it)
	{
	  serializable_topology_record_t record;

	  convert(*it, &record);

	  //	  printf("---> %d ", record.nodeid);

// 	  memcpy(out_ptr, &record, sizeof(record));

	  write<Os, uint8_t, serializable_topology_record_t>(buffer + idx, record);
     idx += serializable_topology_record_t::RECORD_SIZE;
// 	  write<Os>(out_ptr, record);
// 	  do_serialize(out_ptr, &record);
// 	  out_ptr += sizeof(record)/sizeof(*out_ptr);
	}

	return ecSuccess;
}

size_t 
TopologyMessage::serializationBufferSize()
{
  return sizeof(topology_message_header_t)+_topology.size()*sizeof(serializable_topology_record_t);
}

void 
TopologyMessage::convert(serializable_topology_record_t in, topology_record_t* out)
{
  out->nodeid = (nodeid_t)in.nodeid;
  out->distance = in.distance;
  out->is_leader = (in.is_leader)?true:false;
  out->leader = (nodeid_t)in.leader;
  out->parent = (nodeid_t)in.parent;
}

void 
TopologyMessage::convert(topology_record_t in, serializable_topology_record_t* out)
{
  out->nodeid = in.nodeid;
  out->distance = in.distance;
  out->is_leader = (in.is_leader)?0:1;
  out->leader = in.leader;
  out->parent = in.parent;
}

error_code_t
TopologyMessage::applyTo(wiselib::MessageDestination* dest)
{
	return dest->handle(this);
}

error_code_t
TopologyMessage::addTopologyRecord(const topology_record_t& record)
{
	_topology.push_back(record);
	return ecSuccess;
}

TopologyMessage::iterator
TopologyMessage::begin()
{
	return _topology.begin();
}

TopologyMessage::iterator
TopologyMessage::end()
{
	return _topology.end();
}

nodeid_t
TopologyMessage::senderId()
{
	return _sender;
}
