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

#define SS_MBF_TREE_DEBUG 1

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

			enum {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				ROOT_NODE_ID = 1234,
			};

			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { npos = (size_type)(-1) };
			enum { CHILD, PARENT, UNRELATED };
			enum { MAX_NEIGHBORS = 10 }; // TODO: Align with nhood
			enum { PAYLOAD_ID = 2 };

		private:
			struct NeighborInfo {
				node_id_t id;
				node_id_t parent;
				distance_t distance;
				bool child;

				NeighborInfo() {
				}

				NeighborInfo(node_id_t i) : id(i), parent(NULL_NODE_ID), distance(-1), child(0) {
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

				neighborhood_->register_payload_space(PAYLOAD_ID);
				neighborhood_->template reg_event_callback<self_type, &self_type::on_nd_event>(
						PAYLOAD_ID,
						Neighborhood::NEW_NB_BIDI | Neighborhood::NEW_PAYLOAD_BIDI | Neighborhood::LOST_NB_BIDI | Neighborhood::DROPPED_NB,
						this);

				check();
				return SUCCESS;
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

					assert((distance_ == 0) <= (is_root() || (parent() == NULL_NODE_ID)));
					assert(is_root() == (parent() == id()));

				#endif
			}

			/**
			 * Insert neighbor n or update if already in the collection.
			 */
			void insert_neighbor(node_id_t n, node_id_t p, distance_t d) {
				for(typename NeighborInfos::iterator it = neighbor_infos_.begin(); it != neighbor_infos_.end(); ++it) {
					if(it->id == n) {
						it->parent = p;
						it->distance = d;
						return;
					}
					else if(it->id > n) {
						neighbor_infos_.insert(it, NeighborInfo(n, p, d));
						return;
					}
				}
				neighbor_infos_.push_back(NeighborInfo(n, p, d));
			}

			void erase_neighbor(node_id_t c) {
				check();

				typename NeighborInfos::iterator it = neighbor_infos_.find(NeighborInfo(c));
				if(it == neighbor_infos_.end()) { return; }

				size_type pos = (it - neighbor_infos_.begin());
				if(parent() != NULL_NODE_ID) {
					if(pos < parent_index_) {
						parent_index_--;
					}
					else if(pos == parent_index_) {
						parent_index_ = npos;
					}
				}
				neighbor_infos_.erase(it);

				check();
			}

			bool at_parent(typename NeighborInfos::iterator it) {
				return (it - neighbor_infos_.begin()) == parent_index_;
			}

			void update_state() {
				check();

				node_id_t parent_old = parent();
				distance_t distance_old = distance();

				// clear topology
				parent_index_ = npos;
				for(typename NeighborInfos::iterator it = neighbor_infos_.begin(); it != neighbor_infos_.end(); ++it) {
					it->child = false;
				}

				for(typename NeighborInfos::iterator it = neighbor_infos_.begin(); it != neighbor_infos_.end(); ++it) {
					// Ignore nodes that didnt agree on a parent yet
					if(it->distance == (distance_t)(-1)) { continue; }

					// Node chose us as parent
					if(it->parent == id()) {
						it->child = true;
						continue;
					}

					if((it->distance + 1 < distance_) ||
							((it->distance + 1 == distance_) && ((it->id < parent()) || parent() == id() || parent() == NULL_NODE_ID))) {
						parent_index_ = (it - neighbor_infos_.begin());
						distance_ = it->distance + 1;
					}

				} // for

				if(is_root()) {
					distance_ = 0;
					parent_index_ = id();
				}

				if((parent() != parent_old) || (distance() != distance_old)) {
					#if SS_MBF_TREE_DEBUG
						debug_->debug("US p%d d%d", (int)parent(), (int)distance());
					#endif

					// state has actually changed
					TreeMessageT msg;
					msg.set_parent(parent());
					msg.set_distance(distance());
					neighborhood().set_payload(PAYLOAD_ID, msg.data(), msg.size());
				}

				check();
			}

			void on_nd_event(::uint8_t event, node_id_t from, ::uint8_t size, ::uint8_t *data) {
				switch(event) {
					case Neighborhood::DROPPED_NB:
					case Neighborhood::LOST_NB_BIDI:
						erase_neighbor(from);
						update_state();
						break;

					case Neighborhood::NEW_PAYLOAD_BIDI:
					case Neighborhood::NEW_NB_BIDI: {
						TreeMessageT &msg = *reinterpret_cast<TreeMessageT*>(data);
						insert_neighbor(from, msg.parent(), msg.distance());
						update_state();
						break;
					}

					default:
						debug_->debug("!NDev%d", (int)event);
						break;
				}
			}

			typename Debug::self_pointer_t debug_;
			typename Neighborhood::self_pointer_t neighborhood_;
			node_id_t root_;
			node_id_t id_;
			distance_t distance_;
			size_type parent_index_;
			NeighborInfos neighbor_infos_;
		
	}; // SsMbfTree
}

#endif // SS_MBF_TREE_H

