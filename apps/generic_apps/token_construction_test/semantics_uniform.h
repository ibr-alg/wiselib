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

#ifndef SEMANTICS_OFFICE1_H
#define SEMANTICS_OFFICE1_H

namespace wiselib {
	
	
	template<typename TS>
	void ins(TS& ts, const char* s, const char* p, const char* o) {
		DBG("ins(%s %s %s)", s, p, o);
		typename TS::Tuple t;
		t.set(0, (typename TS::block_data_t*)s);
		t.set(1, (typename TS::block_data_t*)p);
		t.set(2, (typename TS::block_data_t*)o);
		
		DBG("ins t (%s %s %s)", s, p, o);
		ts.insert(t);
	}
	
	
	template<
		typename TS,
		typename node_id_t
	>
	void initial_semantics(TS& ts, node_id_t id) {
		DBG("initial semantics");
		
		const char *room1 = "<http://spitfire-project.eu/rooms/officeroom1>";
		
		enum { MAX_URI_LENGTH = 256, DIGITPOS = 45 };
		char myuri[MAX_URI_LENGTH];
		
		snprintf(myuri, MAX_URI_LENGTH, "<http://spitfire-project.eu/sensor/office1/v%lx>", (unsigned long)id);
		myuri[MAX_URI_LENGTH - 1] = '\0';
		
		/*
		strncpy(myuri, "<http://spitfire-project.eu/sensor/office1/v", MAX_URI_LENGTH);
		int n = ltoa(MAX_URI_LENGTH - DIGITPOS - 1, myuri + DIGITPOS, id, 16);
		myuri[DIGITPOS + n] = '\0';
		*/
		
		ins(ts, myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", room1);
	}
	
}

#endif // SEMANTICS_OFFICE1_H

