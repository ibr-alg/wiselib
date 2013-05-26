#ifndef __STATE_STATUS_H_
#define __STATE_STATUS_H_

#include "util/pstl/vector_static.h"
#include "state_status_source_config.h"

namespace wiselib
{
	template	<	typename Os_P,
					typename Radio_P,
					typename Debug_P,
					int SS_VECTOR_SIZE
				>
	class State_Status_Type
	{
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef vector_static<Os, int32_t, SS_VECTOR_SIZE> status_state_vector;
		typedef typename status_state_vector::iterator status_state_vector_iterator;
	public:
		State_Status_Type()
		{}
		// --------------------------------------------------------------------
		~State_Status_Type()
		{}
		// --------------------------------------------------------------------
		uint8_t check_mono_dec()
		{
			if ( ss_vector.size() == ss_vector.max_size() )
			{
				status_state_vector_iterator previous;
				for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
				{
					if ( ( i != ss_vector.begin() ) && ( i != previous ) )
					{
						if ( *i > *previous )
						{
							return 1;
						}
					}
					previous = i;
				}
				return 0;
			}
			return 1;
		}
		// --------------------------------------------------------------------
		uint8_t check_mono_inc()
		{
			if ( ss_vector.size() == ss_vector.max_size() )
			{
				status_state_vector_iterator previous;
				for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
				{
					if ( ( i != ss_vector.begin() ) && ( i != previous ) )
					{
						if ( *i < *previous )
						{
							return 1;
						}
					}
					previous = i;
				}
				return 0;
			}
			return 1;
		}
		// --------------------------------------------------------------------
		void push( int32_t _v )
		{
			status_state_vector tmp;
			tmp.push_back( _v );
			for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
			{
				tmp.push_back( *i );
			}
			ss_vector = tmp;
		}
		// --------------------------------------------------------------------
		int32_t last()
		{
			return ss_vector.back();
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_STATE_STATUS_H
		void print( Debug& _debug, Radio& _radio )
		{
			size_t c=0;
			for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
			{
				_debug.debug("%x:%d:%i\n", _radio.id(), c, *i );
				c++;
			}
		}
		// --------------------------------------------------------------------
#endif
	private :
		status_state_vector ss_vector;
	};

}
#endif
