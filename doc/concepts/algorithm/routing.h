#ifndef ROUTING_H
#define ROUTING_H
// vim: ts=3:sw=3

#include "concepts/basic/basic_algorithm.h"
#include "concepts/extiface/radio.h"
#include "concepts/basic/basic_return_values.h"

namespace wiselib
{
namespace concept_check
{

template <class X>
struct Routing
   : public BasicAlgorithm<X>,
     public Radio<X>,
     public BasicReturnValues<X>
{
   public:
      typedef typename X::Radio           Radio;
      typedef typename X::self_type       self_type;

      BOOST_CONCEPT_USAGE(Routing)
      {
         same_type(routing, self_type_);
      }

      BOOST_CONCEPT_ASSERT((concept_check::Radio<Radio>));

   private:
      X routing;
      self_type self_type_;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif /* end of include guard: ROUTING_H */
