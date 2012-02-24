
#ifndef SELF_STABILIZING_MULTIGROUPING_H
#define SELF_STABILIZING_MULTIGROUPING_H

namespace wiselib
{
   template<
      typename OsModel_P,
      typename Allocator_P
   >
   class SelfStabilizingMultigrouping {
      public:
         typedef OsModel_P OsModel;
         typedef Allocator_P Allocator;
         typedef Property_P Property;
         
         typedef vector_dynamic< ... > buffer_t;
         typedef typename buffer_t::iterator_t buffer_iterator_t;
         
      private:
         class State {
               list_dynamic<Property> properties_;
            public:
               
               static void write(buffer_iterator_t& iter, State& state) {
                  // TODO
               }
               
               static State read(buffer_iterator_t& iter) {
                  // TODO
               }
         };
         
      public:
         typedef SelfStabilizingMultigrouping< ... > self_type;
         typedef typename Allocator::pointer_t<self_type> self_pointer_t;
         
         
         
         int init() {
         }
         
         int destruct() {
         }
   }
   
} // ns


#endif // SELF_STABILIZING_MULTIGROUPING_H

// vim: set ts=3 sw=3 expandtab:
