
#define SHDT_REUSE_PREFIXES 1
#define USE_CODEC 1
#define MAX_STRING_LENGTH 2048

#include "platform.h"

using namespace wiselib;
#include <util/broker/shdt_serializer.h>
#include <util/debugging.h>

typedef wiselib::OSMODEL Os;
typedef Os::size_t size_type;
typedef Os::block_data_t block_data_t;

typedef wiselib::ShdtSerializer<Os, 128, 4> Shdt;

#include <algorithms/codecs/huffman_codec.h>
typedef HuffmanCodec<Os> Codec;


struct Tuple {
	void set(size_t idx, block_data_t* data) {
		data_[idx] = data;
	}
	
	block_data_t *get(size_t idx) { return data_[idx]; }
	
	size_t length(size_t idx) { return strlen((char*)data_[idx]) + 1; }
	
	block_data_t *data_[3];
};


char *test_tuples_mini[] = {
	"foo", "foobar", "foobaring",
	"baz", "foobaz", "blarg",
	
	"<http://de.wikipedia.org/wiki/Llanfairpwllgwyngyllgogerychwyrndrobwllllantysiliogogogoch>",
	"<http://de.wikipedia.org/wiki/Taumatawhakatangihangakoauauotamateaturipukakapikimaungahoronukupokaiwhenuakitanatahu>",
	"<http://de.wikipedia.org/wiki/Chargoggagoggmanchauggagoggchaubunagungamaugg>",
	
	0
};


char *test_tuples_btcsample[][3] = {
	//#include "ssp.cpp"
	//#include "btcsample0.cpp"
	#include "incontextsensing.cpp"
	0
};

/*
			block_data_t* s = (block_data_t*)"<http://de.wikipedia.org/wiki/Llanfairpwllgwyngyllgogerychwyrndrobwllllantysiliogogogoch>";
			w.write_field(1, s, strlen((char*)s));
			
			block_data_t* s2 = (block_data_t*)"<http://de.wikipedia.org/wiki/Taumatawhakatangihangakoauauotamateaturipukakapikimaungahoronukupokaiwhenuakitanatahu>";
*/

class ExampleApplication {
	public:
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			
			//shdt_encode(100);
			
			for(size_type packet_size = 100; packet_size <= 1000100; packet_size += 1000000) {
				for(size_type table_size = 10; table_size <= Shdt::MAX_TABLE_SIZE; table_size += 10) {
			//size_type packet_size = 100;
			//size_type table_size=64;
					sender.reset();
					receiver.reset();
					test_shdt_tuples(table_size, packet_size, (block_data_t**)test_tuples_btcsample);
				}
			}
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
				debug_->debug("\x1b[31merror: verify_idx not at end idx=%d *idx=%s\x1b[m", (int)verify_idx_, (char*)test_tuples_[verify_idx_]);
			}
			debug_->debug("bufsize %d tablesize %d cstr_size %d shdt_size %d packets %d ratio %f", (int)bufsize_, (int)table_size_, (int)cstr_size_, (int)shdt_size_, (int)packets_, (float)shdt_size_ / (float)cstr_size_);
		}
		
		
		/*
		block_data_t* escape_ff(block_data_t *s) {
			static block_data_t r[MAX_STRING_LENGTH];
			for(block_data_t *p = r; *s; s++, p++) {
				if(s == 0xff) {
					*p++ = 0xff;
				}
				*p = *s;
			}
			*p = '\0';
			return r;
		}
		
		block_data_t* unescape_ff(block_data_t *s) {
			static block_data_t r[MAX_STRING_LENGTH];
			bool escaped = false;
			for(block_data_t *p = r; *s; s++, p++) {
				if(s == 0xff) {
					if(escaped) {
						assert(*s == 0xff);
						*p = 0xff;
					*p++ = 0xff;
				}
				*p = *s;
			}
			*p = '\0';
			return r;
		}
		*/
		
		void shdt_test_send_tuples() {
			
			Shdt::Writer w(&sender, buffer_, bufsize_,
					Shdt::write_callback_t::from_method<ExampleApplication, &ExampleApplication::shdt_test_receive_tuples>(this));
			w.write_header(table_size_, 3);
			
			for(block_data_t** t = test_tuples_; t[0]; t += 3) {
				Tuple tuple;
				
				#if USE_CODEC
					block_data_t *s = Codec::encode(t[0]);
					tuple.set(0, s); cstr_size_ += strlen((char*)s) + 1;
					s = Codec::encode(t[1]);
					tuple.set(1, s); cstr_size_ += strlen((char*)s) + 1;
					s = Codec::encode(t[2]);
					tuple.set(2, s); cstr_size_ += strlen((char*)s) + 1;
				
				#else
					tuple.set(0, t[0]); cstr_size_ += strlen((char*)t[0]) + 1;
					tuple.set(1, t[1]); cstr_size_ += strlen((char*)t[1]) + 1;
					tuple.set(2, t[2]); cstr_size_ += strlen((char*)t[2]) + 1;
				#endif
				
				w.write_tuple(tuple);
				
				#if USE_CODEC
					::get_allocator().free_array(tuple.get(0));
					::get_allocator().free_array(tuple.get(1));
					::get_allocator().free_array(tuple.get(2));
				#endif
			}
			
			w.flush();
		}
		
		void shdt_test_receive_tuples(Shdt::Writer& w) { //block_data_t *buffer, size_type buffer_size) {
			//debug_buffer<Os, 16, Os::Debug>(debug_, w.buffer(), w.buffer_used());
			
			packets_++;
			shdt_size_ += w.buffer_used();
			Shdt::Reader r(&receiver, w.buffer(), w.buffer_used());
			
			bool found;
			
			while(true) {
				Tuple t;
				found = r.read_tuple(t);
				if(!found) { break; }
				
				for(size_type i = 0; i < 3; i++, verify_idx_++) {
					char *s;
					#if USE_CODEC
						s = (char*)Codec::decode(t.get(i));
					#else
						s = (char*)t.get(i);
					#endif
					
					
					//DBG("t[%d]=%s test[%d]=%s", i, t.get(i), verify_idx_, test_tuples_[verify_idx_]);
					if(strcmp(s, (char*)test_tuples_[verify_idx_]) != 0) {
						debug_->debug("\x1b[31merror: t[%d]=%s != vrfy[%d]=%s\x1b[m", (int)i, (char*)t.get(i),
								(int)verify_idx_, (char*)test_tuples_[verify_idx_]);
						assert(false);
					}
					
					#if USE_CODEC
						::get_allocator().free_array(s);
					#endif
				}
			}
			
			w.reuse_buffer();
		}
		
	private:
		Shdt sender;
		Shdt receiver;
		
		block_data_t buffer_[512000];
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
};

wiselib::WiselibApplication<Os, ExampleApplication> example_app;
void application_main(Os::AppMainParameter& value) {
	example_app.init(value);
}
