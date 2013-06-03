
#include "platform.h"

using namespace wiselib;
#include <util/broker/shdt_serializer.h>
#include <util/debugging.h>

typedef wiselib::OSMODEL Os;
typedef Os::size_t size_type;
typedef Os::block_data_t block_data_t;

typedef wiselib::ShdtSerializer<Os, 128, 4> Shdt;

class ExampleApplication {
	public:
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			
			shdt_encode(100);
		}
		
		void shdt_encode(size_type bufsize) {
			debug_->debug("<shdt_encode>");
			
			Shdt::Writer w(&sender, buffer_, bufsize,
					Shdt::write_callback_t::from_method<ExampleApplication, &ExampleApplication::send_buffer>(this));
			
			w.write_header(8, 3);
			
			block_data_t* s = (block_data_t*)"<http://de.wikipedia.org/wiki/Llanfairpwllgwyngyllgogerychwyrndrobwllllantysiliogogogoch>";
			w.write_field(1, s, strlen((char*)s));
			
			block_data_t* s2 = (block_data_t*)"<http://de.wikipedia.org/wiki/Taumatawhakatangihangakoauauotamateaturipukakapikimaungahoronukupokaiwhenuakitanatahu>";
			w.write_field(2, s2, strlen((char*)s2));
			
			w.write_field(3, s2, strlen((char*)s2));
			
			/*
			s = (block_data_t*)"<http://www.spitfire-project.eu/>";
			w.write_field(2, s, strlen((char*)s));
			*/
			
			w.flush();
			
			debug_->debug("</shdt_encode>");
		}
		
		// Called by Shdt when a buffer is full
		void send_buffer(Shdt::Writer& w) {
			debug_->debug("  <send_buffer>");
			debug_buffer<Os, 16, Os::Debug>(debug_, w.buffer(), w.buffer_used());
			
			shdt_decode(w.buffer(), w.buffer_used());
			w.reuse_buffer();
			debug_->debug("  </send_buffer>");
		}
		
		
		void shdt_decode(block_data_t* buffer, size_type buffer_size) {
			debug_->debug("    <shdt_decode>");
			Shdt::Reader r(&receiver, buffer, buffer_size);
			
			Shdt::field_id_t field_id;
			block_data_t *value;
			size_type size;
			
			bool found = r.read_field(field_id, value, size);
			if(found) {
				debug_->debug("    -> found=%d fid=%d v=%s sz=%d", found, field_id, value, size);
			}
			else {
				debug_->debug("    -> (not found)");
			}
			debug_->debug("    </shdt_decode>");
		}
		
	private:
		Shdt sender;
		Shdt receiver;
		
		block_data_t buffer_[512];
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
};

wiselib::WiselibApplication<Os, ExampleApplication> example_app;
void application_main(Os::AppMainParameter& value) {
	example_app.init(value);
}
