
#include "platform.h"

using namespace wiselib;
#include <algorithms/protocols/reliable_transport/reliable_transport.h>
#include <util/debugging.h>
#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>

typedef wiselib::OSMODEL Os;
typedef Os::size_t size_type;
typedef Os::block_data_t block_data_t;

struct XMyChannelId {
	bool operator==(const XMyChannelId& other) const {
		return foo == other.foo && bar == other.bar;
	}
	
	::uint32_t foo;
	::uint8_t bar;
};

typedef SemanticEntityId MyChannelId;

typedef ReliableTransport<Os, MyChannelId, Os::Radio, Os::Timer> Transport;
typedef Os::Radio::node_id_t node_id_t;

class ExampleApplication {
	public:
		ExampleApplication() : cid_(123, 1234567) {
		}
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			
			radio_->reg_recv_callback<ExampleApplication, &ExampleApplication::forward>(this);
				
			transport_.init(radio_, timer_, false);
			buffer_size_ = 0;
			
			//cid_.foo = 0x12345678;
			//cid_.bar = 0xAA;
			
			switch(radio_->id()) {
				case 0:
					reg_endpoint(1, true);
					reg_endpoint(3, false);
					break;
					
				case 1:
				case 2:
				case 3:
					reg_endpoint(0, true);
					reg_endpoint(0, false);
					break;
			}
			
			DBG("endpoints registered @%d", radio_->id());
			
			if(radio_->id() == 0) {
				DBG("starting to send pings @%d", radio_->id());
				ping_number_ = 0;
				//transport_.request_send(cid);
				
				size_type n = snprintf((char*)buffer_, 511, "ping!");
				buffer_[n] = '\0';
				buffer_size_ = n + 1;
				
				timer_->set_timer<ExampleApplication, &ExampleApplication::trigger_produce>(1000, this, 0);
			}
		}
		
		void reg_endpoint(node_id_t n, bool initiator) {
			transport_.register_endpoint(
					n,
					cid_, initiator,
					Transport::produce_callback_t::from_method<ExampleApplication, &ExampleApplication::produce>(this),
					Transport::consume_callback_t::from_method<ExampleApplication, &ExampleApplication::consume>(this)
			);
			
		}
		
		void trigger_produce(void*) {
			//DBG("trigger produce @%d", radio_->id());
			transport_.request_send(cid_, true);
			//timer_->set_timer<ExampleApplication, &ExampleApplication::trigger_produce>(1000, this, 0);
		}
		
		size_type produce(block_data_t* buffer, size_type buffer_size) {
			DBG("produce @%d", radio_->id());
			memcpy(buffer, buffer_, buffer_size_);
			return buffer_size_;
		}
		
		void consume(block_data_t* buffer, size_type buffer_size, Transport::Endpoint& endpoint) {
			DBG("@%d: %s", radio_->id(), (char*)buffer);
			//debug_->debug("++++++++++++++++++++++++");
			//debug_buffer<Os, 16, Os::Debug>(debug_, buffer, buffer_size);
			//debug_->debug("++++++++++++++++++++++++");
			if(!endpoint.initiator()) {
				memcpy(buffer_, buffer, buffer_size);
				buffer_size_ = buffer_size;
				transport_.request_send(cid_, true);
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
		MyChannelId cid_;
		size_type ping_number_;
	
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Radio::self_pointer_t radio_;
		
		Transport transport_;
		
		block_data_t buffer_[512];
		size_type buffer_size_;
};

wiselib::WiselibApplication<Os, ExampleApplication> example_app;
void application_main(Os::AppMainParameter& value) {
	example_app.init(value);
}
