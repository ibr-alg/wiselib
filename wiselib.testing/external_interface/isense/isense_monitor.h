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

#ifndef ISENSE_MONITOR_H
#define ISENSE_MONITOR_H

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
		typename Debug_P = typename OsModel_P::Debug
	>
	class IsenseMonitor {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Debug_P Debug;
			
			int init(typename Debug::self_pointer_t debug) {
				debug_ = debug;
				return OsModel::SUCCESS;
			}
			
			void report() {
				debug_->debug("f: %d", free());
			}
			
			void report(const char *remark) {
				debug_->debug("f: %d %s", free(), remark);
			}
			
			static int free() {
				return mem->mem_free();
			}
		
		private:
			typename Debug::self_pointer_t debug_;
		
	}; // IsenseMonitor
}

#endif // ISENSE_MONITOR_H

