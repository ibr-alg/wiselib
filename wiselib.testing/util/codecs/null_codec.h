/* 
 * File:   Coder.h
  * Author: maxpagel
 *
 * Created on 6. Dezember 2011, 13:38
 */

#ifndef _NULL_CODEC_H
#define	_NULL_CODEC_H

#include <util/pstl/BitString.h>

namespace wiselib {

	template<typename OsModel_P,
			typename Allocator_P
			>
	class NullCodec{
	 public:
		typedef OsModel_P OsModel;
		typedef Allocator_P Allocator;
		typedef string_dynamic<OsModel,Allocator> stringDynamic;
		typedef BitString_dynamic<OsModel,Allocator> bitString;
		
		typedef bitString output_t;
		typedef stringDynamic input_t;
		typedef NullCodec<OsModel, Allocator> self_type;
		typedef typename Allocator::template pointer_t<self_type> self_pointer_t;

		NullCodec(){}
		NullCodec(typename Allocator::self_pointer_t allocator):allocator_(allocator)
		{}

		Allocator& allocator() { return *allocator_; }
		void set_allocator(typename Allocator::self_pointer_t alloc) {
			allocator_ = alloc;
		}
		 
		void encode(stringDynamic& in, bitString& bits)
		{
			bits.clear();
			for(size_t i=0; i<in.size(); i++) {
				bits.push_byte(in[i]);
			}
		}

	void decode(stringDynamic& out, bitString& bits){
			//out.clear();
			for(size_t i=0; i<bits.bits().size(); i++) {
				out.push_back(bits.bits()[i]);
			}
		}
	 private:
		typename Allocator::self_pointer_t allocator_;
		
		uint8_t something;

	 };




};



#endif	/* _CODER_H */

