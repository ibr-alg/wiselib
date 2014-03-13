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

#ifndef SE_ND_TOKEN_RING_H
#define SE_ND_TOKEN_RING_H

#include <external_interface/external_interface.h>
#include "token_message.h"

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
		typename Neighborhood_P,
		typename Tree_P
	>
	class SeNdTokenRing {
		
		public:
			typedef SeNdTokenRing self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

			typedef typename OsModel::Radio Radio;

			typedef Neighborhood_P Neighborhood;
			typedef typename Neighborhood::node_id_t node_id_t;
			typedef Tree_P Tree;

			typedef TokenMessage<OsModel, 'k', Radio> TokenMessageT;

			enum {
				MESSAGE_TYPE_TOKEN = TokenMessageT::MESSAGE_TYPE,
				PAYLOAD_ID = 2
			};

			enum {
				NULL_NODE_ID = Neighborhood::NULL_NODE_ID
			};

			enum {
				npos = (size_type)(-1)
			};

			SeNdTokenRing() : neighborhood_(0), tree_(0) {
			}

			void init(Neighborhood& neighborhood, Tree& tree) {
				neighborhood_ = &neighborhood;
				tree_ = &tree;

				neighborhood_->template reg_event_callback<
					self_type, &self_type::on_neighborhood_event
				>(
					2,
					Neighborhood::NEW_NB_BIDI | Neighborhood::DROPPED_NB | Neighborhood::NEW_PAYLOAD_BIDI,
					this
				);

				in_token_phase_ = true;
			}

			void enter_sync_phase() {
				check();
				in_token_phase_ = false;
			}

			void leave_sync_phase() {
			}

			bool enter_token_phase() {
				check();

				in_token_phase_ = true;
				bool r = has_token();
				if(r) {
					activity_rounds_--;
					if(activity_rounds_ <= 0) {
						process_token_count();
						assert(!has_token());
					}
				}

				check();
				return r;
			}

			void leave_token_phase() {
				in_token_phase_ = false;
			}

		
		private:
			void check() {
				assert(neighborhood_ != 0);
				assert(tree_ != 0);
				assert(has_token() == (activity_rounds_ != 0));
			}

			bool is_root() { return tree_->is_root(); }
			bool is_leaf() { return tree_->first_child() == NULL_NODE_ID; }

			bool has_token() {
				return is_root() == (token_count_ != prev_token_count_);
			}

			void process_token_count() {
				if(is_root()) { token_count_++; }
				else { token_count_ = prev_token_count_; }
			}
			
			void on_neighborhood_event(::uint8_t event, node_id_t from, ::uint8_t size, ::uint8_t* data) {
				check();

				if(event & Neighborhood::NEW_PAYLOAD_BIDI) {
					if(size == 0) { return; } // empty payload shouldnt happen actually
					if(*data == MESSAGE_TYPE_TOKEN) {
						TokenMessageT &msg = *reinterpret_cast<TokenMessageT*>(data);
						
						// Should we forward this token?
						node_id_t target;
						forward_address(from, target, upwards_);

						// The token is actually meant for us, process it,
						// see if it activates us
						if(target == id()) {
							prev_token_count_ = msg.token_count();

							if(activity_rounds_ == 0 && has_token()) {
								activity_rounds_ = is_leaf() ? 2 : 1;
								//upwards_ = (from != parent());
							}
						}

						// We just forward the token for one of our childs
						else {
							// reuse the message we received
							msg.set_target(target);
							neighborhood_->set_payload(PAYLOAD_ID, data, size);
							neighborhood_->force_beacon();
						}
					} // MESSAGE_TYPE_TOKEN
				} // NB_PAYLOAD_BIDI

				check();
			} // on_neighborhood_event()



			/**
			 * @param[in,out] upwards if from == id(), sent token upwards or
			 *  downwards according to truth value of this variable.
			 *  Communicate whether it has been sent upwards using
			 *  this as output, regardless of whether from == id().
			 *
			 */
			void forward_address(node_id_t from, node_id_t& to, bool& upwards) {
				check();

				to = NULL_NODE_ID;
				switch(tree_->classify(from)) {
					case Tree::CHILD: {
						node_id_t next_ch = next_child(from);
						if(next_ch == NULL_NODE_ID) {
							// no next child => msg came from last child
							// => token is for us (upwards)
							to = id();
							upwards = true;
						}
						else {
							to = next_ch;
							upwards = false;
						}
						break;
					}

					case Tree::PARENT:
						to = id();
						upwards = false;
						break;

					default:
						if(from == id()) {
							if(is_root()) {
								upwards = false;
								to = first_child();
							}
							else {
								to = upwards ? parent(): first_child();
							}
						}
						else {
							to = NULL_NODE_ID;
						}
						break;
				}

				check();
			}

			node_id_t id() {
				return neighborhood_->id();
			}

			node_id_t parent() {
				return tree_->parent();
			}

			node_id_t next_child(node_id_t c) {
				return tree_->next_child(c);
			}

			node_id_t first_child() {
				return tree_->first_child();
			}

			bool in_token_phase_;
			::uint8_t activity_rounds_;
			bool upwards_;
			::uint16_t token_count_;
			::uint16_t prev_token_count_;

			typename Neighborhood::self_pointer_t neighborhood_;
			typename Tree::self_pointer_t tree_;
		
	}; // SeNdTokenRing
}

#endif // SE_ND_TOKEN_RING_H

