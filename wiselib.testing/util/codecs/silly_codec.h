// vim: set foldenable foldmethod=marker:

#ifndef __UTIL_TUPLE_STORE_SILLY_CODEC_H__
#define __UTIL_TUPLE_STORE_SILLY_CODEC_H__

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Allocator
	>
	class SillyCodec {
		public:
			typedef OsModel_P OsModel;
			
			typedef string_t input_t;
			typedef string_t output_t;
			
			typedef SillyCodec<OsModel, Allocator> self_type;
			typedef typename Allocator::template pointer_t<self_type> self_pointer_t;
			
			static void encode(input_t& in, output_t& out) {
				out = in;
				out.append(" Ceterum censeo Carthaginem esse delendam.");
			}
			
			static void decode(input_t& out, output_t& in) {
				out = in.substr(0, in.size() - strlen(" Ceterum censeo Carthaginem esse delendam."));
			}
	};
}

#endif // __UTIL_TUPLE_STORE_SILLY_CODEC_H__


