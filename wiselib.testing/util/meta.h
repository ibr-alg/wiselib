
#ifndef META_H
#define META_H


/**
 * Provide for binary literals (up to 8 bits).
 * Usage:
 * BIN(11) --> 3
 * BIN(01010) --> 10
 * 
 * (Leading zero is not required but always allowed)
 */
#define BIN(X) Binary< 0 ## X >::value

template<unsigned long long N_>
struct Binary { enum { value = (N_ % 8) + 2 * Binary<N_ / 8>::value }; };
template<>
struct Binary<0> { enum { value = 0}; };


// source: http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence/257382#257382

#define HAS_METHOD(func, name) \
	template<typename T, typename Sig> struct name { \
		typedef char yes[1]; typedef char no[2]; \
		template<typename U, U> struct type_check; \
		template<typename C> static yes& check(type_check<Sig, &C::func>*); \
		template<typename C> static no& check(...); \
		static bool const value = (sizeof(check<T>(0)) == sizeof(yes)); \
	};
	

#define ENABLE_IF(C, T) typename enable_if_c<C, T>::type

template<bool C_, typename T = void>
struct enable_if_c; // { typedef T type; };

template<typename T>
struct enable_if_c<true, T> { typedef T type; };

template<typename Cond, class T = void>
struct enable_if : public enable_if_c<Cond::value, T> { };

/*
 * STATIC PRINT
 */
#define static_print(X) typedef StaticPrint<X> _print_ ## __LINE__
template<int x> struct StaticPrint;


/*
 * STATIC ASSERTIONS
 */

#define static_assert(X) typedef static_assert_test<sizeof(STATIC_ASSERT_FAILURE< X >)> _static_assert_ ## __LINE__

/**
 * Not instantiable for B = false
 * so STATIC_ASSERT_FAILURE pops up in the error messages
 */
template<bool B> struct STATIC_ASSERT_FAILURE;
template<> struct STATIC_ASSERT_FAILURE<true> { };
template<int x> struct static_assert_test { };


/**
 */
template<size_t a, size_t b>
struct Max { static const size_t value = a > b ? a : b; };

/*
 * Calculate ceil(a / b)
 */
template<size_t a, size_t b>
struct DivCeil { static const size_t value = (a + b - 1) / b; };

/**
 */
template<size_t x, size_t base, bool lower_than_base = (x < base)>
struct Log {
	static const size_t value = 1 + Log< DivCeil<x, base>::value , base>::value;
};

template<size_t x, size_t base>
struct Log<x, base, true> {
	static const size_t value = 0;
};

template<size_t x, size_t base, bool layer0 = (x <= 1)>
struct TreeNodes {
	static const size_t value = x + TreeNodes< DivCeil<x, base>::value , base>::value;
};

template<size_t x, size_t base>
struct TreeNodes<x, base, true> { static const size_t value = 1; };


/**
 * Find the smallest unsigned integer type that can represent at least
 * \ref N_ different values.
 */
template< unsigned long long N_>
struct SmallUint {
	typedef typename SmallUint<
		#ifdef PC
		(N_ >= 0x100000001) ? 0x100000001LL :
		#endif
		(N_ >= 0x000010001) ? 0x000010001L :
		(N_ >= 0x000000101) ? 0x000000101 :
			0x000000000
		>::t t;
};

template<> struct SmallUint<0x000000000> { typedef ::uint8_t t; };
template<> struct SmallUint<0x000000101> { typedef ::uint16_t t; };
template<> struct SmallUint<0x000010001L> { typedef ::uint32_t t; };

//#if __WORDSIZE == 64
#ifdef PC
template<> struct SmallUint<0x100000001LL> { typedef ::uint64_t t; };
#endif
//#endif


/**
 * Find the unisigned integer type that has exactly N_ bytes (if exists).
 */
template<int N_> struct Uint { };
template<> struct Uint<1> { typedef ::uint8_t t; };
template<> struct Uint<2> { typedef ::uint16_t t; };
template<> struct Uint<4> { typedef ::uint32_t t; };
template<> struct Uint<8> { typedef ::uint64_t t; };


/**
 *
 */
template<typename T>
//struct AsUint {
//	typedef typename Uint<sizeof(T)>::t t;
	typename Uint<sizeof(T)>::t as_uint(T& src) { return *reinterpret_cast<typename Uint<sizeof(T)>::t*>(&src); }
//};

template<int N_ >
struct UintWithAtLeastBits {
   typedef typename Uint< (N_ + 7) / 8 >::t t;
};
   
   
template<typename T>
struct RemovePointer { typedef T t; };

template<typename T>
struct RemovePointer<T*> { typedef T t; };

#endif // META_H

