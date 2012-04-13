
#ifndef RWSELECT_H
#define RWSELECT_H

#include "varint.h"
#include "string.h"
#include <util/pstl/string_dynamic.h>

namespace wiselib {
	namespace protobuf {

template<
	typename OsModel_P,
	typename Buffer_P,
	typename Integer_P,
	typename T
>
struct RWSelect {
	typedef String<OsModel_P, Buffer_P, Integer_P> rw_t;
};

template<typename OsModel_P, typename Buffer_P, typename Integer_P>
struct RWSelect<OsModel_P, Buffer_P, Integer_P, int> {
	typedef VarInt<OsModel_P, Buffer_P, Integer_P> rw_t;
};

template<typename OsModel_P, typename Buffer_P, typename Integer_P>
struct RWSelect<OsModel_P, Buffer_P, Integer_P, unsigned> {
	typedef VarInt<OsModel_P, Buffer_P, Integer_P> rw_t;
};

template<typename OsModel_P, typename Buffer_P, typename Integer_P>
struct RWSelect<OsModel_P, Buffer_P, Integer_P,  char*> {
	typedef String<OsModel_P, Buffer_P, Integer_P> rw_t;
};

	} // ns protobuf
} // ns wiselib

#endif // RWSELECT_H

