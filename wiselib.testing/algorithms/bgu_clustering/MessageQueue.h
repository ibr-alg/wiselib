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

#ifndef MESSAGEQUEUE_H_
#define MESSAGEQUEUE_H_

#include "algorithms/bgu_clustering/Message.h"
#include "algorithms/bgu_clustering/utils.h"
#include "util/pstl/queue_static.h"

namespace wiselib{


/**A buffer for messages received from a Radio, and a factory for Message objects
 * */
class MessageQueue
{
public:
	MessageQueue();
	virtual ~MessageQueue();

	/**A callback for a Radio class that receives a radio message and queues a Message objects that
	 * represents it.
	 *
	 * @param from the node that sent this message
	 * */
    void receiveBuffer( Radio::node_id_t from, Radio::size_t len, Radio::block_data_t *buf );

    /**Extract the next message from this queue
     *
     * @return the next message in the queue.
     * */
    Message* nextMessage();

private:
  static const size_t MAX_MQUEUE_SIZE=10;

  queue_static<Os,Message*, MAX_MQUEUE_SIZE> _mqueue;
    
};

}
#endif /* MESSAGEQUEUE_H_ */
