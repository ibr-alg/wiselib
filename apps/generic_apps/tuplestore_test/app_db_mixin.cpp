
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
			nextpos = 0;
			tuples = 0;
			
			//gateway_address = Radio::BROADCAST_ADDRESS;

		#if APP_DATABASE_DEBUG
			debug_->debug("bt %lu", (unsigned long)radio_->id());
		#endif

		#if APP_HEARTBEAT
			timer_->set_timer<App, &App::heartbeat>(HEARTBEAT_INTERVAL, this, 0);
		#endif
		}

		void reboot() {
		#if APP_DATABASE_DEBUG
			debug_->debug("rb %lu", (unsigned long)radio_->id());
		#endif
			first_receive = true;
			nextpos = 0;
			tuples = 0;
		}

		#if APP_HEARTBEAT
			void heartbeat(void* v) {
				//debug_->debug("<3");
				debug_->debug("<3 %lu E%d " DATABASE " " MODE " " DATASET, (unsigned long)v, (int)EXP_NR);
				timer_->set_timer<App, &App::heartbeat>(HEARTBEAT_INTERVAL, this, (void*)((unsigned long)v + 10));
			}
		#endif

		enum {
			MAX_ELEMENT_LENGTH = 120 //160
		};

		block_data_t rdf_buffer_[1024];

		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		//Os::Rand::self_pointer_t rand_;

		bool receiving;
		::uint16_t nextpos;
		::uint16_t tuples;

		bool first_receive;
		void on_receive(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data) {
			#if APP_DATABASE_DEBUG
				debug_->debug("R %x %x l%d r%d", (int)data[0], (int)data[1], (int)len, (int)receiving);
			#endif
			

			if(!receiving) { return; }

			if(data[0] == 0x99 && data[1] == (EXP_NR & 0xff)) {
				if(first_receive) {
					first_receive = false;
					initialize_db();
				}

				::uint16_t pos = wiselib::read<Os, block_data_t, ::uint16_t>(data + 2);
				if(pos != 0 && pos != nextpos) {
					#if APP_DATABASE_DEBUG
						debug_->debug("ignP %d!=%d", (int)pos, (int)nextpos);
					#endif
					block_data_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
					wiselib::write<Os, block_data_t, ::uint16_t>(ack + 2, pos);
					radio_->send(from, 4, ack); 
					//radio_->send(from, 4, ack); 
					//radio_->send(from, 4, ack); 
					//radio_->send(from, 4, ack); 
					//radio_->send(from, 4, ack); 

					// only accept "later" writes, especially important
					// so we dont process the end marker twice (triggering
					// timers at the double, ouch!)
					return;
				}
				nextpos = pos + len - 4;

				//gateway_address = from;

			#if APP_DATABASE_DEBUG
				debug_->debug("rcv p%d l%d p'%d", (int)pos, (int)len, (int)nextpos);
			#endif

				if(len == 4) {
					#if APP_DATABASE_DEBUG
						debug_->debug("rcvE");
					#endif
					receiving = false;
					nextpos = 0;
					timer_->set_timer<App, &App::start_insert>(START_INSERT_INTERVAL, this, 0);
					timer_->set_timer<App, &App::disable_radio>(DISABLE_RADIO_INTERVAL, this, 0);
				}
				else {
					memcpy(rdf_buffer_ + pos, data + 4, len - 4);
				}

				block_data_t ack[] = { 0xAA, EXP_NR & 0xff, 0, 0 };
				wiselib::write<Os, block_data_t, ::uint16_t>(ack + 2, pos);
				radio_->send(from, 4, ack); 
			}
			else if(data[0] == 0xbb && data[1] == (EXP_NR & 0xff)) {
				#if APP_DATABASE_DEBUG
					debug_->debug("rb!");
				#endif
				reboot();
			}
			else {
				#if APP_DATABASE_DEBUG
					debug_->debug("ign %02x %02x %02x %02x E%02x",
						(int)data[0], (int)data[1], (int)data[2], (int)data[3],
						(int)EXP_NR);
				#endif
			}
		}


		void disable_radio(void*) {
			#if APP_DATABASE_DEBUG
				debug_->debug("/R");
			#endif
			radio_->disable_radio();
			#if defined(CONTIKI)
				int r = NETSTACK_RDC.off(false);
			#endif
			#if APP_DATABASE_DEBUG
				debug_->debug("/R->%d", r);
			#endif
		}

		void enable_radio(void*) {
			#if APP_DATABASE_DEBUG
				debug_->debug("R");
			#endif
			#if defined(CONTIKI)
				int r = NETSTACK_RDC.on();
			#endif
			#if APP_DATABASE_DEBUG
				debug_->debug("R->%d", r);
			#endif
			radio_->enable_radio();
			receiving = true;
		}

		void start_insert(void*) {
			#if APP_DATABASE_DEBUG
				debug_->debug("SI");
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
						//debug_->debug("chose: (%s,%s,%s)", s, p, o);
						strncpy((char*)find_s_, s, MAX_ELEMENT_LENGTH);
						strncpy((char*)find_p_, p, MAX_ELEMENT_LENGTH);
						strncpy((char*)find_o_, o, MAX_ELEMENT_LENGTH);
						//debug_->debug("after cp: (%s,%s,%s)", find_s_, find_p_, find_o_);
						chosen = true;
					}
					++choice;
				#endif
			}

			#if APP_DATABASE_FIND || MODE_FIND || MODE_ERASE
				if(!chosen && s) {
					//debug_->debug("Xchose: (%s,%s,%s)", s, p, o);
					strncpy((char*)find_s_, s, MAX_ELEMENT_LENGTH);
					strncpy((char*)find_p_, p, MAX_ELEMENT_LENGTH);
					strncpy((char*)find_o_, o, MAX_ELEMENT_LENGTH);
					//debug_->debug("Xafter cp: (%s,%s,%s)", find_s_, find_p_, find_o_);
					chosen = true;
				}
			#endif

			#if APP_DATABASE_FIND || MODE_FIND
				timer_->set_timer<App, &App::start_find>(START_FIND_INTERVAL, this, 0);
			#elif MODE_ERASE
				if(tuples >= NTUPLES) {
					#if APP_DATABASE_DEBUG
						debug_->debug("?e %d>=%d",
							(int)tuples, (int)NTUPLES);
					#endif
					timer_->set_timer<App, &App::start_prepare_erase>(START_ERASE_INTERVAL, this, 0);
				}
				else {
					#if APP_DATABASE_DEBUG
						debug_->debug("?e %d<%d",
							(int)tuples, (int)NTUPLES);
					#endif
					timer_->set_timer<App, &App::enable_radio>(ENABLE_RADIO_INTERVAL, this, 0);
				}
			#else
				timer_->set_timer<App, &App::enable_radio>(ENABLE_RADIO_INTERVAL, this, 0);
			#endif

			#if APP_DATABASE_DEBUG
				debug_->debug("/SI");
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

			//char buf[MAX_ELEMENT_LENGTH];
			void start_find(void*) {

				for(int i = 0; i < FINDS_AT_ONCE; i++) {
					int x = 0; // rand_->operator()() % 3;
					block_data_t *s = find_s_;
					block_data_t *p = find_p_;
					block_data_t *o = find_o_;

					//debug_->debug("start_find: (%s,%s,%s)", (char*)s, (char*)p, (char*)o);

					x = rand() % 3;

					if(x == 0) { s = 0; }
					else if(x == 1) { p = 0; }
					else { o = 0; }
					//debug_->debug("calling find(%s,%s,%s)", (char*)s, (char*)p, (char*)o);
					find(s, p, o, 0);
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
				#if APP_DATABASE_DEBUG
					debug_->debug("PE");
				#endif
				prepare_erase(find_s_, find_p_, find_o_);
				timer_->set_timer<App, &App::start_erase>(ERASE_AFTER_PREPARE_INTERVAL, this, 0);
			}

			void start_erase(void*) {
				#if APP_DATABASE_DEBUG
					debug_->debug("SE");
				#endif
				erase(find_s_, find_p_, find_o_);
				timer_->set_timer<App, &App::start_prepare_erase>(PREPARE_AFTER_ERASE_INTERVAL, this, 0);
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
				if(current >= elements) { return true; }
				long p = RAND_MAX / (elements - current);
				long r = rand();
				//printf("e=%lu c=%lu p=%lu r=%lu\n", (unsigned long)elements, (unsigned long)current, (unsigned long)p, (unsigned long)r);
				return r <= p;
			}

			void operator++() { current++; }
		};



