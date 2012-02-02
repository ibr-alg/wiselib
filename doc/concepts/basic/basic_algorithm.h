#ifndef BASIC_ALGORITHM_H
#define BASIC_ALGORITHM_H
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct BasicAlgorithm
{
   public:
      BOOST_CONCEPT_USAGE(BasicAlgorithm)
      {
         same_type(val_int, basic_algorithm.init());
         same_type(val_int, basic_algorithm.destroy());
      }

   private:
      X basic_algorithm;
      int val_int;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif /* end of include guard: BASIC_ALGORITHM_H */
