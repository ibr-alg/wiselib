
		void init(Os::AppMainParameter& amp) {
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);

			radio_->enable_radio();
			radio_->reg_recv_callback<App, &App::on_receive>(this);

			initialize_db();
		#if APP_DATABASE_DEBUG
			debug_->debug("db boot %lu", (unsigned long)radio_->id());
		#endif

			//block_data_t x;
			//radio_->send(Os::Radio::BROADCAST_ADDRESS, 1, &x);

		}

		block_data_t rdf_buffer_[1024];

		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;



		void on_receive(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data) {

			if(data[0] == 0x99) {
				::uint16_t pos = wiselib::read<Os, block_data_t, ::uint16_t>(data + 1);

			#if APP_DATABASE_DEBUG
				debug_->debug("recv %x %x %x p=%d", (int)data[0], (int)data[1], (int)data[2], (int)pos);
			#endif

				if(len == 3) {
					#if APP_DATABASE_DEBUG
						debug_->debug("recv end");
					#endif
					timer_->set_timer<App, &App::start_insert>(5000, this, 0);
					timer_->set_timer<App, &App::disable_radio>(1000, this, 0);
				}
				else {
					#if APP_DATABASE_DEBUG
						debug_->debug("recv l=%d", (int)len);
					#endif
					memcpy(rdf_buffer_ + pos, data + 3, len - 3);
				}

				block_data_t ack[] = { 0xAA, 0, 0 };
				wiselib::write<Os, block_data_t, ::uint16_t>(ack + 1, pos);
				radio_->send(from, 3, ack); 
			}
		}

		void disable_radio(void*) {
			radio_->disable_radio();
			#if defined(CONTIKI)
				NETSTACK_RDC.off(false);
			#endif
		}

		void enable_radio(void*) {
			#if defined(CONTIKI)
				NETSTACK_RDC.on();
			#endif
			radio_->enable_radio();
		}

		void start_insert(void*) {
			block_data_t *e = rdf_buffer_;

			while(*e) {
				if(*(e + 2) == 0 || *(e + 3) == 0) {
					// e does not point at a tuple but at the two-byte
					// checksum at the end of all tuples which is followed by
					// two 0-bytes ==> ergo, we are done.
					break;
				}

				char *s = (char*)e;
				char *p = s + strlen(s) + 1;
				char *o = p + strlen(p) + 1;
				e = (block_data_t*)o + strlen(o) + 1;

				insert_tuple(s, p, o);

			#if APP_DATABASE_DEBUG
				debug_->debug("ins done");
			#endif
			}

			timer_->set_timer<App, &App::enable_radio>(1000, this, 0);
		}
		

