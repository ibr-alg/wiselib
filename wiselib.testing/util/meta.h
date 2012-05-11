
#ifndef META_H
#define META_H

template<
	size_t a,
	size_t b
>
struct Max {
	static const size_t value = a > b ? a : b;
};


#endif // META_H

