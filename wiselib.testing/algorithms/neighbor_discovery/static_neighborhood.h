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
#ifndef __STATIC_NEIGHBORHOOD_H__
#define __STATIC_NEIGHBORHOOD_H__

#include <external_interface/external_interface.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/vector_static.h>
#include <util/pstl/iterator.h>

namespace wiselib {

	/**
	 * @brief StaticNeighborhood implementation of @ref Neighborhood_concept "Neighborhood Concept".
	 *
	 * @ingroup Neighborhood_concept
	 * @ingroup basic_algorithm_concept
	 *
	 */
	template<
		typename OsModel_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Debug_P = typename OsModel_P::Debug
	>
	class StaticNeighborhood {
		public:
			typedef StaticNeighborhood self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef typename OsModel::block_data_t block_data_t;

			typedef Debug_P Debug;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;

			enum SpecialValues {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};

			class Neighbor {
				public:
					enum State {
						ANY = 0, IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE
					};

					Neighbor() : id_(NULL_NODE_ID), state_(ANY) { }
					Neighbor(node_id_t n, State s) : id_(n), state_(s) { }
					node_id_t id() { return id_; }
					State state() { return state_; }

					node_id_t id_;
					State state_;
			};

			enum {
				MAX_NEIGHBORS = 40
			};

			typedef vector_static<OsModel, Neighbor, MAX_NEIGHBORS> Neighbors;

		private:
			struct EdgeSelector {
				typename Neighbor::State s;
				EdgeSelector(typename Neighbor::State s = Neighbor::ANY) : s(s) { }
				bool operator()(Neighbor& n) { return (n.state() & s) == s; }
			};

		public:
			typedef filter<typename Neighbors::iterator, EdgeSelector> iterator;

			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC,
				ERR_NOMEM = OsModel::ERR_NOMEM,
				ERR_BUSY = OsModel::ERR_BUSY,
				ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
				ERR_NETDOWN = OsModel::ERR_NETDOWN,
				ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
			};

			StaticNeighborhood() : debug_(0) {
			}

			int init(Radio& radio, Debug& debug) {
				radio_ = &radio;
				debug_ = &debug;
				check();
				return SUCCESS;
			}

			int init() {
				return SUCCESS;
			}

			Debug& debug() { return *debug_; }

			iterator neighbors_begin() { return iterator(neighbors_.begin(), neighbors_.end(), EdgeSelector()); }
			iterator neighbors_begin(typename Neighbor::State s) { return iterator(neighbors_.begin(), neighbors_.end(), EdgeSelector(s)); }
			iterator neighbors_end() { return iterator(neighbors_.end(), neighbors_.end(), EdgeSelector()); }
			size_type neighbors_count(typename Neighbor::State s) { return distance(neighbors_begin(s), neighbors_end()); }

			void add_neighbor(node_id_t n, typename Neighbor::State s) {
				neighbors_.push_back(Neighbor(n, s));
			}

		private:
			void check() {
			}

			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			Neighbors neighbors_;

			EdgeSelector in_, out_;
	};

} // namespace wiselib

#endif // __STATIC_NEIGHBORHOOD_H__
