
#ifndef META_H
#define META_H

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
template<int N> struct Uint { };
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


template<typename T>
struct RemovePointer { typedef T t; };

template<typename T>
struct RemovePointer<T*> { typedef T t; };

#endif // META_H

