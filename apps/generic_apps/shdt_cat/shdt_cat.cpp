
#define SHDT_REUSE_PREFIXES 1
#define USE_CODEC 0
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
#include <util/split_n3.h>
typedef HuffmanCodec<Os> Codec;

#include <iostream>
using namespace std;

struct Tuple {
	void set(size_t idx, block_data_t* data) {
		data_[idx] = data;
	}
	
	block_data_t *get(size_t idx) { return data_[idx]; }
	
	size_t length(size_t idx) { return strlen((char*)data_[idx]) + 1; }
	
	block_data_t *data_[3];
};


class ExampleApplication {
	public:
		
		void init(Os::AppMainParameter& value) {
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

			bufsize_ = 1024; // MTU
			cat();
			
			//shdt_encode(100);
			
			//for(size_type packet_size = 20; packet_size <= 140; packet_size += 40) {
				//for(size_type table_size = 10; table_size <= Shdt::MAX_TABLE_SIZE; table_size += 1) {
			//size_type packet_size = 100;
			//size_type table_size=64;
					//sender.reset();
					//receiver.reset();
					//test_shdt_tuples(table_size, packet_size, (block_data_t**)test_tuples_btcsample);
				//}
			//}

		}


		void cat() {
			char line[20480];
			SplitN3<Os> splitter;

			Shdt::Writer w(
					&sender, buffer_, bufsize_,
					Shdt::write_callback_t::from_method<ExampleApplication, &ExampleApplication::shdt_test_receive_tuples>(this)
			);
			w.write_header(128, 3);

			while(std::cin) {
				std::cin.getline(line, 20480);
				splitter.parse_line(line);
				Tuple tuple;
				#if USE_CODEC
					block_data_t *s = Codec::encode((block_data_t*)splitter[0]);
					tuple.set(0, s); cstr_size_ += strlen((char*)s) + 1;
					s = Codec::encode((block_data_t*)splitter[1]);
					tuple.set(1, s); cstr_size_ += strlen((char*)s) + 1;
					s = Codec::encode((block_data_t*)splitter[2]);
					tuple.set(2, s); cstr_size_ += strlen((char*)s) + 1;
				
				#else
					tuple.set(0, (block_data_t*)splitter[0]); cstr_size_ += strlen((char*)splitter[0]) + 1;
					tuple.set(1, (block_data_t*)splitter[1]); cstr_size_ += strlen((char*)splitter[1]) + 1;
					tuple.set(2, (block_data_t*)splitter[2]); cstr_size_ += strlen((char*)splitter[2]) + 1;
				#endif

				//debug_->debug("(%s %s %s)", (char*)tuple.get(0), (char*)tuple.get(1), (char*)tuple.get(2));
				w.write_tuple(tuple);
				#if USE_CODEC
					::get_allocator().free_array(tuple.get(0));
					::get_allocator().free_array(tuple.get(1));
					::get_allocator().free_array(tuple.get(2));
				#endif
			}
			
			w.flush();
		}
		//
		// Tuple encode/decode test
		//
		
		//size_type table_size_;
		size_type bufsize_;
		block_data_t **test_tuples_;
		size_type cstr_size_;
		size_type shdt_size_;
		size_type packets_;
		size_type verify_idx_;
		
		void shdt_test_receive_tuples(Shdt::Writer& w) { //block_data_t *buffer, size_type buffer_size) {
			//debug_buffer<Os, 16, Os::Debug>(debug_, w.buffer(), w.buffer_used());
			cout.write((char*)w.buffer(), w.buffer_used());
			
			/*
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
					//if(strcmp(s, (char*)test_tuples_[verify_idx_]) != 0) {
						//debug_->debug("\x1b[31merror: t[%d]=%s != vrfy[%d]=%s\x1b[m", (int)i, (char*)t.get(i),
								//(int)verify_idx_, (char*)test_tuples_[verify_idx_]);
						//assert(false);
					//}
					
					#if USE_CODEC
						::get_allocator().free_array(s);
					#endif
				}
			}
			
			*/
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
