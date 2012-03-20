#ifndef PROTOCOL_PAYLOAD_H
#define	PROTOCOL_PAYLOAD_H

#include "neighbor_discovery_config.h"

namespace wiselib
{
	template< 	typename Os_P,
				typename Radio_P,
				typename Debug_P>
	class ProtocolPayload_Type
	{
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef ProtocolPayload_Type<Os, Radio, Debug> self_type;
	public:
		ProtocolPayload_Type() :
			payload_size		( NB_MAX_PROTOCOL_PAYLOAD_SIZE )
		{
			for ( size_t i = 0; i < NB_MAX_PROTOCOL_PAYLOAD_SIZE; i++ )
			{
				payload_data[i] = 0;
			}
		}
		// --------------------------------------------------------------------
		ProtocolPayload_Type( uint8_t _pid, size_t _ps, block_data_t* _pd, size_t _offset = 0 )
		{
			protocol_id = _pid;
			payload_size = _ps;
			for ( size_t i = 0; i < NB_MAX_PROTOCOL_PAYLOAD_SIZE; i++ )
			{
				payload_data[i] = 0;
			}
			if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					payload_data[i] = _pd[i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		~ProtocolPayload_Type()
		{}
		// --------------------------------------------------------------------
		uint8_t get_protocol_id()
		{
			return protocol_id;
		}
		// --------------------------------------------------------------------
		void set_protocol_id( uint8_t _pid )
		{
			protocol_id = _pid;
		}
		// --------------------------------------------------------------------
		size_t get_payload_size()
		{
			return payload_size;
		}
		// --------------------------------------------------------------------
		size_t get_max_payload_size()
		{
			return NB_MAX_PROTOCOL_PAYLOAD_SIZE;
		}
		// --------------------------------------------------------------------
		void set_payload_size( size_t _ps )
		{
			payload_size = _ps;
		}
		// --------------------------------------------------------------------
		block_data_t* get_payload_data()
		{
			return payload_data;
		}
		// --------------------------------------------------------------------
		void set_payload_data( block_data_t* _pd, size_t _offset = 0 )
		{
			if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					payload_data[i] = _pd[i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		void set_payload( block_data_t* _pd, size_t _ps, size_t _offset = 0 )
		{
			payload_size = _ps;
			if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					payload_data[i] = _pd[i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		ProtocolPayload_Type& operator=( const ProtocolPayload_Type& _pp )
		{
			protocol_id = _pp.protocol_id;
			payload_size = _pp.payload_size;
			if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0 ; i < payload_size; i++ )
				{
					payload_data[i] = _pp.payload_data[i];
				}
			}
			return *this;
		}
		// --------------------------------------------------------------------
#ifdef NB_DEBUG
		void print( Debug& debug, Radio& radio )
		{
			debug.debug( "-------------------------------------------------------\n");
			debug.debug( "protocol_payload :\n");
			debug.debug( "protocol_id : %d\n", protocol_id );
			debug.debug( "max_payload_size : %d\n", NB_MAX_PROTOCOL_PAYLOAD_SIZE );
			debug.debug( "payload_size : %d\n", payload_size );
			if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0; i < payload_size; i++ )
				{
					debug.debug( "payload %d 'th byte : %d\n", i, payload_data[i] );
				}
			}
			debug.debug( "-------------------------------------------------------");
		}
#endif
		// --------------------------------------------------------------------
		block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			if ( payload_size != 0 )
			{
				size_t PROTOCOL_ID_POS = 0;
				size_t PAYLOAD_SIZE_POS = PROTOCOL_ID_POS + sizeof(uint8_t);
				size_t PAYLOAD_DATA_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
				write<Os, block_data_t, uint8_t>( _buff + PROTOCOL_ID_POS + _offset, protocol_id );
				write<Os, block_data_t, size_t>( _buff + PAYLOAD_SIZE_POS + _offset, payload_size );
				if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
				{
					for ( size_t i = 0 ; i < payload_size; i++ )
					{
						_buff[PAYLOAD_DATA_POS + i + _offset] = payload_data[i];
					}
				}
			}
			return _buff;

		}
		// --------------------------------------------------------------------
		void de_serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t PROTOCOL_ID_POS = 0;
			size_t PAYLOAD_SIZE_POS = PROTOCOL_ID_POS + sizeof(uint8_t);
			size_t PAYLOAD_DATA_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
			protocol_id = read<Os, block_data_t, uint8_t>( _buff + PROTOCOL_ID_POS + _offset );
			payload_size = read<Os, block_data_t, size_t>( _buff + PAYLOAD_SIZE_POS + _offset );
			if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
			{
				for ( size_t i = 0 ; i < payload_size; i++ )
				{
					 payload_data[i] = _buff[PAYLOAD_DATA_POS + i + _offset];
				}
			}
		}
		// --------------------------------------------------------------------
		size_t serial_size()
		{
			if ( payload_size != 0 )
			{
				size_t PROTOCOL_ID_POS = 0;
				size_t PAYLOAD_SIZE_POS = PROTOCOL_ID_POS + sizeof(uint8_t);
				size_t PAYLOAD_DATA_POS = PAYLOAD_SIZE_POS + sizeof(size_t);
				if ( payload_size <= NB_MAX_PROTOCOL_PAYLOAD_SIZE )
				{
					return PAYLOAD_DATA_POS + sizeof( block_data_t) * payload_size;
				}
				else
				{
					return PAYLOAD_SIZE_POS + sizeof(size_t);
				}
			}
			return 0;
		}
		// --------------------------------------------------------------------
	private:
		uint8_t protocol_id;
		size_t payload_size;
		block_data_t payload_data[NB_MAX_PROTOCOL_PAYLOAD_SIZE];
	};
}
#endif
