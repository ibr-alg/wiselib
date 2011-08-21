//
//  RadioController.m
//  Network
//
//  Created by Marcus on 21.12.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RadioController.h"

@implementation RadioController

- (id) init {
   if ( (self = [super init]) != nil ) {
      // use last 4 chars of uniqueIdentifier to generate uint16_t radio id
      radio_id_ = (uint16_t)strtol( [[[[UIDevice currentDevice] uniqueIdentifier] substringFromIndex:32] UTF8String], (char **)NULL, 16 );
      asyncSocket_ = [[AsyncSocket alloc] initWithDelegate:self];
   }
   return self;
   
}
- (void) setConnectionURL:(NSString *)url port:(UInt16)port {
   url_ = url;
   [url_ retain];
   port_ = port;
}


- (int) enable {
      
   if(url_ == nil) {
      return ERROR_UNSPEC;
   }

#ifdef IOS_RADIO_CONTROLLER_DEBUG
    NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: Connecting to %@ on Port %d", url_, port_);
#endif   

   NSError *error = nil;
	//if (![asyncSocket_ connectToHost:url onPort:port withTimeout:NO_TIME_OUT error:&error])
   if (![asyncSocket_ connectToHost:url_ onPort:port_ withTimeout:NO_TIME_OUT error:&error])
	{
#ifdef IOS_RADIO_CONTROLLER_DEBUG
    NSLog(@"Error connecting: %@, %@", [error code], [error localizedDescription]);
#endif
      return ERROR_UNSPEC;
   }
   return SUCCESS;

}

- (int) disable {
    [asyncSocket_ disconnectAfterReadingAndWriting];
    return SUCCESS;
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)remoteHost port:(UInt16)remotePort
{
    
#ifdef IOS_RADIO_CONTROLLER_DEBUG
	NSLog(@"Socket is connected!");
    NSLog(@"Remote Address: %@:%hu", remoteHost, remotePort);
	NSString *localHost = [sock localHost];
	UInt16 localPort = [sock localPort];
	NSLog(@"Local Address: %@:%hu", localHost, localPort);
#endif   

   [sock readDataToLength:HEADER_LENGTH withTimeout:TIME_OUT tag:READ_HEADER];
}

- (void)socketDidDisconnect:(AsyncSocket *)sock withError:(NSError *)err
{
#ifdef IOS_RADIO_CONTROLLER_DEBUG
	NSLog(@"socketDidDisconnect:%p withError: %@", sock, err);
#endif
}

- (void)onSocketDidDisconnect:(AsyncSocket *)sock 
{
#ifdef IOS_RADIO_CONTROLLER_DEBUG
   NSLog(@"socketDidDisconnect");
#endif
}

- (int) send:(NSData*)data {
   
   if([asyncSocket_ isConnected]) {

#ifdef IOS_RADIO_CONTROLLER_DEBUG
      //NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: Send data %@",data);
#endif

      [asyncSocket_ writeData:data withTimeout:TIME_OUT tag:0];
       return SUCCESS;
   }
    
#ifdef IOS_RADIO_CONTROLLER_DEBUG
    NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: can not send because is not connected");
#endif
   
   return ERROR_UNSPEC;
}

- (void)onSocket:(AsyncSocket *)sock didWriteDataWithTag:(long)tag {
   if (tag == 0) {
      //NSLog(@"Data sent!!!");
   }
}

- (void) registerReceiver: (radio_delegate_t)callback {

   callback_ = callback;
}

- (void) fireDelegateWithData: (const uint8_t*)data lenght:(size_t)dataLen from:(node_id_t)nodeId{
   callback_( nodeId, dataLen, const_cast<uint8_t*>(data) );
}

- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag {
#ifdef IOS_RADIO_CONTROLLER_DEBUG
    NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: Incomming Data");
#endif    
}

- (node_id_t) getRadioId {

         // use last 4 chars of uniqueIdentifier to generate uint16_t radio id
         // NSString* myMac = [[NSString alloc]initWithString:[[UIDevice currentDevice] uniqueIdentifier]];
         // NSLog(@"%@",myMac);
         
   return radio_id_;
}

- (void) dealloc {
   [asyncSocket_ setDelegate:nil];
   [asyncSocket_ release];
   asyncSocket_ = nil;
   [url_ release];
   url_ = nil;
   [super dealloc];
}

@end
