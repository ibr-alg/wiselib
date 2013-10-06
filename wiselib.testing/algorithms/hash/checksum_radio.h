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

#ifndef CHECKSUM_RADIO_H
#define CHECKSUM_RADIO_H

#include <util/base_classes/radio_base.h>
#include <util/base_classes/extended_radio_base.h>
#include <util/serialization/serialization.h>

#define CHECKSUM_RADIO_USE_REVISION 0
#if !defined(REVISION)
	#define REVISION 0x12345678
#endif

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
		typename Hash_P,
		typename Debug_P = typename OsModel_P::Debug
	>
            //int MAX_RECEIVERS = RADIO_BASE_MAX_RECEIVERS,
            //typename ExtendedData_P = BaseExtendedData<OsModel_P>
	class ChecksumRadio
		: public ExtendedRadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t, RADIO_BASE_MAX_RECEIVERS, typename Radio_P::ExtendedData> {
		
		public:
			typedef OsModel_P OsModel;
			typedef Radio_P Radio;
			typedef typename Radio::size_t size_type;
			typedef typename Radio::message_id_t message_id_t;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef Debug_P Debug;
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
			typedef ChecksumRadio self_type;
			typedef self_type* self_pointer_t;
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS //, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID,
			};
			
			typedef ::uint32_t revision_t;
			enum {
				#if CHECKSUM_RADIO_USE_REVISION
					HEADER_SIZE = sizeof(hash_t) + sizeof(revision_t),
				#else
					HEADER_SIZE = sizeof(hash_t)
				#endif
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - HEADER_SIZE
			};
			
			int init(Radio& radio, Debug& debug) {
				radio_ = &radio;
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				debug_ = &debug;
				return SUCCESS;
			}
			
			node_id_t id() { return radio_->id(); }
			
			int enable_radio() { return radio_->enable_radio(); }
			int disable_radio() { return radio_->disable_radio(); }
			
			int send(node_id_t dest, size_t len, block_data_t *data) {
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t *p = buf;
				
				hash_t h = Hash::hash(data, len);
				//DBG("checksum send: %04lx", (unsigned long)h);
				wiselib::write<OsModel, block_data_t, hash_t>(p, h);
				p += sizeof(hash_t);
				
			#if CHECKSUM_RADIO_USE_REVISION
				revision_t rev = REVISION;
				wiselib::write<OsModel, block_data_t, revision_t>(p, rev);
				p += sizeof(revision_t);
			#endif
				
				memcpy(p, data, len);
				radio_->send(dest, len + HEADER_SIZE, buf);
				return SUCCESS;
			}
			
		#ifdef SHAWN
			void on_receive(typename Radio::node_id_t from, typename Radio::size_t len, typename Radio::block_data_t *data) {
				typename Radio::ExtendedData ex;
				//ex.set_link_metric(300);
		#else
			void on_receive(typename Radio::node_id_t from, typename Radio::size_t len, typename Radio::block_data_t *data, const typename Radio::ExtendedData& ex) {
		#endif
				
				DBG("@%lu chksum from %lu", (unsigned long)id(), (unsigned long)from);
				
				if(len < HEADER_SIZE) {
					#ifdef SHAWN
					debug_->debug("msg too short: %d < %d", (int)len, (int)HEADER_SIZE);
					#endif
					return;
				}
				
				// hardcoded weird sources that send confusing things
				//if(from == 49465) { return; }
				
				// checksum
				hash_t h_msg = wiselib::read<OsModel, block_data_t, hash_t>(data);
				hash_t h_check = Hash::hash(data + HEADER_SIZE, len - HEADER_SIZE);
				if(h_msg != h_check) {
					#ifdef SHAWN
					debug_->debug("@%lu !C %x,%x", (unsigned long)id(), (unsigned)h_msg, (unsigned)h_check);
					#endif
					return;
				}
				
				// revision
			#if CHECKSUM_RADIO_USE_REVISION
				revision_t rev = wiselib::read<OsModel, block_data_t, revision_t>(data + sizeof(hash_t));
				if(rev != REVISION) {
					debug_->debug("@%lu %lu !R %lx,%lx", (unsigned long)id(), (unsigned long)from, (unsigned long)REVISION, (unsigned long)rev);
					return;
				}
			#endif
				
			#ifdef SHAWN
				this->notify_receivers(from, (typename Radio::size_t)(len - HEADER_SIZE), (typename Radio::block_data_t*)(data + HEADER_SIZE));
			#else
				this->notify_receivers(from, (typename Radio::size_t)(len - HEADER_SIZE), (typename Radio::block_data_t*)(data + HEADER_SIZE), ex);
			#endif
			}
		
		private:
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			
	}; // ChecksumRadio
}

#endif // CHECKSUM_RADIO_H

