#ifndef WISELIB_CONCEPT_CHECK_OS
#define WISELIB_CONCEPT_CHECK_OS
// vim: ts=3:sw=3

#include "concepts/basic/basic_return_values.h"
#include "concepts/extiface/radio.h"
#include "concepts/extiface/timer.h"
#include "concepts/extiface/debug.h"

namespace wiselib
{
namespace concept_check
{

template <class X>
struct Os : public BasicReturnValues<X>
{
   public:
      typedef typename X::size_t          size_t;
      typedef typename X::block_data_t    block_data_t;
      typedef typename X::Radio           Radio;
      typedef typename X::Timer           Timer;
      typedef typename X::Debug           Debug;

      BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<size_t>));
      BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<block_data_t>));
      BOOST_CONCEPT_ASSERT((concept_check::Radio<Radio>));
      BOOST_CONCEPT_ASSERT((concept_check::Timer<Timer>));
      BOOST_CONCEPT_ASSERT((concept_check::Debug<Debug>));
};

}
}

#endif  // WISELIB_CONCEPT_CHECK_RADIO
