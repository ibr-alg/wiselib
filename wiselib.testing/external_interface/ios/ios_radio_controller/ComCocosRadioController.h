//
//  ComCocosRadioController.h
//  Network
//
//  Created by Marcus on 21.12.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "proto/IOSAppMessage.pb.h"

#import "RadioController.h"

@interface ComCocosRadioController : RadioController {
}
- (int) send:(Envelope*) envelope;
- (int) sendTo: (node_id_t)to withData: (block_data_t*)data withLenght: (size_t)len;

- (void)login;
- (void)logout;

@end
