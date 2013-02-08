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

#ifndef BLOCK_TUPLE_STORE_H
#define BLOCK_TUPLE_STORE_H

namespace wiselib {
	
	
	namespace {
		// Iterator stuff
		// {{{
		
		/**
		 * TupleStore comparison function comparing by length and binary
		 * equality of the tuple elements (similar to strcmp).
		 */
		int datacmp(
				int col, ::uint8_t *a, int alen, ::uint8_t *b, int blen) {
			if(alen != blen) { return (int)blen - (int)alen; }
			for(int i=0; i<alen; i++) {
				if(a[i] != b[i]) { return (int)b[i] - (int)a[i]; }
			}
			return 0;
		}
		
		/**
		 */
		template<
			typename OsModel_P,
			typename TupleStore_P,
			int DICTIONARY_COLUMNS_P,
			int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
		>
		class Iterator {
			// {{{
			public:
				typedef OsModel_P OsModel;
				typedef TupleStore_P TupleStore;
				typedef typename TupleStore::Tuple Tuple;
				typedef typename TupleStore::Dictionary Dictionary;
				typedef typename OsModel::block_data_t block_data_t;
				typedef typename OsModel::size_t size_type;
				typedef typename TupleStore::column_mask_t column_mask_t;
				enum { DICTIONARY_COLUMNS = DICTIONARY_COLUMNS_P };
				enum { COLUMNS = Tuple::SIZE };
				
				Iterator() {
				}
				
				Iterator(const Iterator& other) { *this = other; }
				
				Iterator& operator=(const Iterator& other) {
					// TODO
				}
				
				Tuple& operator*() {
					// TODO
				}
				
				Tuple* operator->() { return &operator*(); }
				
				Iterator& operator++() {
					// TODO
				}
				
				bool operator==(const Itertaor& other) {
					// TODO
				}
				bool operator!=(const Iterator& other) { return !(*this == other); }
				
				Tuple& query() { return query_; }
				column_mask_t mask() { return column_mask_; }
				
			private:
				Tuple query_;
				column_mask_t column_mask_;
				Tuple current_;
				
			// }}}
		};
		
		
		// }}}
	}
	
	/**
	 * \brief
	 * 
	 * \ingroup
	 * 
	 * \tparam 
	 */
	template<
		typename OsModel_P,
		
		// TODO:
		// Total cool wärs, wenn wir gar keinen block_tuple_store
		// bräuchten sondern den normalen mit speziellem container und
		// dictionary nehmen könnten.... geht das?
		// 
		// --> hmm, ich glaub ja! fetzomatisch!
		// 
		typename BlockStorage_P,
		
		typename TupleContainer_P, // = UnsortedBlockList
		typename Dictionary_P,
		typename Debug_P,
		::uint64_t DICTIONARY_COLUMNS_P,
		int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
	>
	class BlockTupleStore {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Debug_P Debug;
			typedef BlockStorage_P BlockStorage;
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::key_type key_type;
			typedef typename Dictionary::mapped_type mapped_type;
			enum { DICTIONARY_COLUMNS = DICTIONARY_COLUMNS_P };
			typedef BlockTupleStore<OsModel, BlockStorage, Dictionary, Debug, DICTIONARY_COLUMNS, Compare> self_type;
			typedef self_type* self_pointer_t;
			
			typedef ... Tuple;
			typedef ... iterator;
			
			
			void init(typename Debug::self_pointer_t debug) {
				debug_ = debug;
				// TODO
			}
			
			template<typename UserTuple>
			iterator insert(UserTuple& t) {
				// TODO
				// 
				// mytuple = Tuple()
				// for each column c in t:
				//   if c is dictcol:
				//   	k = dictionary_.insert(t.get(c))
				//   	mytuple.set(c, k)
				//   else:
				//   	mytuple.set_deep(c, t.get(c))
				//   	
				// return container_.insert(mytuple)
			}
			
			iterator erase(iterator iter) {
				// TODO
				// 
				// for column c:
				//   if c is dictcol:
				//     dictionary_.erase(iter->
			}
			
			iterator begin(Tuple* query = 0, column_mask_t mask = 0) {
				// TODO
			}
			
			iterator end() {
				// TODO
			}
		
		private:
			typename Debug::self_pointer_t debug_;
		
	}; // BlockTupleStore
}

#endif // BLOCK_TUPLE_STORE_H

