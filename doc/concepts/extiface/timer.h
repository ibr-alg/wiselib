#ifndef TIMER_H
#define TIMER_H
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct Timer
{
   public:
      typedef typename X::OsModel OsModel;
      typedef typename X::millis_t millis_t;

      BOOST_CONCEPT_USAGE(Timer)
      {
          same_type(int_, timer.template set_timer<callback, &callback::method>(millis, &callback_obj, (void *) 0));
      }

      // BOOST_CONCEPT_ASSERT((concept_check::Os<OsModel>));

   private:
      X timer;
      int int_;
      millis_t millis;

      class callback {
         public:
            void method(void *userdata) {}
      };
      callback callback_obj;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif /* end of include guard: TIMER_H */
