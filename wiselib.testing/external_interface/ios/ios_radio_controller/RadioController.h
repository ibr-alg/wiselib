/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

//
//  RadioController.h
//  Network
//
//  Created by Marcus on 21.12.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "util/delegates/delegate.hpp"

#include "util/wisebed_node_api/command_types.h"
#include "util/ios_node_api/ios_link_message.h"

#include "AsyncSocket/AsyncSocket.h"

typedef uint16_t node_id_t;
typedef uint8_t block_data_t;
typedef uint8_t message_id_t;
typedef __darwin_size_t size_t;

typedef delegate3<void, node_id_t, size_t, block_data_t*> radio_delegate_t;

enum DECODER {
   SUCCESS = 0,
   ERROR_UNSPEC = -1,
   READ_HEADER = 100,
   READ_BODY = 101,
   HEADER_LENGTH = 4,
   TIME_OUT = 60,
   NO_TIME_OUT = -1
   
};

@interface RadioController : NSObject<NSStreamDelegate> {
     
      AsyncSocket *asyncSocket_;      
      radio_delegate_t callback_;
      uint16_t radio_id_;
      
      NSString* url_;
      UInt16 port_;
}

- (int) enable;
- (int) disable;
- (int) send:(NSData*)data;
- (void) registerReceiver:(radio_delegate_t)callback;
- (void) fireDelegateWithData:(const uint8_t*)aBuffer lenght:(size_t)len from:(node_id_t)aFrom;
- (node_id_t) getRadioId;
- (void) setConnectionURL:(NSString *)url port:(UInt16)port;

@end
