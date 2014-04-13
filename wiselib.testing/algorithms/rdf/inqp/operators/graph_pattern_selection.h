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

#ifndef GRAPH_PATTERN_SELECTION_H
#define GRAPH_PATTERN_SELECTION_H

#include "../row.h"
#include "../projection_info.h"
#include "operator.h"
#include "../operator_descriptions/graph_pattern_selection_description.h"

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
	class GraphPatternSelection : public Operator<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Processor_P Processor;
			typedef typename Processor::TupleStoreT TupleStoreT;
			typedef typename Processor::RowT RowT;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef GraphPatternSelection<OsModel_P, Processor_P> self_type;
			typedef typename RowT::Value Value;
			
			//enum { MAX_STRING_LENGTH = 256 };
			enum { TS_SEMANTIC_COLUMNS = 3 };
			
			void init(GraphPatternSelectionDescription<OsModel, Processor> *gpsd, Query *query) {
				Base::init(reinterpret_cast<OperatorDescription<OsModel, Processor>* >(gpsd), query);
				for(size_type i = 0; i < 3; i++) {
					affected_[i] = gpsd->affects(i);
					if(affected_[i]) {
						values_[i] = gpsd->value(i);
					}
				}
			}
			
			void init(Query* query, uint8_t id, uint8_t parent_id, uint8_t parent_port, ProjectionInfo<OsModel> projection,
					bool affected0, bool affected1, bool affected2, Value value0, Value value1, Value value2) {
				Base::init(Base::Description::GRAPH_PATTERN_SELECTION, query, id, parent_id, parent_port, projection);
				
				affected_[0] = affected0;
				affected_[1] = affected1;
				affected_[2] = affected2;
				values_[0] = value0;
				values_[1] = value1;
				values_[2] = value2;
				
			}
			
			void execute(TupleStoreT& ts) {
				GET_OS.debug("gps x cols %d", (int)this->projection_info().columns());
				typedef typename TupleStoreT::TupleContainer Container;
				typedef typename Container::iterator Citer;
				
				RowT *row = RowT::create(this->projection_info().columns()); //TupleStoreT::COLUMNS);
				
				for(Citer iter = ts.container().begin(); iter != ts.container().end(); ++iter) {
					bool match = true;
					size_type row_idx = 0;
					for(size_type i = 0; i < TS_SEMANTIC_COLUMNS; i++) {
						typename Processor::Value v = this->translator().translate(iter->get_key(i));
						
						if(affected_[i]) {
							if(values_[i] != v) {
								match = false;
								break;
							}
						}
						
						switch(this->projection_info().type(i)) {
							case ProjectionInfoBase::IGNORE:
								break;
							case ProjectionInfoBase::INTEGER: {
								block_data_t *s = this->dictionary().get_value(iter->get_key(i));
								long l = atol((char*)s);
								(*row)[row_idx++] = *reinterpret_cast<Value*>(&l);
								this->dictionary().free_value(s);
								break;
							}
							case ProjectionInfoBase::FLOAT: {
								block_data_t *s = this->dictionary().get_value(iter->get_key(i));
								float f = atof((char*)s);
								(*row)[row_idx++] = *reinterpret_cast<Value*>(&f);
								this->dictionary().free_value(s);
								break;
							}
							case ProjectionInfoBase::STRING:
								(*row)[row_idx++] = v;
								this->reverse_translator().offer(iter->get_key(i), v);
								break;
						}
					}
					if(match) {
						this->parent().push(*row);
					}
				}
				
				row->destroy();
				this->parent().push(Base::END_OF_INPUT);
			}
			
		private:
			typename Processor::Value values_[3];
			bool affected_[3];
		
	}; // GraphPatternSelection
}

#endif // GRAPH_PATTERN_SELECTION_H

