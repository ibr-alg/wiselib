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

#ifndef TO_SET_WRITE_H
#define TO_SET_WRITE_H

#include <util/meta.h>

#include "set_erase_to_set_write.h"

namespace wiselib {
	
	namespace {
		
		HAS_METHOD(read, has_read);
		HAS_METHOD(set, has_set);
		HAS_METHOD(erase, has_erase);
		
		template<typename T>
		struct is_set_erase {
			static const bool value =
				has_read<T,  int (T::*)(typename T::block_data_t*, typename T::address_t)>::value &&
				has_set<T,   int (T::*)(typename T::block_data_t*, typename T::address_t)>::value &&
				has_erase<T, int (T::*)(typename T::erase_block_address_t)>::value;
		};
		
		template<bool IsSetErase, typename OsModel_P, typename Storage_P>
		struct select_implementation {
			//typedef WriteToSetWrite impl;
		};
		
		template<typename OsModel_P, typename Storage_P>
		struct select_implementation<true, OsModel_P, Storage_P> {
			typedef SetEraseToSetWrite<OsModel_P, Storage_P> impl;
		};
	}
	
	/**
	 * \brief
	 * 
	 * \ingroup
	 * 
	 * \tparam 
	 */
	template<
		typename OsModel_P,
		typename Storage_P
	>
	class ToSetWrite
			: public select_implementation<is_set_erase<Storage_P>::value, OsModel_P, Storage_P>::impl {
		public:
		
		private:
	}; // ToSetWrite
}

#undef IF_SET_ERASE
#undef IF_NOT_SET_ERASE

#endif // TO_SET_WRITE_H


