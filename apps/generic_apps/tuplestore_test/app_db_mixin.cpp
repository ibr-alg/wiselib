
//#ifndef NTUPLES
	//#define NTUPLES 76
//#endif

		//node_id_t gateway_address;

		enum {
			HEARTBEAT_INTERVAL = 10000,
			START_INSERT_INTERVAL = 1000,
			DISABLE_RADIO_INTERVAL = 100,
			START_FIND_INTERVAL = 5000,
			ENABLE_RADIO_INTERVAL = 5000,

			FINDS_AT_ONCE = 10,
			START_ERASE_INTERVAL = 10000,
			ERASE_AFTER_PREPARE_INTERVAL = 100,
			PREPARE_AFTER_ERASE_INTERVAL = 500,
		};
			

		void init(Os::AppMainParameter& amp) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			//rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet(amp);

			radio_->enable_radio();
			receiving = true;
			radio_->reg_recv_callback<App, &App::on_receive>(this);

			//initialize_db();
			first_receive = true;
			lastpos = 0;
			nextpos = 0;
			tuples = 0;
			
			//gateway_address = Radio::BROADCAST_ADDRESS;

		#if APP_DATABASE_DEBUG
			debug_->debug("db boot %lu", (unsigned long)radio_->id());
		#endif

		#if APP_HEARTBEAT
			timer_->set_timer<App, &App::heartbeat>(HEARTBEAT_INTERVAL, this, 0);
		#endif
		}

		void reboot() {
		#if APP_DATABASE_DEBUG
			debug_->debug("db soft reboot %lu", (unsigned long)radio_->id());
		#endif
			first_receive = true;
			lastpos = 0;
			nextpos = 0;
			tuples = 0;
		}

		#if APP_HEARTBEAT
			void heartbeat(void* v) {
				debug_->debug("<3 %lu E%d " DATABASE " " MODE " " DATASET, (unsigned long)v, (int)EXP_NR);
				timer_->set_timer<App, &App::heartbeat>(HEARTBEAT_INTERVAL, this, (void*)((unsigned long)v + 10));
			}
		#endif

		enum {
			MAX_ELEMENT_LENGTH = 160
		};

		block_data_t rdf_buffer_[1024];

		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		//Os::Rand::self_pointer_t rand_;

		bool receiving;
		::uint16_t lastpos;
		::uint16_t nextpos;
		::uint16_t tuples;

		bool first_receive;
		void on_receive(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data) {
			if(!receiving) { return; }

			if(data[0] == 0x99 && data[1] == (EXP_NR & 0xff)) {
				if(first_receive) {
					first_receive = false;
					#if APP_DATABASE_DEBUG
						debug_->debug("first rcv re-initing db");
					#endif
					initialize_db();
				}

				::uint16_t pos = wiselib::read<Os, block_data_t, ::uint16_t>(data + 2);
				//if(pos != 0 && pos <= lastpos) {
				if(pos != 0 && pos != nextpos) {
					#if APP_DATABASE_DEBUG
						debug_->debug("ignpos %d <= %d", (int)pos, (int)lastpos);
					#endif
					block_data_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
					wiselib::write<Os, block_data_t, ::uint16_t>(ack + 2, pos);
					radio_->send(from, 4, ack); 

					// only accept "later" writes, especially important
					// so we dont process the end marker twice (triggering
					// timers at the double, ouch!)
					return;
				}
				lastpos = pos;
				nextpos = pos + len - 4;

				//gateway_address = from;

			#if APP_DATABASE_DEBUG
				debug_->debug("(rcv %x %x %x p%d l%d)", (int)data[0], (int)data[1], (int)data[2], (int)pos, (int)len);
			#endif

				if(len == 4) {
					#if APP_DATABASE_DEBUG
						debug_->debug("recv end");
					#endif
					receiving = false;
					lastpos = 0;
					nextpos = 0;
					timer_->set_timer<App, &App::start_insert>(START_INSERT_INTERVAL, this, 0);
					timer_->set_timer<App, &App::disable_radio>(DISABLE_RADIO_INTERVAL, this, 0);
				}
				else {
					#if APP_DATABASE_DEBUG
						debug_->debug("recv l=%d", (int)len);
					#endif
					memcpy(rdf_buffer_ + pos, data + 4, len - 4);
				}

				block_data_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
				wiselib::write<Os, block_data_t, ::uint16_t>(ack + 2, pos);
				radio_->send(from, 4, ack); 
			}
			else if(data[0] == 0xbb && data[1] == (EXP_NR & 0xff)) {
				#if APP_DATABASE_DEBUG
					debug_->debug("reboot!");
				#endif
				//gateway_address = from;
				// REBOOT command
				reboot();
			}
			else {
				#if APP_DATABASE_DEBUG
					debug_->debug("ign %02x %02x %02x %02x expnr %02x",
						(int)data[0], (int)data[1], (int)data[2], (int)data[3],
						(int)EXP_NR);
				#endif
			}
		}


		void disable_radio(void*) {
			#if APP_DATABASE_DEBUG
				debug_->debug("radio off");
			#endif
			radio_->disable_radio();
			#if defined(CONTIKI)
				NETSTACK_RDC.off(false);
			#endif
		}

		void enable_radio(void*) {
			#if APP_DATABASE_DEBUG
				debug_->debug("radio on");
			#endif
			#if defined(CONTIKI)
				NETSTACK_RDC.on();
			#endif
			radio_->enable_radio();
			receiving = true;
		}

		void start_insert(void*) {
			#if APP_DATABASE_DEBUG
				debug_->debug("<SI>");
			#endif
			block_data_t *e = rdf_buffer_;
			
			#if APP_DATABASE_FIND || MODE_FIND || MODE_ERASE
				RandomChoice choice(NTUPLES);
				bool chosen = false;
			#endif

			char *s = 0;
			char *p = 0;
			char *o = 0;
			while(*e) {
				if(*(e + 2) == 0 || *(e + 3) == 0) {
					// e does not point at a tuple but at the two-byte
					// checksum at the end of all tuples which is followed by
					// two 0-bytes ==> ergo, we are done.
					break;
				}

				s = (char*)e;
				p = s + strlen(s) + 1;
				o = p + strlen(p) + 1;
				e = (block_data_t*)o + strlen(o) + 1;

				insert_tuple(s, p, o);
				tuples++;

				#if APP_DATABASE_FIND || MODE_FIND || MODE_ERASE
					if(choice.choose() && !chosen) {
						strncpy((char*)find_s_, s, MAX_ELEMENT_LENGTH);
						strncpy((char*)find_p_, p, MAX_ELEMENT_LENGTH);
						strncpy((char*)find_o_, o, MAX_ELEMENT_LENGTH);
						chosen = true;
					}
					++choice;
				#endif
			}

			#if APP_DATABASE_FIND || MODE_FIND || MODE_ERASE
				if(!chosen && s) {
					strncpy((char*)find_s_, s, MAX_ELEMENT_LENGTH);
					strncpy((char*)find_p_, p, MAX_ELEMENT_LENGTH);
					strncpy((char*)find_o_, o, MAX_ELEMENT_LENGTH);
					chosen = true;
				}
			#endif

			#if APP_DATABASE_FIND || MODE_FIND
				timer_->set_timer<App, &App::start_find>(START_FIND_INTERVAL, this, 0);
			#elif MODE_ERASE
				if(tuples >= NTUPLES) {
					#if APP_DATABASE_DEBUG
						debug_->debug("%d >= %d tuples received, going to erase mode",
							(int)tuples, (int)NTUPLES);
					#endif
					timer_->set_timer<App, &App::start_prepare_erase>(START_ERASE_INTERVAL, this, 0);
				}
				else {
					timer_->set_timer<App, &App::enable_radio>(ENABLE_RADIO_INTERVAL, this, 0);
				}
			#else
				timer_->set_timer<App, &App::enable_radio>(ENABLE_RADIO_INTERVAL, this, 0);
			#endif
		}

		#if APP_DATABASE_FIND || MODE_FIND || MODE_ERASE
			block_data_t find_s_[MAX_ELEMENT_LENGTH];
			block_data_t find_p_[MAX_ELEMENT_LENGTH];
			block_data_t find_o_[MAX_ELEMENT_LENGTH];

			/**
			 * Select a tuple to find.
			 */
			#if 0
			void start_find_select(void*) {
				RandomChoice choice(/*rand_,*/ size());

				for(iter_rewind(); !iter_end(); iter_inc(), ++choice) {
					if(choice.choose()) {
						iter_get(find_s_, find_p_, find_o_);
						iter_free();

						timer_->set_timer<App, &App::start_find>(1000, this, 0);
						return;
					}
					++choice;
				}

				debug_->debug("!fnd %d %d", (int)choice.elements, (int)choice.current);
			}
			#endif

			char buf[MAX_ELEMENT_LENGTH];
			void start_find(void*) {

				for(int i = 0; i < FINDS_AT_ONCE; i++) {
					int x = 0; // rand_->operator()() % 3;
					block_data_t *s = find_s_;
					block_data_t *p = find_p_;
					block_data_t *o = find_o_;
		
					// find a random value for x such that
					// at least one of the lower 3 bits are set
					while(x == 0) {
						x = rand() & BIN(111);
					}

					if((x & BIN(001)) == 0) { s = 0; }
					if((x & BIN(010)) == 0) { p = 0; }
					if((x & BIN(100)) == 0) { o = 0; }

					find(s, p, o, buf);
				}
	
				timer_->set_timer<App, &App::enable_radio>(ENABLE_RADIO_INTERVAL, this, 0);
			}

			//#if APP_DATABASE_ERASE || MODE_ERASE
				//void start_find_erase(void*) {
					//find_erase(find_s_, find_p_, find_o_);
				//}
			//#endif

		#endif // APP_DATABASE_FIND

		#if MODE_ERASE || APP_DATABASE_ERASE
			void start_prepare_erase(void*) {
				prepare_erase(find_s_, find_p_, find_o_);
				timer_->set_timer<App, &App::start_erase>(ERASE_AFTER_PREPARE_INTERVAL, this, 0);
			}

			void start_erase(void*) {
				erase(find_s_, find_p_, find_o_);
				timer_->set_timer<App, &App::start_erase>(PREPARE_AFTER_ERASE_INTERVAL, this, 0);
			}

		#endif


		struct RandomChoice {
			//Os::Rand *rand;
			size_type elements;
			size_type current;

			RandomChoice(/*Os::Rand* r,*/ size_type e) : /*rand(r),*/ elements(e), current(0) {
			}

			//float p() {
				//return 1.0 / (float)(elements - current);
			//}

			bool choose() {
				long p = RAND_MAX / (elements - current);
				return rand() <= p;
			}

			void operator++() { current++; }
		};



