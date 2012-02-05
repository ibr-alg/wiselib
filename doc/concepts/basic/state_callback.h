#ifndef STATE_CALLBACK_H
#define STATE_CALLBACK_H
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct StateCallback : public BasicReturnValues<X>
{
   public:
      typedef typename X::OsModel OsModel;
      typedef typename X::value_t value_t;

      BOOST_CONCEPT_ASSERT((concept_check::Os<OsModel>));

      BOOST_CONCEPT_USAGE(StateCallback)
      {
         same_type(st, X::READY);
         same_type(st, X::NO_VALUE);
         same_type(st, X::INACTIVE);
         same_type(st, X::DATA_AVAILABLE);

         same_type(int_, state_callback.template register_state_callback<callback, &callback::method>(&callback_obj));
         same_type(int_, state_callback.unregister_state_callback(int_));
      }

   private:
      X state_callback;
      State st;
      int int_;

      struct callback {
         void method(int state) {}
      };
      callback callback_obj;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif /* end of include guard: STATE_CALLBACK_H */
