#ifndef REQUEST_H
#define REQUEST_H
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct Request
{
   public:
      typedef typename X::OsModel OsModel;
      typedef typename X::value_t value_t;

      BOOST_CONCEPT_USAGE(Request)
      {
         same_type(value_t_, request());
      }

   private:
      X request;
      value_t value_t_;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif /* end of include guard: REQUEST_H */
