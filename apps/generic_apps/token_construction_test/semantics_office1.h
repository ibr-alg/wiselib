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
		typename TS::Tuple t;
		t.set(0, (typename TS::block_data_t*)s);
		t.set(1, (typename TS::block_data_t*)p);
		t.set(2, (typename TS::block_data_t*)o);
		ts.insert(t);
	}
	
	
	template<
		typename TS,
		typename node_id_t
	>
	void initial_semantics(TS& ts, node_id_t id) {
		const char *table1 = "<table1>";
		const char *table2 = "<table2>";
		const char *window1 = "<window1>";
		const char *door1 = "<door1>";
		const char *door2 = "<door2>";
		const char *door3 = "<door3>";
		const char *couch1 = "<couch1>";
		const char *chair1 = "<chair1>";
		const char *laptop1 = "<laptop1>";
		const char *cupboard1 = "<cupboard1>";
		const char *whiteboard1 = "<whiteboard1>";
		const char *room1 = "<officeroom1>";
		
		enum { MAX_FOIS = 4 };
		
		static const char *fois[][MAX_FOIS] = {
			/*  0 */ { window1, table1, 0, 0 },
			/*  1 */ { window1, 0, 0, 0 },
			/*  2 */ { window1, 0, 0, 0 },
			/*  3 */ { window1, 0, 0, 0 },
			/*  4 */ { table1, laptop1, 0, 0 },
			/*  5 */ { table1, 0, 0, 0 },
			/*  6 */ { chair1, 0, 0, 0 },
			/*  7 */ { table2, 0, 0, 0 },
			/*  8 */ { table2, 0, 0, 0 },
			/*  9 */ { table2, 0, 0, 0 },
			/* 10 */ { table2, 0, 0, 0 },
			/* 11 */ { table2, 0, 0, 0 },
			/* 12 */ { table2, 0, 0, 0 },
			/* 13 */ { table2, 0, 0, 0 },
			/* 14 */ { table2, 0, 0, 0 },
			/* 15 */ { door1, 0, 0, 0 },
			/* 16 */ { door1, 0, 0, 0 },
			/* 17 */ { couch1, 0, 0, 0 },
			/* 18 */ { couch1, 0, 0, 0 },
			/* 19 */ { couch1, 0, 0, 0 },
			/* 20 */ { couch1, 0, 0, 0 },
			/* 21 */ { door2, 0, 0, 0 },
			/* 22 */ { door2, cupboard1, 0, 0 },
			/* 23 */ { cupboard1, 0, 0, 0 },
			/* 24 */ { cupboard1, 0, 0, 0 },
			/* 25 */ { cupboard1, door3, 0, 0 },
			/* 26 */ { whiteboard1, door3, 0, 0 },
			/* 27 */ { whiteboard1, 0, 0, 0 },
			/* 28 */ { whiteboard1, 0, 0, 0 },
		};
		
		enum { MAX_URI_LENGTH = 256 };
		char myuri[MAX_URI_LENGTH];
		snprintf(myuri, MAX_URI_LENGTH, "<http://spitfire-project.eu/sensor/office1/v%d>", id);
		myuri[MAX_URI_LENGTH - 1] = '\0';
		
		
		ins(ts, myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", room1);
		for(int idx = 0; idx < MAX_FOIS && fois[id][idx]; idx++) {
		   ins(ts, myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", fois[id][idx]);
		}
	}
	
}

#endif // SEMANTICS_OFFICE1_H

