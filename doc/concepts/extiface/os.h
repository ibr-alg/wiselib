#ifndef WISELIB_CONCEPT_CHECK_OS
#define WISELIB_CONCEPT_CHECK_OS
// vim: ts=3:sw=3

#include "/home/jan/ba/wiselib/doc/concepts/basic/basic_return_values.h"
#include "/home/jan/ba/wiselib/doc/concepts/extiface/radio.h"

template <class X>
struct OsConcept : public BasicReturnValuesConcept<X>
{
   public:
      typedef typename X::size_t          size_t;
      typedef typename X::block_data_t    block_data_t;
      typedef typename X::Radio           Radio;
      // TODO timer & debug typedefs

      BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<size_t>));
      BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<block_data_t>));
      BOOST_CONCEPT_ASSERT((RadioConcept<Radio>));
};

#endif  // WISELIB_CONCEPT_CHECK_RADIO
