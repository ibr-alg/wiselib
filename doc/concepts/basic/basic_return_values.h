// vim: ts=3:sw=3

template <class X>
struct BasicReturnValuesConcept
{
   public:
      typedef typename X::ErrorCodes   ErrorCodes;
      typedef typename X::StateValues  StateValues;

      BOOST_CONCEPT_USAGE(BasicReturnValuesConcept)
      {
         same_type(ec, X::SUCCESS);
         same_type(ec, X::ERR_UNSPEC);
         same_type(ec, X::ERR_NOMEM);
         same_type(ec, X::ERR_BUSY);
         same_type(ec, X::ERR_NOTIMPL);
         same_type(ec, X::ERR_NETDOWN);
         same_type(ec, X::ERR_HOSTUNREACH);

         same_type(sv, X::READY);
         same_type(sv, X::NO_VALUE);
         same_type(sv, X::INACTIVE);
      }

   private:
      X basic_return_values;

      ErrorCodes ec;
      StateValues sv;

      template <typename T>
      void same_type(const T&, const T&);
};
