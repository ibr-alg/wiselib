
#include "app.h"

class App {
	public:
		
		size_t initcount ;
		
		void init(Os::AppMainParameter& value) {
			hardware_radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(value);
			
		#if APP_QUERY && USE_INQP
			uart_ = &wiselib::FacetProvider<Os, Os::Uart>::get_facet( value );
			uart_->enable_serial_comm();
			uart_->reg_read_callback<App, &App::uart_receive>(this);
			uart_query_pos = 0;
		#endif
			
			debug_->debug("INSE boot. max_neighbors=%d", INSE_MAX_NEIGHBORS);
			
			
			initcount = INSE_START_WAIT;
			//debug_->debug("\npre-boot @%lu t%lu\n", (unsigned long)hardware_radio_->id(), (unsigned long)now());
			timer_->set_timer<App, &App::init2>(1000, this, 0);
			
			monitor_.init(debug_);
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
			debug_->debug("bt");
			
			hardware_radio_->enable_radio();
			radio_.init(*hardware_radio_, *debug_);
			
			rand_->srand(radio_.id());
			monitor_.init(debug_);
			
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
			
			init_inqp();
			init_simple_rule_processor();
			
			// Fill node with initial semantics and construction rules
			
			initial_semantics(ts, &radio_, rand_);
				
			create_rules();
			rule_processor_.execute_all();
			
			monitor_.report("/init");
			
			#if defined(CONTIKI_TARGET_sky) && APP_BLINK
				//light_sensor
				//
				light_se.set(23, 42);
				
				//sensors_light_init();
				/*
				if(radio_.id() == token_construction_.tree().root()) {
					light_on = true;
					leds_on(LEDS_BLUE);
				}
				else {
					light_on = false;
					leds_off(LEDS_BLUE);
				}
				*/
				light_on = false;
				SENSORS_ACTIVATE(light_sensor);
				check_light(0);
			#endif // CONTIKI_TARGET_sky
		
		}
		
		char rdf_uri_[64];
		#if APP_QUERY && defined(ISENSE)
			char pir_uri_[64];
			IsensePirDataProvider<Os, TS, TC::SemanticEntityRegistryT, Aggregator> data_provider_;
		#endif
			
			
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
				
			#if USE_STRING_INQUIRY
				string_anycast_radio_.init(
						&token_construction_.semantic_entity_registry(),
						&token_construction_.neighborhood(),
						&radio_, timer_, debug_
				);
			#endif
				
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
				
			#if USE_STRING_INQUIRY
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
		
		template<typename TS>
		void ins(TS& ts, const char* s, const char* p, const char* o) {
			debug_->debug("ins(%s %s %s)", s, p, o);
			
			TupleT t;
			t.set(0, (block_data_t*)const_cast<char*>(s));
			t.set(1, (block_data_t*)const_cast<char*>(p));
			t.set(2, (block_data_t*)const_cast<char*>(o));
			ts.insert(t);
		}
		
		void insert_room(int room) {
			const char *foi = "<http://purl.oclc.org/NET/ssnx/ssn#featureOfInterest>";
			char room_uri[64];
			snprintf(room_uri, 64, "<http://spitfire-project.eu/rooms/room-100%d>", (int)room);
			ins(ts, rdf_uri_, foi, room_uri);
		}
		
		void insert_tuples() {
			
			const char *attachedSystem =  "<http://purl.oclc.org/NET/ssnx/ssn#attachedSystem>";
			const char *hasloc = "<http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#hasLocation>";
		
		#if HAS_PIR
			ins(ts, rdf_uri_, attachedSystem, pir_uri_);
		#endif
		#if HAS_ENV
			ins(ts, rdf_uri_, attachedSystem, light_uri_);
		#endif
			
			int room = 1;
			
			if(radio_.id() <= 0x1202) {
				insert_room(1);
				insert_room(2);
				insert_room(3);
				insert_room(4);
			
			}
			else if(radio_.id() <= 0x1204) { insert_room(2); }
			else if(radio_.id() <= 0x1206) { insert_room(3); }
			else if(radio_.id() <= 0x1208) { insert_room(4); }
			
			ins(ts, rdf_uri_, hasloc, "CTI Conference Room");
			ins(ts, rdf_uri_, "<http://www.w3.org/2003/01/geo/wgs84_pos#lat>", "38.2909");
			ins(ts, rdf_uri_, "<http://www.w3.org/2003/01/geo/wgs84_pos#long>", "21.796");
			
			/*
			int i = 0;
			for( ; tuples[i][0]; i++) {
				//monitor_.report("ins");
				
				//for(int j = 0; j < 3; j++) {
					//debug_->debug("%-60s %08lx", tuples[i][j], (unsigned long)Hash::hash((block_data_t*)tuples[i][j], strlen(tuples[i][j])));
				//}
				
				ins(ts, tuples[i][0], tuples[i][1], tuples[i][2]);
			}
			debug_->debug("ins done: %d tuples", (int)i);
			*/
		}
		
		abs_millis_t absolute_millis(const Os::Clock::time_t& t) { return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
		abs_millis_t now() { return absolute_millis(clock_->time()); }
		
		//
		// Actions to perform
		//
		
		bool light_on;
		unsigned light_val;
		
		#if defined(CONTIKI_TARGET_sky) && APP_BLINK
		void check_light(void*) {
			unsigned v = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
			
			light_val = (1.0 - LIGHT_ALPHA / 100.0) * light_val + (LIGHT_ALPHA / 100.0) * v;
			
			debug_->debug("light: %u<%u<%u", (unsigned)LIGHT_OFF, light_val, (unsigned)LIGHT_ON);
					//(unsigned)sensors_light1(),
					//(unsigned)sensors_light2()
					//(unsigned)light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC),
					//(unsigned)light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR)
			//);
			if(light_on && light_val < LIGHT_OFF && (radio_.id() != token_construction_.tree().root())) {
				debug_->debug("@%lu leave %u", (unsigned long)radio_.id(), light_val);
				// unregister se
				light_on = false;
				token_construction_.erase_entity(light_se);
			}
			else if(!light_on && (light_val > LIGHT_ON || (radio_.id() == token_construction_.tree().root()))) {
				debug_->debug("@%lu join %u", (unsigned long)radio_.id(), light_val);
				light_on = true;
				token_construction_.add_entity(light_se);
			}
			
			if(light_on) { leds_on(LEDS_GREEN); }
			else { leds_off(LEDS_GREEN); }
			
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
		#elif defined(ISENSE)
			IsenseMonitor<Os> monitor_;
		#else
			NullMonitor<Os> monitor_;
		#endif
		
		TC token_construction_;
		RuleProcessor rule_processor_;
		
		#if USE_STRING_INQUIRY
			StringAnycastRadio string_anycast_radio_;
			StringInquiryT string_inquiry_;
		#endif
			
		#if USE_DICTIONARY
			//Dictionary::key_type aggr_key_temp_;
			//Dictionary::key_type aggr_key_centigrade_;
		#endif
			
		#if USE_BLOCK_DICTIONARY || USE_BLOCK_CONTAINER
			BlockMemory block_memory_;
			BlockAllocator block_allocator_;
		#endif
};


wiselib::WiselibApplication<Os, App> app;
void application_main(Os::AppMainParameter& value) { app.init(value); }

