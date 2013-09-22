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

#ifndef PACKING_RADIO_H
#define PACKING_RADIO_H

#include <external_interface/external_interface.h>
#include <util/base_classes/radio_base.h>
#include <util/debugging.h>
#include "message_packer.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Packer_P = MessagePacker<OsModel_P>,
		typename Debug_P = typename OsModel_P::Debug,
		typename Timer_P = typename OsModel_P::Timer
	>
	class PackingRadio
		: public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Packer_P Packer;
			
			typedef Debug_P Debug;
			typedef Timer_P Timer;
			typedef PackingRadio<OsModel_P, Radio_P, Packer_P> self_type;
			typedef self_type* self_pointer_t;
			typedef RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t> base_type;
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialValues {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - 1 - sizeof(message_id_t)
			};
			
			enum {
				MESSAGE_ID = 200
			};
			
			enum {
				TIMER_INTERVAL = 100 * WISELIB_TIME_FACTOR
			};
			
			int init(Radio& radio, Debug& debug) {
				radio_ = &radio;
				debug_ = &debug;
				timer_ = 0;
				timer_set_ = false;
				packer_.init(buffer_ + sizeof(message_id_t), Radio::MAX_MESSAGE_LENGTH - sizeof(message_id_t));
				message_id_t msg_id = MESSAGE_ID;
				wiselib::write<OsModel, block_data_t, message_id_t>(buffer_, msg_id);
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				return SUCCESS;
			}
			
			int init(Radio& radio, Debug& debug, Timer& timer) {
				init(radio, debug);
				timer_ = &timer;
				timer_->template set_timer<self_type, &self_type::on_time>(TIMER_INTERVAL, this, 0);
				return SUCCESS;
			}
			
			void enable_radio() {
				radio_->enable_radio();
			}
			
			void disable_radio() {
				radio_->disable_radio();
			}
			
			node_id_t id() { return radio_->id(); }
			
			void send(node_id_t receiver, size_t size, block_data_t* data) {
				debug_->debug("@%d prad snd l %d", (int)radio_->radio().id(), (int)size);
				assert(size <= MAX_MESSAGE_LENGTH);
				
				if(receiver != current_receiver_) {
					flush();
				}
				
				bool fit = packer_.append(size, data);
				
				if(!fit) {
					flush();
					fit = packer_.append(size, data);
					assert(fit);
				}
				else if(!timer_set_) {
					timer_set_ = true;
				}
				
				current_receiver_ = receiver;
			}
			
			void flush() {
				if(!buffer_virgin()) {
					debug_->debug("@%d prad flsh l %d", (int)radio_->radio().id(),
							(int)(sizeof(message_id_t) + packer_.size()));
					
					radio_->send(current_receiver_, sizeof(message_id_t) + packer_.size(), buffer_);
					packer_.clear();
				}
			}
			
			Radio& radio() { return *radio_; }
			
		private:
			
			void on_time(void*) {
				flush();
				timer_->template set_timer<self_type, &self_type::on_time>(TIMER_INTERVAL, this, 0);
			}
		
			bool buffer_virgin() { return packer_.empty(); }
		
			void on_receive(node_id_t from, size_t size, block_data_t* data) {
				debug_->debug("@%d prad recv", (int)radio_->radio().id());
				
				typedef typename Packer::length_t length_t;
				
				message_id_t msg_id = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(msg_id != MESSAGE_ID) { return; }
				data += sizeof(message_id_t);
				size -= sizeof(message_id_t);
				
				Packer p;
				p.init(data, size);
				
				length_t len;
				block_data_t *d;
				bool cont;
				while(true) {
					cont = p.next(len, d);
					if(!cont) { break; }
					
					//DBG("unpkg");
					debug_->debug("@%d prad recv unpkg", (int)radio_->radio().id());
					base_type::notify_receivers(from, len, d);
				}
					//DBG("unpkg don");
				
				/*
				
				debug_->debug("packing radio receiving:");
				wiselib::debug_buffer<Os, 16>(debug_, data, size);
				
				
				
				size_t pos = 0;
				while(pos < size) {
					length_t len = wiselib::read<OsModel, block_data_t, length_t>(data + pos);
					printf("total=%d len=%d\n", size, len);
					base_type::notify_receivers(from, len, data + sizeof(length_t));
					pos += sizeof(length_t) + len;
					printf("pos=%d sz=%d", pos, size);
				}
				*/
			}
			
			node_id_t current_receiver_;
			Packer packer_;
			block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];
			bool timer_set_;
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			typename Timer::self_pointer_t timer_;
	}; // PackingRadio
}

#endif // PACKING_RADIO_H

