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

#ifndef SEMANTIC_ENTITY_ID_H
#define SEMANTIC_ENTITY_ID_H

#include <external_interface/external_interface.h>
#include <util/serialization/serialization.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 */
	class SemanticEntityId {
		public:
			typedef ::uint32_t Value;
			typedef ::uint32_t Rule;
			
			enum SpecialRules {
				RULE_SPECIAL = (Rule)(-1),
			};
			
			enum SpecialValues {
				VALUE_INVALID = (Value)(-1),
				VALUE_ALL = 0,
				VALUE_ROOT = 1,
			};
			
			/**
			 * Create the invalid SE id.
			 */
			SemanticEntityId() : value_(VALUE_INVALID), rule_(RULE_SPECIAL) {
			}
			
			SemanticEntityId(Rule r, Value v) : value_(v), rule_(r) {
			}
			
			bool operator==(const SemanticEntityId& other) const {
				return value_ == other.value_ && rule_ == other.rule_;
			}
			
			bool operator!=(const SemanticEntityId& other) const {
				return !(*this == other);
			}
			
			bool operator<(const SemanticEntityId& other) const {
				return rule_ < other.rule_ || (rule_ == other.rule_ && value_ < other.value_);
			}
			
			Value value() const { return value_; }
			Rule rule() const { return rule_; }
			
			::uint8_t hash8() const {
				::uint32_t a = rule_ ^ value_;
				::uint16_t b = ((a * 101) & 0xffff) ^ ((a * 127) >> 16);
				return ((b * 5) & 0xff) ^ ((b * 11) >> 8);
			}
			
			::uint8_t hash() const { return hash8(); }
			
			void set(Rule r, Value v) {
				rule_ = r;
				value_ = v;
			}
			
			///@name Special values
			///@{
			
			static SemanticEntityId invalid() { return SemanticEntityId(); }
			static SemanticEntityId all() { return SemanticEntityId(RULE_SPECIAL, VALUE_ALL); }
			static SemanticEntityId root() { return SemanticEntityId(RULE_SPECIAL, VALUE_ROOT); }
			
			bool is_special() const { return rule_ == RULE_SPECIAL; }
			bool is_valid() const { return !is_special() || value_ != VALUE_INVALID; }
			bool is_invalid() const { return is_special() && value_ == VALUE_INVALID; }
			bool is_all() const { return is_special() && value_ == VALUE_ALL; }
			bool is_root() const { return is_special() && value_ == VALUE_ROOT; }
			
			///@}
			
		private:
			Value value_;
			Rule rule_;
			
		template<
			typename OsModel_,
			Endianness Endianness_,
			typename BlockData_,
			typename T
		>
		friend class Serialization;
		
	}; // SemanticEntityId
	
	
	/*
	template<
		typename OsModel_P,
		Endianness Endianness_P,
		typename BlockData_P
	>
	struct Serialization<OsModel_P, Endianness_P, BlockData_P, SemanticEntityId> {
		typedef OsModel_P OsModel;
		typedef BlockData_P block_data_t;
		typedef typename SemanticEntityId::Rule Rule;
		typedef typename SemanticEntityId::Value Value;
		typedef typename OsModel::size_t size_type;
		
		static size_type write(block_data_t *data, SemanticEntityId& value) {
			wiselib::write<OsModel>(data, value.rule_); data += sizeof(Rule);
			wiselib::write<OsModel>(data, value.value_); data += sizeof(Value);
			
			//DBG("writing SE ID r=%d v=%d %02x %02x %02x %02x %02x");
			return sizeof(Rule) + sizeof(Value);
		}
		
		static SemanticEntityId read(block_data_t *data) {
			DBG("reading SE ID !!!");
			SemanticEntityId value;
			wiselib::read<OsModel>(data, value.rule_); data += sizeof(Rule);
			wiselib::read<OsModel>(data, value.value_); data += sizeof(Value);
			return value;
		}
	};
	*/
}

#endif // SEMANTIC_ENTITY_ID_H


