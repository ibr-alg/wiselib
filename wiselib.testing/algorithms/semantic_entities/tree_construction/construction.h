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


#ifndef __WISELIB_ALGORITHMS_SE_CONSTRUCTION_H
#define __WISELIB_ALGORITHMS_SE_CONSTRUCTION_H

#include <util/pstl/map_static_vector.h>
#include <algorithms/crypto/hash/fnv.h>
#include <algorithms/neighbor_discovery/echo.h>

namespace wiselib {
	
	/**
	 * Semantic Entity Construction Process
	 */
	template<
		typename OsModel_P,
		typename Broker_P,
		typename Radio_P,
		typename Hash_P = Fnv32<OsModel_P>,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug,
		typename Neighborhood_P = Echo<OsModel_P, Radio_P, Timer_P, Debug_P>
	>
	class SEConstruction {
		public:
			//typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			static const Endianness endianness = OsModel::endianness;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			typedef Radio_P Radio;
			typedef Broker_P Broker;
			typedef Hash_P Hash;
			typedef Neighborhood_P Neighborhood;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef typename Hash::hash_t Feature;
			typedef typename Broker::TupleStore TupleStore; 
			typedef typename TupleStore::Tuple Tuple;
			typedef typename Broker::CompressedTupleStore CompressedTupleStore; 
			typedef typename Broker::bitmask_t bitmask_t;
			typedef typename Broker::column_mask_t column_mask_t;
			typedef typename Broker::iterator iterator;
			typedef typename Broker::compressed_iterator compressed_iterator;
			
			typedef SEConstruction<OsModel, Broker, Radio, Hash, Timer, Debug, Neighborhood> self_type;
			
			enum { MAX_FEATURES = 4 };
			
		private:
			class State {
				public:
					enum { nfeature = 0 };
					State() {
						memset(features_, 0, sizeof(features_));
					}
					size_t max_features() { return MAX_FEATURES; }
					Feature& feature(size_t i) { return features_[i]; }
					node_id_t& leader(size_t i) { return leaders_[i]; }
				
				protected:
					Feature features_[MAX_FEATURES];
					node_id_t leaders_[MAX_FEATURES];
			};
			
			class MyState : public State {
				public:
					node_id_t& parent(size_t i) { return parents_[i]; }
					
				private:
					node_id_t parents_[MAX_FEATURES];
			};
				
			
		public:
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum { PAYLOAD_ID = 1 };
			
			int init(
					char *myuri,
					bitmask_t docmask, ///< look for foisand insert partOfSE statements here
					
					typename Radio::self_pointer_t radio,
					typename Neighborhood::self_pointer_t neighborhood,
					typename Broker::self_pointer_t broker,
					typename Timer::self_pointer_t timer,
					typename Debug::self_pointer_t debug
			) {
				radio_ = radio;
				neighborhood_ = neighborhood;
				broker_ = broker;
				timer_ = timer;
				debug_ = debug;
				inses_changed_ = false;
				
				debug_->debug("INSE construction starting\n");
				
				feature_query_.set(myuri, "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>", 0, docmask);
				feature_query_mask_ = (1 << 0) | (1 << 1); // | (1 << 3);
				
				inse_query_.set(myuri, "<http://spitfire-project.eu/inse#partOfSE>", 0, docmask);
				inse_query_mask_ = (1 << 0) | (1 << 1); // | (1 << 3);
				
				update_rdf_features();
				for(int i=0; i<state_.max_features(); i++) {
					Feature myfeature = state_.feature(i);
					if(myfeature == state_.nfeature) { continue; }
					make_se(myfeature, radio_->id(), radio_->id());
				}
				
				neighborhood_->register_payload_space(PAYLOAD_ID);
				update_nd_payload();
				
				neighborhood_->template reg_event_callback
					<self_type, &self_type::on_nd_event>(
						PAYLOAD_ID,
						Neighborhood::NEW_NB_BIDI | Neighborhood::LOST_NB_BIDI | Neighborhood::DROPPED_NB | Neighborhood::NEW_NB_BIDI |
						Neighborhood::NEW_PAYLOAD | Neighborhood::NEW_PAYLOAD_BIDI,
						this
					);
				
				neighborhood_->enable();
				
				return SUCCESS;
			}
			
			//int init() {
				//return SUCCESS;
			//}
			
			void on_nd_event(uint8_t event_id, node_id_t neighbor, uint8_t payload_id, uint8_t *payload) {
				
				//debug_->debug("+++++++ ND EVENT %d pid=%d payload != 0: %d\n", (::int32_t)event_id, (::int32_t)payload_id, (::int32_t)(payload != 0));
				State neighbor_state; // = *(State*)payload;
				
				if(event_id & (Neighborhood::DROPPED_NB | Neighborhood::LOST_NB_BIDI)) {
					for(int i=0; i<state_.max_features(); i++) {
						Feature myfeature = state_.feature(i);
						if(myfeature == state_.nfeature) { continue; }
						debug_->debug("AD %s %llx 0 $$", get_feature(myfeature), state_.parent(i));
						make_se(myfeature, radio_->id(), radio_->id());
					}
					update_nd_payload();
				}
				
				if(event_id & (/*Neighborhood::NEW_NB_BIDI |*/ Neighborhood::NEW_PAYLOAD_BIDI | Neighborhood::NEW_PAYLOAD)) {
					if(payload) {
						//debug_packet(payload, sizeof(State));
						memcpy(&neighbor_state, payload, sizeof(State));
						new_neighbor_state(neighbor, neighbor_state);
					}
				}
			}
			
			void new_neighbor_state(node_id_t neighbor_address, State& neighbor_state) {
				//debug_->debug("+++++++ NEW ND STATE me=%llx neigh=%llx\n", radio_->id(), neighbor_address);
				node_id_t me = radio_->id();
				if(neighbor_address == me) { return; }
				
				// iterate over my features
				for(int i=0; i<state_.max_features(); i++) {
					Feature myfeature = state_.feature(i);
					if(myfeature == neighbor_state.nfeature) { continue; }
					
					node_id_t myleader = state_.leader(i);
					node_id_t myparent = state_.parent(i);
					
					// find according neighbor feature entry
					bool had_feature = false;
					for(int j=0; j<neighbor_state.max_features(); j++) {
						Feature feature = neighbor_state.feature(i);
						//debug_->debug("myfeature: %d '%s' neigh feature: %d '%s'\n", myfeature, get_feature(myfeature), feature, get_feature(feature));
						if(feature != myfeature) { continue; }
						node_id_t neighbor_leader = neighbor_state.leader(i);
						
						if(neighbor_leader > myleader) { // -> adopt neighs leader, make neigh parent
							//debug_->debug("case 1\n");
							make_se(myfeature, neighbor_leader, neighbor_address);
						}
						else if(neighbor_address > myleader && neighbor_address > me) { // -> neigh is leader, make neigh parent
							//debug_->debug("case 2x\n");
							make_se(myfeature, neighbor_address, neighbor_address);
						}
						else if(neighbor_address == myparent) { // parent still has the feature but now lower leader -> become leader
							//debug_->debug("case 2\n");
							if(neighbor_leader < me) {
								//debug_->debug("case 2a\n");
								make_se(myfeature, me, me);
							}
						}
						else { // some node with lower leader -> node is our child -> become leader
							//debug_->debug("case 3\n");
							if(myleader == Radio::NULL_NODE_ID) {
								//debug_->debug("case 3a\n");
								make_se(myfeature, me, me);
							}
						}
						had_feature = true;
						break;
					}
					
					if(neighbor_address == myparent && !had_feature) {
								//debug_->debug("case 4\n");
						// our former parent lost the feature, destroy se (for
						// now)
						//destroy_se(myfeature);
						make_se(myfeature, radio_->id(), radio_->id());
					}
				}
				
				update_nd_payload();
			}
			
			void make_se(Feature feature, node_id_t leader, node_id_t parent) {
				//debug_->debug("make se: %s %llx", get_feature(feature), leader);
				if(get_feature(feature)) {
					debug_->debug("ARR %s %llx %llx $$", get_feature(feature), radio_->id(), parent);
				}
				
				//debug_->debug("id=%04x f=%08lx %s-%04x\n", radio_->id(), feature, get_feature(feature), leader);
				
				int first_free = -1;
				int i=0;
				for( ; i<state_.max_features(); i++) {
					if((first_free == -1) && (state_.feature(i) == state_.nfeature)) {
						first_free = i;
					}
					else if(state_.feature(i) == feature) {
						if(state_.leader(i) != leader || state_.parent(i) != parent) {
							state_.leader(i) = leader;
							state_.parent(i) = parent;
							inses_changed_ = true;
						}
						break;
					}
				}
				if(i == state_.max_features() && first_free != -1) {
					if(state_.feature(first_free) != feature || state_.leader(first_free) != leader || state_.parent(first_free) != parent) {
						state_.feature(first_free) = feature;
						state_.leader(first_free) = leader;
						state_.parent(first_free) = parent;
						inses_changed_ = true;
					}
				}
				
				if(inses_changed_) {
					update_nd_payload();
					update_rdf_inses();
					inses_changed_ = false;
				}
			}
			
			//void destroy_se(Feature feature) {
				//debug_->debug("destroy se:");
				//debug_->debug("id=%llx f=%08lx %s\n", radio_->id(), feature, get_feature(feature));
				
				//for(int i=0; i<state_.max_features(); i++) {
					//if(state_.feature(i) == feature) {
						////state_.feature(i) = state_.nfeature;
						//state_.leader(i) = Radio::NULL_NODE_ID;
						//state_.parent(i) = Radio::NULL_NODE_ID;
						//break;
					//}
				//}
				
				//update_nd_payload();
				//update_rdf_inses();
			//}
			
			void update_nd_payload() {
				State *s = &state_;
				
				//debug_->debug("setting payload: sz=%d\n", sizeof(State));
				//for(int i=0; i<s->max_features(); i++) {
					//debug_->debug("%2d f=%x l=%x\n", i, s->feature(i), s->leader(i));
				//}
				//debug_packet(reinterpret_cast< ::uint8_t* >(s), sizeof(State));
				
				neighborhood_->set_payload(PAYLOAD_ID, reinterpret_cast< ::uint8_t* >(s), sizeof(State));
			}
			
			//void debug_packet(::uint8_t* packet, int l) {
				//debug_->debug("packet len=%d\n", l);
				//for(int i=0; i<l; i += 4) {
					//debug_->debug("%02x %02x %02x %02x\n", packet[i], packet[i+1], packet[i+2], packet[i+3]);
				//}
			//}
			
			void update_rdf_features() {
				//node_id_t me = radio_->id();
				//debug_->debug("update_rdf_features()\n");
				//debug_ts();
				
				iterator iter = tuple_store().begin(&feature_query_, feature_query_mask_);
				for( ; iter != tuple_store().end(); ++iter) {
					block_data_t *s = iter->get(2);
					//debug_->debug("s=%s\n", (char*)s);
					Feature f = (Hash::hash(s) + 1) & 0xffffffff;
					
					//debug_->debug("(%s %s %s)\n", iter->get(0), iter->get(1), iter->get(2));
					//debug_->debug("(%s %s *)\n", feature_query_.get(0), feature_query_.get(1));
					//debug_->debug("-----> f=%08x\n", f);
					
					
					if(!feature_names_.contains(f)) {
						size_t l = strlen((char*)s) + 1;
						char *s2 = get_allocator().allocate_array<char>(l).raw();
						//debug_->debug("copying '%s' to %x\n", s, (::uint32_t)(void*)s2);
						memcpy(s2, s, l);
						feature_names_[f] = s2;
						for(int i=0; i<MAX_FEATURES; i++) {
							if(state_.feature(i) == state_.nfeature) {
								state_.feature(i) = f;
								state_.leader(i) = Radio::NULL_NODE_ID;
								state_.parent(i) = Radio::NULL_NODE_ID;
								break;
							}
						}
						
						//debug_->debug("%s has feature %s\n", uri(), s2);
						
					}	
				}
			}
			
			char* uri() { return (char*)feature_query_.get(0); }
			
			void debug_ts() {
				debug_->debug("debug_ts()\n");
				iterator iter = tuple_store().begin();
				for( ; iter != tuple_store().end(); ++iter) {
					bitmask_t mask;
					memcpy(&mask, iter->get(3), sizeof(mask));
					debug_->debug("(%s %s %s %d)",
							(char*)iter->get(0), (char*)iter->get(1), (char*)iter->get(2), mask);
				}
							
							
			}
			
			void update_rdf_inses() {
				static const int MAX_INSE_LEN = 200;
				char inse[MAX_INSE_LEN];
				
				// delete all INSE tuples
				
				//debug_->debug("erasing: (%s, %s, %s) docmask=%x qmask=%x\n", inse_query_.get(0), inse_query_.get(1), inse_query_.get(2),
						//(::int32_t)inse_query_.bitmask(), (::int32_t)inse_query_mask_);
				iterator iter = tuple_store().begin(&inse_query_, inse_query_mask_);
				while(iter != tuple_store().end()) {
					//debug_->debug("erase: (%s %s %s)\n", iter->get(0), iter->get(1), iter->get(2));
					iter = tuple_store().erase(iter);
				}
				
				// insert new INSE tuples
				
				Tuple t;
				t.set_bitmask(inse_query_.bitmask());
				t.set(0, inse_query_.get(0));
				t.set(1, inse_query_.get(1));
				for(int i=0; i<state_.max_features(); i++) {
					if(state_.feature(i) == state_.nfeature) { continue; }
					if(state_.leader(i) == Radio::NULL_NODE_ID) { continue; }
					
					char *feature_name = get_feature(state_.feature(i));
					if(!feature_name) {
						debug_->debug("warning: no feature name found for feature %08x, ignoring!\n", state_.feature(i));
						continue;
					}
					
					snprintf(inse, MAX_INSE_LEN, "%.*s-%04llx>", strlen(feature_name) - 1, feature_name, state_.leader(i));
					t.set(2, (block_data_t*)inse);
					
					debug_->debug("SE %llx %s $$", radio_->id(), inse);
							
					//debug_->debug("insert: (%s %s %s)\n", t.get(0), t.get(1), t.get(2));
					
					tuple_store().insert(t);
				}
				//debug_ts();
				//debug_->debug("end update_rdf_inses()\n");
			}
			
			char* get_feature(Feature f) { return feature_names_[f]; }
			
			CompressedTupleStore& compressed_tuple_store() { return broker_->compressed_tuple_store(); }
			TupleStore& tuple_store() { return broker_->tuple_store(); }
			bitmask_t document_mask() { return feature_query_.bitmask(); }
		
		private:
			
			Tuple feature_query_;
			Tuple inse_query_;
			column_mask_t feature_query_mask_;
			column_mask_t inse_query_mask_;
			MyState state_;
			
			typename Broker::self_pointer_t broker_;
			typename Radio::self_pointer_t radio_;
			typename Neighborhood::self_pointer_t neighborhood_;
			typename Debug::self_pointer_t debug_;
			typename Timer::self_pointer_t timer_;
			
			MapStaticVector<OsModel, Feature, char*, 3 * MAX_FEATURES> feature_names_;
			bool inses_changed_;
			
	}; // class SEConstruction
	
} // namespace

#endif // SE_CONSTRUCTION_H

