/*
 * Message.cpp
 *
 *  Created on: Nov 15, 2010
 *      Author: wiselib
 */

#include "algorithms/bgu_clustering/Message.h"

using namespace wiselib;

Message::Message(uint8_t opcode)
:_opcode(opcode)
{

}

Message::~Message()
{
}
