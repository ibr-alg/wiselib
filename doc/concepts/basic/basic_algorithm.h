// vim: ts=3:sw=3

template <class X>
struct BasicAlgorithmConcept
{
   public:
      BOOST_CONCEPT_USAGE(BasicAlgorithmConcept)
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
