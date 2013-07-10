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

#ifndef SEMANTIC_ENTITY_FORWARDING_H
#define SEMANTIC_ENTITY_FORWARDING_H

namespace wiselib {
	
	/**
	 * @brief Forwarding of packets within semantic entities.
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename AmqNHood_P
	>
	class SemanticEntityForwarding {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename AmqNHood_P AmqNHood;
			
			void init(typename Radio::self_pointer_t radio, AmqNHood* amq_nhood) {
				radio_ = radio;
				amq_nhood_ = amq_nhood_;
			}
			
			/**
			 * Return if the packet has been forwarded, false else.
			 */
			bool on_receive(...) {
				// if reliable transport packet we should forward:
				//   forward it by se_id using amq
					node_id_t target = amq_nhood_.forward_address(radio_->id(), se_id, from);
					if(target == radio_->id()) {
						// TODO
					}
					
			}
		
		private:
		
	}; // SemanticEntityForwarding
}

#endif // SEMANTIC_ENTITY_FORWARDING_H

