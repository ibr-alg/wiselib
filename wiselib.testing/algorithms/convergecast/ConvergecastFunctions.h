#ifndef CONVERGECAST_FUNCTIONS_H
#define CONVERGECAST_FUNCTIONS_H

template<typename Debug_P, typename Container_P, typename MessageType_P>

/** \brief Example implementation for some basic convergecast functions.
 *
 *  \ingroup Ccast_concept
 *
 *  Example implementation of the \ref CCast_functions_concept "ConvergecastFunctions" concept.
 */
class ConvergecastFunctions
{
public:
   typedef Container_P Container;
   typedef MessageType_P Message;
   typedef Debug_P Debug;

   typedef typename Container::iterator container_iterator_t;
   typedef typename Debug::self_pointer_t Debug_t;

   void init( Debug_t debug )
   {
      debug_ = debug;
   }

   inline Message count( Container &buffer, Message &own )
   {
      own.set_intvalue1( 1 );
      return sum( buffer, own );
   }

   inline Message sum( Container &buffer, Message &own )
   {
      uint16_t result = own.intvalue1();

      Message buffer_msg;
      for ( container_iterator_t c_iter = buffer.begin(); c_iter != buffer.end(); ++c_iter )
      {
         buffer_msg = *c_iter;
         result = result + buffer_msg.intvalue1();
      }

      Message message;
      message.set_command_type( Message::CMD_CCRESPONSE );
      message.set_cc_id( own.cc_id() );
      message.set_intvalue1( result );
      return message;
   }

   inline Message min( Container &buffer, Message &own )
   {
      uint16_t result = own.intvalue1();

      Message buffer_msg;
      for ( container_iterator_t c_iter = buffer.begin(); c_iter != buffer.end(); ++c_iter )
      {
         buffer_msg = *c_iter;
         if ( buffer_msg.intvalue1() < result )
         {
            result = buffer_msg.intvalue1();
         }
      }

      Message message;
      message.set_command_type( Message::CMD_CCRESPONSE );
      message.set_cc_id( own.cc_id() );
      message.set_intvalue1( result );
      return message;
   }

   inline Message max( Container &buffer, Message &own )
   {
      uint16_t result = own.intvalue1();

      Message buffer_msg;
      for ( container_iterator_t c_iter = buffer.begin(); c_iter != buffer.end(); ++c_iter )
      {
         buffer_msg = *c_iter;
         if ( buffer_msg.intvalue1() > result )
         {
            result = buffer_msg.intvalue1();
         }
      }

      Message message;
      message.set_command_type( Message::CMD_CCRESPONSE );
      message.set_cc_id( own.cc_id() );
      message.set_intvalue1( result );
      return message;
   }

   inline Message avg( Container &buffer, Message &own )
   {
      uint16_t count = 1;
      float result = own.floatvalue1();

      Message buffer_msg;
      for ( container_iterator_t c_iter = buffer.begin(); c_iter != buffer.end(); ++c_iter )
      {
         buffer_msg = *c_iter;
         count = count + buffer_msg.intvalue1();
         result = result + ( buffer_msg.intvalue1() * buffer_msg.floatvalue1() );
      }

      Message message;
      message.set_command_type( Message::CMD_CCRESPONSE );
      message.set_cc_id( own.cc_id() );
      message.set_intvalue1( count );
      message.set_floatvalue1( result / count );
      return message;
   }

private:
   Debug_t debug_;

};

#endif // CONVERGECAST_FUNCTIONS_H
