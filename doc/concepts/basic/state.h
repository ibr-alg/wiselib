#ifndef STATE_H
#define STATE_H
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct State
{
   public:
      BOOST_CONCEPT_USAGE(State)
      {
         same_type(st, X::READY);
         same_type(st, X::NO_VALUE);
         same_type(st, X::INACTIVE);
         same_type(st, X::DATA_AVAILABLE);

         same_type(int_, state.state());
      }

   private:
      X state;
      State st;
      int int_;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif /* end of include guard: STATE_H */
