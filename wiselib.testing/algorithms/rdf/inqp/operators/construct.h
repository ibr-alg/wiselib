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

#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include "operator.h"
#include "../row.h"
#include "../operator_descriptions/construct_description.h"
#include <util/types.h>
#include <util/string_util.h>

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
		typename Processor_P
	>
	class Construct : public Operator<OsModel_P, Processor_P> {
		public:
			typedef Construct self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Processor_P Processor;
			typedef typename Processor::Value Value;;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef typename Processor::TupleStoreT::Tuple TupleT;
			typedef typename Processor::TupleStoreT::Dictionary Dictionary;
			
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpmf-conversions"
			void init(ConstructDescription<OsModel, Processor> *cd, Query *query) {
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
				Base::init(Base::Description::CONSTRUCT, query, id, parent_id, parent_port, projection);
				
				affected_[0] = affected0;
				affected_[1] = affected1;
				affected_[2] = affected2;
				values_[0] = value0;
				values_[1] = value1;
				values_[2] = value2;
				hardcore_cast(this->push_, &self_type::push);
			}
			
			void push(size_type port, Row<OsModel>& row) {
				if(&row) {
					TupleT t;
					size_type j = 0;
					for(size_type i = 0; i < 3; i++) {
						DBG("i=%d j=%d aff=%d t=%d", (int)i, (int)j, (int)affected_[i],
								(int)this->child(Base::CHILD_LEFT).result_type(j));
						
						if(affected_[i]) {
							t.set_key(i, this->processor().reverse_translator().translate(values_[i]));
						}
						else {
							switch(this->child(Base::CHILD_LEFT).result_type(j)) {
								case ProjectionInfoBase::INTEGER: {
									char buffer[64];
									long v1 = *reinterpret_cast<long*>(&row[j]);
									ltoa(64, buffer, v1);
									typename Dictionary::key_type k = this->processor().tuple_store().dictionary().insert((block_data_t*)buffer);
									t.set_key(i, k);
									break;
								}
								case ProjectionInfoBase::FLOAT: {
									char buffer[64];
									float v1 = *reinterpret_cast<float*>(&row[j]);
									ftoa(64, buffer, v1, 6);
									typename Dictionary::key_type k = this->processor().tuple_store().dictionary().insert((block_data_t*)buffer);
									t.set_key(i, k);
									break;
								}
								case ProjectionInfoBase::STRING:
									t.set_key(i, this->processor().reverse_translator().translate(row[j]));
									break;
								default:
									assert(false);
							} // switch
							j++;
						}
					}
					this->processor().tuple_store().insert_raw(t);
				}
			}
			
			void execute() { }
		
		private:
			typename Processor::Value values_[3];
			bool affected_[3];
		
	}; // Construct
}

#endif // CONSTRUCT_H

