#ifndef CONVERGECAST_MESSAGE_H
#define CONVERGECAST_MESSAGE_H

#include "util/serialization/simple_types.h"
#include "util/serialization/floating_point.h"

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio>

/** \brief Example implementation of a convergecast message.
 *
 *  \ingroup Ccast_concept
 *
 *  Example implementation of the \ref CCast_message_concept "ConvergecastMessage" concept.
 */
class ConvergecastMessage
{
public:
   typedef OsModel_P OsModel;
   typedef Radio_P Radio;
   typedef typename Radio::block_data_t block_data_t;
   typedef typename Radio::size_t size_t;
   typedef typename Radio::node_id_t node_id_t;

   enum CommandType
   {
      CMD_CCINIT = 1, CMD_CCRESPONSE = 2
   };

   ConvergecastMessage()
   {
   }
   // --------------------------------------------------------------------
   uint8_t command_type()
   {
      return wiselib::read<OsModel, block_data_t, uint8_t>( buffer );
   }
   // --------------------------------------------------------------------
   void set_command_type( uint8_t type )
   {
      wiselib::write<OsModel, block_data_t, uint8_t>( buffer, type );
   }
   // --------------------------------------------------------------------
   uint8_t cc_id()
   {
      return wiselib::read<OsModel, block_data_t, uint8_t>( buffer + CC_ID_POS );
   }
   // --------------------------------------------------------------------
   void set_cc_id( uint8_t id )
   {
      wiselib::write<OsModel, block_data_t, uint8_t>( buffer + CC_ID_POS, id );
   }
   // --------------------------------------------------------------------
   uint16_t intvalue1()
   {
      return wiselib::read<OsModel, block_data_t, uint16_t>( buffer + INTVALUE1_POS );
   }
   // --------------------------------------------------------------------
   void set_intvalue1( uint16_t number )
   {
      wiselib::write<OsModel, block_data_t, uint16_t>( buffer + INTVALUE1_POS, number );
   }
   // --------------------------------------------------------------------
   float floatvalue1()
   {
      return wiselib::read<OsModel, block_data_t, float>( buffer + FLOATVALUE1_POS );
   }
   // --------------------------------------------------------------------
   void set_floatvalue1( float number )
   {
      wiselib::write<OsModel, block_data_t, float>( buffer + FLOATVALUE1_POS, number );
   }

   size_t size()
   {
      return _SIZE;
   }

   block_data_t* data()
   {
      return buffer;
   }

private:
   enum data_in_positions
   {
      COMMAND_TYPE = 0, CC_ID_POS = 1, INTVALUE1_POS = 2,
      FLOATVALUE1_POS = INTVALUE1_POS + 2,
      _SIZE = FLOATVALUE1_POS + 4
   };

   // --------------------------------------------------------------------
   block_data_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
};

#endif // CONVERGECAST_MESSAGE_H
