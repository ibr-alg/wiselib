/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef FNV_H
#define FNV_H

namespace wiselib {

	template<
		typename OsModel_P,
		typename Hash_P,
		Hash_P Init_P,
		Hash_P MagicPrime_P,
		bool Alternate_P
	>
	class FnvBase {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Hash_P hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t hashval = Init_P;
				const hash_t magicprime = MagicPrime_P;
				const block_data_t *end = s + l;
				for( ; s != end; s++) {
					hashval *= magicprime;
					hashval ^= *s;
				}
				return hashval;
			}
	};
	
	template<
		typename OsModel_P,
		typename Hash_P,
		Hash_P Init_P,
		Hash_P MagicPrime_P
	>
	class FnvBase<OsModel_P, Hash_P, Init_P, MagicPrime_P, true> : public FnvBase<OsModel_P, Hash_P, Init_P, MagicPrime_P, false> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename FnvBase::hash_t hash_t;
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t hashval = Init_P;
				const hash_t magicprime = MagicPrime_P;
				const block_data_t *end = s + l;
				for( ; s != end; s++) {
					hashval ^= *s;
					hashval *= magicprime;
				}
				return hashval;
			}
	};
	
	template<
		typename OsModel_P,
		typename Hash_P
	>
	class Fnv1;
	
	template<
		typename OsModel_P
	>
	class Fnv1<OsModel_P, ::uint16_t> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint16_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				::uint32_t h = Fnv1<OsModel_P, ::uint32_t>::hash(s, l);
				return (h >> 16) ^ (h & 0xffff);
			}
	};
	
	template<
		typename OsModel_P
	>
	class Fnv1<OsModel_P, ::uint32_t> :
		public FnvBase<OsModel_P, ::uint32_t, 0x811c9dc5UL, 0x1000193UL, false> {
	};
	
	template<
		typename OsModel_P
	>
	class Fnv1<OsModel_P, ::uint64_t> :
		public FnvBase<OsModel_P, ::uint64_t, 0xcbf29ce484222325ULL, 0x00000100000001b3ULL, false> {
	};
	
	
	template<
		typename OsModel_P,
		typename Hash_P
	>
	class Fnv1a;
	
	template<
		typename OsModel_P
	>
	class Fnv1a<OsModel_P, ::uint16_t> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint16_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				::uint32_t h = Fnv1a<OsModel_P, ::uint32_t>::hash(s, l);
				return (h >> 16) ^ (h & 0xffff);
			}
	};
		
	
	template<
		typename OsModel_P
	>
	class Fnv1a<OsModel_P, ::uint32_t> :
		public FnvBase<OsModel_P, ::uint32_t, 0x811c9dc5UL, 0x1000193UL, true> {
	};
	
	template<
		typename OsModel_P
	>
	class Fnv1a<OsModel_P, ::uint64_t> :
		public FnvBase<OsModel_P, ::uint64_t, 0xcbf29ce484222325ULL, 0x00000100000001b3ULL, true> {
	};
}

#endif // FNV_H

