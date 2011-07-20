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

#ifndef __BASE_EXTENDED_DATA_H__
#define __BASE_EXTENDED_DATA_H__

namespace wiselib {

	template <typename OsModel_P>
	class BaseExtendedData
	{
	public:
		BaseExtendedData(){}

		uint16_t link_metric() const {
			return link_metric_;
		};

		void set_link_metric( uint16_t lm ) {
			link_metric_ = lm;
		};

	protected:
		uint16_t link_metric_;
	};

}

#endif
