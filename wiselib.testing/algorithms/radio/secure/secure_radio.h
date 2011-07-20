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

/*
 * Author: Henning Hasemann <hasemann@ibr.cs.tu-bs.de>
 */

#ifndef __ALGORITHMS_RADIO_SECURE_RADIO_H__
#define __ALGORITHMS_RADIO_SECURE_RADIO_H__

#include "util/base_classes/radio_base.h"

#include <iostream>

namespace wiselib {
	
	template<typename OsModel_P, typename Radio_P, typename Cipher_P>
	class SecureRadio
		: public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
		public:
			typedef OsModel_P OsModel;
			typedef Radio_P Radio;
			typedef Cipher_P cipher_t;
			typedef SecureRadio<OsModel, Radio, cipher_t> self_type;
			
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef typename Radio::message_id_t message_id_t;
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS //, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID,
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
			};
			
			int init(Radio& radio, cipher_t& cipher) {
				radio_ = &radio;
				cipher_ = &cipher;
				//radio_->template reg_recv_callback<SecureRadio, &SecureRadio::receive>(this);
				return SUCCESS;
			}
			
			void enable_radio() {
				radio_->enable_radio();
				radio_->template reg_recv_callback<SecureRadio, &SecureRadio::receive>(this);
			}
			void disable_radio() { radio_->disable_radio(); }
			node_id_t id() { return radio_->id(); }
			
			void send(node_id_t receiver, size_t size, block_data_t* data);
			void receive(node_id_t from, size_t size, block_data_t* data);
			
		private:
			typename Radio::self_pointer_t radio_;
			cipher_t* cipher_;
	};
	
	template<typename OsModel_P, typename Radio_P, typename Cipher_P>
	void
	SecureRadio<OsModel_P, Radio_P, Cipher_P>::
	send(node_id_t receiver, size_t size, block_data_t* data) {
		block_data_t buffer[size];
		cipher_->encrypt(data, buffer, size);
		radio_->send(receiver, size, buffer);
	}
	
	template<typename OsModel_P, typename Radio_P, typename Cipher_P>
	void
	SecureRadio<OsModel_P, Radio_P, Cipher_P>::
	receive(node_id_t sender, size_t size, block_data_t* data) {
		block_data_t buffer[size];
		cipher_->decrypt(data, buffer, size);
		notify_receivers(sender, size, buffer);
	}

} // namespace

#endif // __ALGORITHMS_RADIO_SECURE_RADIO_H__

