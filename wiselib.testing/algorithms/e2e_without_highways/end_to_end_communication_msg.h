#ifndef __ALGORITHMS_END_TO_END_COMMUNICATION_END_TO_END_COMMUNICATION_MSG_H__
#define __ALGORITHMS_END_TO_END_COMMUNICATION_END_TO_END_COMMUNICATION_MSG_H__
	
#include "util/serialization/simple_types.h"
 	
namespace wiselib
{

	template<typename OsModel_P, typename Radio_P>
	class CommunicationMessage
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;

		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::message_id_t message_id_t;

		enum {		
			END_TO_END_MESSAGE = 245,
			NODE_IN_CLUSTER = 246,
			};
		//--------------------------------------------------------------------
		inline CommunicationMessage( node_id_t src=Radio::NULL_NODE_ID, node_id_t dest=Radio::NULL_NODE_ID )
		{
			set_msg_id( END_TO_END_MESSAGE );
			set_payload_size( 0 );
			set_seq_no( cur_seq_no++ );
			set_source( src );
			set_dest( dest );
			set_timestamp( 0 );
		}
		// --------------------------------------------------------------------
		inline message_id_t msg_id()
		{ return read<OsModel, block_data_t, message_id_t>( buffer + MSG_ID_POS ); };
		// --------------------------------------------------------------------
		inline uint8_t seq_no()
		{ return read<OsModel, block_data_t, uint8_t>(buffer + SEQ_NO_POS); }
		// --------------------------------------------------------------------
		inline uint8_t payload_size()
		{ return read<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_SIZE_POS); }
		// --------------------------------------------------------------------
		inline node_id_t source()
		{ return read<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS); }
		// --------------------------------------------------------------------
		inline void set_source( node_id_t src )
		{ write<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS, src); }
		// -----------------------------------------------------------------------
		inline node_id_t dest()
		{ return read<OsModel, block_data_t, node_id_t>(buffer + DESTINATION_POS); }
		// --------------------------------------------------------------------
		inline void set_dest( node_id_t dest )
		{ write<OsModel, block_data_t, node_id_t>(buffer + DESTINATION_POS, dest); }
		// -----------------------------------------------------------------------
		inline uint16_t timestamp()
		{ return read<OsModel, block_data_t, node_id_t>(buffer + TIMESTAMP_POS); }
		// --------------------------------------------------------------------
		inline void set_timestamp( uint16_t timestamp )
		{ write<OsModel, block_data_t, node_id_t>(buffer + TIMESTAMP_POS, timestamp); }
		// -----------------------------------------------------------------------

		inline void set_payload( uint8_t len, block_data_t* data )
		{
			set_payload_size( len );
			if (len > 0)
				memcpy( buffer + PAYLOAD_POS, data, len );
		}
		// -----------------------------------------------------------------------
		inline block_data_t* payload( void )
		{ return buffer + PAYLOAD_POS; }
		// --------------------------------------------------------------------
		inline uint8_t buffer_size()
		{ return PAYLOAD_POS + payload_size(); };
		inline void set_msg_id( message_id_t id )
		{ write<OsModel, block_data_t, message_id_t>( buffer+MSG_ID_POS, id ); }

	
	private:
		static uint8_t cur_seq_no;

		inline void set_payload_size( uint8_t len )
		{ write<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_SIZE_POS, len); }
		// --------------------------------------------------------------------
		inline void set_seq_no( uint8_t seq_no )
		{ write<OsModel, block_data_t, uint8_t>(buffer + SEQ_NO_POS, seq_no); }
		// --------------------------------------------------------------------
		// --------------------------------------------------------------------

		enum data_positions
		{
			MSG_ID_POS  = 0,
			SOURCE_POS  = sizeof( message_id_t ),
			DESTINATION_POS  = SOURCE_POS + sizeof( node_id_t ),
			TIMESTAMP_POS = DESTINATION_POS + sizeof( uint16_t ),
			SEQ_NO_POS = TIMESTAMP_POS + sizeof( node_id_t ),
			PAYLOAD_SIZE_POS = SEQ_NO_POS + sizeof( uint8_t ),
			PAYLOAD_POS = PAYLOAD_SIZE_POS + sizeof( uint8_t ),
		};

		block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];

	public:
		enum {

			MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH,
			MAX_PAYLOAD_LENGTH = Radio::MAX_MESSAGE_LENGTH - PAYLOAD_POS,// Maximal length of the payload.
			HEADER_LENGTH = PAYLOAD_POS,
		};
	};
	
	// -----------------------------------------------------------------------

	template<typename OsModel_P, typename Radio_P>
	uint8_t
	CommunicationMessage<OsModel_P, Radio_P>::cur_seq_no = 0;
}
#endif
