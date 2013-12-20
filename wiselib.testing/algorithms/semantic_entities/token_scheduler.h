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

#ifndef TOKEN_SCHEDULER_H
#define TOKEN_SCHEDULER_H

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
	class TokenScheduler {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			
			void on_transfer_interval_start(void* _) {
				// TODO
			}
			
			void on_transfer_interval_end(void* _) {
				// TODO
			}
			
			void prepare_beacon() {
				BeaconMessageT& b = beacon_;
				
				b.set_sequence_number(rand_->rand());
				
				if(am_root()) { b.set_root_distance(0); }
				else {
					for(NeighborhoodT::iterator iter = neighborhood_.begin(); iter != end(); ++iter) {
						
					
					b.set_root_distance
			}
			
			void on_receive_beacon(BeaconMessageT& msg, node_id_t from) {
			}
		
		private:
		
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

