
#include "platform.h"

using namespace wiselib;
#include <algorithms/protocols/reliable_transport/one_at_a_time_reliable_transport.h>
#include <util/debugging.h>
#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>

typedef wiselib::OSMODEL Os;
typedef Os::size_t size_type;
typedef Os::block_data_t block_data_t;

//typedef SemanticEntityId MyChannelId;
typedef int MyChannelId;

typedef OneAtATimeReliableTransport<Os, MyChannelId, 'R'> Transport;
typedef Os::Radio::node_id_t node_id_t;

		const char *payloads[] = {
			"ene", "mene", "miste",
			"es", "rappelt", "in", "der", "Kiste",
			"ene2", "mene2", "meck",
			"und", "du", "bist", "weg"
		};

class ExampleApplication {
	public:
		ExampleApplication() {
		}
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			rand_ = &wiselib::FacetProvider<Os, Os::Rand>::get_facet( value );
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
			
			//radio_->reg_recv_callback<ExampleApplication, &ExampleApplication::forward>(this);
			
			transport_.init(radio_, timer_, clock_, rand_, debug_, true);
			
			transport_.register_event_callback(
					Transport::callback_t::from_method<ExampleApplication, &ExampleApplication::on_event>(this)
			);
			
			state = 0;
			
			buffer_size_ = 0;
			if(radio_->id() == 0) {
				DBG("starting to send pings @%d", radio_->id());
				ping_number_ = 0;
				//transport_.request_send(cid);
				
				size_type n = snprintf((char*)buffer_, 511, "ping!");
				buffer_[n] = '\0';
				buffer_size_ = n + 1;
				
				transport_.open(MyChannelId(1234), 1);
				
				//timer_->set_timer<ExampleApplication, &ExampleApplication::trigger_produce>(1000, this, 0);
			}
		}
		
		bool on_event(int event, Transport::Message& msg) {
			switch(event) {
				case Transport::EVENT_PRODUCE:
					return produce(msg);
					break;
					
				case Transport::EVENT_CONSUME:
					consume(msg);
					break;
			}
			return false;
		}
		
		// convenience method
		void set_payload(Transport::Message& msg, const char* s) {
			memcpy(msg.payload(), s, strlen(s) + 1);
			msg.set_payload_size(strlen(s) + 1);
		}
		
		
		enum { STATES = sizeof(payloads) / sizeof(char*) };
		int state;
		
		bool produce(Transport::Message& msg) {
			DBG("produce @%d", radio_->id());
			
			
			set_payload(msg, payloads[state]);
			if(state == STATES - 1) {
				msg.set_close();
			}
			
			return true;
		}
		
		void consume(Transport::Message& msg) {
			
			for(int s = 0; s < STATES; s++) {
				if(strcmp(payloads[s], (char*)msg.payload()) == 0) {
					state = s + 1;
				}
			}
			
			DBG("consume @%d: %s -> state %d", radio_->id(), (char*)msg.payload(), (int)state);
			
			if(!msg.is_close()) {
				transport_.request_send();
			}
		}
		
		void forward(node_id_t from, Os::Radio::size_t len, block_data_t* buffer) {
			Transport::Message msg = *reinterpret_cast<Transport::Message*>(buffer);
			if(msg.type() != Transport::Message::MESSAGE_TYPE) { return; }
			
			if(radio_->id() == 0) {
				if(msg.is_ack()) {
					switch(from) {
						case 1:
							transport_.on_receive(from, len, buffer);
							break;
						case 2:
							radio_->send(1, len, buffer);
							break;
						case 3:
							radio_->send(2, len, buffer);
							break;
					}
				}
				else {
					switch(from) {
						case 1:
							radio_->send(2, len, buffer);
							break;
						case 2:
							radio_->send(3, len, buffer);
							break;
						case 3:
							transport_.on_receive(from, len, buffer);
							break;
					}
				}
			}
			else {
				transport_.on_receive(from, len, buffer);
			}
		}
		
	private:
		size_type ping_number_;
	
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Radio::self_pointer_t radio_;
		Os::Clock::self_pointer_t clock_;
		Os::Rand::self_pointer_t rand_;
		
		Transport transport_;
		
		block_data_t buffer_[512];
		size_type buffer_size_;
};

wiselib::WiselibApplication<Os, ExampleApplication> example_app;
void application_main(Os::AppMainParameter& value) {
	example_app.init(value);
}
