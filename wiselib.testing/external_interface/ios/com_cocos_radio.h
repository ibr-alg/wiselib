#ifndef CONNECTOR_COM_COCOS_RADIOMODEL_H
#define CONNECTOR_COM_COCOS_RADIOMODEL_H

#include "external_interface/ios/ios_system.h"
#include "external_interface/ios/ios_model.h"
#include "util/delegates/delegate.hpp"
#include "util/base_classes/radio_base.h"

#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"

#include "external_interface/ios/ios_radio_controller/ComCocosRadioController.h"

namespace wiselib
{

   template<typename OsModel_P>
   class ComCocosRadioModel
      : public RadioBase<OsModel_P, uint16_t, __darwin_size_t, uint8_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef ComCocosRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;
      
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
      ComCocosRadioModel(iOsSystem& system)
         : system_(system)
      {
         iOsRadio_ = [[ComCocosRadioController alloc] init];
      }
      // --------------------------------------------------------------------
      virtual ~ComCocosRadioModel()
      {
         [iOsRadio_ release];
         iOsRadio_ = nil;
      }
      
      // --------------------------------------------------------------------
      // todo 
       void set_testbed( NSString* url, UInt16 port )
      {
         [iOsRadio_ setConnectionURL:url port:port];
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
#ifdef IOS_RADIO_DEBUG
      NSLog(@"COM_COCOS_RADIO_DEBUG: Radio enable");
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
          return [iOsRadio_ sendTo:to withData:data withLenght:len];
      }
      // --------------------------------------------------------------------
      void receive_message( node_id_t from, size_t len, block_data_t *buf )
      {
          this->notify_receivers( from, len, buf );
      }
      
   private:
      iOsSystem& system_;
      ComCocosRadioController* iOsRadio_;
   };
}

#endif
