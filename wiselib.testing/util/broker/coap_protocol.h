/*
 * File:   coap_protocol.h
 * Author: anilshanbhag
 *
 * Created on 31. May 2012, 13:53
 */

#ifndef COAP_PROTOCOL_H_
#define COAP_PROTOCOL_H_

#include "external_interface/default_return_values.h"
#include "util/pstl/map_static_vector.h"
#include "algorithms/coap/coap.h"

#ifdef COAP_USE_PROTOBUF
#include <util/broker/protobuf_rdf_serializer.h>
#endif


namespace wiselib{

	template<typename OsModel_P,
			typename Broker_P,
			typename Radio_P = typename OsModel_P::Radio,
			typename Timer_P = typename OsModel_P::Timer,
			typename Debug_P = typename OsModel_P::Debug,
			typename Rand_P = typename OsModel_P::Rand
			>
	class CoapProtocol {

#ifdef COAP_USE_PROTOBUF
		typedef ProtobufRdfSerializer<Os> Protobuf;
#endif
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef Rand_P Rand;
        typedef Broker_P broker_t;

        typedef CoapProtocol<OsModel,broker_t,Radio,Timer,Debug,Rand> self_type;
        typedef self_type* self_pointer_t;

        typedef wiselib::Coap<OsModel, Radio, Timer, Debug, Rand> coap_t;

	public:
		
		CoapProtocol() : output_buffer_(0) {
			//debug_->debug("CoapProtocol can debug.\n");
		}
		
        void receive_radio_message(typename Radio::node_id_t source, typename Radio::size_t len, typename Radio::block_data_t *buf)
        {
			//debug_->debug("coap proto recv from %d len=%d me=%d\n", source, len, radio_->id());
					
		   if (buf[0] == WISELIB_MID_COAP)
        	{
#ifdef ISENSE
                //debug_->debug( "Node %x received msg from %x msg type: CoAP length: %d\n", radio_->id(), source, len );
#else
                //debug_->debug("Node received msg from %x | msg type: CoAP | length: %d\n", source, len);
#endif
                // debug_hex( buf, len );
                coap_.receiver( &len, buf, &source );
        	}
        }

        void init(broker_t& broker, typename Radio::self_pointer_t radio,
        		typename Timer_P::self_pointer_t timer, typename Debug_P::self_pointer_t debug, typename Rand_P::self_pointer_t rand)
        {

        	// initializing private variables
        	broker_ = &broker;
        	radio_ = radio;
        	radio_->enable_radio();
        	timer_ = timer;
            debug_ = debug;
            rand_ = rand;
            res = 0;

            // coap necessities
#ifdef ISENSE
            rand_->srand( radio_->id() );
            mid_ = ( uint16_t )rand_->operator()( 65536 / 2 );
#else
            mid_ = ( uint16_t )rand_->operator()( 65536 / 2 );
#endif
			
			//broker_->template subscribe<self_type, self_type::on_documents_changed>(this, broker_t::DOCUMENT_CREATED | broker_t::DOCUMENT_ERASED);
        }
		
		/*
		void on_documents_changed(document_name_t docname, int flag) {
			if(flag & broker_t::DOCUMENT_ERASED) {
				remove_resource(docname);
			}
			if(flag & broker_t::DOCUMENT_CREATED) {
				uint8_t rid = add_resource(docname, fast_resource, 120, 5, TEXT_PLAIN);
				add_method<self_type>(rid, 0 
			}
		}

		uint8_t add_resource( StaticString name, bool fast_resource, uint16_t notify_time, uint8_t resource_len, uint8_t content_type )
		{
			if (res < CONF_MAX_RESOURCES)
			{
				resources[res].init();
				resources[res].reg_resource(name,fast_resource,notify_time,resource_len,content_type);

				res++;
				return res-1;
			}
			// TODO : See what to do if > max initialized
			return 1;
		}
		
		bool remove_resource(char* name)
		{
			for(size_t i=0; i<CONF_MAX_RESOURCES; i++) {
				if(resources[i].is_set() && strcmp(resources[i].name(), name) == 0) {
					resources[i].destruct();
				}
			}
		}

		template<class T, char* ( T::*TMethod ) ( uint8_t,uint16_t& )>
		void add_method( uint8_t resource_id, uint8_t qid, uint8_t method, T *objpnt, StaticString query="")
		{
			// TODO : Check if resource initialized ? Any other loops ?
			resources[resource_id].set_method( qid, method );
			resources[resource_id].template reg_callback<T, TMethod>( objpnt, qid );
			if (method == PUT) resources[resource_id].reg_query( qid, query );
		}
		*/
		void coap_start( )
		{
            coap_.init( *radio_, *timer_, *debug_, mid_, resources );
            radio_->template reg_recv_callback<self_type, &self_type::receive_radio_message > (this);
			
			resource_t &default_resource = resources[CONF_MAX_RESOURCES];
			default_resource.init();
			default_resource.reg_resource("", false, 120, 5, TEXT_PLAIN);
			
			// qid 0 --> GET
			default_resource.set_method(0, GET);
			default_resource.template reg_callback<self_type, &self_type::on_get>(this, 0);
		}
		
		char* on_get(uint8_t qid, uint16_t& size, char *name) {
			// DANGER MOUSE
			name[16] = '\0';
			
			//debug_->debug("on_get(%s)\n", name);
			bitmask_t mask = broker_->get_document_mask(name);
			//debug_->debug("docmask=%d\n", mask);
			
#ifdef COAP_USE_PROTOBUF
			Protobuf protobuf;
			
			if(!output_buffer_) {
				output_buffer_size_ = 32;
			}
			
			for( ; output_buffer_size_ < 16 * 1024; output_buffer_size_ *= 2) {
				//debug_->debug("output buffer size: %d\n",
						//output_buffer_size_);
				
				if(output_buffer_) {
					get_allocator().free_array(output_buffer_);
				}
				output_buffer_ = get_allocator().template allocate_array<char>(output_buffer_size_).raw();
				
				typename broker_t::iterator iter = broker_->begin_document(mask);
				size_t written = protobuf.fill_buffer((block_data_t*)output_buffer_, output_buffer_size_, iter, broker_->end_document(mask));
				
				if(iter == broker_->end_document(mask)) {
					
					size = written; //output_buffer_size_;
					//debug_->debug("returning: %x %x %x %x bufsize=%d written=%d\n",
							//(int)(uint8_t)output_buffer_[0],
							//(int)(uint8_t)output_buffer_[1],
							//(int)(uint8_t)output_buffer_[2],
							//(int)(uint8_t)output_buffer_[3],
							//output_buffer_size_,
							//written
							//);
					return output_buffer_;
				}
			}
#endif
			
			size = 0;
			return 0;
		} // on_get()

	private:

        //void debug_hex( const uint8_t * payload, typename Radio::size_t length )
        //{
           //char buffer[2048];
           //int bytes_written = 0;
           //bytes_written += sprintf( buffer + bytes_written, "DATA:!" );
           //for ( size_t i = 0; i < length; i++ )
           //{
              //bytes_written += sprintf( buffer + bytes_written, "%x!", payload[i] );
           //}
           //buffer[bytes_written] = '\0';
           //debug_->debug( "%s\n", buffer );
        //}

        typename broker_t::self_pointer_t broker_;

        typename Radio::self_pointer_t radio_;
        typename Timer_P::self_pointer_t timer_;
        typename Debug_P::self_pointer_t debug_;
        typename Rand_P::self_pointer_t rand_;

        uint16_t mid_;
        coap_t coap_;
        coap_packet_t packet;
        resource_t resources[CONF_MAX_RESOURCES + 1];
        uint8_t res;
		
		char *output_buffer_;
		size_t output_buffer_size_;
	};
}

#endif
