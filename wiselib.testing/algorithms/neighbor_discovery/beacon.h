#ifndef BEACON_H
#define BEACON_H

#include "neighbor.h"
#include "protocol_settings.h"
#include "protocol_payload.h"
#include "protocol.h"
#include "util/pstl/vector_static.h"

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Debug_P>
	class Beacon_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Clock_P Clock;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Timer::millis_t millis_t;
		typedef Neighbor_Type<Os, Radio, Clock, Timer, Debug> Neighbor;
		typedef ProtocolPayload_Type<Os, Radio, Debug> ProtocolPayload;
		typedef ProtocolSettings_Type<Os, Radio, Timer, Debug> ProtocolSettings;
		typedef Protocol_Type<Os, Radio, Clock, Timer, Debug> Protocol;
		typedef vector_static<Os, Neighbor, NB_MAX_NEIGHBORS> Neighbor_vector;
		typedef typename Neighbor_vector::iterator Neighbor_vector_iterator;
		typedef vector_static<Os, ProtocolPayload, NB_MAX_REGISTERED_PROTOCOLS> ProtocolPayload_vector;
		typedef typename ProtocolPayload_vector::iterator ProtocolPayload_vector_iterator;
		typedef vector_static<Os, Protocol, NB_MAX_REGISTERED_PROTOCOLS> Protocol_vector;
		typedef typename Protocol_vector::iterator Protocol_vector_iterator;
		typedef Beacon_Type<Os, Radio, Clock, Timer, Debug> self_type;
		// --------------------------------------------------------------------
		Beacon_Type() :
			beacon_period					( 0 ),
			beacon_period_update_counter	( 0 )
		{}
		// --------------------------------------------------------------------
		~Beacon_Type()
		{}
		// --------------------------------------------------------------------
		Neighbor_vector get_neighborhood()
		{
			return neighborhood;
		}
		// --------------------------------------------------------------------
		Neighbor_vector* get_neighborhood_ref()
		{
			return &neighborhood;
		}
		// --------------------------------------------------------------------
		void set_neighborhood( Neighbor_vector& _nv, node_id_t _nid )
		{
			neighborhood.clear();
			for (Neighbor_vector_iterator it = _nv.begin(); it != _nv.end(); ++it )
			{
				if ( it->get_id() != _nid )
				{
					neighborhood.push_back( *it );
				}
			}
		}
		// --------------------------------------------------------------------
		millis_t get_beacon_period()
		{
			return beacon_period;
		}
		// --------------------------------------------------------------------
		void set_beacon_period( millis_t _bp )
		{
			beacon_period = _bp;
		}
		// --------------------------------------------------------------------
		uint32_t get_beacon_period_update_counter()
		{
			return beacon_period_update_counter;
		}
		// --------------------------------------------------------------------
		void set_beacon_period_update_counter( uint32_t _bpuc )
		{
			beacon_period_update_counter = _bpuc;
		}
		// --------------------------------------------------------------------
		ProtocolPayload_vector get_protocol_payloads()
		{
			return protocol_payloads;
		}
		// --------------------------------------------------------------------
		ProtocolPayload_vector* get_protocol_payloads_ref()
		{
			return &protocol_payloads;
		}
		// --------------------------------------------------------------------
		void set_protocol_payloads( ProtocolPayload_vector& _ppv )
		{
			protocol_payloads = _ppv;
		}
		// --------------------------------------------------------------------
		void set_protocol_payloads( Protocol_vector& _pv )
		{
			for ( Protocol_vector_iterator it = _pv.begin(); it != _pv.end(); ++it )
			{
				if ( it->get_protocol_settings_ref()->get_protocol_payload_ref()->get_payload_size() > 0 )
				{
					protocol_payloads.push_back( it->get_protocol_settings_ref()->get_protocol_payload() );
				}
			}
		}
		// --------------------------------------------------------------------
		Beacon_Type& operator=( const Beacon_Type& _b )
		{
			beacon_period_update_counter = _b.beacon_period_update_counter;
			beacon_period = _b.beacon_period;
			neighborhood = _b.neighborhood;
			protocol_payloads = _b.protocol_payloads;
			return *this;
		}
		// --------------------------------------------------------------------
#ifdef NB_DEBUG
		void print( Debug& debug, Radio& radio )
		{
			debug.debug( "-------------------------------------------------------\n");
			debug.debug( "beacon :\n");
			for ( ProtocolPayload_vector_iterator it = protocol_payloads.begin(); it != protocol_payloads.end(); ++it )
			{
				it->print( debug, radio );
			}
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				it->print( debug, radio );
			}
			debug.debug( "beacon_period : %d\n", beacon_period );
			debug.debug( "beacon_period_update_counter : %d\n", beacon_period_update_counter );
			debug.debug( "-------------------------------------------------------\n");
		}
#endif
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t PROTOCOL_PAYLOADS_SIZE_POS = 0;
			size_t PROTOCOL_PAYLOADS_POS = PROTOCOL_PAYLOADS_SIZE_POS + sizeof(size_t);
			size_t pps_size = protocol_payloads.size();
			write<Os, block_data_t, size_t>( _buff + PROTOCOL_PAYLOADS_SIZE_POS + _offset, pps_size );
			size_t pp_size = 0;
			for ( ProtocolPayload_vector_iterator it = protocol_payloads.begin(); it != protocol_payloads.end(); ++it )
			{
				it->serialize( _buff + PROTOCOL_PAYLOADS_POS + pp_size, _offset );
				pp_size = it->serial_size() + pp_size;
			}
			size_t NEIGHBORHOOD_SIZE_POS = PROTOCOL_PAYLOADS_POS + pp_size;
			size_t NEIGHBORHOOD_POS = NEIGHBORHOOD_SIZE_POS + sizeof(size_t);
			size_t ns_size = neighborhood.size();
			write<Os, block_data_t, size_t>( _buff + NEIGHBORHOOD_SIZE_POS + _offset, ns_size );
			size_t n_size = 0;
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				it->serialize( _buff + NEIGHBORHOOD_POS + n_size, _offset );
				n_size = it->serial_size() + n_size;
			}
			size_t BEACON_PERIOD_POS = NEIGHBORHOOD_POS + n_size;
			size_t BEACON_PERIOD_UPDATE_COUNTER_POS = BEACON_PERIOD_POS + sizeof(millis_t);
			write<Os, block_data_t, millis_t>( _buff + BEACON_PERIOD_POS + _offset, beacon_period );
			write<Os, block_data_t, uint32_t>( _buff + BEACON_PERIOD_UPDATE_COUNTER_POS + _offset, beacon_period_update_counter );
			return _buff;
		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t PROTOCOL_PAYLOADS_SIZE_POS = 0;
			size_t PROTOCOL_PAYLOADS_POS = PROTOCOL_PAYLOADS_SIZE_POS + sizeof(size_t);
			size_t pps_size = read<Os, block_data_t, size_t>( _buff + PROTOCOL_PAYLOADS_SIZE_POS + _offset );
			protocol_payloads.clear();
			for ( size_t i = 0; i < pps_size; i++ )
			{
				ProtocolPayload pp;
				pp.de_serialize( _buff + PROTOCOL_PAYLOADS_POS, _offset );
				protocol_payloads.push_back( pp );
				PROTOCOL_PAYLOADS_POS = pp.serial_size() + PROTOCOL_PAYLOADS_POS;
			}
			size_t NEIGHBORHOOD_SIZE_POS = PROTOCOL_PAYLOADS_POS;
			size_t NEIGHBORHOOD_POS = NEIGHBORHOOD_SIZE_POS + sizeof(size_t);
			size_t ns_size = read<Os, block_data_t, size_t>( _buff + NEIGHBORHOOD_SIZE_POS + _offset );
			neighborhood.clear();
			for ( size_t i = 0; i < ns_size; i++ )
			{
				Neighbor n;
				n.de_serialize( _buff + NEIGHBORHOOD_POS , _offset );
				neighborhood.push_back( n );
				NEIGHBORHOOD_POS = n.serial_size() + NEIGHBORHOOD_POS;
			}
			size_t BEACON_PERIOD_POS = NEIGHBORHOOD_POS;
			size_t BEACON_PERIOD_UPDATE_COUNTER_POS = BEACON_PERIOD_POS + sizeof(millis_t);
			beacon_period = read<Os, block_data_t, millis_t>( _buff + BEACON_PERIOD_POS + _offset );
			beacon_period_update_counter = read<Os, block_data_t, uint32_t>( _buff + BEACON_PERIOD_UPDATE_COUNTER_POS + _offset );
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			size_t pp_size = 0;
			for ( ProtocolPayload_vector_iterator it = protocol_payloads.begin(); it != protocol_payloads.end(); ++it )
			{
				pp_size = it->serial_size() + pp_size;
			}
			size_t n_size = 0;
			for ( Neighbor_vector_iterator it = neighborhood.begin(); it != neighborhood.end(); ++it )
			{
				n_size = it->serial_size() + n_size;
			}
			size_t PROTOCOL_PAYLOADS_SIZE_POS = 0;
			size_t PROTOCOL_PAYLOADS_POS = PROTOCOL_PAYLOADS_SIZE_POS + sizeof(size_t);
			size_t NEIGHBORHOOD_SIZE_POS = PROTOCOL_PAYLOADS_POS + pp_size;
			size_t NEIGHBORHOOD_POS = NEIGHBORHOOD_SIZE_POS + sizeof(size_t);
			size_t BEACON_PERIOD_POS = NEIGHBORHOOD_POS + n_size;
			size_t BEACON_PERIOD_UPDATE_COUNTER_POS = BEACON_PERIOD_POS + sizeof(millis_t);
			return BEACON_PERIOD_UPDATE_COUNTER_POS + sizeof(uint32_t);
		}
		// --------------------------------------------------------------------
	private:
		ProtocolPayload_vector protocol_payloads;
		Neighbor_vector neighborhood;
		millis_t beacon_period;
		uint32_t beacon_period_update_counter;
	};
}
#endif
