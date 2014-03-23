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

#define SE_ND_TOKEN_RING_DEBUG_WARN 1

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
		typename Tree_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SeNdTokenRing {
		
		public:
			typedef SeNdTokenRing self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

			//typedef typename OsModel::Radio Radio;

			typedef Neighborhood_P Neighborhood;
			typedef typename Neighborhood::node_id_t node_id_t;
			typedef typename Neighborhood::Radio Radio;

			typedef Tree_P Tree;

			typedef Debug_P Debug;

			typedef TokenMessage<OsModel, 'k', Radio> TokenMessageT;

			enum {
				MESSAGE_TYPE_TOKEN = TokenMessageT::MESSAGE_TYPE,
				PAYLOAD_ID = 3
			};

			enum {
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};

			enum {
				npos = (size_type)(-1)
			};

			SeNdTokenRing() : neighborhood_(0), tree_(0) {
			}

			void init(Neighborhood& neighborhood, Tree& tree, Debug& debug) {
				neighborhood_ = &neighborhood;
				tree_ = &tree;
				debug_ = &debug;

				neighborhood_->register_payload_space(PAYLOAD_ID);
				neighborhood_->template reg_event_callback<
					self_type, &self_type::on_neighborhood_event
				>(
					PAYLOAD_ID,
					Neighborhood::NEW_NB_BIDI | Neighborhood::DROPPED_NB | Neighborhood::NEW_PAYLOAD_BIDI,
					this
				);

				tree_->template register_update_callback<
					self_type, &self_type::on_topology_update
				>(this);

				in_token_phase_ = true;
				if(has_token()) {
					activity_rounds_ = 1;
				}

				update_beacon();
			}

			void enter_sync_phase() {
				debug_->debug("TR:esp");
				debug_state();
				check();
				in_token_phase_ = false;
				check();
			}

			void leave_sync_phase() {
				debug_->debug("TR:lsp");
				debug_state();
				check();
			}

			bool enter_token_phase() {
				debug_->debug("TR:etp");
				debug_state();
				check();

				if(has_token()) {
					debug_->debug("TOKEN a%d u%d c%d", (int)activity_rounds_, (int)sending_upwards(), (int)token_count());
				}

				in_token_phase_ = true;
				check();
				return has_token();
			}

			void leave_token_phase() {
				debug_->debug("TR:ltp T%d a%d", (int)has_token(), (int)activity_rounds_);
				debug_state();
				check();
				bool r = has_token();
				if(r) {
					activity_rounds_--;
					if(activity_rounds_ <= 0) {
						process_token_count();
						assert(!has_token());
					}
				}

				in_token_phase_ = false;
				check();
			}

		
		private:

			/**
			 * Some class invariants, usually checked at end of methods.
			 */
			void check() {
				#if !NDEBUG
					assert(neighborhood_ != 0);
					assert(tree_ != 0);

					// When we get the token, we plan at least one activity
					// round, as soon as we have used up our activity rounds, the
					// token count will be processed such that we do not have the
					// token anymore.
					assert(has_token() == (activity_rounds_ != 0));

					// Root does not use token_count_[1] and prev_token_count_[1]
					// AT ALL.
					assert(is_root() <= (
								token_count_[1] == 0 &&
								prev_token_count_[1] == 0
					));
				#endif
			}

			bool is_root() { return tree_->is_root(); }
			bool is_leaf() { return tree_->first_child() == NULL_NODE_ID; }

			bool has_token() {
				return is_root() == (
						token_count_[0] == prev_token_count_[0] &&
						token_count_[1] == prev_token_count_[1]
					);
			}

			void process_token_count() {
				if(is_root()) {
					token_count_[0]++;
				}
				else if(is_leaf()) {
					token_count_[0] =
					token_count_[1] =
					prev_token_count_[1] =
					prev_token_count_[0];
				}
				else {
					token_count_[0] = prev_token_count_[0];
					token_count_[1] = prev_token_count_[1];
				}

				update_beacon();
			}

			void update_beacon() {
				node_id_t target = send_to_address();
				
				debug_state();
				//debug_->debug("TR:set_payload tgt=%lu c[%d]=%lu", (unsigned long)target, (int)sending_upwards(), (unsigned long)token_count());
				TokenMessageT msg;
				msg.set_target(target);
				msg.set_token_count(token_count());

				neighborhood_->set_payload(PAYLOAD_ID, msg.data(), msg.size());
				neighborhood_->force_beacon();
			}

			/**
			 * True iff we are currently adressing our parent with our beacon.
			 */
			bool sending_upwards() {
				return !is_root() && (token_count_[0] == token_count_[1]);
			}

			::uint16_t& token_count() { return token_count_[sending_upwards()]; }
			::uint16_t& prev_token_count() { return prev_token_count_[sending_upwards()]; }
			
			void on_neighborhood_event(::uint8_t event, node_id_t from, ::uint8_t size, ::uint8_t* data, ::uint32_t _) {
				check();

				if(event & Neighborhood::NEW_PAYLOAD_BIDI) {
					if(size == 0) { return; } // empty payload shouldnt happen actually
					if(*data == MESSAGE_TYPE_TOKEN) {
						TokenMessageT &msg = *reinterpret_cast<TokenMessageT*>(data);
						if(msg.target() != id()) { return; }
						//debug_->debug("TR:recv_payload %lu from %lu isp=%d tc=%d", (unsigned long)id(), (unsigned long)from, (int)(from == parent()), (int)msg.token_count());
						
						// Should we forward this token?
						node_id_t target = forward_address(from);

						// The token is actually meant for us, process it,
						// see if it activates us
						if(target == id()) {

							// This should halt at least in an ideal
							// simulation (without packet loss and thelike)
							assert(is_leaf() <= (from == parent()));

							// use upwards token slot (token_count_[1])
							// exactly when we are not root and the token came
							// from a child (root always uses slot 0 only)
							bool use_upwards_token = !is_root() && (from != parent());

							if(msg.token_count() == prev_token_count_[use_upwards_token]) {
								// We already saw this token, nothing to do
								// here
								return;
							}

							prev_token_count_[use_upwards_token] = msg.token_count();

							debug_->debug("TR:f%lu c%d a%d t%d", (unsigned long)from, (int)msg.token_count(), (int)activity_rounds_, (int)has_token());

							if((activity_rounds_ == 0) && has_token()) {
								/*
								 * Be active this many rounds. That is, one
								 * for inner nodes, two for leafs.
								 * If we receive this within a token phase,
								 * this does not count (e.g. we want the full next
								 * phase), so just add another round.
								 *
								 * This is especially important in shawn where
								 * a two-hop message send can already take
								 * longer than 1s.
								 */
								activity_rounds_ = (is_leaf() ? 2 : 1) + in_token_phase_;

								#if SE_ND_TOKEN_RING_DEBUG_WARN
									if(in_token_phase_) {
										debug_->debug("!TL t%lu f%lu", (unsigned long)_, (unsigned long)from);
									}
								#endif

							}
							else if(!has_token()) {
								// Token count from predecessor actually
								// deactives us
								activity_rounds_ = 0;
							}
						}

						// We just forward the token for one of our childs
						else {
							// reuse the message we received
							TokenMessageT m(msg);
							m.set_target(target);
							//debug_->debug("TR:set_payload f tgt=%lu c[%d]=%lu", (unsigned long)target, (int)sending_upwards(), (unsigned long)token_count());
							neighborhood_->set_payload(PAYLOAD_ID, m.data(), m.size());
							neighborhood_->force_beacon();
						}
					} // MESSAGE_TYPE_TOKEN
					debug_state();
				} // NB_PAYLOAD_BIDI

				check();
			} // on_neighborhood_event()

			void on_topology_update() {
				check();
				//debug_->debug("TR:topology update");
				update_beacon();
				check();
			}

			void debug_state() {
				debug_->debug("TR:c%d,%d|%d,%d u%d a%d T%d p%d",
						(int)prev_token_count_[0], (int)token_count_[0],
						(int)prev_token_count_[1], (int)token_count_[1],
						(int)sending_upwards(),
						(int)activity_rounds_,
						(int)has_token(),
						(int)in_token_phase_
						);
			}

			/**
			 */
			node_id_t forward_address(node_id_t from) {
				check();

				node_id_t to = NULL_NODE_ID;
				switch(tree_->classify(from)) {
					case Tree::CHILD: {
						node_id_t next_ch = next_child(from);
						if(next_ch == NULL_NODE_ID) {
							// no next child => msg came from last child
							// => token is for us (upwards)
							to = id();
							//upwards = true;
						}
						else {
							to = next_ch;
							//upwards = false;
						}
						break;
					}

					case Tree::PARENT:
						to = id();
						//upwards = false;
						break;

					default:
						break;
				}

				check();
				return to;
			}

			node_id_t send_to_address() {
				node_id_t to = NULL_NODE_ID;
				if(is_root()) {
					to = first_child();
				}
				else {
					to = sending_upwards() ? parent(): first_child();
				}
				return to;
			}

			node_id_t id() {
				return neighborhood_->radio().id();
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

			::uint16_t token_count_[2];
			::uint16_t prev_token_count_[2];
			
			::uint8_t activity_rounds_;
			bool in_token_phase_;

			typename Neighborhood::self_pointer_t neighborhood_;
			typename Tree::self_pointer_t tree_;
			typename Debug::self_pointer_t debug_;
		
	}; // SeNdTokenRing
}

#endif // SE_ND_TOKEN_RING_H


