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
	 * @brief This is not actually a neighborhood but maintains and provides
	 * routing information for INSEs by the use of AMQs on top of any
	 * tree-shaped neighborhood.
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
				
				for(size_type i = start; i < global_tree_->childs(); i++) {
					if(global_tree_->child_user_data(i).contains(se_id)) { return i; }
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
			 * @return true iff this node  is the root of the ND tree.
			 */
			bool am_root() {
				return radio_->id() == global_tree_->root();
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
				
				if(forward) {
					if(sender == global_tree_->parent()) {
						if(registry_->contains(se_id)) { return radio_->id(); }
						size_type idx = find_first_se_child(se_id, 0);
						if(idx == npos) { return global_tree_->parent(); }
						return global_tree_->child(idx);
					}
					
					size_type child_idx = global_tree_->child_index(sender);
					if(child_idx == npos) { return NULL_NODE_ID; }
					
					size_type idx = find_first_se_child(se_id, child_idx + 1);
					if(idx == npos) {
						// received from last child
						if(am_root() && registry_->contains(se_id)) { return radio_->id(); }
						return global_tree_->parent();
					}
					return global_tree_->child(idx);
				}
				
				// forward == false
				
				if(sender == global_tree_->parent()) {
					size_type last_child = find_last_se_child(se_id);
					if(last_child == npos) {
						if(registry_->contains(se_id)) { return radio_->id(); }
						return global_tree_->parent();
					}
					return global_tree_->child(last_child);
				}
				
				size_type child_idx = global_tree_->child_index(sender);
				if(child_idx == npos) { return NULL_NODE_ID; }
				
				// find predecessor-child
				size_type idx = find_last_se_child(se_id, child_idx);
				if(idx == npos) {
					if(registry_->contains(se_id)) { return radio_->id(); }
					return global_tree_->parent();
				}
				return global_tree_->child(idx);
				
				check();
			}
			AmqT& amq() {
				check();
				return global_tree_->user_data();
			}
			
			void on_neighborhood_changed(typename GlobalTreeT::EventType event_type, node_id_t addr) {
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
				global_tree_->set_user_data(a);
				check();
			}
			
			GlobalTreeT& tree() { return *global_tree_; }
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(global_tree_ != 0);
					global_tree_->check();
					assert(radio_ != 0);
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

