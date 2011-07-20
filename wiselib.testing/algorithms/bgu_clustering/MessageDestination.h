#ifndef MESSAGEDESTINATION_H_
#define MESSAGEDESTINATION_H_

#include "utils.h"

namespace wiselib{

class TopologyMessage;

class MessageDestination
{
public:
	virtual ~MessageDestination();

	virtual error_code_t handle(wiselib::TopologyMessage* msg)=0;
};

}

#endif /* MESSAGEDESTINATION_H_ */
