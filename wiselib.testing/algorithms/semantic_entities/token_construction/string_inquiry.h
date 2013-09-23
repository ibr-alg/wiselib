/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef STRING_INQUIRY_H
#define STRING_INQUIRY_H

#include "semantic_entity_id.h"
#include <util/pstl/map_static_vector.h>
#include "string_inquiry_message.h"
#include "string_inquiry_answer_message.h"

//#ifndef INSE_MESSAGE_TYPE_STRING_INQUIRY
	//#define INSE_MESSAGE_TYPE_STRING_INQUIRY 0x46
//#endif

//#ifndef INSE_MESSAGE_TYPE_STRING_INQUIRY_ANSWER
	//#define INSE_MESSAGE_TYPE_STRING_INQUIRY_ANSWER 0x47
//#endif

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename QueryProcessor_P
	>
	class StringInquiry {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef QueryProcessor_P QueryProcessor;
			typedef typename QueryProcessor::Value Value;
			typedef typename QueryProcessor::Dictionary Dictionary;
			typedef typename Dictionary::key_type DictionaryKey;
			
			enum {
				MESSAGE_TYPE_INQUIRY = INSE_MESSAGE_TYPE_STRING_INQUIRY,
				MESSAGE_TYPE_ANSWER = INSE_MESSAGE_TYPE_STRING_INQUIRY_ANSWER
			};
			
			typedef StringInquiryMessage<OsModel, Radio, Value, MESSAGE_TYPE_INQUIRY> InquiryMessageT;
			typedef StringInquiryAnswerMessage<OsModel, Radio, Value, MESSAGE_TYPE_ANSWER> AnswerMessageT;
			typedef delegate2<void, Value, const char*> answer_delegate_t;
			
			class StringConstruction {
				public:
					StringConstruction() : string_(0), length_(0) {
					}
					
					void init(size_type l) {
						string_ = ::get_allocator().template allocate_array<char>(l + 1) .raw();
						length_ = l;
						memset(string_, 0, l + 1);
					}
					
					void destruct() {
						if(string_) {
							::get_allocator().free_array(string_);
							string_ = 0;
							length_ = 0;
						}
					}
					
					void add(size_type offset, size_type length, char *part) {
						memcpy(string_ + offset, part, length);
					}
					
					bool complete() {
						char *end = string_ + length_;
						for(char *p = string_; p < end; p++) {
							if(*p == 0) { return false; }
						}
						return true;
					}
				private:
					size_type length_;
					char *string_;
			};
			
			void init(typename Radio::self_pointer_t radio, typename QueryProcessor::self_pointer_t qp) {
				radio_ = radio;
				query_processor_ = qp;
			}
			
			void reg_answer_callback(answer_delegate_t ans) {
				answer_delegate_ = ans;
			}
			
			void inquire(const SemanticEntityId& scope, Value hash) {
				InquiryMessageT msg;
				msg.set_scope(scope);
				msg.set_hash(hash);
				radio_.send(msg.data_size(), msg.data());
			}
			
			void on_receive(const SemanticEntityId& from, typename Radio::size_t len, typename Radio::block_data_t* data) {
				typename Radio::message_id_t t;
				t = wiselib::read<OsModel, block_data_t, typename Radio::message_id_t>(data);
				
				if(t == MESSAGE_TYPE_ANSWER) {
					AnswerMessageT &msg = *reinterpret_cast<AnswerMessageT>(data);
					Value h = msg.hash();
					if(!constructing_.contains(h)) {
						constructing_[h].init(msg.total_string_length());
					}
					constructing_[h].add(msg.part_offset(), msg.part_length(), msg.part());
					if(constructing_[h].complete()) {
						//notify_receivers(h, constructing_[h].string());
						if(answer_delegate_) {
							answer_delegate_(h, constructing_[h].string());
						}
						constructing_[h].destruct();
						constructing_.erase(h);
					}
				}
			}
			
			bool accept(const typename Radio::MessageT& m) {
				typename Radio::message_id_t t;
				t = wiselib::read<OsModel, block_data_t, typename Radio::message_id_t>(m.payload());
				if(t == MESSAGE_TYPE_INQUIRY) {
					InquiryMessageT &msg = *reinterpret_cast<InquiryMessageT>(m.payload());
					Value h = msg.hash();
					DictionaryKey k = reverse_translator().translate(h);
					if(k == Dictionary::NULL_KEY) {
						return false;
					}
					block_data_t *string = dictionary()->get_value(k);
					size_type len = strlen((char*)string) + 1;
					size_type bytes_sent = 0;
					
					AnswerMessageT ans;
					ans.set_hash(h);
					ans.set_total_length(len);
					while(bytes_sent < len) {
						size_type l = min(len - bytes_sent, AnswerMessageT::MAX_PAYLOAD_SIZE);
						ans.set_part(l, string + bytes_sent);
						radio_->send(SemanticEntityId::all(), ans.data_size(), ans.data());
						bytes_sent += l;
					}
					dictionary()->free_value(string);
					return true;
				}
				else if(t == MESSAGE_TYPE_ANSWER) {
					return true;
				}
				return false;
			}
			
			Dictionary& dictionary() { return query_processor_->dictionary(); }
			typename QueryProcessor::ReverseTranslator& reverse_translator() { return query_processor_->reverse_translator(); }
			
		private:
			
			MapStaticVector<OsModel, Value, StringConstruction, 10> constructing_;
			typename QueryProcessor::self_pointer_t query_processor_;
			typename Radio::self_pointer_t radio_;
			answer_delegate_t answer_delegate_;
		
	}; // StringInquiry
}

#endif // STRING_INQUIRY_H

