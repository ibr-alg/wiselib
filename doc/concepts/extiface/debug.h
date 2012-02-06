#ifndef DEBUG_H
#define DEBUG_H
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct Debug
{
   public:
      typedef typename X::OsModel OsModel;

      BOOST_CONCEPT_USAGE(Debug)
      {
         debug.debug("%d", int_);
         debug.debug("%d %c", int_, char_);
         debug.debug("%d %c %f", int_, char_, double_);
      }

      // BOOST_CONCEPT_ASSERT((concept_check::Os<OsModel>));

   private:
      X debug;

      int int_;
      char *char_;
      double double_;
};

}
}

#endif /* end of include guard: DEBUG_H */
