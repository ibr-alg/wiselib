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

#ifndef DELETE_H
#define DELETE_H

#include <util/pstl/set_vector.h>
#include <util/pstl/vector_dynamic.h>
#include "../operator_descriptions/delete_description.h"

namespace wiselib {
	
	/*
	 *     _____----_____                      ,
	 *   ,' ____\  /____ `.      \`.         /,
	 *  | ,' /   ~~  \  `. |      \ `-------'|
	 *  | | |         |  | |      |  DELETE! \
	 *  | | | .() (). |  | |      |       ___ \
	 *  | `-'         `-'  |      |/\  ,-'   `.\
	 *  `---.   ____   .--'      /   \|        `
	 *      \  |___|  /      ---'
	 *       \       /
	 *        `-._,-'
	 * 
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Processor_P
	>
	class Delete : public Operator<OsModel_P, Processor_P> {
		public:
			typedef Delete self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Processor_P Processor;
			typedef typename Processor::TupleStoreT::Tuple TupleT;
			typedef typename Processor::TupleStoreT TupleStoreT;
			typedef typename Processor::Value Value;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef set_vector<OsModel, vector_dynamic<OsModel, TupleT> > DeleteList;
			
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpmf-conversions"
			void init(DeleteDescription<OsModel, Processor> *cd, Query *query) {
				Base::init(reinterpret_cast<OperatorDescription<OsModel, Processor>* >(cd), query);
				
				for(size_type i = 0; i < 3; i++) {
					affected_[i] = cd->affects(i);
					if(affected_[i]) {
						values_[i] = cd->value(i);
					}
				}
				hardcore_cast(this->push_, &self_type::push);
			}
			#pragma GCC diagnostic pop
			
			void init(Query *query, ::uint8_t id, ::uint8_t parent_id, ::uint8_t parent_port, ProjectionInfo<OsModel> projection, bool affected0, bool affected1, bool affected2, Value value0, Value value1, Value value2) {
				Base::init(Base::Description::DELETE, query, id, parent_id, parent_port, projection);
				
				affected_[0] = affected0;
				affected_[1] = affected1;
				affected_[2] = affected2;
				values_[0] = value0;
				values_[1] = value1;
				values_[2] = value2;
				hardcore_cast(this->push_, &self_type::push);
				to_delete_.clear();
			}
			
			void push(size_type port, Row<OsModel>& row) {
				if(&row) {
					TupleT t;
					size_type j = 0;
					for(size_type i = 0; i < Processor::TupleStoreT::COLUMNS; i++) {
						
						if(affected_[i]) {
							t.set(i, (block_data_t*)this->processor().reverse_translator().translate(values_[i]));
						}
						else {
							t.set(i, (block_data_t*)this->processor().reverse_translator().translate(row[j++]));
						}
					}
					
					to_delete_.insert(t);
				}
				else {
					for(typename DeleteList::iterator iter = to_delete_.begin(); iter != to_delete_.end(); ++iter) {
						typename TupleStoreT::iterator it = this->processor().tuple_store().find_raw(*iter);
						if(it != this->processor().tuple_store().end()) {
							this->processor().tuple_store().erase(it);
						}
					} // for 
					to_delete_.clear();
				} // if(row)
			} // push()
			
			void execute() { }
		
		private:
			typename Processor::Value values_[3];
			bool affected_[3];
			DeleteList to_delete_;
		
	}; // Delete
}

#endif // DELETE_H

