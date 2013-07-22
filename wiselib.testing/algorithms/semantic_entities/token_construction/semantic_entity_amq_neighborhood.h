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

#ifndef SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H
#define SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H

#include "semantic_entity_id.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup nhood_concept
	 * @ingroup better_nhood_concept
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename GlobalTree_P,
		typename Amq_P,
		typename Registry_P,
		typename Radio_P
	>
	class SemanticEntityAmqNeighborhood {
		
		public:
			typedef SemanticEntityAmqNeighborhood self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Registry_P Registry;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			typedef Amq_P AmqT;
			//typedef ... iterator;
			typedef GlobalTree_P GlobalTreeT;
			
			enum State { IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE };
			enum { npos = (size_type)(-1) };
			
			SemanticEntityAmqNeighborhood() : global_tree_(0), radio_(0) {
			}
			
			void init(typename GlobalTreeT::self_pointer_t global_tree, typename Registry::self_pointer_t registry, typename Radio::self_pointer_t radio) {
				global_tree_ = global_tree;
				registry_ = registry;
				radio_ = radio;
				
				global_tree_->reg_event_callback(
						GlobalTreeT::event_callback_t::template from_method<self_type, &self_type::on_neighborhood_changed>(this)
				);
				check();
			}
			
			/*
			iterator begin_childs(const SemanticEntityId& se_id) {
				// TODO
			}
			
			iterator end_childs() {
				// TODO
			}
			*/
			
			//void set_child_amq(node_id_t child_id, AmqT& amq) {
				//// TODO
			//}
			
			/**
			 * Assuming we have the token, where should we send it to?
			 */
			node_id_t next_token_node(const SemanticEntityId& se_id) {
				check();
				
				size_type idx = find_first_se_child(se_id, 0);
				if(idx == npos) {
					if(global_tree_->root() == radio_->id()) { return radio_->id(); }
					return global_tree_->parent();
				}
				
				check();
				return global_tree_->child(idx);
			}
			
			/**
			 * Assuming we have the token, where did we get it from?
			 */
			node_id_t prev_token_node(const SemanticEntityId& se_id) {
				check();
				
				if(global_tree_->root() == radio_->id()) {
					size_type idx = find_last_se_child(se_id);
					if(idx == npos) { return radio_->id(); }
					return global_tree_->child(idx);
				}
				return global_tree_->parent();
			}
			
			size_type find_first_se_child(const SemanticEntityId& se_id, size_type start) {
				check();
				
				
				//DBG("node %d // find first se child start=%d n=%d", (int)radio_->id(),
						//(int)start, (int)global_tree_->childs());
				
				for(size_type i = start; i < global_tree_->childs(); i++) {
					//DBG("node %d // find first se child i=%d contains=%d",
							//(int)radio_->id(), (int)i, (int)global_tree_->child_user_data(i).contains(se_id));
					
				/*
				DBG("node %d // find first se child userdata %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x",
						(int)radio_->id(),
						global_tree_->child_user_data(i).data()[0], global_tree_->child_user_data(i).data()[1], global_tree_->child_user_data(i).data()[2], global_tree_->child_user_data(i).data()[3],
						global_tree_->child_user_data(i).data()[4], global_tree_->child_user_data(i).data()[5], global_tree_->child_user_data(i).data()[6], global_tree_->child_user_data(i).data()[7],
						global_tree_->child_user_data(i).data()[8], global_tree_->child_user_data(i).data()[9], global_tree_->child_user_data(i).data()[10], global_tree_->child_user_data(i).data()[11],
						global_tree_->child_user_data(i).data()[12], global_tree_->child_user_data(i).data()[13], global_tree_->child_user_data(i).data()[14], global_tree_->child_user_data(i).data()[15],
						global_tree_->child_user_data(i).data()[16], global_tree_->child_user_data(i).data()[17], global_tree_->child_user_data(i).data()[18], global_tree_->child_user_data(i).data()[19],
						global_tree_->child_user_data(i).data()[20], global_tree_->child_user_data(i).data()[21], global_tree_->child_user_data(i).data()[22], global_tree_->child_user_data(i).data()[23],
						global_tree_->child_user_data(i).data()[24], global_tree_->child_user_data(i).data()[25], global_tree_->child_user_data(i).data()[26], global_tree_->child_user_data(i).data()[27],
						global_tree_->child_user_data(i).data()[28], global_tree_->child_user_data(i).data()[29], global_tree_->child_user_data(i).data()[30], global_tree_->child_user_data(i).data()[31]
				);
				*/
				
					if(global_tree_->child_user_data(i).contains(se_id)) {
						return i;
					}
				}
				
				check();
				return npos;
			}
			
			size_type find_last_se_child(const SemanticEntityId& se_id, size_type end = npos) {
				check();
				
				for(int i = (end == npos) ? global_tree_->childs() - 1 : (int)end - 1; i >= 0; i--) {
					if(global_tree_->child_user_data(i).contains(se_id)) {
						return i;
					}
				}
				
				check();
				return npos;
			}
			
			
			/**
			 * Find the next receiver in the ring that should receive
			 * communication coming from @a sender.
			 * That is, the next child with that se_id, if that doesnt exist
			 * the parent, if it came from parent, we wanna receive the packet
			 * ourselves.
			 */
			node_id_t forward_address(const SemanticEntityId& se_id, node_id_t sender, bool forward) {
				check();
				
				DBG("// forward_address from %d fwd %d self %d parent %d childs %d",
						(int)sender, (int)forward, (int)radio_->id(), (int)global_tree_->parent(), (int)global_tree_->childs());
				
				if(forward) {
					if(sender == global_tree_->parent()) {
						return radio_->id();
					}
					
					size_type child_idx = global_tree_->child_index(sender);
					if(child_idx == npos) { return NULL_NODE_ID; }
					size_type idx = find_first_se_child(se_id, child_idx + 1);
					if(idx == npos) {
						if(radio_->id() == global_tree_->root()) { return radio_->id(); }
						return global_tree_->parent();
					}
					return global_tree_->child(idx);
				}
				else {
					if(sender == global_tree_->parent()) {
						size_type last_child = find_last_se_child(se_id);
						if(last_child == npos) { return radio_->id(); }
						return global_tree_->child(last_child);
					}
					
					size_type child_idx = global_tree_->child_index(sender);
					if(child_idx == npos) { return NULL_NODE_ID; }
					
					// find predecessor-child
					size_type idx = find_last_se_child(se_id, child_idx);
					if(idx == npos) { return radio_->id(); }
					return global_tree_->child(idx);
				}
				
				check();
			}
				
				/*
				node_id_t next = sender;
				do {
					AmqT amq;
					next = tree_forward_address(next, forward, amq);
					if((next == global_tree_->parent()) || (next == radio_->id()) || amq.contains(se_id)) {
						return next;
					}
				} while(next != sender);
				
				check();
				return radio_->id();
				*/
			//}
			
			
			node_id_t tree_forward_address(node_id_t sender, bool forward, AmqT& amq) {
				check();
				
				if(sender == global_tree_->parent()) { return radio_->id(); }
				if(sender == radio_->id()) {
					if(global_tree_->childs()) { return global_tree_->child(0); }
					else { return global_tree_->parent(); } // TODO: what if we also are root?
				}
				
				size_type child_idx = global_tree_->child_index(sender);
				if(child_idx == GlobalTreeT::npos) { return NULL_NODE_ID; }
				
				if(child_idx == global_tree_->childs() - 1) {
					if(global_tree_->tree_state().root() == radio_->id()) {
						amq = global_tree_->child_user_data(0);
						return global_tree_->child(0);
					}
					else { return global_tree_->parent(); }
				}
				
				amq = global_tree_->child_user_data(child_idx + 1);
				
				check();
				return global_tree_->child(child_idx + 1);
			}
			
			AmqT& amq() {
				check();
				return global_tree_->user_data();
			}
			
			void on_neighborhood_changed(typename GlobalTreeT::EventType event_type) {
				check();
				
				AmqT a;
				for(typename GlobalTreeT::iterator iter = global_tree_->begin_neighbors(); iter != global_tree_->end_neighbors(); ++iter) {
					if(iter->state() == GlobalTreeT::IN_EDGE) {
						a |= iter->user_data();
					}
				}
				
				for(typename Registry::iterator iter = registry_->begin(); iter != registry_->end(); ++iter) {
					a.insert(iter->first);
				}
				
				/*
				DBG("node %d // setting filter to %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x",
						(int)radio_->id(),
						a.data()[0], a.data()[1], a.data()[2], a.data()[3],
						a.data()[4], a.data()[5], a.data()[6], a.data()[7],
						a.data()[8], a.data()[9], a.data()[10], a.data()[11],
						a.data()[12], a.data()[13], a.data()[14], a.data()[15],
						a.data()[16], a.data()[17], a.data()[18], a.data()[19],
						a.data()[20], a.data()[21], a.data()[22], a.data()[23],
						a.data()[24], a.data()[25], a.data()[26], a.data()[27],
						a.data()[28], a.data()[29], a.data()[30], a.data()[31]
				);
				*/
				global_tree_->set_user_data(a);
				
				check();
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(global_tree_);
					global_tree_->check();
					assert(radio_);
				#endif
			}
			
		private:
			
			AmqT amq_;
			typename GlobalTreeT::self_pointer_t global_tree_;
			typename Radio::self_pointer_t radio_;
			typename Registry::self_pointer_t registry_;
		
	}; // SemanticEntityAmqNeighborhood
}

#endif // SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H

