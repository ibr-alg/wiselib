
#ifndef __WISELIB_ALGORITHMS_SE_CONSTRUCTION_H
#define __WISELIB_ALGORITHMS_SE_CONSTRUCTION_H

#pragma warning("SE construction is not usable yet!")

#include "util/pstl/list_dynamic.h"
#include "se_construction_message.h"
#include "util/serialization/endian.h"

#define SE_CONSTRUCTION_DEBUG 1

namespace wiselib {
	
	/**
	 * Semantic Entity Construction Process
	 */
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SEConstruction {
		
			struct ClassInfo;
		public:
			typedef SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			static const Endianness endianness = OsModel::endianness;
			typedef Allocator_P Allocator;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef typename Radio::message_id_t message_id_t;
			
			typedef SEConstructionMessage<Radio, ClassInfo> message_t;
			
			typedef typename Allocator::template Ref<ClassInfo>::pointer_t classinfo_ptr_t;
			typedef list_dynamic<OsModel, classinfo_ptr_t, Allocator> ClassContainer;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_CLASSNAME_LENGTH = 64,
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
					- (2*sizeof(node_id_t) + sizeof(size_t))
			};
			
			enum AdvertiseIntervals {
				CONSTRUCTING_INTERVAL = 5 * 1000,
				OPERATING_INTERVAL = 60 * 1000
			};
			
			int init(typename Allocator::self_pointer_t, typename Radio::self_pointer_t, typename Timer::self_pointer_t, typename Debug::self_pointer_t = 0);
			int init();
			int destruct();
			
			void addClass(size_t, const block_data_t*);
			
			void advertise(void* _=0);
			void timed_advertise(void* _=0);
			void on_receive(node_id_t, size_t, block_data_t*);
			
			#if SE_CONSTRUCTION_DEBUG
			void show_debug_info() {
				for(typename ClassContainer::iterator iter = classes_.begin(); iter != classes_.end(); ++iter) {
					debug_->debug("Node %3d class %-10s state %2d\n", radio_->id(), (*iter)->name, (*iter)->state);
				}
				debug_->debug("\n");
			}
			#endif // SE_CONSTRUCTION_DEBUG
			
			template<typename DebugPtr>
			void label(DebugPtr d) {
				char buf[256];
				char *b = buf;
				for(typename ClassContainer::iterator iter = classes_.begin(); iter != classes_.end(); ++iter) {
					if((*iter)->state == ClassInfo::STATE_OPEN) { *b++ = '['; }
					b += snprintf(b, buf - b - 1, "%s(%3x)", (*iter)->name, (*iter)->leader);
					if((*iter)->state == ClassInfo::STATE_OPEN) { *b++ = ']'; }
					*b++ = ' ';
				}
				*b++ = '\0';
				d->debug("%3x: %s", radio_->id(), buf);
			}
			
			// TODO: Add radio interface for entity wide communication
			// use leader-tree/bcast + sequence numbers (for identifying
			// wheter or not a node has received a message)
			/*
			int enable_radio();
			int disable_radio();
			int send(node_id_t, size_t, block_data_t*)
			node_id_t id() { return radio_->id(); }
			template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
			int reg_recv_callback(T *obj_pnt);
			int unreg_recv_callback(int idx);
			*/
			
			/// For debugging only
			ClassContainer& classes() { return classes_; }
			
		private:
			struct ClassInfo {
				enum State { STATE_OPEN, STATE_SE };
				typename Allocator::template Ref<block_data_t>::pointer_t ClassContainer;
				void init() {
					state = STATE_OPEN;
					leader = 0; //Radio::NULL_NODE_ID;
					to_leader = 0; //Radio::NULL_NODE_ID;
					name_size = 0;
				}
				int cmp(size_t sz, const block_data_t* data) {
					if(name_size != sz) return name_size < sz ? -1 : 1;
					for(size_t i=0; i<sz; ++i) {
						if(name[i] != data[i]) return name[i] < data[i] ? -1 : 1;
					}
					return 0;
				}
				
				bool sameClassAs(const classinfo_ptr_t other) const {
					if(name_size != other->name_size) return false;
					for(size_t i=0; i<name_size; ++i) {
						if(name[i] != other->name[i]) return false;
					}
					return true;
				}
				
				size_t write_to(block_data_t* target) {
					block_data_t *start = target;
					Serialization<OsModel, endianness, block_data_t, uint8_t>::write(target, state);
					target += sizeof(uint8_t);
					Serialization<OsModel, endianness, block_data_t, node_id_t>::write(target, leader);
					target += sizeof(node_id_t);
					Serialization<OsModel, endianness, block_data_t, node_id_t>::write(target, to_leader);
					target += sizeof(node_id_t);
					size_t i=0;
					while(i<name_size) *(target++) = name[i++];
					return target - start;
				}
				
				void read_from(size_t size, block_data_t* source) {
					state = Serialization<OsModel, endianness, block_data_t, uint8_t>::read(source);
					source += sizeof(uint8_t);
					leader = Serialization<OsModel, endianness, block_data_t, node_id_t>::read(source);
					source += sizeof(node_id_t);
					to_leader = Serialization<OsModel, endianness, block_data_t, node_id_t>::read(source);
					source += sizeof(node_id_t);
					name_size = size - sizeof(uint8_t) - 2*sizeof(node_id_t);
					size_t i=0;
					while(i<name_size) name[i++] = *(source++);
				}
				
				uint8_t state;
				node_id_t leader, to_leader;
				size_t name_size;
				block_data_t name[0];
			};
			
			
			typename Radio::self_pointer_t radio_;
			ClassContainer classContainer_;
			typename Timer::self_pointer_t timer_;
			
			ClassContainer classes_;
			typename Allocator::self_pointer_t allocator_;
			typename Debug::self_pointer_t debug_;
			
			size_t advertise_interval_;
	};
	
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P,
		typename Debug_P
	>
	int
	SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P, Debug_P>::
	init(
			typename Allocator::self_pointer_t allocator,
			typename Radio::self_pointer_t radio,
			typename Timer::self_pointer_t timer,
			typename Debug::self_pointer_t debug
	) {
		allocator_ = allocator;
		radio_ = radio;
		timer_ = timer;
		debug_ = debug;
		advertise_interval_ = CONSTRUCTING_INTERVAL;
		classes_.set_allocator(*allocator_);
		radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
		radio_->enable_radio();
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P,
		typename Debug_P
	>
	int
	SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P, Debug_P>::
	destruct() {
		// TODO: Stop timers, etc...
		
		for(typename ClassContainer::iterator iter = classes_.begin(); iter != classes_.end(); ++iter) {
			allocator_->free(*iter);
		}
		classes_.clear();
		
		return SUCCESS;
	}
	
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P,
		typename Debug_P
	>
	void
	SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P, Debug_P>::
	addClass(size_t size, const block_data_t* data) {
		// check if we already have that class
		for(typename ClassContainer::iterator iter = classes_.begin(); iter != classes_.end(); ++iter) {
			if((*iter)->cmp(size, data) == 0) {
				return;
			}
		}
		
		classinfo_ptr_t info = allocator_->template allocate<ClassInfo>(size);
		info->init();
		for(size_t i=0; i<size; ++i) {
			info->name[i] = data[i];
		}
		info->name_size = size;
		
		classes_.push_back(info);
	}
	
	
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P,
		typename Debug_P
	>
	void
	SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P, Debug_P>::
	advertise(void*) {
		message_t msg;
		msg.setType(message_t::MSG_STATE);
		
		typename ClassContainer::iterator iter;
		for(iter = classes_.begin(); iter != classes_.end(); ++iter) {
			msg.pushClassInfo(**iter);
		}
		
		radio_->send(Radio::BROADCAST_ADDRESS, msg.size(), msg.data());
	}
	
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P,
		typename Debug_P
	>
	void
	SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P, Debug_P>::
	timed_advertise(void*) {
		show_debug_info();
		advertise();
		timer_->template set_timer<self_type, &self_type::timed_advertise>(advertise_interval_, this, 0);
	}
	
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename Timer_P,
		typename Debug_P
	>
	void
	SEConstruction<OsModel_P, Allocator_P, Radio_P, Timer_P, Debug_P>::
	on_receive(node_id_t sender, size_t size, block_data_t* data) {
		if(sender == radio_->id()) { return; }
		
		message_t msg(size, data);
		typename ClassContainer::iterator iter;
		typename message_t::iterator miter;
		classinfo_ptr_t m = allocator_->template allocate<ClassInfo>(MAX_CLASSNAME_LENGTH);
		bool have_open_classes = false, trigger_advertise = false;
		
		for(iter = classes_.begin(); iter != classes_.end(); ++iter) {
			for(miter = msg.begin(); miter != msg.end(); ++miter) {
				//debug_->debug("buffer=%lx size=%d end buffer=%lx\n", miter.buffer_, miter.size(), msg.end().buffer_);
				m->read_from(miter.size(), *miter);
				
				if(m->sameClassAs(*iter)) {
					// other open, we SE -> advertise state
					if(m->state == ClassInfo::STATE_SE && (*iter)->state == ClassInfo::STATE_OPEN) {
						trigger_advertise = true;
					}
					
					// both open -> create (state := SE, leader := this)
					else if(m->state == ClassInfo::STATE_OPEN && (*iter)->state == ClassInfo::STATE_OPEN) {
						(*iter)->state = ClassInfo::STATE_SE;
						(*iter)->to_leader = radio_->id();
						(*iter)->leader = radio_->id();
						trigger_advertise = true;
					}
					
					// other SE, we open -> join (state := SE, leader := other)
					else if(m->state == ClassInfo::STATE_SE && (*iter)->state == ClassInfo::STATE_OPEN) {
						(*iter)->state = ClassInfo::STATE_SE;
						(*iter)->to_leader = sender;
						(*iter)->leader = m->leader;
						trigger_advertise = true;
					}
					
					// both SE, different leaders -> merge (leader := max(otherleader, leader))
					else if(m->state == ClassInfo::STATE_SE && (*iter)->state == ClassInfo::STATE_SE) {
						if((*iter)->leader < m->leader) {
							(*iter)->leader = m->leader;
							(*iter)->to_leader = sender;
							trigger_advertise = true;
						}
					}
				} // if same class
			} // for message classes
			
			if((*iter)->state == ClassInfo::STATE_OPEN) { have_open_classes = true; }
		} // for own classes
		
		advertise_interval_ = have_open_classes ? CONSTRUCTING_INTERVAL : OPERATING_INTERVAL;
		
		allocator_->template free<ClassInfo>(m);
		
		if(trigger_advertise) {
			advertise();
		}
		
		//allocator_->print_stats(debug_);
	} // on_receive
	
} // namespace

#endif // SE_CONSTRUCTION_H

