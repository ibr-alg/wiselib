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
