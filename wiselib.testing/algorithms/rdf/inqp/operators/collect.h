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

#ifndef COLLECT_H
#define COLLECT_H

#include "operator.h"
#include "../row.h"
#include "../operator_descriptions/collect_description.h"
#include <util/types.h>

#ifdef PC
#include <iostream>
#include <iomanip>
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
		typename Processor_P
	>
	class Collect : public Operator<OsModel_P, Processor_P> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Processor_P Processor;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef Collect<OsModel, Processor> self_type;
			
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpmf-conversions"
			void init(CollectDescription<OsModel, Processor> *cd, Query *query) {
				//DBG("Collect init");
				
				Base::init(reinterpret_cast<OperatorDescription<OsModel, Processor>* >(cd), query);
			
				//this->push_ = reinterpret_cast<typename Base::my_push_t>(&self_type::push);
				hardcore_cast(this->push_, &self_type::push);
			}
			#pragma GCC diagnostic pop
			
			void push(size_type port, Row<OsModel>& row) {
				if(&row) {
				
					for(size_type i = 0; i < this->child(Base::CHILD_LEFT).columns(); i++) {
						DBG("result row [%lu] = %08x", i, row[i]);
					}
					
					this->processor().send_row(
							Base::Processor::COMMUNICATION_TYPE_SINK,
							this->child(Base::CHILD_LEFT).columns(),
							row,
							this->query().id(),
							this->id()
					);
				}
				
				
				/*
			#if defined(PC) || defined(SHAWN)
				std::cout << "[";
				for(size_type i = 0; i < this->child(Base::CHILD_LEFT).columns(); i++) {
					std::cout << row[i] << " ";
				}
				std::cout << "]" << std::endl;
				
				for(size_type i = 0; i < this->child(Base::CHILD_LEFT).columns(); i++) {
					std::cout << "  " << row[i] << ": "
						<< comm_state_->dictionary().get_value(row[i]) << std::endl;
				}
				
			#endif // PC
				*/
				
			}
			
			void execute() {
				//DBG("Collect execute");
			}
			
		private:
	}; // Collect
}

#endif // COLLECT_H

