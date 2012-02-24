
#ifndef SELF_STABILIZING_SCHEDULER_H
#define SELF_STABILIZING_SCHEDULER_H

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename NeighborDiscovery_P,
		typename State_P,
		typename Debug_P,
		typename Allocator_P,
		OsModel_P::size_t MAX_GCS = 10
	>
	class Scheduler {
		public:
			typedef OsModel_P OsModel;
			typedef NeighborDiscovery_P NeighborDiscovery;
			typedef State_P State;
			typedef Debug_P Debug;
			typedef Allocator_P Allocator;
			
			typedef OsModel::size_t size_t;
			
			typedef list_dynamic<OsModel, State:self_pointer_t, Allocator> neighborhood_t;
			
			typedef delegate1<bool, neighborhood_t::self_pointer_t> guard_delegate_t;
			typedef delegate1<bool, neighborhood_t::self_pointer_t> command_delegate_t;
			
			Scheduler()
				: index_(0) {
				for(size_t i=0; i<MAX_GCS; i++) {
					guards_[i] = 0;
					commands_[i] = 0;
				}
			}
			
			void addGuardedCommand(guard_delegate_t guard, command_delegate_t command) {
				// TODO
			}
			
			void step() {
				// TODO: Get state from neighbors
				execute_next_();
				// TODO: Publish new state to neighbors
			}
			
		private:
			void execute_next_() {
				int old_idx = index_;
				for(index_++; index_ < MAX_GCS; index_++) {
					if(commands_[index_]) {
						if(guards_[index_](neighborhood_)) {
							commands_[index_](neighborhood_);
						}
						return;
					}
				}
				
				for(index_ = 0; index_ < old_idx; index_++) {
					if(commands_[index_]) {
						if(guards_[index_](neighborhood_)) {
							commands_[index_](neighborhood_);
						}
						return;
					}
				}
			}
				
			int index_;
			guard_delegate_t guards_[MAX_GCS];
			command_delegate_t commands_[MAX_GCS];
	};
	
}

#endif // SELF_STABILIZING_SCHEDULER_H

