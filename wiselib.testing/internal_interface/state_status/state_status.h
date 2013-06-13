#ifndef __STATE_STATUS_H_
#define __STATE_STATUS_H_

#include "util/pstl/vector_static.h"
#include "state_status_source_config.h"

namespace wiselib
{
	template	<	typename Os_P,
					typename Radio_P,
					typename Debug_P,
					typename Numeric_P,
					int SS_VECTOR_SIZE
				>
	class StateStatus_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Numeric_P Numeric;
		typedef vector_static<Os, Numeric, SS_VECTOR_SIZE> status_state_vector;
		typedef typename status_state_vector::iterator status_state_vector_iterator;
		typedef StateStatus_Type<Os, Radio, Debug, Numeric, SS_VECTOR_SIZE> self_type;

		StateStatus_Type() :
			lock_flag ( 1 )
		{}
		// --------------------------------------------------------------------
		~StateStatus_Type()
		{}
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
						if ( *i > *previous )
						{
							return 0;
						}
					}
					previous = i;
				}
				return 1;
			}
			return 0;
		}
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
						if ( *i < *previous )
						{
							return 0;
						}
					}
					previous = i;
				}
				return 1;
			}
			return 0;
		}
		// --------------------------------------------------------------------
		uint8_t check_constant()
		{
			if ( ss_vector.size() == ss_vector.max_size() )
			{
				status_state_vector_iterator previous;
				for ( status_state_vector_iterator i = ss_vector.begin(); i != ss_vector.end(); ++i )
				{
					if ( ( i != ss_vector.begin() ) && ( i != previous ) )
					{
						if ( *i != *previous )
						{
							return 0;
						}
					}
					previous = i;
				}
				return 1;
			}
			return 0;
		}
		// --------------------------------------------------------------------
		Numeric cartesian_product( self_type _v )
		{
			Numeric R = 0;
			for ( size_t i = 0; i< _v.ss_vector.size(); i++ )
			{
				R = R + _v.get_ss_vector_ref()->at(i) * ss_vector[i];
			}
			return R;
		}
		// --------------------------------------------------------------------
		void push( Numeric _v )
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
		Numeric first()
		{
			return ss_vector.back();
		}
		// --------------------------------------------------------------------
		Numeric last()
		{
			return ss_vector.front();
		}
		// --------------------------------------------------------------------
		void lock()
		{
			lock_flag = 0;
		}
		// --------------------------------------------------------------------
		void unlock()
		{
			lock_flag = 1;
		}
		// --------------------------------------------------------------------
		uint8_t try_lock()
		{
			return lock_flag;
		}
		// --------------------------------------------------------------------
		uint8_t full()
		{
			if ( ss_vector.size() == ss_vector.max_size() )
			{
				return 1;
			}
			return 0;
		}
		// --------------------------------------------------------------------
		void clear()
		{
			ss_vector.clear();
			lock_flag = 0;
		}
		// --------------------------------------------------------------------
		status_state_vector* get_ss_vector_ref()
		{
			return &ss_vector;
		}
		// --------------------------------------------------------------------
#ifdef DEBUG_STATE_STATUS_H
		void print( Debug& _debug, Radio& _radio )
		{
			size_t c=0;
			for ( status_state_vector_iterator i = ( ss_vector.end() - 1 ); i != ss_vector.begin(); --i )
			{
				_debug.debug("%x:%d:%i\n", _radio.id(), c, *i );
				c++;
			}
			_debug.debug("%x:%d:%i\n", _radio.id(), c, *ss_vector.begin() );
		}
		// --------------------------------------------------------------------
#endif
	private :
		status_state_vector ss_vector;
		uint8_t lock_flag;
	};

}
#endif
