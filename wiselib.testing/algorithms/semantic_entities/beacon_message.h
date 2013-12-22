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

#ifndef BEACON_MESSAGE_H
#define BEACON_MESSAGE_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class BeaconMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			enum {
				POS_MESSAGE_TYPE = 0,
				POS_SEQUENCE_NUMBER = POS_MESSAGE_TYPE + sizeof(message_id_t),
				POS_ROOT_DISTANCE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_PARENT = POS_ROOT_DISTANCE + sizeof(distance_t),
				
				POS_SES_START
			};
			
			enum {
				SEPOS_ID = 0,
				SEPOS_DISTANCE_FIRST = 8,
				SEPOS_DISTANCE_LAST = 9,
				SEPOS_TOKEN_COUNT = 10,
				SEPOS_TRANSFER_INTERVAL = 11,
				SEPOS_TARGET = 12,
				SEPOS_END = SEPOS_TARGET + sizeof(node_id_t)
			};
			
			sequence_number_t sequence_number() { return rd(POS_SEQUENCE_NUMBER); }
			void set_sequence_number(sequence_number_t s) { wr(POS_SEQUENCE_NUMBER, s); }
			
			distance_t root_distance() { return rd(POS_ROOT_DISTANCE); }
			void set_root_distance(distance_t d) { wr(POS_ROOT_DISTANCE, d); }
			
			node_id_t parent() { return rd(POS_PARENT); }
			void set_parent(node_id_t n) { wr(POS_PARENT, n); }
			
			::uint8_t ses() { return rd(POS_SES); }
			void set_ses(::uint8_t n) { wr(POS_SES, n); }
			
			void add_se(SemanticEntityT& se, node_id_t target = NULL_NODE_ID) {
				::uint8_t s = ses();
				::uint8_t p = POS_SES_START + s * SEPOS_END;
				
				wr(p + SEPOS_ID, se.id());
				wr(p + SEPOS_DISTANCE_FIRST, se.distance_first());
				wr(p + SEPOS_DISTANCE_LAST, se.distance_last());
				wr(p + SEPOS_TOKEN_COUNT, se.token_count());
				wr(p + SEPOS_TRANSFER_INTERVAL, se.transfer_interval());
				wr(p + SEPOS_TARGET, target);
			}
			
		
		private:
			
			template<typename T>
			T rd(size_type p) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + p);
			}
			
			template<typename T>
			void wr(size_type p, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + p, v);
			}
			
			block_data_t data_[BUFFER_SIZE];
		
	}; // BeaconMessage
}

#endif // BEACON_MESSAGE_H



