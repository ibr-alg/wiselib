
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

class ExampleApplication {
	public:
		ExampleApplication() : cid_(123, 1234567) {
		}
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			
			transport_.init(radio_, timer_);
			
			//cid_.foo = 0x12345678;
			//cid_.bar = 0xAA;
			
			
			transport_.register_endpoint(
					1 - radio_->id(),
					cid_,
					Transport::produce_callback_t::from_method<ExampleApplication, &ExampleApplication::produce>(this),
					Transport::consume_callback_t::from_method<ExampleApplication, &ExampleApplication::consume>(this)
			);
			
			DBG("endpoints registered @%d", radio_->id());
			
			if(radio_->id() == 0) {
				DBG("starting to send pings @%d", radio_->id());
				ping_number_ = 0;
				//transport_.request_send(cid);
				
				timer_->set_timer<ExampleApplication, &ExampleApplication::trigger_produce>(1000, this, 0);
			}
		}
		
		void trigger_produce(void*) {
			//DBG("trigger produce @%d", radio_->id());
			transport_.request_send(cid_);
			timer_->set_timer<ExampleApplication, &ExampleApplication::trigger_produce>(1000, this, 0);
		}
		
		size_type produce(block_data_t* buffer, size_type buffer_size) {
			DBG("produce @%d", radio_->id());
			if(buffer == 0) {
				DBG("send aborted!");
				return 0;
			}
			size_type n = snprintf((char*)buffer, buffer_size, "ping number %lu!", ping_number_++);
			return n;
		}
		
		void consume(block_data_t* buffer, size_type buffer_size) {
			DBG("consume @%d", radio_->id());
			debug_buffer<Os, 16, Os::Debug>(debug_, buffer, buffer_size);
		}
		
	private:
		MyChannelId cid_;
		size_type ping_number_;
	
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		Os::Radio::self_pointer_t radio_;
		
		Transport transport_;
};

wiselib::WiselibApplication<Os, ExampleApplication> example_app;
void application_main(Os::AppMainParameter& value) {
	example_app.init(value);
}
