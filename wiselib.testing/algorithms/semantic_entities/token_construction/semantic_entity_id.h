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

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 */
	class SemanticEntityId {
		public:
			typedef ::uint32_t Value;
			typedef ::uint8_t Rule;
			
			/**
			 * Create the invalid SE id.
			 */
			SemanticEntityId() : value_(-1), rule_(-1) {
			}
			
			SemanticEntityId(Rule r, Value v) : value_(v), rule_(r) {
			}
			
			bool operator==(SemanticEntityId& other) {
				return value_ == other.value_ && rule_ == other.rule_;
			}
			bool operator<(SemanticEntityId& other) {
				return rule_ < other.rule_ || (rule_ == other.rule_ && value_ < other.value_);
			}
			
			Value value() { return value_; }
			Rule rule() { return rule_; }
			
		private:
			Value value_;
			Rule rule_;
	}; // SemanticEntityId
}

#endif // SEMANTIC_ENTITY_ID_H


