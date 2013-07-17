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
		typename Radio_P
	>
	class SemanticEntityAmqNeighborhood {
		
		public:
			typedef SemanticEntityAmqNeighborhood self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			typedef Amq_P AmqT;
			//typedef ... iterator;
			typedef GlobalTree_P GlobalTreeT;
			
			enum State { IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE };
			
			void init(typename GlobalTreeT::self_pointer_t global_tree) {
				global_tree_ = global_tree;
				global_tree_->reg_event_callback(
						GlobalTreeT::event_callback_t::template from_method<self_type, &self_type::on_neighborhood_changed>(this)
				);
			}
			
			/*
			iterator begin_childs(const SemanticEntityId& se_id) {
				// TODO
			}
			
			iterator end_childs() {
				// TODO
			}
			*/
			
			void set_child_amq(node_id_t child_id, AmqT& amq) {
				// TODO
			}
			
			/**
			 * Find the next receiver in the ring that should receive
			 * communication coming from @a sender.
			 * That is, the next child with that se_id, if that doesnt exist
			 * the parent, if it came from parent, we wanna receive the packet
			 * ourselves.
			 * 
			 * TODO: Implement case forward=false (ie. forward acks the other way)
			 */
			node_id_t forward_address(const SemanticEntityId& se_id, node_id_t sender, bool forward) {
				node_id_t next = sender;
				do {
					AmqT amq;
					next = tree_forward_address(next, forward, amq);
					if((next == global_tree_->parent()) || (next == radio_->id()) || amq.contains(se_id)) {
						return next;
					}
				} while(next != sender);
				return radio_->id();
			}
			
			
			node_id_t tree_forward_address(node_id_t sender, bool forward, AmqT& amq) {
				if(sender == global_tree_->parent()) { return radio_->id(); }
				if(sender == radio_->id()) {
					if(global_tree_->childs()) { return global_tree_->child(0); }
					else { return global_tree_->parent(); } // TODO: what if we also are root?
				}
				
				size_type child_idx = global_tree_->child_index(sender);
				if(child_idx == GlobalTreeT::npos) { return NULL_NODE_ID; }
				
				if(child_idx == global_tree_->childs() - 1) {
					if(global_tree_->state().root() == radio_->id()) {
						amq = global_tree_->child_user_data(0);
						return global_tree_->child(0);
					}
					else { return global_tree_->parent(); }
				}
				
				amq = global_tree_->child_user_data(child_idx + 1);
				return global_tree_->child(child_idx + 1);
			}
			
			AmqT& amq() {
				return global_tree_->user_data();
			}
			
			void on_neighborhood_changed(typename GlobalTreeT::EventType event_type) {
				AmqT a;
				for(typename GlobalTreeT::iterator iter = global_tree_->begin_neighbors(); iter != global_tree_->end_neighbors(); ++iter) {
					if(iter->state() == GlobalTreeT::IN_EDGE) {
						a |= iter->user_data();
					}
				}
				
				global_tree_->set_user_data(a);
			}
			
		private:
			
			AmqT amq_;
			typename GlobalTreeT::self_pointer_t global_tree_;
			typename Radio::self_pointer_t radio_;
		
	}; // SemanticEntityAmqNeighborhood
}

#endif // SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H

