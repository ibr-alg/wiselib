
#ifndef DIRECT_BROKER_PROTOCOL_H
#define DIRECT_BROKER_PROTOCOL_H

#include "direct_broker_protocol_command_message.h"
#include <util/pstl/map_static_vector.h>

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Broker_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Debug_P = typename OsModel_P::Debug
	>
	class DirectBrokerProtocol {
		public:
			typedef OsModel_P OsModel;
			typedef Broker_P Broker;
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef DirectBrokerProtocol<OsModel, Broker, Radio, Debug> self_type;
			typedef self_type* self_pointer_t;
			
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_type;
			typedef typename Radio::block_data_t block_data_t;
			
			typedef DirectBrokerProtocolCommandMessage<OsModel> CommandMessage;
			typedef typename Broker::bitmask_t bitmask_t;
			typedef typename Broker::Tuple BrokerTuple;
			typedef typename Broker::document_name_t document_name_t;
			
			enum { MAX_SUBSCRIPTIONS = 8 };
			
			class TransactionTuple : public BrokerTuple {
				// {{{
				public:
					enum {
						SIZE = 5,
						COL_ACTION = 4
					};
					
					TransactionTuple() {
					}
					
					TransactionTuple(const TransactionTuple& other) { *this = other; }
					
					TransactionTuple& operator=(const TransactionTuple& other) {
						BrokerTuple::operator=(other);
						action_ = other.action_;
						return *this;
					}
					
					block_data_t* get(size_type i) {
						if(i == COL_ACTION) { return (block_data_t*)&action_; }
						return BrokerTuple::get(i);
					}
					
					size_type length(size_type i) {
						if(i == COL_ACTION) { return sizeof(uint8_t); }
						return BrokerTuple::length(i);
					}
					
					void set(size_type i, block_data_t* data) {
						if(i == COL_ACTION) { action_ = *(uint8_t*)data; }
						else { BrokerTuple::set(i, data); }
					}
					
					void set_deep(size_type i, block_data_t* data) {
						if(i == COL_ACTION) { action_ = *(uint8_t*)data; }
						else { BrokerTuple::set_deep(i, data); }
					}
					
					uint8_t action() { return action_; }
					
				private:
					uint8_t action_;
					
				// }}}
			}; // class TransactionTuple
			
			class Subscription {
				public:
					Subscription() : mask_(0), target_document_(0) {
					}
					
					~Subscription() {
						if(target_document_) {
							get_allocator().free_array(target_document_);
						}
					}
					
					bool used() { return mask_ != 0; }
					void unset_used() { mask_ = 0; }
					bitmask_t bitmask() { return mask_; }
					void set_bitmask(bitmask_t m) { mask_ = m; }
					node_id_t node() { return node_; }
					void set_node(node_id_t n) { node_ = n; }
					char* target_document() { return target_document_; }
					void set_target_document(char* t) {
						size_type l = strlen(t) + 1;
						target_document_ = get_allocator().allocate_array<char>(l).raw();
						memcpy((void*)target_document_, (void*)t, l);
					}
					
				private:
					bitmask_t mask_;
					node_id_t node_;
					char *target_document_;
			};
			
			typedef MapStaticVector<OsModel, uint8_t, TransactionTuple, 16> TransactionMap;
			
			int init(Broker& broker, typename Radio::self_pointer_t radio, typename Debug::self_pointer_t debug = 0) {
				broker_ = &broker;
				radio_ = radio;
				debug_ = debug;
				debug_ = 0;
				
				broker_->template subscribe<self_type, &self_type::on_document_changed>(this);
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				return OsModel::SUCCESS;
			}
			
			void on_receive(node_id_t source, size_type length, block_data_t* buffer) {
				typedef typename CommandMessage::transaction_id_t transaction_id_t;
				
				CommandMessage *message = (CommandMessage*)buffer;
				
				debug_->debug("B2B.on_receive(src=%d, len=%d, req type=%d)\n",
						source, length, message->request_type());
						
				document_name_t doc = 0, target_doc = 0;
				bitmask_t mask = 0;
				transaction_id_t tid = 0;
				
				switch(message->request_type()) {
					case CommandMessage::GET_DOCUMENT:
						doc = message->document_name();
						target_doc = message->target_document_name();
						send_document(source, target_doc, doc);
						break;
						
					case CommandMessage::SUBSCRIBE:
						for(size_type i=0; i<MAX_SUBSCRIPTIONS; i++) {
							Subscription& s = subscriptions_[i];
							if(!s.used()) {
								bitmask_t mask = broker_->get_document_mask(message->document_name());
								document_name_t target_docname = message->target_document_name();
								s.set_bitmask(mask);
								s.set_node(source);
								s.set_target_document(target_docname);
								break;
							}
						}
						break;
						
					case CommandMessage::UNSUBSCRIBE:
						for(size_type i=0; i<MAX_SUBSCRIPTIONS; i++) {
							Subscription& s = subscriptions_[i];
							if(s.node() == source) { s.unset_used(); }
						}
						break;
					
					case CommandMessage::TRANSACTION: {
						transaction_id_t tid = message->transaction_id();
						TransactionTuple& tuple = transaction_map_.operator[](tid);
						
						if(message->column() == CommandMessage::DOCUMENT_NAME) {
							doc = message->document_name();
							mask = broker_->get_document_mask(doc);
							tuple.set_deep(message->column(), (block_data_t*)&mask);
						}
						else {
							tuple.set_deep(message->column(), message->tuple_element());
						}
						
						if(message->commit_transaction()) {
							if(message->compressed()) {
								broker_->insert_compressed_tuple(tuple, tuple.bitmask());
							}
							else {
								broker_->insert_tuple(tuple, tuple.bitmask());
							}
							
							tuple.destruct_deep();
							transaction_map_.erase(tid);
						}
						break;
						}
						
					case CommandMessage::ERASE_DOCUMENT:
						doc = message->document_name();
						mask = broker_->get_document_mask(doc);
						broker_->erase_document(mask);
						break;
						
				}
				
				
			} // on_receive();
			
			void on_document_changed(char* docname) {
				bitmask_t mask = broker_->get_document_mask(docname);
				for(size_type i=0; i<MAX_SUBSCRIPTIONS; i++) {
					Subscription& s = subscriptions_[i];
					if(s.used() && (s.bitmask() & mask)) {
						send_document(s.node(), s.target_document(), docname);
					}
				}
			}
			
			void send_document(node_id_t target, document_name_t target_docname, document_name_t source_docname) {
				bitmask_t mask = broker_->get_document_mask(source_docname);
				
				debug_->debug("B2B.send_document(tgt=%d, mask=%d src_docname=%s tgt_docname=%s)\n", target, mask, source_docname, target_docname);
				
				CommandMessage message;
				message.set_request_type(CommandMessage::TRANSACTION);
				
				uint8_t request_id = 0;
				
				for(typename Broker::compressed_iterator iter = broker_->begin_compressed_document(mask); iter != broker_->end_compressed_document(mask); ++iter) {
					message.set_request_id(request_id);
					message.set_flags(CommandMessage::FLAG_COMPRESSED);
					
					// subject, predicate, object
					
					for(size_type column = CommandMessage::SUBJECT; column <= CommandMessage::OBJECT; column++) {
						message.set_column(column);
						message.set_tuple_element(iter->get(column));
						radio_->send(target, message.length(), (block_data_t*)&message);
					}
					
					// target document name
					
					message.set_column(CommandMessage::DOCUMENT_NAME);
					message.set_document_name(target_docname);
					radio_->send(target, message.length(), (block_data_t*)&message);
					
					// action (insert) & commit
					
					message.set_flags(CommandMessage::FLAG_COMPRESSED | CommandMessage::FLAG_COMMIT_TRANSACTION);
					block_data_t action[] = { CommandMessage::ACTION_INSERT, 0 };
					message.set_column(CommandMessage::ACTION);
					message.set_tuple_element(action);
					radio_->send(target, message.length(), (block_data_t*)&message);
					
					request_id++;
				}
			}
			
		private:
			typename Broker::self_pointer_t broker_;
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			TransactionMap transaction_map_;
			Subscription subscriptions_[MAX_SUBSCRIPTIONS];
	};
	
} // namespace wiselib

#endif // DIRECT_BROKER_PROTOCOL_H
	
// vim: set foldmethod=marker:

