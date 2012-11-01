
#ifndef META_H
#define META_H

// source: http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence/257382#257382

#define HAS_METHOD(func, name) \
	template<typename T, typename Sig> struct name { \
		typedef char yes[1]; typedef char no[2]; \
		template<typename U, U> struct type_check; \
		template<typename C> static yes& check(type_check<Sig, &C::func>*); \
		template<typename C> static no& check(...); \
		static bool const value = (sizeof(check<T>(0)) == sizeof(yes)); \
	}
	


template<bool C, typename T = void>
struct enable_if_c { typedef T type; };

template<typename T>
struct enable_if_c<false, T> { };

template<typename Cond, class T = void>
struct enable_if : public enable_if_c<Cond::value, T> { };


template<
	size_t a,
	size_t b
>
struct Max {
	static const size_t value = a > b ? a : b;
};


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
 * Find the unisigned integer type that has exactly N bytes (if exists).
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

