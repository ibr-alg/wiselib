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

#ifndef SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H
#define SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H

#include <util/pstl/list_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include "semantic_entity_id.h"
#include "semantic_entity.h"
#include <util/pstl/highscore_set.h>

#ifndef INSE_ROOT_NODE_ID
	#define INSE_ROOT_NODE_ID 0
#endif

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
		typename Radio_P
	>
	class SemanticEntityOnehopNeighborhood {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef SemanticEntity<OsModel> SemanticEntityT;
			typedef ::uint32_t abs_millis_t;
			typedef ::uint16_t link_metric_t;
			
			enum { npos = (size_type)(-1) };
			
			enum SpecialNodeIds {
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				ROOT_NODE_ID = INSE_ROOT_NODE_ID
			};
			
			enum Restrictions {
				MAX_NEIGHBORS = INSE_MAX_NEIGHBORS,
				MAX_TOTAL_SEMANTIC_ENTITIES = INSE_MAX_SEMANTIC_ENTITIES
			};
			
			enum MessageOrientation {
				UP, DOWN
			};
			
			enum SemanticEntityState {
				UNAFFECTED = SemanticEntityT::UNAFFECTED,
				JOINED = SemanticEntityT::JOINED,
				ADOPTED = SemanticEntityT::ADOPTED
			};
			
			class NeighborEntity {
				public:
					NeighborEntity() :
						semantic_entity_id_(),
						semantic_entity_state_(UNAFFECTED),
						fresh_(true),
						token_count_(0),
						prev_token_count_(0)
					{
					}
					
					NeighborEntity(SemanticEntityId id, SemanticEntityState state) :
						semantic_entity_id_(id),
						semantic_entity_state_(state),
						fresh_(true),
						token_count_(0),
						prev_token_count_(0)
					{
					}
					
					bool operator<(NeighborEntity& other) { return semantic_entity_id_ < other.semantic_entity_id_; }
					
					SemanticEntityId semantic_entity_id() { return semantic_entity_id_; }
					void set_semantic_entity_id(SemanticEntityId x) { semantic_entity_id_ = x; }
					
					::uint8_t semantic_entity_state() { return semantic_entity_state_; }
					void set_semantic_entity_state(::uint8_t x) { semantic_entity_state_ = x; }
					
					void refresh() { fresh_ = true; }
					void outdate() { fresh_ = false; }
					bool fresh() { return fresh_; }
					
					::uint8_t token_count() { return token_count_; }
					void set_token_count(::uint8_t x) { token_count_ = x; }
					
					::uint8_t prev_token_count() { return prev_token_count_; }
					void set_prev_token_count(::uint8_t x) { prev_token_count_ = x; }
					
					::uint8_t activity_rounds() { return activity_rounds_; }
					void set_activity_rounds(::uint8_t x) { activity_rounds_ = x; }
					
					// TODO
				private:
					SemanticEntityId semantic_entity_id_;
					::uint8_t semantic_entity_state_ : 2;
					::uint8_t fresh_ : 1;
					::uint8_t activity_rounds_ : 2;
					::uint8_t token_count_;
					::uint8_t prev_token_count_;
			};
			
			class Neighbor {
					typedef list_dynamic<OsModel, NeighborEntity> Entities;
				
				public:
					Neighbor() : id_(NULL_NODE_ID) {
					}
					
					Neighbor(node_id_t id) : id_(id) {
					}
					
					void init(node_id_t id) {
						id_ = id;
					}
					
					node_id_t id() { return id_; }
					void set_id(node_id_t x) { id_ = x; }
					
					abs_millis_t last_beacon_received() { return last_beacon_received_; }
					void set_last_beacon_received(abs_millis_t x) { last_beacon_received_ = x; }
					
					node_id_t parent() { return parent_; }
					void set_parent(node_id_t p) { parent_ = p; }
					
					::uint8_t root_distance() { return root_distance_; }
					void set_root_distance(::uint8_t d) { root_distance_ = d; }
					
					bool is_valid() { return id_ != NULL_NODE_ID; }
					
					bool operator<(Neighbor& other) { return id_ < other.id_; }
					
					::uint8_t semantic_entity_state(SemanticEntityId id) {
						for(typename Entities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
							if(iter->semantic_entity_id() == id) {
								return iter->semantic_entity_state();
							}
						}
						return UNAFFECTED;
					}
					
					NeighborEntity& find_or_create_semantic_entity(SemanticEntityId id) {
						for(typename Entities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
							if(iter->semantic_entity_id() == id) {
								return *iter;
							}
						}
						NeighborEntity ne(id, UNAFFECTED);
						typename Entities::iterator it = entities_.insert(ne);
						return *it;
					}
					
					void outdate_semantic_entities() {
						for(typename Entities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
							iter->outdate();
						}
					}
					
					void erase_outdated_semantic_entities() {
						for(typename Entities::iterator iter = entities_.begin(); iter != entities_.end(); ) {
							if(iter->fresh()) { ++iter; }
							else { iter = entities_.erase(iter); }
						}
					}
					
				private:
					list_dynamic<OsModel, NeighborEntity> entities_;
					node_id_t id_;
					abs_millis_t last_beacon_received_;
					node_id_t parent_;
					::uint8_t root_distance_;
			};
			
			//typedef vector_static<OsModel, Neighbor, MAX_NEIGHBORS> Neighbors;
			//typedef MapStaticVector<OsModel, node_id_t, Neighbor, MAX_NEIGHBORS> Neighbors;
			typedef HighscoreSet<OsModel, Neighbor, link_metric_t, MAX_NEIGHBORS> Neighbors;
			typedef typename Neighbors::iterator iterator;
			
			typedef MapStaticVector<OsModel, SemanticEntityId, SemanticEntityT, MAX_TOTAL_SEMANTIC_ENTITIES> SemanticEntities;
			typedef typename SemanticEntities::iterator semantic_entity_iterator;
			
			SemanticEntityOnehopNeighborhood() : radio_(0), parent_(NULL_NODE_ID) {
			}
			
			void init(typename Radio::self_pointer_t radio) {
				radio_ = radio;
				parent_ = NULL_NODE_ID;
			}
			
			
			///@{
			///@name Entity properties
			
			bool is_joined(SemanticEntityId id) {
				return semantic_entities_.contains(id) && semantic_entities_[id].is_joined();
			}
			
			bool is_in_subtree(SemanticEntityId id) {
				// TODO
				bool r;
				
				assert(r == (first_child(id) != NULL_NODE_ID));
				return r;
			}
			
			bool is_adopted(SemanticEntityId id) {
				return is_in_subtree(id) && !is_joined(id);
			}
			
			///@}
			
			///@{
			///@name This nodes routing properties
			
			bool is_leaf(SemanticEntityId id) {
				// TODO
				for(iterator iter = begin(); iter != end(); ++iter) {
					if(iter->semantic_entity_state(id) != UNAFFECTED) { return false; }
					return true;
				}
			}
			
			bool is_root() { return radio_->id() == ROOT_NODE_ID; }
			bool is_connected() { return is_root() || (parent_ != NULL_NODE_ID); }
			
			::uint8_t root_distance() {
				if(is_root()) { return 0; }
				if(!is_connected()) { return -1; }
				return parent().root_distance() + 1;
			}
			
			node_id_t parent_id() { return parent_; }
			void set_parent_id(node_id_t x) { parent_ = x; }
			
			Neighbor& parent() { return *find_neighbor(parent_id()); }
			
			///@}
			
		///@{
		///@name Neighbors
			
			iterator begin() { return neighbors_.begin(); }
			iterator end() { return neighbors_.end(); }
			
			iterator find_neighbor(node_id_t id) {
				for(iterator iter = begin(); iter != end(); ++iter) {
					if(iter->id() == id) { return iter; }
				}
				return end();
			}
			
		///@}
		
		///@{
		///@name Semantic Entities
			
			semantic_entity_iterator begin_semantic_entities() { return semantic_entities_.begin(); }
			semantic_entity_iterator end_semantic_entities() { return semantic_entities_.end(); }
			
			void add_semantic_entity(SemanticEntityId id) {
				semantic_entities_[id].set_id(id);
			}
			
		///@}
			
			template<typename BeaconMessageT>
			void update_from_beacon(BeaconMessageT& msg, node_id_t source, abs_millis_t t_recv, link_metric_t lm) {
				
				iterator iter( find_neighbor(source) );
				if(iter == end()) {
					Neighbor n(source);
					iter = neighbors_.insert(n, lm);
				}
				
				iter->set_parent(msg.parent());
				iter->set_root_distance(msg.root_distance());
				iter->set_last_beacon_received(t_recv);
				update_tree_state();
				
				iter->outdate_semantic_entities();
				
				// Go through all SEs neighbor *iter reported in this beacon
				
				for(size_type i = 0; i<msg.semantic_entities(); i++) {
					node_id_t target = msg.target(i);
					SemanticEntityId se_id = msg.semantic_entity_id(i);
					::uint8_t token_count = msg.token_count(i);
					NeighborEntity &ne = iter->find_or_create_semantic_entity(se_id);
					ne.set_semantic_entity_state(msg.semantic_entity_state(i));
					
					// Mark info as fresh, so we know, neigh still has this SE
					ne.refresh();
					
					// If neigh has a token count for us for this SE,
					// save the new token count and see whether it activates
					// us (and for how long depending on the position in the
					// tree)
					
					if(target == radio_->id()) {
						ne.set_prev_token_count(token_count);
						
						// Dijkstras token ring
						bool has_token = (is_root() == (ne.prev_token_count() == ne.token_count()));
						
						if(ne.activity_rounds() == 0 && has_token) {
							ne.set_activity_rounds(is_leaf(se_id) ? 2 : 1);
						}
					}
				}
				
				// Erase all SEs the neighbor has not reported in this beacon
				iter->erase_outdated_semantic_entities();
				
				/*
				neighbors_[source].set_parent(msg.parent());
				neighbors_[source].set_root_distance(msg.root_distance());
				update_tree_state();
				
				for(size_type i = 0; i < msg.ses(); i++) {
					node_id_t target = msg.target(i);
					CacheKey k(source, msg.se_id(i));
					bool there_before = beacon_cache_.contains(k);
					beacon_cache_[k].distance_first = msg.distance_first(i);
					beacon_cache_[k].distance_last = msg.distance_last(i);
					beacon_cache_[k].token_count = msg.token_count(i);
					
					//if(msg.target(i) == radio_->id()) {
				}
				*/
			}
			
			void update_tree_state() {
				if(is_root()) {
					parent_ = NULL_NODE_ID;
					changed_parent_ = false;
					return;
				}
				
				
				::uint8_t min_dist = -1;
				node_id_t parent = NULL_NODE_ID;
				
				for(typename Neighbors::iterator iter = neighbors_.begin(); iter != neighbors_.end(); ++iter) {
					node_id_t addr = iter->id();
					Neighbor& neigh = *iter;
					
					if( (neigh.root_distance() != (::uint8_t)(-1)) && (
							(neigh.root_distance() < min_dist) ||
							(neigh.root_distance() == min_dist && addr < parent))) {
						
						min_dist = neigh.root_distance();
						parent = addr;
					}
				}
				
				changed_parent_ = (parent != parent_);
				parent_ = parent;
				
			}
			
			bool changed_parent() { return changed_parent_; }
			
			
			/**
			 * Return the maximum number of hops a message that will be
			 * routed through us from a member of a SE to its successor
			 * somewhere else in our subtree.
			 */
			::uint8_t max_hops_for_adopted() {
				// TODO
				return 2;
			}
			
			node_id_t next_hop(SemanticEntityId id) { //, node_id_t source, MessageOrientation orientation) {
				
				// Orientation is only relevant if the token comes from
				// ourselves (source == radio_->id())
				// and tells us whether it should travel upwards or downwards
				// now.
				MessageOrientation orientation = semantic_entities_[id].orientation;
				node_id_t source = semantic_entities_[id].source;
				
				if(source == radio_->id()) {
					if(orientation == UP) {
						return parent();
					}
					else {
						return is_in_subtree(id) ? first_child(id) : parent();
					}
				}
				else {
					if(source == parent()) {
						return radio_->id();
					}
					
					size_type idx = child_index(id, source);
					if(idx == npos) {
						// source is neither or our parent or a child, just
						// ignore it
						return NULL_NODE_ID;
					}
					else {
						size_type next_idx = child_index(id, source, idx);
						if(next_idx == NULL_NODE_ID) {
							// source was our last child, its for us
							return radio_->id();
						}
					}
				}
			} // next_hop()
		
		private:
			
			void check() {
				assert(radio_ != 0);
				assert(!is_root() || (parent_ == NULL_NODE_ID));
			}
			
			node_id_t first_child(SemanticEntityId id) {
				// TODO
				return NULL_NODE_ID;
			}
			
			Neighbors neighbors_;
			SemanticEntities semantic_entities_;
			
			typename Radio::self_pointer_t radio_;
			//typename Timer::self_pointer_t timer_;
			//typename Clock::self_pointer_t clock_;
			//typename Rand::self_pointer_t rand_;
			
			node_id_t parent_;
			bool changed_parent_;
		
	}; // SemanticEntityOnehopNeighborhood
}

#endif // SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H

