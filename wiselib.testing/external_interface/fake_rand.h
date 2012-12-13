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

// vim: set noexpandtab ts=4 sw=4:

#ifndef FAKE_RAND_H
#define FAKE_RAND_H

namespace wiselib {
	
	template<typename OsModel_P>
	class FakeRandModel {
		public:
			typedef OsModel_P OsModel;
			typedef uint32_t value_t;
			
			typedef FakeRandModel<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum { RANDOM_MAX = 10 };
			
			enum States
			{
				READY = OsModel::READY,
				NO_VALUE = OsModel::NO_VALUE,
				INACTIVE = OsModel::INACTIVE
			};
			
			FakeRandModel();
			FakeRandModel(value_t seed);
			void srand(value_t seed);
			value_t operator()();
			value_t operator()(value_t max);
			int state();
			
		private:
	};
	
	template<typename OsModel_P>
	FakeRandModel<OsModel_P>::
	FakeRandModel() {
	}
	
	template<typename OsModel_P>
	FakeRandModel<OsModel_P>::
	FakeRandModel(value_t seed) {
	}
	
	template<typename OsModel_P>
	void FakeRandModel<OsModel_P>::
	srand(value_t seed) {
	}
	
	template<typename OsModel_P>
	typename FakeRandModel<OsModel_P>::value_t FakeRandModel<OsModel_P>::
	operator()() {
		return 4; // chosen by fair dice roll.
	}

	template<typename OsModel_P>
	typename FakeRandModel<OsModel_P>::value_t FakeRandModel<OsModel_P>::
	operator()( value_t max ) {
	        return ( operator()() % max );
	}	
	
	template<typename OsModel_P>
	int FakeRandModel<OsModel_P>::
	state() {
		return READY;
	}
	
} // ns wiselib

#endif // FAKE_RAND_H

