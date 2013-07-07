
#include "platform.h"

using namespace wiselib;
#include <util/broker/shdt_serializer.h>
#include <util/debugging.h>

typedef wiselib::OSMODEL Os;
typedef Os::size_t size_type;
typedef Os::block_data_t block_data_t;

typedef wiselib::ShdtSerializer<Os, 128, 4> Shdt;

struct Tuple {
	void set(size_t idx, block_data_t* data) {
		data_[idx] = data;
	}
	
	block_data_t *get(size_t idx) { return data_[idx]; }
	
	size_t length(size_t idx) { return strlen((char*)data_[idx]); }
	
	block_data_t *data_[3];
};


char *test_tuples_mini[] = {
	"foo", "foobar", "foobaring",
	"baz", "foobaz", "blarg",
	0
};


class ExampleApplication {
	public:
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			
			//shdt_encode(100);
			test_shdt_tuples(64, 100, (block_data_t**)test_tuples_mini);
		}
		
		
		//
		// Tuple encode/decode test
		//
		
		size_type table_size_;
		size_type bufsize_;
		block_data_t **test_tuples_;
		size_type cstr_size_;
		size_type shdt_size_;
		size_type packets_;
		size_type verify_idx_;
		
		void test_shdt_tuples(size_type table_size, size_type bufsize, block_data_t **tuples) {
			table_size_ = table_size;
			bufsize_ = bufsize;
			test_tuples_ = tuples;
			cstr_size_ = 0;
			shdt_size_ = 0;
			packets_ = 0;
			verify_idx_ = 0;
			
			shdt_test_send_tuples();
			
			if(test_tuples_[verify_idx_]) {
				DBG("verify_idx not at end idx=%d *idx=%s", (int)verify_idx_, (char*)test_tuples_[verify_idx_]);
			}
			DBG("cstr_size %d shdt_size %d packets %d",
					(int)cstr_size_, (int)shdt_size_, (int)packets_);
		}
		
		
		void shdt_test_send_tuples() {
			
			Shdt::Writer w(&sender, buffer_, bufsize_,
					Shdt::write_callback_t::from_method<ExampleApplication, &ExampleApplication::shdt_test_receive_tuples>(this));
			w.write_header(table_size_, 3);
			
			for(block_data_t** t = test_tuples_; t[0]; t += 3) {
				Tuple tuple;
				tuple.set(0, t[0]); cstr_size_ += strlen((char*)t[0]) + 1;
				tuple.set(1, t[1]); cstr_size_ += strlen((char*)t[1]) + 1;
				tuple.set(2, t[2]); cstr_size_ += strlen((char*)t[2]) + 1;
				
				w.write_tuple(tuple);
			}
			
			w.flush();
		}
		
		void shdt_test_receive_tuples(Shdt::Writer& w) { //block_data_t *buffer, size_type buffer_size) {
			packets_++;
			shdt_size_ += w.buffer_used();
			Shdt::Reader r(&receiver, w.buffer(), w.buffer_used());
			
			Tuple t;
			DBG("&r = %p", &r);
			bool found = r.read_tuple(t);
			if(found) {
				for(size_type i = 0; i < 3; i++, verify_idx_++) {
					if(strcmp((char*)t.get(i), (char*)test_tuples_[verify_idx_]) != 0) {
						DBG("t[%d]=%s != vrfy[%d]=%s", (int)i, (char*)t.get(i),
								(int)verify_idx_, (char*)test_tuples_[verify_idx_]);
					}
				}
			}
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
