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

#ifndef SS_MBF_TREE_H
#define SS_MBF_TREE_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include "tree_message.h"
#include <util/pstl/vector_static.h>

#define SS_MBF_TREE_DEBUG 1
#define SS_MBF_TREE_DEBUG_VERBOSE 0

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
		typename Radio_P = typename Neighborhood_P::Radio,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SsMbfTree {
		public:
			typedef SsMbfTree self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

			typedef Neighborhood_P Neighborhood;
			
			typedef Debug_P Debug;

			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;

			typedef ::uint8_t distance_t;

			typedef TreeMessage<OsModel> TreeMessageT;

			typedef delegate0<void> callback_t;
			typedef vector_static<OsModel, callback_t, 4> Callbacks;

			enum {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				MAX_ROOT_DISTANCE = 10,

				#if SHAWN
					ROOT_NODE_ID = 1,
				#else
					//ROOT_NODE_ID = 26668, // GREEN TUBS/ALG TelosB node
					//ROOT_NODE_ID = 2, // w.ilab.t inode008
					//ROOT_NODE_ID = 50053, // w.ilab.t inode019
					//ROOT_NODE_ID = 50053, // w.ilab.t inode019
					ROOT_NODE_ID = 53851, // w.ilab.t inode029
				#endif
			};

			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { npos = (size_type)(-1) };
			enum { CHILD, PARENT, UNRELATED };
			enum { MAX_NEIGHBORS = Neighborhood::MAX_NEIGHBORS };
			enum { PAYLOAD_ID = 2 };

		private:
			struct NeighborInfo {
				node_id_t id;
				node_id_t parent;
				distance_t distance;
				bool child;

				NeighborInfo() {
				}

				NeighborInfo(node_id_t i) : id(i), parent(NULL_NODE_ID), distance(-1), child(false) {
				}

				NeighborInfo(node_id_t i, node_id_t p, distance_t d) : id(i), parent(p), distance(d), child(false) {
				}

				bool operator==(const NeighborInfo& other) {
					return id == other.id;
				}
			};

		public:
			typedef vector_static<OsModel, NeighborInfo, MAX_NEIGHBORS> NeighborInfos;

			SsMbfTree() : debug_(0), root_(NULL_NODE_ID), id_(NULL_NODE_ID), distance_(-1) {
			}

			int init(node_id_t id, typename Neighborhood::self_pointer_t neighborhood, typename Debug::self_pointer_t debug) {
				id_ = id;
				neighborhood_ = neighborhood;
				debug_ = debug;
				distance_ = -1;
				parent_index_ = npos;
				
				#if SS_MBF_TREE_DEBUG 
					debug_->debug("TREE INIT @%lu", (unsigned long)this->id());
				#endif

				neighborhood_->register_payload_space(PAYLOAD_ID);
				neighborhood_->template reg_event_callback<self_type, &self_type::on_neighborhood_event>(
						PAYLOAD_ID,
						Neighborhood::NEW_NB_BIDI | Neighborhood::NEW_PAYLOAD_BIDI | Neighborhood::LOST_NB_BIDI | Neighborhood::DROPPED_NB,
						this);

				update_state(true);

				check();
				return SUCCESS;
			}

			template<class T, void (T::*TMethod)()>
			void register_update_callback(T *obj_pnt) {
				callbacks_.push_back(callback_t::template from_method<T, TMethod>(obj_pnt));
			}

			node_id_t root() { return root_; }
			void set_root(node_id_t n) { root_ = n; }

			node_id_t id() { return id_; }
			void set_id(node_id_t n) { id_ = n; }

			node_id_t parent() {
				if(parent_index_ == npos) { return NULL_NODE_ID; }
				return neighbor_infos_[parent_index_].id;
			}

			distance_t distance() {
				return distance_;
			}

			bool is_root() { return id() == ROOT_NODE_ID; }

			int classify(node_id_t c) {
				if(c == NULL_NODE_ID) { return UNRELATED; }
				else {
					typename NeighborInfos::iterator iter = neighbor_infos_.find(NeighborInfo(c));
					if(at_parent(iter)) { return PARENT; }
					if(iter != neighbor_infos_.end()) { return CHILD; }
				}
				return UNRELATED;
			}

			node_id_t first_child() {
				typename NeighborInfos::iterator it = neighbor_infos_.begin();
				if(it == neighbor_infos_.end()) { return NULL_NODE_ID; }
				while(at_parent(it) || !it->child) {
					++it;
					if(it == neighbor_infos_.end()) { return NULL_NODE_ID; }
				}
				return it->id;
			}

			node_id_t next_child(node_id_t c) {
				typename NeighborInfos::iterator it = neighbor_infos_.find(NeighborInfo(c));
				if(it == neighbor_infos_.end()) { return NULL_NODE_ID; }
				++it;
				if(it == neighbor_infos_.end()) { return NULL_NODE_ID; }
				while(at_parent(it) || !it->child) {
					++it;
					if(it == neighbor_infos_.end()) { return NULL_NODE_ID; }
				}
				return it->id;
			}

			Neighborhood& neighborhood() {
				return *neighborhood_;
			}

		private:
			void check() {
				#if !NDEBUG
					assert(debug_ != 0);
					assert(id_ != NULL_NODE_ID);

					assert(parent_index_ == npos ||
							(parent_index_ >= 0 && parent_index_ < neighbor_infos_.size()));

					// childs are always sorted
					node_id_t c, cprev = NULL_NODE_ID;
					c = first_child();
					size_type s = 0;
					while(c != NULL_NODE_ID) {
						assert((cprev != NULL_NODE_ID) <= (c > cprev));
						cprev = c;
						c = next_child(c);
						s++;
					}

					// If we are connected to the tree and not root,
					// our root-distance is the root distance of our parent + 1
					assert(
							parent() == NULL_NODE_ID ||
							neighbor_infos_[parent_index_].distance + 1 == distance()
					);

					assert(
							(distance() == 0) ==
							is_root()
					);

					// Parent can be NULL_NODE_ID if either we are not connected
					// to the tree yet (distance == npos), or we are the root.
					assert(
							(parent() == NULL_NODE_ID) ==
							(distance() == (distance_t)(-1) || is_root())
					);


				#endif
			}

			void notify_receivers() {
				for(typename Callbacks::iterator iter = callbacks_.begin(); iter != callbacks_.end(); ++iter) {
					(*iter)();
				}
			}

			/**
			 * Insert neighbor n or update if already in the collection.
			 */
			void insert_neighbor(node_id_t n, node_id_t p, distance_t d) {
				check();

				for(typename NeighborInfos::iterator it = neighbor_infos_.begin(); it != neighbor_infos_.end(); ++it) {
					size_type pos = (it - neighbor_infos_.begin());
					if(it->id == n) {
						// Neighbor with that node id is already known,
						// just update
						it->parent = p;
						it->distance = d;

						check();
						return;
					}
					else if(it->id > n) {
						// We found the place to insert.
						#if !defined(NDEBUG)
							node_id_t parent_before = parent();
						#endif
						if(neighbor_infos_.full()) {
							// Neighborhood is full
							// The reasonable choice
							// for a neighbor to remove is non-trivial:
							// The view has to be consistent in the network,
							// dont remove a good parent or childs that have
							// no other way of connecting, etc...
							//
							// For now, just complain forever
							assert(false);
							while(true) { debug_->debug("!NF"); }
							//neighbor_infos_.pop_back();
						}
						neighbor_infos_.insert(it, NeighborInfo(n, p, d));
						if(parent_index_ != npos && pos <= parent_index_) {
							parent_index_++;
						}
						#if !defined(NDEBUG)
							assert(parent() == parent_before);
						#endif
						
						check();
						return;
					}
				}

				if(neighbor_infos_.full()) {
					assert(false);
					while(true) { debug_->debug("!NF"); }
				}
				neighbor_infos_.push_back(NeighborInfo(n, p, d));

				check();
			}

			void erase_neighbor(node_id_t c) {
				check();

				typename NeighborInfos::iterator it = neighbor_infos_.find(NeighborInfo(c));
				if(it == neighbor_infos_.end()) { return; }

				size_type pos = (it - neighbor_infos_.begin());
				if(parent() != NULL_NODE_ID) {
					if(parent_index_ != npos && pos < parent_index_) {
						parent_index_--;
					}
					else if(pos == parent_index_) {
						parent_index_ = npos;
						distance_ = (distance_t)(-1);
					}
				}
				neighbor_infos_.erase(it);

				check();

				//update_state();

				//check();
			}

			bool at_parent(typename NeighborInfos::iterator it) {
				return (it - neighbor_infos_.begin()) == (int)parent_index_;
			}

			void update_state(bool force=false) {
				if(!force) {
					// we force the update on initialization,
					// during that phase actually some class invariants
					// might not yet hold
					//check();
				}

				#if SS_MBF_TREE_DEBUG_VERBOSE
					debug_->debug("mbf st");
				#endif

				node_id_t parent_old = parent();
				distance_t distance_old = distance();

				// clear topology
				parent_index_ = npos;
				distance_ = (distance_t)(-1);
				for(typename NeighborInfos::iterator it = neighbor_infos_.begin(); it != neighbor_infos_.end(); ++it) {
					it->child = false;
				}

				for(typename NeighborInfos::iterator it = neighbor_infos_.begin(); it != neighbor_infos_.end(); ++it) {
					#if SS_MBF_TREE_DEBUG_VERBOSE
						debug_->debug("mbf neigh%lu d%d p%lu", (unsigned long)it->id, (int)it->distance, (unsigned long)it->parent);
					#endif

					// Node chose us as parent
					if(it->parent == id()) {
						#if SS_MBF_TREE_DEBUG_VERBOSE
							debug_->debug(" mbf c");
						#endif
						it->child = true;
						continue;
					}

					// Ignore nodes that didnt agree on a parent yet
					if(it->distance == (distance_t)(-1)) { continue; }


					if((int)it->distance + 1 > (int)MAX_ROOT_DISTANCE) {
						continue;
					}

					if((it->distance + 1 < distance_) ||
							((it->distance + 1 == distance_) && (
								it->id < parent() //|| parent() == NULL_NODE_ID)
							))
					) {
						#if SS_MBF_TREE_DEBUG_VERBOSE
							debug_->debug(" mbf p");
						#endif

						parent_index_ = (it - neighbor_infos_.begin());
						assert(&neighbor_infos_[parent_index_] == &*it);
						distance_ = it->distance + 1;
					}
				} // for

				if(is_root()) {
					#if SS_MBF_TREE_DEBUG_VERBOSE
						debug_->debug("mbf r");
					#endif
					distance_ = 0;
					parent_index_ = npos;
				}

				if((parent() != parent_old) || (distance() != distance_old) || force) {
					#if SS_MBF_TREE_DEBUG
					//if(!force) {
						debug_->debug("MBF %lu p%lu,%lu d%d,%d", (unsigned long)id(), (unsigned long)parent_old, (unsigned long)parent(), (int)distance_old,  (int)distance());
					//}
					#endif

					// state has actually changed
					TreeMessageT msg;
					msg.set_parent(parent());
					msg.set_distance(distance());
					neighborhood().set_payload(PAYLOAD_ID, msg.data(), msg.size());
					neighborhood().force_beacon();

					notify_receivers();
				}

				check();
			}

			void on_neighborhood_event(::uint8_t event, node_id_t from, ::uint8_t size, ::uint8_t *data, ::uint32_t _) {
				//debug_->debug("@%lu NDev %d %lu", (unsigned long)id(),
						//(int)event, (unsigned long)from);
				switch(event) {
					case Neighborhood::DROPPED_NB:
					case Neighborhood::LOST_NB_BIDI:
						//debug_->debug("MBF--");
						erase_neighbor(from);
						update_state();
						notify_receivers();
						break;

					case Neighborhood::NEW_NB_BIDI:
						break;

					case Neighborhood::NEW_PAYLOAD_BIDI: {
						//debug_->debug("MBFpp");
						//debug_->debug("MBF:recv_payload %lu from %lu", (unsigned long)neighborhood_->radio().id(), (unsigned long)from);
						TreeMessageT &msg = *reinterpret_cast<TreeMessageT*>(data);
						insert_neighbor(from, msg.parent(), msg.distance());
						update_state();
						notify_receivers();
						break;
					}

					default:
						debug_->debug("!NDev%d", (int)event);
						break;
				}

				check();
			}

			typename Debug::self_pointer_t debug_;
			typename Neighborhood::self_pointer_t neighborhood_;
			node_id_t root_;
			node_id_t id_;
			distance_t distance_;
			size_type parent_index_;
			NeighborInfos neighbor_infos_;
			Callbacks callbacks_;
		
	}; // SsMbfTree
}

#endif // SS_MBF_TREE_H

