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

#ifndef SELECTION_DESCRIPTION_H
#define SELECTION_DESCRIPTION_H

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
	class SelectionDescription : public OperatorDescription<OsModel_P, Processor_P> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Processor_P Processor;
			typedef typename Processor::Value Value;
			typedef OperatorDescription<OsModel, Processor> Base;
			
			enum {
				OFFSET_COLUMNS = Base::OFFSET_BASE_END,
				OFFSET_VALUE_COUNT = OFFSET_COLUMNS + 1,
				OFFSET_CRITERIA = OFFSET_COLUMNS + 2
			};
			
			enum CriterionType {
				IGNORE = 0,
				
				EQ = 0x08,
				LT = 0x10,
				GT = 0x20,
				
				LEQ = EQ | LT, GEQ = EQ | GT, NEQ = LT | GT,
				
				AGAIN = 0x80, // 1 bit
					
				MASK_VALUE_INDEX = 0x07,
				MASK_CRITERION = 0x78,
			};
			
			size_type selection_columns() {
				return this->data_[OFFSET_COLUMNS];
			}
			
			::uint8_t *selection_criteria() {
				return this->data_ + OFFSET_CRITERIA;
			}
			
			::uint8_t selection_criterion(size_type i) {
				return this->data_[OFFSET_CRITERIA + i];
			}
			
			::uint8_t value_count() {
				return this->data_[OFFSET_VALUE_COUNT];
			}
			
			Value value(size_type i) {
				Value r;
				wiselib::read<OsModel, block_data_t, Value>(
						this->data_ + OFFSET_CRITERIA + selection_columns() + sizeof(Value) * i, r
				);
				return r;
			}
		
		private:
		
	}; // SelectionDescription
}

#endif // SELECTION_DESCRIPTION_H

