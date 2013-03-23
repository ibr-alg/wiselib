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

#ifndef GRAPH_PATTERN_SELECTION_DESCRIPTION_H
#define GRAPH_PATTERN_SELECTION_DESCRIPTION_H

#include <util/serialization/serialization.h>
#include "operator_description.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * GPSD gpsd;
	 * size_t vidx = 0;
	 * for(size_t c = 0; c < 3; c++) {
	 *   if(gpsd.affects(c)) {
	 *     printf("col %d must be equal to %d", c, gpsd.value(vidx));
	 *     vidx++;
	 *   }
	 * }
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Processor_P
	>
	class GraphPatternSelectionDescription : public OperatorDescription<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef OperatorDescription<OsModel_P, Processor_P> Base;
			//typedef typename Base::Value Value;
			typedef Processor_P Processor;
			typedef typename Processor::Value Value;;
			
			//typedef ... Dictionary;
			//typedef ... DictionaryKey;
			
			enum {
				OFFSET_AFFECTS = Base::OFFSET_BASE_END,
				OFFSET_VALUES = OFFSET_AFFECTS + 1,
			};
			
			bool affects(size_type column) {
				block_data_t &a = *(this->data_ + OFFSET_AFFECTS);
				return a & (1 << column);
			}
			
			size_type affected_columns() {
				// funktioniert bestimmt...
				
				return (size_type)affects(0) + (size_type)affects(1) + (size_type)affects(2);
			}
			
			Value value(size_type column) {
				// funktioniert bestimmt...
				
				block_data_t &affects = *(this->data_ + OFFSET_AFFECTS);
				if(!(affects & (1 << column))) {
					return 0;
				}
				
				block_data_t b = affects & ((1 << column) - 1);
				size_type offs = 0;
				while(b) {
					if(b & 0x01) { offs++; }
					b >>= 1;
				}
				
				//return *reinterpret_cast<Value*>(this->data_ + OFFSET_VALUES + offs * sizeof(Value));
				Value value;
				wiselib::read<OsModel, block_data_t, Value>(this->data_ + OFFSET_VALUES + offs * sizeof(Value), value);
				return value;
			}
			
			size_type size() {
				return OperatorDescription<OsModel, Processor>::size() + 1 + this->affected_columns() * sizeof(Value);
			}
		
		private:

		
	}; // GraphPatternSelectionDescription
}

#endif // GRAPH_PATTERN_SELECTION_DESCRIPTION_H

