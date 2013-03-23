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

#ifndef PROJECTION_INFO_H
#define PROJECTION_INFO_H

namespace wiselib {
	
	class ProjectionInfoBase {
		public:
			enum TypeInfo {
				IGNORE = 0, INTEGER = 1, FLOAT = 2, STRING = 3
			};
	};
	
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		int COLUMNS_P = 16
	>
	class ProjectionInfo : public ProjectionInfoBase {
		public:
			enum { COLUMNS = COLUMNS_P };
			
		private:
			enum { COLUMN_BYTES = (COLUMNS + 3) / 4 };
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			/**
			 * Return the number of not-ignored columns.
			 */
			size_type columns() {
				size_type r = 0;
				for(size_type i = 0; i < COLUMN_BYTES; i++) {
					block_data_t b = columns_[i];
					for( ; b != 0; b >>= 2) {
						if((b & 0x3) != IGNORE) { r++; }
					}
				}
				return r;
			}
			
			int type(size_type col) {
				return (columns_[col / 4] >> ((col % 4) * 2)) & 0x3;
			}
			
			/*
			 * Return the type of given column as seen by the parent operator.
			 */
			int result_type(size_type col) {
				size_type j = 0;
				size_type i = -1;
				while(i != col) {
					while(type(j) == IGNORE) { j++; }
					i++;
					j++;
				}
				j--;
				return type(j);
			}
			
			// feststellung:
			// - wir brauchen wahrscheinlich nie mehr als 16 variablen,
			//   dementsprechend wird eine row nie breiter als 16 columns!
		
		private:
			
			block_data_t columns_[COLUMN_BYTES];
		
	}; // ProjectionInfo
}

#endif // PROJECTION_INFO_H

