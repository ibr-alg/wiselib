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
		typename Radio_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SemanticEntityOnehopNeighborhood {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Debug_P Debug;
			
			typedef SemanticEntity<OsModel> SemanticEntityT;
			typedef ::uint32_t abs_millis_t;
			typedef ::uint16_t link_metric_t;
			
			enum { npos = (size_type)(-1) };
			
			enum SpecialNodeIds {
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				ROOT_NODE_ID = INSE_ROOT_NODE_ID
			};
			
			enum Restrictions {
				MAX_NEIGHBORS = INSE_MAX_NEIGHBORS,
				MAX_CHILDS = MAX_NEIGHBORS - 1,
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
						fresh_(true)
						//token_count_(0),
						//prev_token_count_(0)
					{
					}
					
					NeighborEntity(SemanticEntityId id, SemanticEntityState state) :
						semantic_entity_id_(id),
						semantic_entity_state_(state),
						fresh_(true)
						//token_count_(0),
						//prev_token_count_(0)
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
					
					//::uint8_t token_count() { return token_count_; }
					//void set_token_count(::uint8_t x) { token_count_ = x; }
					
					//::uint8_t prev_token_count() { return prev_token_count_; }
					//void set_prev_token_count(::uint8_t x) { prev_token_count_ = x; }
					
					// TODO
				private:
					SemanticEntityId semantic_entity_id_;
					::uint8_t semantic_entity_state_ : 2;
					::uint8_t fresh_ : 1;
					//::uint8_t token_count_;
					//::uint8_t prev_token_count_;
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
			
			void init(typename Radio::self_pointer_t radio, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				debug_ = debug;
				parent_ = NULL_NODE_ID;
				
				check();
			}
			
			
			///@{
			///@name Entity properties
			
			bool is_joined(SemanticEntityId id) {
				return semantic_entities_.contains(id) && semantic_entities_[id].is_joined();
			}
			
			bool is_in_subtree(SemanticEntityId id) {
				return first_child(id) != NULL_NODE_ID;
			}
			
			bool is_adopted(SemanticEntityId id) {
				return is_in_subtree(id) && !is_joined(id);
			}
			
			///@}
			
			///@{
			///@name This nodes routing properties
			
			bool is_leaf(SemanticEntityId id) {
				for(iterator iter = begin(); iter != end(); ++iter) {
					if(iter->semantic_entity_state(id) != UNAFFECTED) { return false; }
				}
				return true;
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
				check();
				
				SemanticEntityT &se = semantic_entities_[id];
				se.set_id(id);
				se.set_source(radio_->id());
				se.set_state(SemanticEntityT::JOINED);
				
				check();
				assert(semantic_entities_.contains(id));
			}
			
		///@}
			
			/**
			 * @param[in] msg Message containing the received beacon.
			 * @param[in] source Source that sent the beacon.
			 * @param[in] t_recv time the beacon was received.
			 * @param[in] lm link metric when receiving the beacon.
			 * @param[out] fwd new messages with parts of @a msg that need to
			 * be forwarded to neighboring nodes.
			 * @return true iff parts where added to @a fwd.
			 */
			template<typename BeaconMessageT>
			bool update_from_beacon(BeaconMessageT& msg, node_id_t source, abs_millis_t t_recv, link_metric_t lm, BeaconMessageT& fwd) {
				check();
				
				bool r = false;
				
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
				
				// Go through all SEs the neighbor '*iter' reported in this beacon.
				
				debug_->debug("@%lu recv beacon with %d SEs", (unsigned long)radio_->id(), (int)msg.semantic_entities());
				for(size_type i = 0; i<msg.semantic_entities(); i++) {
					node_id_t target = msg.target(i);
					SemanticEntityId se_id = msg.semantic_entity_id(i);
					::uint8_t token_count = msg.token_count(i);
					NeighborEntity &ne = iter->find_or_create_semantic_entity(se_id);
					
					assert(msg.semantic_entity_state(i) != SemanticEntityT::UNAFFECTED);
					ne.set_semantic_entity_state(msg.semantic_entity_state(i));
					
					debug_->debug("@%lu beacon %d src %lu tgt %lu S%lx.%lx c%d st %d",
							(unsigned long)radio_->id(),
							(int)i,
							(unsigned long)source,
							(unsigned long)target,
							(unsigned long)se_id.rule(),
							(unsigned long)se_id.value(),
							(int)token_count,
							(int)msg.semantic_entity_state(i));
					
					// Mark info as fresh, so we know, neigh still has this SE
					ne.refresh();
					
					// If neigh has a token count for us for this SE,
					// save the new token count and see whether it activates
					// us (and for how long depending on the position in the
					// tree)
					
					if(target == radio_->id()) {
						// TODO: Handle the case were we have not actually
						// joined this SE (but eg. a child sending token info
						// to us has)!
						
						// Should we forward the SE info with the next beacon
						// wave or is it for us to process?
						
						if(is_child(source)) {
							node_id_t n = next_child(se_id, source);
							if(n == NULL_NODE_ID) {
								// msg came from our last child, accept it!
								process_token(se_id, source, token_count);
							}
							else {
								r = true;
								
								// TODO: edit target for msg part accordingly
								
								// TODO: see whether the SE is already there...
								
								// TODO: what then?
								//   say everybody is in the same SE and we
								//   have 100 childs that want to communicate
								//   to the next in the ring at the same time
								//   
								//   (a) only forward (all) parts that will
								//       activate. Problem: Our info might be
								//       just one round out of date!
								//       
								//   (b) send multiple beacons if necessary
								//   
								//   (c) only forward the highest token count
								//       (if multiple, highest node-id)
								//       -> only works if the scheduling is
								//       already stable!
								//       
								//   (c') as in (c), but when childs see a
								//        beacon for a higher sibling, they
								//        silently switch their token-count to
								//        that value.
								fwd.add_semantic_entity_from(msg, i);
							}
						}
						else if(source == parent_id()) {
							process_token(se_id, source, token_count);
						}
					} // if for us
				} // for SEs in msg
				
				// Erase all SEs the neighbor has not reported in this beacon
				iter->erase_outdated_semantic_entities();
				
				check();
				return r;
			}
			
			void process_token(SemanticEntityId se_id, node_id_t source, ::uint8_t token_count) {
				SemanticEntityT &se = semantic_entities_[se_id];
				se.set_source(source);
				se.set_prev_token_count(token_count);
				se.set_orientation(source == parent_id() ? SemanticEntityT::DOWN : SemanticEntityT::UP);
				
				if(se.state() == SemanticEntityT::UNAFFECTED) {
					se.set_state(SemanticEntityT::ADOPTED);
				}
				
				// Dijkstras token ring
				bool has_token = (is_root() == (se.prev_token_count() == se.token_count()));
				
				debug_->debug("@%lu tok r%d c%d,%d", (unsigned long)radio_->id(), (int)is_root(),
						(int)se.prev_token_count(), (int)se.token_count());
				
				if(se.activity_rounds() == 0 && has_token) {
					se.set_activity_rounds(is_leaf(se_id) ? 2 : 1);
				}
			}
			
			void update_tree_state() {
				check();
				
				if(is_root()) {
					parent_ = NULL_NODE_ID;
					changed_parent_ = false;
					
					check();
					return;
				}
				
				// 
				// Determine parent
				// 
				
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
				
				check();
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
				assert(semantic_entities_.contains(id));
				check();
				
				// Orientation is only relevant if the token comes from
				// ourselves (source == radio_->id())
				// and tells us whether it should travel upwards or downwards
				// now.
				::uint8_t orientation = semantic_entities_[id].orientation();
				node_id_t source = semantic_entities_[id].source();
				
				debug_->debug("@%lu next_hop: src %lu rt%d", (unsigned long)radio_->id(), (unsigned long)source, (int)is_root());
				
				
				if(is_root()) {
					if(!is_child(source)) {
						debug_->debug("@%lu next_hop not child!", (unsigned long)radio_->id());
						return NULL_NODE_ID;
					}
					node_id_t nxt = next_child(id, source);
					if(nxt == NULL_NODE_ID) {
						debug_->debug("@%lu next_hop last child", (unsigned long)radio_->id());
						return first_child(id);
					}
					return nxt;
				}
				
				
				if(source == radio_->id()) {
					debug_->debug("@%lu next_hop: from ourselves o=%d in_subtree=%d", (unsigned long)radio_->id(), (int)orientation, (int)is_in_subtree(id));
					if(orientation == UP) { return parent_id(); }
					return is_in_subtree(id) ? first_child(id) : parent_id();
				}
				else {
					if(source == parent_id()) { return radio_->id(); }
					
					debug_->debug("@%lu next_hop: not parent", (unsigned long)radio_->id());
					
					if(is_child(source)) {
						node_id_t nxt = next_child(id, source);
						// source was our last child, its for us
						if(nxt == NULL_NODE_ID) { return radio_->id(); }
						return nxt;
					}
					debug_->debug("@%lu next_hop: not child", (unsigned long)radio_->id());
				}
				
				check();
				return NULL_NODE_ID;
			} // next_hop()
			
			
			/**
			 * If there is at least one entity that has an activity round
			 * scheduled, return true and decrease all the activity round
			 * schedules on SEs by one (where they are > 0).
			 * Else, return false.
			 */
			bool be_active() {
				// TODO
				bool r = false;
				for(typename SemanticEntities::iterator iter = semantic_entities_.begin(); iter != semantic_entities_.end(); ++iter) {
					SemanticEntityT &se = iter->second;
					if(se.activity_rounds()) {
						r = true;
						se.set_activity_rounds(se.activity_rounds() - 1);
						if(se.activity_rounds() == 0) {
							se.set_token_count(se.prev_token_count() + is_root() ? 1 : 0);
							se.set_source(radio_->id());
						}
					}
				}
				return r;
			}
		
		private:
			
			void check() {
				assert(radio_ != 0);
				assert(!is_root() || (parent_ == NULL_NODE_ID));
				
				// There should be no SE in the SE container marked as
				// "UNAFFECTED" (as it wouldnt make sense to keep track of
				// those, so this signals some kind of error).
				
				for(typename SemanticEntities::iterator iter = semantic_entities_.begin(); iter != semantic_entities_.end(); ++iter) {
					assert(iter->second.state() != SemanticEntityT::UNAFFECTED);
				}
			}
			
			///@{
			///@name Child operations.
			
			bool is_child(node_id_t n) {
				if(n == parent_ || n == NULL_NODE_ID || n == BROADCAST_ADDRESS) { return false; }
				
				for(typename Neighbors::iterator iter = neighbors_.begin(); iter != neighbors_.end(); ++iter) {
					if(n == iter->id()) { return true; }
				}
				return false;
			}
			
			/**
			 * Will ignore childs that have ID 0, (conveniently fixed by
			 * defining 0 as the root!)
			 */
			node_id_t first_child(SemanticEntityId id) { return next_child(id, 0); }
			
			node_id_t next_child(SemanticEntityId id, node_id_t n) {
				debug_->debug("@%lu next_child(%lx.%lx, %lu)", (unsigned long)radio_->id(),
						(unsigned long)id.rule(), (unsigned long)id.value(), (unsigned long)n);
				
				node_id_t m = NULL_NODE_ID;
				
				for(typename Neighbors::iterator iter = neighbors_.begin(); iter != neighbors_.end(); ++iter) {
					node_id_t addr = iter->id();
					
					debug_->debug("@%lu next_child %lu m%lu n%lu p%lu s%d %d%d%d%d",
							(unsigned long)radio_->id(),
							(unsigned long)addr,
							(unsigned long)m,
							(unsigned long)n,
							(unsigned long)parent_,
							(int)iter->semantic_entity_state(id),
							(int)(addr > n),
							(int)(addr < m),
							(int)(addr != parent_),
							(int)(iter->semantic_entity_state(id) != UNAFFECTED)
							);
					
					if(addr > n && (m == NULL_NODE_ID || addr < m) &&
							addr != parent_ && iter->semantic_entity_state(id) != UNAFFECTED) {
						m = addr;
					}
				}
				return m;
			}
			
			///@}
			
			Neighbors neighbors_;
			SemanticEntities semantic_entities_;
			
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			//typename Timer::self_pointer_t timer_;
			//typename Clock::self_pointer_t clock_;
			//typename Rand::self_pointer_t rand_;
			
			node_id_t parent_;
			bool changed_parent_;
		
	}; // SemanticEntityOnehopNeighborhood
}

#endif // SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H

