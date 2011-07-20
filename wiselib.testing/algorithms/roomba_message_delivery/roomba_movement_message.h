#ifndef __ROOMBA_MOVEMENT_MESSAGE_H__
#define __ROOMBA_MOVEMENT_MESSAGE_H__

#include <stdint.h>

#include "util/serialization/serialization.h"
#include "external_interface/external_interface.h"


template<typename OsModel_P, typename Radio_P>
class RoombaMovementMessage
{
public:
   typedef OsModel_P OsModel;
   typedef Radio_P Radio;

   typedef typename Radio::node_id_t node_id_t;
   typedef typename Radio::block_data_t block_data_t;
   typedef typename Radio::message_id_t message_id_t;

   typedef RoombaMovementMessage<OsModel, Radio> self_type;
   typedef RoombaMovementMessage<OsModel, Radio>* self_pointer_t;

   typedef uint8_t message_type_t;

   enum RoombaMovementSpecialNodeIds
   {
	   BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS
   };

   enum RoombaMovementMessageType
   {
	   STOP_MOVEMENT = 0x00, START_MOVEMENT
   };

   enum MessageIds
   {
      ROOMBA_MOVEMENT_MESSAGE_ID = 0x17 //23
   };

   enum
   {
	   MAX_MESSAGE_LENGTH = sizeof(message_id_t) + sizeof( message_type_t )
   };

   RoombaMovementMessage();

   inline uint8_t msg_id()
   {
	   return ROOMBA_MOVEMENT_MESSAGE_ID;
   }
   // --------------------------------------------------------------------
   inline void set_msg_id( uint8_t id )
   {
	   // Changing the msg_id is not allowed!
   }
   // --------------------------------------------------------------------
   inline uint8_t set_msg_type( RoombaMovementMessageType mt )
   {
	   return wiselib::write<OsModel, block_data_t, message_type_t>(buffer + MESSAGE_TYPE_POS, (message_type_t&)mt);
   }
   // --------------------------------------------------------------------
   inline uint8_t msg_type()
   {
	   return wiselib::read<OsModel, block_data_t, message_type_t>( buffer + MESSAGE_TYPE_POS );
   }
   // --------------------------------------------------------------------
   inline block_data_t* payload()
   {
	   return 0;
   }
   // --------------------------------------------------------------------
   inline size_t payload_size()
   {
	   return 0;
   }
   // --------------------------------------------------------------------
   inline void set_payload( size_t len, block_data_t *data )
   {
	   //NOT SUPPORTED
   }
   // --------------------------------------------------------------------
   inline size_t buffer_size()
   {
	   switch( msg_type() )
	   {
	   	   case START_MOVEMENT:
	   	   case STOP_MOVEMENT:
	   		   return PAYLOAD_SIZE_POS;
	   		   break;
	   	   default:
	   		   return PAYLOAD_SIZE_POS;
	   }
	}

private:
   enum data_positions
   {
	   MESSAGE_TYPE_POS = sizeof( message_id_t ),
	   NODE_ID_POS = sizeof(message_id_t) + sizeof( message_type_t ),
	   PAYLOAD_SIZE_POS = sizeof(message_id_t) + sizeof( message_type_t ),
	   PAYLOAD_POS = sizeof(message_id_t) + sizeof( message_type_t ) + sizeof( size_t )
   };

   block_data_t buffer[MAX_MESSAGE_LENGTH];
};

template<typename OsModel_P, typename Radio_P>
RoombaMovementMessage<OsModel_P, Radio_P>::RoombaMovementMessage()
{
	message_id_t tmp = ROOMBA_MOVEMENT_MESSAGE_ID;//write expects reference
	wiselib::write<OsModel, block_data_t, message_id_t>( buffer, tmp );
	set_msg_type( STOP_MOVEMENT );
}

#endif //__ROOMBA_MOVEMENT_MESSAGE_H__
