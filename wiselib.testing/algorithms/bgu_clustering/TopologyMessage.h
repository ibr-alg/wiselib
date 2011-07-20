#ifndef TOPOLOGYMESSAGE_H_
#define TOPOLOGYMESSAGE_H_

#include "algorithms/bgu_clustering/Message.h"

//#include <list>
#include "util/pstl/list_static.h"

namespace wiselib {

#define BGU_TOPOLOGY_MESSAGE_ID 50

/**A message sent by a sensor in the BGU clustering algorithm.
 * This message contains topology information as known to some node at some time.
 * */
class TopologyMessage: public Message
{
public:
	typedef list_iterator<topology_record_t> iterator;
	
	/**Construct an empty message
	 *
	 * @param sender the nodeid of the sending node
	 * */
	TopologyMessage(nodeid_t sender);

	/**De-serialize a message from a received buffer.
	 *
	 * @param buffer a buffer that was created by the serialize method.
	 * @param buffer_length the length of the buffer
	 * */
	TopologyMessage(uint8_t* buffer, uint32_t buffer_length);
	~TopologyMessage();

	/**Convert this message to a byte buffer.
	 *
	 * @param buffer the buffer where the result is stored
	 * @param buffer_size the size of the buffer.
	 *
	 * @return ecSuccess on success, some other code on failure.
	 * */
	error_code_t serialize(uint8_t *buffer, uint32_t buffer_size);
	
	/**@return the required size for a buffer that can contain a representation of this message.
	 * */
	size_t serializationBufferSize();

	/**Allow a MessageDestination to handle this message.
	 * We work this way to make it possible to decouple the messages
	 * from the destinations.
	 *
	 * @param dest the message destination
	 *
	 * @return ecSuccess on success, some other code on failure.
	 * */
	error_code_t applyTo(wiselib::MessageDestination* dest);

	/**Add some topology information to this message
	 *
	 * @param record the information to add
	 * */
	error_code_t addTopologyRecord(const topology_record_t& record);

	/**The following two methods enable iterating over the topology information stored
	 * in the message.
	 * */
	iterator begin();
	iterator end();

	nodeid_t senderId();

private:
	typedef list_static<Os, topology_record_t, 10> topology_container_t;

	topology_container_t _topology;
	nodeid_t _sender;
//we need a struct that has a known size so we can serialize it. We don't want to enforce 
//size requirements on the general struct, and we can't be sure about sizeof(bool) or sizeof(nodeit_t)
//anyway, so:
public:
	struct serializable_topology_record_t
	{
      typedef Radio::node_id_t node_id_t;

      enum SizeInformation
      {
         RECORD_SIZE = 3*sizeof(node_id_t) + 2
      };
		
	   nodeid_t nodeid;
		uint8_t distance;
		bool is_leader;
// 		uint8_t _padding[2];

	  nodeid_t leader;
	  nodeid_t parent;
	  /*		
		uint32_t leader;
		uint32_t parent;
	  */
	};
// 	__attribute__((packed));
	
	struct topology_message_header_t
	{
      typedef uint8_t message_id_t;
      typedef uint8_t num_rec_t;
      typedef Radio::node_id_t node_id_t;

      enum SizeInformation
      {
         RECORD_SIZE = sizeof(message_id_t) + sizeof(num_rec_t) + sizeof(node_id_t)
      };

		message_id_t msgid;
		num_rec_t num_records;
		node_id_t sender;
	};
	
	void convert(serializable_topology_record_t, topology_record_t*);
	void convert(topology_record_t, serializable_topology_record_t*);
};

}

#endif /* TOPOLOGYMESSAGE_H_ */
