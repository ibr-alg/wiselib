/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.			  **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.		  **
 **																							  **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as		  **
 ** published by the Free Software Foundation, either version 3 of the	  **
 ** License, or (at your option) any later version.							  **
 **																							  **
 ** The Wiselib is distributed in the hope that it will be useful,		  **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of		  **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			  **
 ** GNU Lesser General Public License for more details.						  **
 **																							  **
 ** You should have received a copy of the GNU Lesser General Public		  **
 ** License along with the Wiselib.													  **
 ** If not, see <http://www.gnu.org/licenses/>.									  **
 ***************************************************************************/

#ifndef MOCK_NEIGHBORHOOD_H
#define MOCK_NEIGHBORHOOD_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/pstl/vector_static.h>
#include <util/delegates/delegate.hpp>

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
		typename Radio_P = typename OsModel_P::Radio,
		typename Clock_P = typename OsModel_P::Clock,
		typename Debug_P = typename OsModel_P::Debug
	>
	class MockNeighborhood {
		public:
			typedef MockNeighborhood self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;

			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			
			typedef Debug_P Debug;

			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { npos = (size_type)(-1) };
			enum {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};

			struct Neighbor {
				node_id_t id;
				uint32_t total_beacons;
				time_t last_echo;
				time_t timeout;
				time_t first_beacon;
				uint16_t last_lqi;
				uint16_t avg_lqi;
				uint8_t beacons_in_row;
				uint8_t inverse_link_assoc;
				bool active;
				bool stable;
				bool bidi;
			};

			enum event_codes {
				NEW_NB = 1, /*!< Event code for a newly added stable neighbor */
				NEW_NB_BIDI = 2, /*!< Event code for a newly added bidi neighbor */
				DROPPED_NB = 4, /*!< Event code for a neighbor removed from nb list */
				NEW_PAYLOAD = 8, /*!< Event code for a newly arrived pg payload */
				NEW_PAYLOAD_BIDI = 16, /*!< Event code for a newly arrived pg payload from a bidi neighbor */
				LOST_NB_BIDI = 32, /*!< Event code generated when we loose bidi comm with a nb */
				NB_READY = 64, /*!< Event code generated after the nb module has generated a stable nhd
									 * Useful for starting other modules that must wait until the nb has
									 * produced a stable neighborhood */

				NEW_SYNC_PAYLOAD = 128, /*!< We received the syncing beacon of this sync phase */

				DEFAULT = 5
				/*!< Event code for NEW_NB + DROPED_NB*/
			};

			typedef vector_static<OsModel, Neighbor, 10> Neighbors;

			typedef delegate4<void, uint8_t, node_id_t, uint8_t, uint8_t*> event_callback_t;

			struct EventCallback {
				::uint8_t payload_id;
				::uint8_t events;
				event_callback_t callback;
			};
			typedef vector_static<OsModel, EventCallback, 4> EventCallbacks;


			MockNeighborhood() : debug_(0) {
			}

			int init(typename Debug::self_pointer_t debug) {
				debug_ = debug;

				check();
				return SUCCESS;
			}

			node_id_t id() { return 1; }

			Neighbors& topology() { return neighbors_; }

			::uint8_t register_payload_space(::uint8_t payload_id) {
				debug_->debug("ND:register_payload_space(%d)", (int)payload_id);
				return 0;
			}

			::uint8_t set_payload(::uint8_t payload_id, ::uint8_t *data, ::uint8_t len) {
				debug_->debug("ND:set_payload(id=%d, [0]='%c', len=%d", (int)payload_id, data[0], (int)len);
				return 0;
			}

			void force_beacon() {
				debug_->debug("ND:force_beacon");
			}

			void enter_sync_phase() {
				debug_->debug("ND:enter_sync_phase");
			}

			void leave_sync_phase() {
				debug_->debug("ND:leave_sync_phase");
			}

			void enter_token_phase() {
				debug_->debug("ND:enter_token_phase");
			}

			void leave_token_phase() {
				debug_->debug("ND:leave_token_phase");
			}

			void set_force_sync_phases(bool f) {
				debug_->debug("ND:set_force_sync_phases(%d)", (int)f);
			}

			template<class T, void(T::*TMethod)(uint8_t, node_id_t, uint8_t, uint8_t*) >
			uint8_t reg_event_callback(uint8_t alg_id, uint8_t events_flag, T *obj_pnt) {
				EventCallback ec;
				ec.payload_id = alg_id;
				ec.events = events_flag;
				ec.callback = event_callback_t::template from_method<T, TMethod>(obj_pnt);
				event_callbacks_.push_back(ec);
				debug_->debug("ND:reg_event_callback(payload_id=%d, evts=%d)", (int)alg_id, (int)events_flag);
				return 0;
			}

			void mock_insert_event(::uint8_t event, ::uint8_t payload_id, node_id_t from, ::uint8_t len, ::uint8_t *data) {
				for(typename EventCallbacks::iterator it = event_callbacks_.begin(); it != event_callbacks_.end(); ++it) {
					if(it->payload_id == payload_id && !!(event & it->events)) {
						it->callback(event, from, len, data);
					}
				}
			}
		
		private:
			void check() {
				assert(debug_ != 0);
			}

			typename Debug::self_pointer_t debug_;
			Neighbors neighbors_;
			EventCallbacks event_callbacks_;
		
	}; // MockNeighborhood
}

#endif // MOCK_NEIGHBORHOOD_H

