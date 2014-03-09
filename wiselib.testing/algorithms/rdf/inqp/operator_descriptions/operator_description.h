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

#ifndef OPERATOR_DESCRIPTION_H
#define OPERATOR_DESCRIPTION_H

#include "../projection_info.h"

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
	class OperatorDescription {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Processor_P Processor;
			typedef typename Processor::Value Value;
			
			typedef ::uint8_t operator_id_t;
			
			enum {
				GRAPH_PATTERN_SELECTION = 'g',
				SELECTION = 's',
				SIMPLE_LOCAL_JOIN = 'j',
				COLLECT = 'c',
				CONSTRUCTION_RULE = 'R',
				CONSTRUCT = 'C',
				AGGREGATE = 'a',
				DELETE = 'D',
			};
			
			enum {
				OFFSET_ID = 0,
				OFFSET_TYPE = 1,
				OFFSET_PARENT_ID = 2,
				OFFSET_PROJECTION_INFO = 3,
				OFFSET_BASE_END = OFFSET_PROJECTION_INFO + sizeof(ProjectionInfo<OsModel>)
			};
			
			int type() {
				return data_[OFFSET_TYPE];
			}
			
			size_type parent_id() {
				return data_[OFFSET_PARENT_ID] & 0x7f;
			}
			
			size_type parent_port() {
				return data_[OFFSET_PARENT_ID] >> 7;
			}
			
			ProjectionInfo<OsModel> projection_info() {
				ProjectionInfo<OsModel> inf;
				memcpy(&inf, &data_[OFFSET_PROJECTION_INFO], sizeof(ProjectionInfo<OsModel>));
				return inf;
			}
			
			size_type size() {
				return OFFSET_BASE_END;
			}
			
			operator_id_t id() { return data_[OFFSET_ID]; }
			
		protected:
			block_data_t data_[0];
		
	}; // OperatorDescription
}

#endif // OPERATOR_DESCRIPTION_H

