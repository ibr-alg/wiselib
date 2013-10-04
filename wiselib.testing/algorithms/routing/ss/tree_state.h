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

#ifndef TREE_STATE_H
#define TREE_STATE_H

#include <external_interface/external_interface.h>
#include <util/serialization/serialization.h>

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
	class TreeState {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef ::uint8_t distance_t;
			
			TreeState() : parent_(0), root_(-1), distance_(-1) {
			}
			
			node_id_t parent() const { return parent_; }
			bool set_parent(node_id_t p) {
				bool r = (p != parent_);
				parent_ = p;
				return r;
			}
			
			node_id_t root() const { return root_; }
			bool set_root(node_id_t roo) {
				bool r = (roo != root_);
				root_ = roo;
				return r;
			}
			
			distance_t distance() const { return distance_; }
			bool set_distance(distance_t s) {
				bool r = (s != distance_);
				distance_ = s;
				return r;
			}
			
			bool operator==(const TreeState& other) {
				bool r = /*(parent_ == other.parent_) &&*/ (root_ == other.root_) && (distance_ == other.distance_);
				if(!r) {
					//DBG("not same tree state parent %d vs %d root %d vs %d distance %d vs %d",
							//(int)parent_, (int)other.parent_,
							//(int)root_, (int)other.root_,
							//(int)distance_, (int)other.distance_);
				}
				return r;
			}
			
			bool operator!=(const TreeState& other) {
				return !(*this == other);
			}
			
		private:
			node_id_t parent_;
			node_id_t root_;
			distance_t distance_;
		
		template<
			typename OsModel_,
			Endianness Endianness_,
			typename BlockData_,
			typename Radio_
		>
		friend class Serialization;
		
	}; // TreeState
	
	/*
	 * iSense:
	 * 
serialization.h:113: error: ambiguous class template instantiation for 'struct Serialization<iSenseOsModel, WISELIB_BIG_ENDIAN, unsigned char, TreeState<> >'
serialization.h:66: error: candidates are: struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, Type_P>
tree_state.h:109: error:                 struct Serialization<OsModel_P, Endianness_P, BlockData_P, TreeState<> >
serialization.h:113: error: incomplete type 'Serialization<iSenseOsModel, WISELIB_BIG_ENDIAN, unsigned char, TreeState<> >' used in nested name specifier
	 * 
	 */ 
	#ifndef ISENSE
	template<
		typename OsModel_P,
		Endianness Endianness_P,
		typename BlockData_P,
		typename Radio_P
	>
	struct Serialization<OsModel_P, Endianness_P, BlockData_P, TreeState<OsModel_P, Radio_P> > {
		typedef OsModel_P OsModel;
		typedef BlockData_P block_data_t;
		typedef typename OsModel::size_t size_type;
		typedef TreeState<OsModel_P, Radio_P> TreeStateT;
		typedef Radio_P Radio;
		
		static size_type write(block_data_t *data, TreeStateT& value) {
			wiselib::write<OsModel>(data, value.parent_); data += sizeof(typename Radio::node_id_t);
			wiselib::write<OsModel>(data, value.root_); data += sizeof(typename Radio::node_id_t);
			wiselib::write<OsModel>(data, value.distance_); data += sizeof(typename TreeStateT::distance_t);
			return 2 * sizeof(typename Radio::node_id_t) + sizeof(typename TreeStateT::distance_t);
		}
		
		static TreeStateT read(block_data_t *data) {
			TreeStateT value;
			wiselib::read<OsModel>(data, value.parent_); data += sizeof(typename Radio::node_id_t);
			wiselib::read<OsModel>(data, value.root_); data += sizeof(typename Radio::node_id_t);
			wiselib::read<OsModel>(data, value.distance_); data += sizeof(typename TreeStateT::distance_t);
			return value;
		}
	};
	#endif
	/**/
}

#endif // TREE_STATE_H

