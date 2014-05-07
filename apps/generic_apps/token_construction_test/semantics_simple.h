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
		typename node_id_t,
		typename RandPtr
	>
	void initial_semantics(TS& ts, node_id_t id, RandPtr _) {
		static const char *table1 = "<table1>";
		//static const char *table2 = "<table2>";
		static const char *window1 = "<window1>";
		//static const char *door1 = "<door1>";
		//static const char *door2 = "<door2>";
		//static const char *door3 = "<door3>";
		//static const char *couch1 = "<couch1>";
		//static const char *chair1 = "<chair1>";
		//static const char *laptop1 = "<laptop1>";
		//static const char *cupboard1 = "<cupboard1>";
		//static const char *whiteboard1 = "<whiteboard1>";
		static const char *room1 = "<officeroom1>";
		
		enum { MAX_FOIS = 4 };
		
		static const char *all_fois[] = {
			table1, window1 /*,
			table2, door1,
			door2, door3, couch1, chair1,
			laptop1, cupboard1, whiteboard1 */
		};
			
		static const char *fois[][MAX_FOIS] = {
			{ 0, 0, 0, 0 },
			
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			
			// v3
			{ window1, 0, 0, 0 },
			{ window1, 0, 0, 0 },
			{ window1, 0, 0, 0 },
			{ window1, table1, 0, 0 },
			{ window1, 0, 0, 0 },
			{ 0, table1, 0, 0 },
			{ window1, 0, 0, 0 },
			{ window1, 0, 0, 0 },
			{ window1, 0, 0, 0 },
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			
			// v14
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			{ 0, table1, 0, 0 },
			{ window1, 0, 0, 0 },
			{ window1, 0, 0, 0 },
			{ window1, table1, 0, 0 },
			{ window1, table1, 0, 0 }
		};
		
		
		enum { MAX_URI_LENGTH = 256 };
		char myuri[MAX_URI_LENGTH];
		snprintf(myuri, MAX_URI_LENGTH, "<http://spitfire-project.eu/sensor/office1/v%d>", id);
		myuri[MAX_URI_LENGTH - 1] = '\0';
		
		
		ins(ts, myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", room1);
		
		if(id == 0) {
			// 0 = sink = global root = member of all SEs
			
			for(int idx = 0; idx < sizeof(all_fois) / sizeof(all_fois[0]); ++idx) {
				ins(ts, myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", all_fois[idx]);
			}
		}
		else {
			for(int idx = 0; idx < MAX_FOIS && fois[id][idx]; idx++) {
			   ins(ts, myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", fois[id][idx]);
			}
		}
	}
	
}

#endif // SEMANTICS_OFFICE1_H

