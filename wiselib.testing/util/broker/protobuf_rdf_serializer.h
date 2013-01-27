/* 
 * File:   protobuf_rdf_serializer.h
 * Author: maxpagel
 *
 * Created on 8. Mai 2012, 16:20
 */

#ifndef _PROTOBUF_RDF_SERIALIZER_H
#define	_PROTOBUF_RDF_SERIALIZER_H

#include "util/protobuf/message.h"

#include "util/protobuf/varint.h"
#include "util/protobuf/string.h"
//#include "util/protobuf/buffer_dynamic.h"

namespace wiselib
{

    template<typename OsModel_P>
    class ProtobufRdfSerializer
    {
        typedef OsModel_P OsModel;
		typedef typename OsModel::block_data_t block_data_t;
        typedef unsigned int int_t;        
		typedef block_data_t* buffer_t;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::size_t size_type;

        typedef wiselib::protobuf::Message<OsModel, buffer_t, int_t> dynamic_message_t;
        typedef wiselib::protobuf::Message<OsModel, typename OsModel::Radio::block_data_t*, int_t> static_message_t;

    public:
		
		void reset() {
		}

		template<typename iterator>
        size_type fill_buffer(buffer_t buffer, size_t max_packet_size, iterator& current, const iterator& end)
        {
			iterator it = current;
			block_data_t *buf = buffer, *buf_end = buffer + max_packet_size;
			
			const size_t description_size = 1000;
			block_data_t description[description_size];
			block_data_t *descr = description, *descr_end = description + description_size;
			
            for ( ; it != end; ++it)
			{
				const size_t stmt_size = 200;
                block_data_t statement[stmt_size];
				block_data_t *stmt = statement, *stmt_end = stmt + stmt_size;
                if(!static_message_t::write(stmt, stmt_end, 1, (char*)(*it).get(0))) { return 0; }
                if(!static_message_t::write(stmt, stmt_end, 2, (char*)(*it).get(1))) { return 0; }
				if(!static_message_t::write(stmt, stmt_end, 3, (char*)(*it).get(2))) { return 0; }
                if(!static_message_t::write(descr, descr_end, 1, statement, stmt - statement)) { return 0; }
            }
			
			if(!static_message_t::write(buf, buf_end, 1, "sensornode1")) { return 0; }
			if(!static_message_t::write(buf, buf_end, 4, description, descr - description)) { return 0; }
			
			current = it;
            return buf - buffer;
        }

		
		template<typename Tuple>
		size_type read_buffer(Tuple& tuple, block_data_t* buffer, size_type buffer_size) {
			int_t field;
			block_data_t *buf = buffer, *end = buffer + buffer_size;
			
			do {
				block_data_t description[1000], *descr=description, *descr_end = description + 1000;
				switch(static_message_t::field_number(buf, end)) {
					case 4:
						if(!static_message_t::read(buf, end, field, description)) { return 0; }
						do {
							block_data_t statement[200], *stmt=statement, *statement_end = statement + 200;
							switch(static_message_t::field_number(buf, end)) {
								case 1:
									if(!static_message_t::read(buf, end, field, statement)) { return 0; }
									//do {
										char s[200];
										for(size_type i=0; i<3;i++) {
											int_t field2 = static_message_t::field_number(stmt, end);
											if(!static_message_t::read(stmt, statement_end, field, s)) { return 0; }
											tuple.set_deep(field2 - 1, (block_data_t*)s);
										}
										return buf - buffer;
									//} while(buf < end);
							} // switch
						} while(buf < end);
						break;
				} // switch
			} while(buf < end);
			
			return 0;
		}
		
    }; // class ProtobufRdfSerializer
}


#endif	/* _PROTOBUF_RDF_SERIALIZER_H */

