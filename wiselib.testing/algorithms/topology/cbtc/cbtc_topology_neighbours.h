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
#ifndef __ALGORITHMS_TOPOLOGY_CBTC_TOPOLOGY_NEIGHBOURS_H__
#define __ALGORITHMS_TOPOLOGY_CBTC_TOPOLOGY_NEIGHBOURS_H__

#define DEBUG_CBTC_TOPOLOGY

#include "util/pstl/vector_static.h"

namespace wiselib
{
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	class CbtcTopologyNeighbours
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
#ifdef DEBUG_CBTC_TOPOLOGY
		typedef typename OsModel::Debug Debug;
#endif
	
		typedef typename OsModel_P::size_t size_type;

		typedef typename Radio::node_id_t node_id_t;
		
		typedef struct triplet_t{
			double angle;
			node_id_t id;
			int power;
			bool asymmetric;
		} TIPA_t;
		
		typedef struct ndp_struct{
			node_id_t id;
			int power;
			bool first_time_seen;
			uint8_t ndp_counter;
		} ndp_t;
		
		typedef vector_static<OsModel, TIPA_t, MAX_NODES> Nodes;
		Nodes N;
		typedef normal_iterator<OsModel, TIPA_t*, Nodes> Niter;
		
		//Asymmetric Nodes to be removed
		vector_static<OsModel, node_id_t, MAX_NODES> ATR;
		vector_static<OsModel, ndp_t, MAX_NODES> NDP;
		
		bool first_phase_done;
		
		CbtcTopologyNeighbours();
		
		inline void set_id(node_id_t id) {id_ = id; }
		
		inline size_type size(){ return N.size(); }
		inline TIPA_t& operator[](size_type n) { return N[n];}
		
		void add_update_neighbour(node_id_t id, int p, double angle, bool asymmetric);
		inline void delete_by_id(node_id_t id);
		inline void delete_by_index(size_type index);
		bool add_update_ndp(node_id_t id, int p, double angle);
		bool ndp_update();
		
		inline void delete_by_power(int p){
			size_type i;
			for ( i = 0; i < N.size(); ++i ){
				if(N[i].power == p){
					delete_by_index(i);
					i--;
				}
			}
		}
		
		inline void add_asymmetric_to_remove(node_id_t from){
			size_type i;
			for(i = 0; i < ATR.size(); i++)
				if(ATR[i] == from)
					break;
			
			if(i == ATR.size())
				ATR.push_back(from);
		}
		
		inline void copy_to_NDP();
		
#ifdef DEBUG_CBTC_TOPOLOGY
		inline void print_basic();
		inline void print_optimization(const char *s);
#endif
		
	private:
		node_id_t id_;
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	CbtcTopologyNeighbours()
	{
		first_phase_done = false;
	};
	

	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	void
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	add_update_neighbour(node_id_t id, int p, double angle, bool asymmetric){
		size_type i, j;
		
		if(first_phase_done){
			//Add it to NDP or update it if it's already there
			for ( i = 0; i < NDP.size(); ++i ){
				if(NDP[i].id == id) {
					NDP[i].first_time_seen = true;
					if(NDP[i].power > p) NDP[i].power = p;
				}
			}
			if(i == NDP.size()){
				ndp_t ndp;
				ndp.id = N[i].id;
				ndp.power = N[i].power;
				ndp.ndp_counter = 0;
				ndp.first_time_seen = true;
				NDP.push_back(ndp);
			}
		}
		
		
		//Remove from ATR if it's there and if now is symmetric
		if(!first_phase_done and !asymmetric){
			for ( i = 0; i < ATR.size(); ++i )
				if(ATR[i] == id)
					break;
			
			if(i < ATR.size()){
				for(; i < ATR.size() - 1; i++)
					ATR[i] = ATR[i+1];
			
				ATR.pop_back();
			}
		}
		
		//Update it in N
		//If we are in second phase just update don't add it to N if
		//it's not there
		for ( i = 0; i < N.size(); ++i ){
			if (N[i].id == id) {
				if (N[i].power >  p ) N[i].power = p;
				N[i].angle = angle;
				N[i].asymmetric = N[i].asymmetric && asymmetric;
				break;
			}
		}
		
		if(first_phase_done or i < N.size())
			return;
		
		//Add it to N (just if we are in first phase)
		TIPA_t t;
		t.id = id;
		t.power = p;
		t.angle = angle;
		t.asymmetric = asymmetric;
		
		for ( i = 0; i < N.size(); ++i ){
			if (angle < N[i].angle) {
				N.push_back(t);
				
				for(j = N.size() - 1; j > i; j--){
					N[j] = N[j-1];
				}
				N[i] = t;
				return;
			}
		}
		
		N.push_back(t);
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	inline void 
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	delete_by_id(node_id_t id){
		size_type i;
		
		for( i = 0; i < N.size(); i++) 
			if(N[i].id == id)
				break;
		
		for(; i < N.size() - 1; i++) 
			N[i] = N[i+1];
			
		N.pop_back();
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	inline void 
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	delete_by_index(size_type index){
		size_type i;
		
		for(i = index; i < N.size() - 1; i++) 
			N[i] = N[i+1];
			
		N.pop_back();
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	inline void 
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	copy_to_NDP(){
		size_type i;
		ndp_t ndp;
		for(i = 0; i < N.size(); i++){
			ndp.id = N[i].id;
			ndp.power = N[i].power;
			ndp.ndp_counter = 0;
			ndp.first_time_seen = true;
			NDP.push_back(ndp);
		}
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	bool
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	add_update_ndp(node_id_t id, int p, double angle){
		size_type i, j;
		ndp_t ndp;
		bool has_to_check_gap = false;
		
		//Update it in NDP
		for(i = 0; i < NDP.size(); i++){
			if(id == NDP[i].id){
				NDP[i].ndp_counter += 1;
				break;
			}
		}
		
		//If it's not in NDP add it
		if(i == NDP.size()){
			ndp.id = id;
			ndp.power = p;
			ndp.ndp_counter = 0;
			ndp.first_time_seen = true;
			NDP.push_back(ndp);
		}
		
		//Update it in N
		for(j = 0; j < N.size(); j++){
			if(id == N[j].id){
				if(N[j].angle != angle)
					has_to_check_gap = true;
				N[j].angle = angle;
				break;
			}
		}
		
		//If it's not in N add it sorted
		if(j == N.size()){
			TIPA_t t;
			t.id = id;
			t.power = NDP[i].power;
			t.angle = angle;
			t.asymmetric = false;

			Niter it;
			
			for(it = N.begin(); it != N.end(); it++){
				if(angle < (*it).angle){
					N.insert(it, t);
					break;
				}
			}
			if(it == N.end()){
				N.push_back(t);
			}
		}

		return has_to_check_gap;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	bool
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	ndp_update(){
		size_type i, j;
		bool check_for_gap = false;
		node_id_t id;
		
		for(i = 0; i < NDP.size(); i++){
			if(NDP[i].first_time_seen){
				NDP[i].first_time_seen = false;
				NDP[i].ndp_counter = 0;
			} else {
				if(NDP[i].ndp_counter) {
					NDP[i].ndp_counter = 0;
				} else {
					id = NDP[i].id;
					for(j = i; j < NDP.size() - 1; j++) {
						NDP[j] = NDP[j+1];
					}
					NDP.pop_back();
					for(j = 0; j < N.size(); j++){
						if(N[j].id == id) {
							check_for_gap = true;
							delete_by_index(j);
							break;
						}
					}
				}
			}
		}
		
		return check_for_gap;
	}


#ifdef DEBUG_CBTC_TOPOLOGY
	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	inline void 
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	print_basic(){
		size_type i;
		char nodes[MAX_NODES*7+1];
		int n = 0;

		for ( i = 0; i < N.size(); ++i )
			n += snprintf(nodes + n, 7, "%5i ", N[i].id );
		//Debug::debug( os(), "%i: Nodes[Firstphase]: %s\n", id_, nodes );
		
		n = 0;
		for ( i = 0; i < N.size(); ++i )
			n += sprintf(nodes + n, "%5.2f ", N[i].angle );
		//Debug::debug( os(), "%i: Nodes[Angles]:     %s\n", id_, nodes );
		
		n = 0;
		for ( i = 0; i < N.size(); ++i )
			n += sprintf(nodes + n, "%5i ", N[i].power );
		//Debug::debug( os(), "%i: Nodes[Powers]:     %s\n", id_, nodes );
		
		n = 0;
		for ( i = 0; i < N.size(); ++i )
			n += sprintf(nodes + n, "%s ", N[i].asymmetric? "  yes": "   no");
		//Debug::debug( os(), "%i: Nodes[asymmetry]:  %s\n", id_, nodes );
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P, typename Radio_P, uint16_t MAX_NODES>
	inline void 
	CbtcTopologyNeighbours<OsModel_P, Radio_P, MAX_NODES>::
	print_optimization(const char *s){
		size_type i;
		char nodes[MAX_NODES*7+1];
		int n = 0;
		
		for ( i = 0; i < N.size(); ++i )
			n += snprintf(nodes + n, 7, "%5i ", N[i].id );
		//Debug::debug( os(), "%i: %s %s\n", id_, s, nodes );
	}
#endif

}

#endif
