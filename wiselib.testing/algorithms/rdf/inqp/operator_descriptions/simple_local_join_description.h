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

#ifndef SIMPLE_LOCAL_JOIN_DESCRIPTION_H
#define SIMPLE_LOCAL_JOIN_DESCRIPTION_H

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
	class SimpleLocalJoinDescription : public OperatorDescription<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef OperatorDescription<OsModel_P, Processor_P> Base;
			
			enum {
			  OFFSET_COLUMNS = Base::OFFSET_BASE_END,
			};
			
			block_data_t left_column() {
				return this->data_[OFFSET_COLUMNS] >> 4;
			}
			
			block_data_t right_column() {
				return this->data_[OFFSET_COLUMNS] & 0xf;
			}
			
			size_type size() {
				return OperatorDescription<OsModel, Processor_P>::size() + 1;
			}
		
		private:
		
	}; // SimpleLocalJoinDescription
}

#endif // SIMPLE_LOCAL_JOIN_DESCRIPTION_H

