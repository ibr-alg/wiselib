
#ifndef DIRECT_BROKER_PROTOCOL_COMMAND_MESSAGE
#define DIRECT_BROKER_PROTOCOL_COMMAND_MESSAGE

#include "util/serialization/simple_types.h"

template<
	typename OsModel_P,
	typename Radio_P = typename OsModel_P::Radio
>
class DirectBrokerProtocolCommandMessage {
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef typename Radio::size_t size_type;
		typedef typename Radio::block_data_t block_data_t;
		typedef ::uint8_t transaction_id_t;
		
		enum RequestTypes {
			GET_DOCUMENT = 0,
			SUBSCRIBE = 1,
			UNSUBSCRIBE = 2,
			TRANSACTION = 3,
			ERASE_DOCUMENT = 4
		};
		
		enum Flags {
			FLAG_COMPRESSED = 0x01,
			FLAG_COMMIT_TRANSACTION = 0x02,
			FLAG_ABORT_TRANSACTION = 0x03
		};
		
		enum Columns {
			SUBJECT = 0, PREDICATE = 1, OBJECT = 2,
			DOCUMENT_NAME = 3, ACTION = 4
		};
		
		enum Actions {
			ACTION_INSERT = 1,
			ACTION_ERASE = 2
		};
		
		::uint8_t request_type() {
			return read<OsModel, block_data_t, ::uint8_t>(data_ + O_REQUEST_TYPE);
		}
		
		void set_request_type(::uint8_t v) {
			write<OsModel, block_data_t, ::uint8_t>(data_ + O_REQUEST_TYPE, v);
		}
		
		::uint8_t request_id() {
			return read<OsModel, block_data_t, ::uint8_t>(data_ + O_REQUEST_ID);
		}
		
		transaction_id_t transaction_id() { return request_id(); }
		
		void set_request_id(::uint8_t v) {
			write<OsModel, block_data_t, ::uint8_t>(data_ + O_REQUEST_ID, v);
		}
		
		::uint8_t flags() {
			return read<OsModel, block_data_t, ::uint8_t>(data_ + O_FLAGS);
		}
		
		void set_flags(::uint8_t v) {
			write<OsModel, block_data_t, ::uint8_t>(data_ + O_FLAGS, v);
		}
		
		bool compressed() { return flags() & FLAG_COMPRESSED; }
		bool commit_transaction() { return flags() & FLAG_COMMIT_TRANSACTION; }
		bool abort_transaction() { return flags() & FLAG_ABORT_TRANSACTION; }
		
		::uint8_t column() {
			return read<OsModel, block_data_t, ::uint8_t>(data_ + O_COLUMN);
		}
		
		void set_column(::uint8_t v) {
			write<OsModel, block_data_t, ::uint8_t>(data_ + O_COLUMN, v);
		}
		
		char* document_name() {
			return (char*)(data_ + O_DOCUMENT_NAME);
		}
		
		void set_document_name(char* src) {
			size_type l = strlen(src);
			if(l > (Radio::MAX_MESSAGE_LENGTH - 1 - O_DOCUMENT_NAME)) {
				l = (Radio::MAX_MESSAGE_LENGTH - 1 - O_DOCUMENT_NAME);
			}
			memcpy((void*)(data_ + O_DOCUMENT_NAME), (void*)src, l);
			*(data_ + O_DOCUMENT_NAME + l) = 0;
		}
		
		char* target_document_name() {
			return (char*)(data_ + O_DOCUMENT_NAME + strlen((char*)(data_ + O_DOCUMENT_NAME)) + 1);
		}
		
		/**
		 * Important: Only call this *after* set_document_name()!
		 */
		void set_target_document_name(char* src) {
			size_type l = strlen(src);
			size_type docnamelen = strlen(document_name());
			if(l > (Radio::MAX_MESSAGE_LENGTH - 1 - O_DOCUMENT_NAME - docnamelen - 1)) {
				l = (Radio::MAX_MESSAGE_LENGTH - 1 - O_DOCUMENT_NAME - docnamelen - 1);
			}
			memcpy((void*)(data_ + O_DOCUMENT_NAME + docnamelen + 1), (void*)src, l);
			*(data_ + O_DOCUMENT_NAME + docnamelen + 1 + l) = 0;
		}
		
		block_data_t* tuple_element() {
			return data_ + O_TUPLE_ELEMENT;
		}
		
		void set_tuple_element(block_data_t* src) {
			set_document_name((char*)src);
		}
		
		size_type length() {
			size_type l = O_TUPLE_ELEMENT + strlen(document_name()) + 1;
			if(request_type() == GET_DOCUMENT) {
				l += strlen(target_document_name()) + 1;
			}
			return l;
		}
		

	private:
		enum Offsets {
			O_REQUEST_TYPE = 0,
			O_REQUEST_ID = O_REQUEST_TYPE + 1,
			O_FLAGS = O_REQUEST_ID + 1,
			O_COLUMN = O_FLAGS + 1,
			O_DOCUMENT_NAME = O_COLUMN + 1,
			O_TUPLE_ELEMENT = O_COLUMN + 1,
		};
		
		block_data_t data_[Radio::MAX_MESSAGE_LENGTH + 1];		
};

#endif // DIRECT_BROKER_PROTOCOL_COMMAND_MESSAGE

