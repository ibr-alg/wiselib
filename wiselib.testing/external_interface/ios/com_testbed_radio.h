#ifndef CONNECTOR_COM_TESTBED_RADIOMODEL_H
#define CONNECTOR_COM_TESTBED_RADIOMODEL_H

#include "external_interface/ios/ios_system.h"
#include "external_interface/ios/ios_model.h"
#include "util/delegates/delegate.hpp"
#include "util/ios_node_api/ios_link_message.h"
#include "util/base_classes/radio_base.h"

#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"

#include "external_interface/ios/ios_radio_controller/ComTestbedRadioController.h"

namespace wiselib
{

   template<typename OsModel_P, bool iOsLinkRadio = true>
   class ComTestbedRadioModel
      : public RadioBase<OsModel_P, uint16_t, __darwin_size_t, uint8_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef ComTestbedRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef IOsLinkMessage<OsModel, self_type> IOSLinkMessage;
      
      typedef uint16_t node_id_t;
      typedef uint8_t  block_data_t;
      typedef __darwin_size_t  size_t;
      typedef uint8_t  message_id_t;
   
      typedef delegate3<void, node_id_t, size_t, block_data_t*> radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffff, ///< All nodes in communication range
         NULL_NODE_ID      = 0       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         // todo
         MAX_MESSAGE_LENGTH = 512    ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      ComTestbedRadioModel(iOsSystem& system)
         : system_(system)
      {
         iOsRadio_ = [[ComTestbedRadioController alloc] init];
      }
      // --------------------------------------------------------------------
      virtual ~ComTestbedRadioModel()
      {
         [iOsRadio_ release];
         iOsRadio_ = nil;
      }
      
      // --------------------------------------------------------------------
      void set_testbed( NSString* url, UInt16 port )
      {
         [iOsRadio_ setConnectionURL:url port:port];
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
#ifdef IOS_RADIO_DEBUG
      NSLog(@"IOS_RADIO_DEBUG: Radio enable");
#endif
         if([iOsRadio_ enable] == SUCCESS) {
            radio_delegate_t d = radio_delegate_t::template from_method<self_type, &self_type::receive_message>(this);
            [iOsRadio_ registerReceiver: d];
            
            return SUCCESS;
         }
         return ERROR_UNSPEC;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
#ifdef IOS_RADIO_DEBUG
      NSLog(@"IOS_RADIO_DEBUG: Radio disable");
#endif
         return [iOsRadio_ disable];
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {    
         return [iOsRadio_ getRadioId];
      }
      // --------------------------------------------------------------------
      int send( node_id_t to, size_t len, block_data_t *data )
      {
#ifdef IOS_RADIO_DEBUG
      NSLog(@"IOS_RADIO_DEBUG: Send %d with lenght %d", to, len);
#endif

         if(!iOsLinkRadio) {
            return [iOsRadio_ sendTo:to withData:data withLenght:len];
         
         } else {
            IOSLinkMessage message;
            message.set_command_type( IOS_LINK_MESSAGE );
            message.set_destination( to );
            message.set_source( this->id() );
            message.set_payload( len, data );
         
            //[iOsRadio_ sendTo:to withData:(uint8_t*)(&message) withLenght:message.buffer_size()];
            return [iOsRadio_ sendTo:to withData:(uint8_t*)(&message) withLenght:message.buffer_size()];
         }
            
      }
      // --------------------------------------------------------------------
      void receive_message( node_id_t from, size_t len, block_data_t *buf )
      {
            
                  
         if(!iOsLinkRadio) {
            this->notify_receivers( from, len, buf );
         
         } else {
             
             /*
             NSMutableString* outputString = [NSMutableString stringWithFormat:@"output: "];
             
             for (int i =0; i<len; i++) {
                 [outputString appendFormat:@"%d ", buf[i]];
             }
             
             NSLog(@"%@", outputString); 
             */
             
            switch (*buf) {
               case IOS_LINK_MESSAGE: 
               {

                  IOSLinkMessage *msg = (IOSLinkMessage*)buf;
                  
                  #ifdef IOS_RADIO_DEBUG
                     //NSLog( @"IOS_RADIO_DEBUG: RECEIVED IOsLinkMessage from %d to %d with size %d payload: %s",
                     //      (node_id_t)msg->source(), (node_id_t)msg->destination(), msg->payload_length(), msg->payload() );
                  #endif
                  
                  this->notify_receivers( msg->source() , msg->payload_length(), msg->payload() );
               } break;
               
               default:
               {
                  #ifdef IOS_RADIO_DEBUG
                     NSLog( @"IOS_RADIO_DEBUG: Received message is not a IOsLinkMessage: %s\n", buf );
                  #endif
               } break;
            }
         }
         
      }
      
   private:
      iOsSystem& system_;
      ComTestbedRadioController* iOsRadio_;
   };
}

#endif
