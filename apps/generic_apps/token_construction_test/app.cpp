

#include "app.h"

class App {
	public:
		void init(Os::AppMainParameter value) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			
			debug_->debug("\nboot @%d t%d\n", (int)radio_->id(), (int)now());
			
			radio_->enable_radio();
			rand_->srand(radio_->id());
			monitor_.init(debug_);
			
			// TupleStore
			
			init_blockmemory();
			init_ram();
			
			#if USE_DICTIONARY
				ts.init(&dictionary, &container, debug_);
			#else
				ts.init(0, &container, debug_);
			#endif
			
			// Token Scheduler
			 
			token_construction_.init(&ts, radio_, timer_, clock_, debug_, rand_);
			/*
			token_construction_.set_end_activity_callback(
				TC::end_activity_callback_t::from_method<App, &App::on_end_activity>(this)
			);
			*/
			token_construction_.disable_immediate_answer_mode();
			
			init_inqp();
			init_simple_rule_processor();
			
			// Fill node with initial semantics and construction rules
			
			initial_semantics(ts, radio_->id());
			//create_rules();
			//rule_processor_.execute_all();
			
			
			//timer_->set_timer<App, &App::distribute_query>(500000000UL, this, 0);
		}
		
		void init_blockmemory() {
			#if USE_BLOCK_DICTIONIONARY || USE_BLOCK_CONTAINER
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
			#if USE_PRESCILLA
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
				
#if 0	
				distributor_.init(
						radio_,
						&token_construction_.tree(),
						&token_construction_.nap_control(),
						&token_construction_.semantic_entity_registry(),
						&query_processor_,
						debug_, timer_, clock_, rand_
				);
				
				anycast_radio_.init(
						&token_construction_.semantic_entity_registry(),
						&token_construction_.neighborhood(),
						radio_, timer_, debug_
				);
				
				// Transport rows for collect and aggregation operators
				packing_anycast_radio_.init(anycast_radio_, *debug_);
				
				debug_->debug("initing rowc");
				row_collector_.init(&packing_anycast_radio_, &query_processor_, debug_);
				row_collector_.reg_collect_callback(
						RowCollectorT::collect_delegate_t::from_method<App, &App::on_result_row>(this)
				);
				
				// Ask nodes for strings belonging to hashes
				
				string_inquiry_.init(&anycast_radio_, &query_processor_);
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
		Coll collect1;
		GPS gps1;
		
		void create_rules() {
			/*
			 * Add rule 1: (* <...#featureOfInterest> X)
			 */
			::uint8_t qid = 1;
			q1.init(&query_processor_, qid);
			q1.set_expected_operators(2);
			
			collect1.init(
					&q1, 2 /* opid */, 0 /* parent */, 0 /* parent port */,
					Projection((long)BIN(11))
					);
			q1.add_operator(&collect1);
			
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
		
		abs_millis_t absolute_millis(const typename Os::Clock::time_t& t) { return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
		abs_millis_t now() { return absolute_millis(clock_->time()); }
		
	
	

	//#if INSE_SINK
		
		//
		// Actions to perform
		//
		
		#if USE_INQP
		void distribute_query(void*) {
			debug_->debug("distributing query");
			
			// temperature query from ../inqp_test/example_app.cpp,
			// each operator preceeded with one byte length information,
			// without message types and query id
			char *query_temp =
				"\x08\x04" "c\x00\x01\x00\x00\x00"
				"\x09\x03" "j\x04\x10\x00\x00\x00\x00"
				"\x0d\x02" "g\x83\x13\x00\x00\x00\x02\x4d\x0f\x60\xb4"
				"\x11\x01" "g\x03\x03\x00\x00\x00\x06\xbf\x26\xb8\x2e\xb2\x38\x60\xb3";
			
			int opsize = 0x8 + 0x9 + 0xd + 0x11;
			int opcount = 4;
			
			distributor_.distribute(
					SemanticEntityId::all(),
					66 /* qid */,
					1 /* rev */,
					Distributor::QUERY,
					5000 * WISELIB_TIME_FACTOR, 5000 * WISELIB_TIME_FACTOR,
					opcount, opsize, (block_data_t*)query_temp);
		}
		#endif // USE_INQP
		
		//
		// Reactions to events in the network
		// 
		
		void on_string_answer(Value v, const char *s) {
			debug_->debug("resolv %08lx => %s", (unsigned long)v, s);
		}
		
		#if USE_INQP
		void on_result_row(
				QueryProcessor::query_id_t query_id,
				QueryProcessor::operator_id_t operator_id,
				QueryProcessor::RowT& row
		) {
			QueryProcessor::Query *q = query_processor_.get_query(query_id);
			if(q == 0) { return; } // query not found
			QueryProcessor::BasicOperator *op = q->get_operator(operator_id);
			if(op == 0) { return; } // operator not found
			
			debug_->debug("result %02d %02d:", (int)query_id, (int)operator_id);
			int l = op->projection_info().columns();
			for(int i = 0; i < l; i++) {
				debug_->debug("  %d/%d %08lx", (int)i, (int)l, (unsigned long)row[i]);
			}
		} // on_result_row()
	
		#endif // USE_INQP
	
	//#endif
		
		
		
		//
		// Members
		//
		
		#if USE_DICTIONARY
			Dictionary dictionary;
		#endif
		TupleContainer container;
		TS ts;
		
		Os::Radio::self_pointer_t radio_;
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
			AnycastRadio anycast_radio_;
			PackingAnycastRadio packing_anycast_radio_;
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

