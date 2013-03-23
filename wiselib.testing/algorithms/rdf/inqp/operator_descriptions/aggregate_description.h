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

#ifndef AGGREGATE_DESCRIPTION_H
#define AGGREGATE_DESCRIPTION_H

#include <util/serialization/serialization.h>
#include "operator_description.h"

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
	class AggregateDescription : public OperatorDescription<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef OperatorDescription<OsModel_P, Processor_P> Base;
			typedef Processor_P Processor;
			
			enum AggregationType { GROUP = 0, SUM = 1, AVG = 2, COUNT = 3, MIN = 4, MAX = 5, AGAIN = 0x80 };
			
			enum {
				OFFSET_COLUMNS = Base::OFFSET_BASE_END,
				OFFSET_AGGREGATION_METHODS = Base::OFFSET_BASE_END + 1
			};
			
			size_type aggregation_columns() {
				return this->data_[OFFSET_COLUMNS];
			}
			
			::uint8_t *aggregation_types() {
				return this->data_ + OFFSET_AGGREGATION_METHODS;
			}	
			
			::uint8_t aggregation_type(size_type i) {
				return this->data_[OFFSET_AGGREGATION_METHODS + i];
			}
		
		private:
		
	}; // AggregateDescription
}

#endif // AGGREGATE_DESCRIPTION_H

