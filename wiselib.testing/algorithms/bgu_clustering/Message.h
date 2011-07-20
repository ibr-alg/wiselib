#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "utils.h"

namespace wiselib {

class MessageDestination;

class Message
{
public:
	Message(uint8_t opcode);
	virtual ~Message();

	virtual error_code_t serialize(uint8_t *buffer, uint32_t buffer_size)=0;
	virtual error_code_t applyTo(MessageDestination* dest)=0;
private:
	uint8_t _opcode;
};

}
#endif /* MESSAGE_H_ */
