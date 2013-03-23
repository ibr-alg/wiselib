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

#ifndef SINK_JOIN_H
#define SINK_JOIN_H

#include <external_interface/external_interface.h>
#include "../row.h"
#include "../table.h"
#include "../projection_info.h"
#include "operator.h"
#include "../operator_descriptions/sink_join_description.h"
#include "../compare_values.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class SinkJoin : public Operator<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef Processor_P Processor;
			typedef typename Processor::CommState CommState;
			
			typedef Row<OsModel> RowT;
			typedef Table<OsModel, RowT> TableT;
			
			void init(SinkJoinDescription<OsModel, Processor> *sjd, CommState& communication_state) {
				Base::init(reinterpret_cast<OperatorDescription<OsModel, Processor>*>(sjd));
				this->push_ = reinterpret_cast<typename Base::my_push_t>(&self_type::push);
			}
			
			void push(size_type port, Row<OsModel>& row, uint8_t caller_id) {
				if(!&row) { return; }
				
				processor().send_intermediate_result(SINK, row, caller_id);
			}
			
			void on_row_received(...) {
				if(left) {
					put_row_into_table();
				}
				else {
					match_row_with_table();
					if(hit) {
						this->parent().push(row);
					}
				}
			}
			
		
		private:
		
	}; // SinkJoin
}

#endif // SINK_JOIN_H

