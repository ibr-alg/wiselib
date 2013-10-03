
#include "app.h"

#ifdef CONTIKI_TARGET_sky
	extern "C" {
		#include <dev/light-sensor.h>
	}
#endif

class App {
	public:
		
		size_t initcount ;
		
		void init(Os::AppMainParameter& value) {
			hardware_radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			initcount = INSE_START_WAIT;
			debug_->debug("\npre-boot @%lu t%lu\n", (unsigned long)hardware_radio_->id(), (unsigned long)now());
			timer_->set_timer<App, &App::init2>(1000, this, 0);
		}
		
		void init2(void*) {
			initcount--;
			if(initcount == 0) {
				init3();
			}
			else {
				timer_->set_timer<App, &App::init2>(1000, this, 0);
			}
		}
		
		void init3() {
			debug_->debug("\nboot @%lu t%lu\n", (unsigned long)hardware_radio_->id(), (unsigned long)now());
			
			hardware_radio_->enable_radio();
			radio_.init(*hardware_radio_, *debug_);
			
			
			rand_->srand(radio_.id());
			monitor_.init(debug_);
			monitor_.report("bt");
			
			// TupleStore
			
			init_blockmemory();
			init_ram();
			
			#if USE_DICTIONARY
				ts.init(&dictionary, &container, debug_);
			#else
				#warning "USING NO DICTIONARY FOR TS"
				ts.init(0, &container, debug_);
			#endif
			
			// Token Scheduler
			 
			token_construction_.init(&ts, &radio_, timer_, clock_, debug_, rand_);
			
			#if INSE_USE_AGGREGATOR
				token_construction_.set_end_activity_callback(
					TC::end_activity_callback_t::from_method<App, &App::on_end_activity>(this)
				);
			#endif
			token_construction_.disable_immediate_answer_mode();
			
			init_inqp();
			init_simple_rule_processor();
			
			// Fill node with initial semantics and construction rules
			
			#if !APP_BLINK
				initial_semantics(ts, &radio_, rand_);
				create_rules();
				rule_processor_.execute_all();
			#endif
			
			#if USE_DICTIONARY
				aggr_key_temp_ = dictionary.insert((::uint8_t*)"<http://spitfire-project.eu/property/Temperature>");
				aggr_key_centigrade_ = dictionary.insert((::uint8_t*)"<http://spitfire-project.eu/uom/Centigrade>");
			#endif
				
			//debug_->debug("hash(temp)=%lx", (long)STRHASH("<http://spitfire-project.eu/property/Temperature>"));
			//debug_->debug("hash(centigrade)=%lx", (long)STRHASH("<http://spitfire-project.eu/uom/Centigrade>"));
			
			//debug_->debug("@%d /init t=%d", (int)radio_->id(), (int)now());
			
			monitor_.report("/init");
			#if USE_INQP
				timer_->set_timer<App, &App::distribute_query>(500L * 1000L * WISELIB_TIME_FACTOR, this, 0);
			#endif
			//timer_->set_timer<App, &App::query_strings>(500000UL * WISELIB_TIME_FACTOR, this, 0);
			
			#if defined(CONTIKI_TARGET_sky) && APP_BLINK
				//light_sensor
				//
				light_se.set(23, 42);
				
				//sensors_light_init();
				light_on = false;
				SENSORS_ACTIVATE(light_sensor);
				check_light(0);
			#endif // CONTIKI_TARGET_sky
		}
		
		SemanticEntityId light_se;
		
		void init_blockmemory() {
			#if USE_BLOCK_DICTIONARY || USE_BLOCK_CONTAINER
				block_memory_.physical().init();
				block_memory_.init();
				block_allocator_.init(&block_memory_, debug_);
				
				#if USE_BLOCK_CONTAINER
					container.init(&block_allocator_, debug_);
				#endif
					
				#if USE_BLOCK_DICTIONARY
					dictionary.init(&block_allocator_, debug_);
				#endif
			#endif
		}
		
		void init_ram() {
			#if USE_PRESCILLA_DICTIONARY
				dictionary.init(debug_);
			#elif USE_TREE_DICTIONARY
				dictionary.init();
			#endif
		}
		
		void init_inqp() {
			#if USE_INQP
				query_processor_.init(&ts, timer_);
				rule_processor_.init(&query_processor_, &token_construction_);
				
				//query_communicator_.init(query_processor_,
				
				
				// Distribute queries to nodes
				
				distributor_.init(
						&radio_,
						&token_construction_.tree(),
						&token_construction_.nap_control(),
						&token_construction_.semantic_entity_registry(),
						&query_processor_,
						debug_, timer_, clock_, rand_
				);
				
				row_anycast_radio_.init(
						&token_construction_.semantic_entity_registry(),
						&token_construction_.neighborhood(),
						&radio_, timer_, debug_
				);
				
				string_anycast_radio_.init(
						&token_construction_.semantic_entity_registry(),
						&token_construction_.neighborhood(),
						&radio_, timer_, debug_
				);
				
				// Transport rows for collect and aggregation operators
				
				// --- packing on top of anycast
				row_radio_.init(row_anycast_radio_, *debug_, *timer_);
				
				// --- anycast on top of packing
				//static PackingOsRadio packing_radio_;
				//packing_radio_.init(*radio_, *debug_, *timer_);
				//row_radio_.init(
						//&token_construction_.semantic_entity_registry(),
						//&token_construction_.neighborhood(),
						//&packing_radio_, timer_, debug_
				//);
						
				
				debug_->debug("initing rowc");
				row_collector_.init(&row_radio_, &query_processor_, &token_construction_.tree(), debug_);
				row_collector_.reg_collect_callback(
						RowCollectorT::collect_delegate_t::from_method<App, &App::on_result_row>(this)
				);
				
				// Ask nodes for strings belonging to hashes
				
				#if USE_INQP
				string_inquiry_.init(&string_anycast_radio_, &query_processor_);
				string_inquiry_.reg_answer_callback(
						StringInquiryT::answer_delegate_t::from_method<App, &App::on_string_answer>(this)
				);
				#endif
				
				/*
				anycast_radio_.reg_recv_callback<
					ExampleApplication, &ExampleApplication::on_test_anycast_receive
				>(this);
				*/
			#endif
		}
		
		void init_simple_rule_processor() {
			#if !USE_INQP
				rule_processor_.init(&ts, &token_construction_);
			#endif
		}
		
	#if USE_INQP
		Query q1;
		Cons cons1;
		GPS gps1;
		
		void create_rules() {
			/*
			 * Add rule 1: (* <...#featureOfInterest> X)
			 */
			::uint8_t qid = 1;
			q1.init(&query_processor_, qid);
			q1.set_expected_operators(2);
			
			cons1.init(
					&q1, 2 /* opid */, 0 /* parent */, 0 /* parent port */,
					Projection((long)BIN(11))
					);
			q1.add_operator(&cons1);
			
			gps1.init(
					&q1, 1 /* op id */, 2 /* parent */, 0 /* parent port */,
					Projection((long)BIN(110000)),
					false, true, false,
					0, STRHASH("<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>"), 0
					);
			q1.add_operator<GPS>(&gps1);
			
			rule_processor_.add_rule(qid, &q1);
		}
	#else
		void create_rules() {
			/*
			 * Add rule 1: (* <...#featureOfInterest> X)
			 */
			::uint8_t qid = 1;
			const char *p = "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>";
			TupleT t;
			t.set(1, (block_data_t*)p);
			rule_processor_.add_rule(qid, t, BIN(010), 2);
		}
	#endif
		
		
		
		#if INSE_USE_AGGREGATOR
		typedef TC::SemanticEntityAggregatorT Aggregator;
		void on_end_activity(TC::SemanticEntityT& se, Aggregator& aggregator) {
			monitor_.report("/act");
			
			if(radio_->id() == token_construction_.tree().root()) {
			//if(radio_->id() == SINK_ID) {
				//debug_->debug("@%d aggr begin list BEFORE", (int)radio_->id());
				//for(Aggregator::iterator iter = aggregator.begin(); iter != aggregator.end(); ++iter) {
					//debug_->debug("@%d aggr BEFORE SE %lx.%lx type %8lx uom %8lx datatype %d => current n %2d %2d/%2d/%2d total n %2d %2d/%2d/%2d",
							//(int)radio_->id(), (int)se.id().rule(), (long)se.id().value(),
							//(long)iter->first.type_key(), (long)iter->first.uom_key(), (long)iter->first.datatype(),
							//(int)iter->second.count(), (int)iter->second.min(), (int)iter->second.max(), (int)iter->second.mean(),
							//(int)iter->second.total_count(), (int)iter->second.total_min(), (int)iter->second.total_max(), (int)iter->second.total_mean());
				//}
				
				//debug_->debug("@%d aggr end list BEFORE", (int)radio_->id());
				
				
				
				aggregator.set_totals(se.id());
				aggregator.aggregate(se.id(), aggr_key_temp_, aggr_key_centigrade_, (radio_->id() + 1) * 10, Aggregator::INTEGER);
				
				
				
				
				debug_->debug("@%d ag<", (int)radio_.id());
				for(Aggregator::iterator iter = aggregator.begin(); iter != aggregator.end(); ++iter) {
					debug_->debug("@%d ag S%lx.%lx dt%d C:%2d %2d/%2d/%2d T:%2d %2d/%2d/%2d",
							(int)radio_.id(), (long)se.id().rule(), (long)se.id().value(),
						//	(long)iter->first.type_key(), (long)iter->first.uom_key(),
							(long)iter->first.datatype(),
							(int)iter->second.count(), (int)iter->second.min(), (int)iter->second.max(), (int)iter->second.mean(),
							(int)iter->second.total_count(), (int)iter->second.total_min(), (int)iter->second.total_max(), (int)iter->second.total_mean());
				}
				debug_->debug("@%d ag>", (int)radio_.id());
				
			}
			else {
				aggregator.aggregate(se.id(), aggr_key_temp_, aggr_key_centigrade_, (radio_.id() + 1) * 10, Aggregator::INTEGER);
			}
			
		}
		#endif
		
		abs_millis_t absolute_millis(const Os::Clock::time_t& t) { return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
		abs_millis_t now() { return absolute_millis(clock_->time()); }
		
		//
		// Actions to perform
		//
		
		#if USE_INQP
		void distribute_query(void*) {
			if(radio_.id() != token_construction_.tree().root()) { return; }
			
			debug_->debug("@%d distributing query", (int)radio_.id());
			
			// temperature query from ../inqp_test/example_app.cpp,
			// each operator preceeded with one byte length information,
			// without message types and query id
			
			/*
			
			COLLECT(?v) WHERE {
			   ?sens <http://purl.oclc.org/NET/ssnx/ssn#observedProperty> <http://spitfire-project.eu/property/Temperature> .
			   ?sens <http://www.loa-cnr.it/ontologies/DUL.owl#hasValue> ?v .
			}

			<http://www.loa-cnr.it/ontologies/DUL.owl#hasValue>          4d0f60b4
			<http://spitfire-project.eu/property/Temperature>            b23860b3
			<http://purl.oclc.org/NET/ssnx/ssn#observedProperty>         bf26b82e
			
			*/
			
			
			/*
			// collect all temperature values
			char *query_temp = const_cast<char*>(
				"\x08\x04" "c\x00\x01\x00\x00\x00"
				"\x09\x03" "j\x04\x10\x00\x00\x00\x00"
				"\x0d\x02" "g\x83\x13\x00\x00\x00\x02\x4d\x0f\x60\xb4"
				"\x11\x01" "g\x03\x03\x00\x00\x00\x06\xbf\x26\xb8\x2e\xb2\x38\x60\xb3" );
			int opsize = 0x8 + 0x9 + 0xd + 0x11;
			int opcount = 4;
			*/
			
			// min, avg, max all temperature values
			// BIN(10101010) = 0xaa 
			// BIN(01010101) = 0x55
			char *query_temp = const_cast<char*>(
				"\x0c\x04" "a\x00\x55\x00\x00\x00\x03\x84\x82\x05"
				"\x09\x03" "j\x04\x10\x00\x00\x00\x00"
				"\x0d\x02" "g\x83\x13\x00\x00\x00\x02\x4d\x0f\x60\xb4"
				"\x11\x01" "g\x03\x03\x00\x00\x00\x06\xbf\x26\xb8\x2e\xb2\x38\x60\xb3" );
			int opsize = 0xc + 0x9 + 0xd + 0x11;
			int opcount = 4;
				
			
			distributor_.distribute(
					SemanticEntityId::all(),
					66 /* qid */,
					1 /* rev */,
					Distributor::QUERY,
					60000 * WISELIB_TIME_FACTOR, 60000 * WISELIB_TIME_FACTOR,
					opcount, opsize, (block_data_t*)query_temp);
		}
		#endif // USE_INQP
		
		
		#if USE_INQP
		void query_strings(void*) {
			// The SE from semantics_uniform
			string_inquiry_.inquire(SemanticEntityId::all(), 0xda00726c);
			
		}
		#endif
		
		
		//
		// Reactions to events in the network
		// 
		
		#if USE_INQP
		void on_string_answer(Value v, const char *s) {
			debug_->debug("resolv %08lx => %s", (unsigned long)v, s);
		}
		#endif
		
		#if USE_INQP
		void on_result_row(
				QueryProcessor::query_id_t query_id,
				QueryProcessor::operator_id_t operator_id,
				QueryProcessor::RowT& row
		) {
			QueryProcessor::Query *q = query_processor_.get_query(query_id);
			if(q == 0) {
				debug_->debug("@%d rowc x !q%d", (int)radio_.id(), (int)query_id);
				return;
			} // query not found
			QueryProcessor::BasicOperator *op = q->get_operator(operator_id);
			if(op == 0) {
				debug_->debug("@%d rowc x !o%d", (int)radio_.id(), (int)operator_id);
				return;
			} // operator not found
			
			debug_->debug("@%d R%c %02d %02d:", (int)radio_.id(), (char)op->type(), (int)query_id, (int)operator_id);
			int l = op->projection_info().columns();
			for(int i = 0; i < l; i++) {
				debug_->debug("@%d R%c  %d/%d %08lx", (int)radio_.id(), (char)op->type(), (int)i, (int)l, (unsigned long)row[i]);
			}
		} // on_result_row()
	
		#endif // USE_INQP
		
		
		bool light_on;
		unsigned light_val;
		
		#if defined(CONTIKI_TARGET_sky) && APP_BLINK
		void check_light(void*) {
			unsigned v = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
			
			light_val = (1.0 - LIGHT_ALPHA / 100.0) * light_val + (LIGHT_ALPHA / 100.0) * v;
			
			debug_->debug("light: %u %u",
					//(unsigned)sensors_light1(),
					//(unsigned)sensors_light2()
					(unsigned)light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC),
					(unsigned)light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR)
			);
			if(light_on && light_val < LIGHT_OFF) {
				debug_->debug("@%lu leave %u", (unsigned long)radio_.id(), light_val);
				// unregister se
				light_on = false;
				token_construction_.erase_entity(light_se);
			}
			else if(!light_on && light_val > LIGHT_ON) {
				debug_->debug("@%lu join %u", (unsigned long)radio_.id(), light_val);
				light_on = true;
				token_construction_.add_entity(light_se);
			}
						
			timer_->set_timer<App, &App::check_light>(CHECK_LIGHT_INTERVAL, this, 0);
		}
		#endif // CONTIKI_TARGET_sky
		
		
		
		//
		// Members
		//
		
		#if USE_DICTIONARY
			Dictionary dictionary;
		#endif
		TupleContainer container;
		TS ts;
		
		Os::Radio::self_pointer_t hardware_radio_;
		Radio radio_;
		
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;
		
		#if defined(ARDUINO)
			ArduinoMonitor<Os, Os::Debug> monitor_;
		#else
			NullMonitor<Os> monitor_;
		#endif
		
		TC token_construction_;
		RuleProcessor rule_processor_;
		
		#if USE_INQP
			QueryProcessor query_processor_;
			//INQPCommunicator<Os, QueryProcessor> query_communicator_;
			Distributor distributor_;
			RowAnycastRadio row_anycast_radio_;
			StringAnycastRadio string_anycast_radio_;
			RowRadio row_radio_;
			StringInquiryT string_inquiry_;
			RowCollectorT row_collector_;
		#endif
			
		#if USE_DICTIONARY
			Dictionary::key_type aggr_key_temp_;
			Dictionary::key_type aggr_key_centigrade_;
		#endif
			
		#if USE_BLOCK_DICTIONARY || USE_BLOCK_CONTAINER
			BlockMemory block_memory_;
			BlockAllocator block_allocator_;
		#endif
};


wiselib::WiselibApplication<Os, App> app;
void application_main(Os::AppMainParameter& value) { app.init(value); }

