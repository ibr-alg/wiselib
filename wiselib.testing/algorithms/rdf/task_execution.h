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

#ifndef TASK_EXECUTION_H
#define TASK_EXECUTION_H

#include <util/pstl/set_vector.h>
#include <util/pstl/vector_static.h>

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
		typename Broker_P,
		typename QueryProcessor_P,
		typename Timer_P = typename OsModel_P::Timer
	>
	class TaskExecution {
		public:
			typedef TaskExecution self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Broker_P Broker;
			typedef typename Broker::document_name_t document_name_t;
			typedef QueryProcessor_P QueryProcessor;
			typedef typename QueryProcessor::query_id_t query_id_t;
			typedef Timer_P Timer;
			
			typedef set_vector<OsModel, vector_static<Os, query_id_t, 8> > TaskIds;
			
			enum {
				EXECUTE_DELAY = 5000
			};
			
			void init(typename Broker::self_pointer_t broker, typename QueryProcessor::self_pointer_t qp, typename Timer::self_pointer_t timer) {
				broker_ = broker;
				query_processor_ = qp;
				timer_ = timer;
				
				execution_planned_ = false;
				
				broker_->template subscribe<self_type, &self_type::on_document_changed>(this);
			}
			
			/*
			Query& create_task(query_id_t qid) {
				Query& q = *query_processor_->create_query(qid);
				plan_execution();
				return q;
			}
			*/
			
			/**
			 * Add the query already present in query_processor, identified by
			 * qid to the list of tasks to be watched over.
			 */
			void add_task(query_id_t qid) {
				tasks_.insert(qid);
				plan_execution();
			}
		
		private:
			void plan_execution() {
				if(!execution_planned_) {
					execution_planned_ = true;
					timer_->template set_timer<self_type, &self_type::execute>(EXECUTE_DELAY, this, 0);
				}
			}
			
			void on_document_changed(document_name_t docname) {
				plan_execution();
			}
			
			void execute(void* = 0) {
				execution_planned_ = false;
				
				for(typename TaskIds::iterator iter = tasks_.begin(); iter != tasks_.end(); ++iter) {
					DBG("executing task %d", (int)*iter);
					query_processor_->execute(
							query_processor_->get_query(*iter)
					);
				}
			}
			
			bool execution_planned_;
			TaskIds tasks_;
			typename QueryProcessor::self_pointer_t query_processor_;
			typename Broker::self_pointer_t broker_;
			typename Timer::self_pointer_t timer_;
			
	}; // TaskExecution
}

#endif // TASK_EXECUTION_H

