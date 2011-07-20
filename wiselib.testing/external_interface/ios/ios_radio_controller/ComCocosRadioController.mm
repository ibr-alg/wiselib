//
//  ComCocosRadioController.m
//  Network
//
//  Created by Marcus on 21.12.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ComCocosRadioController.h"

@implementation ComCocosRadioController

- (void)login {
    // build cocos login msg
    Envelope *envelope = [[[[Envelope builder] 
                            setMsgType:Envelope_MsgTypeLogin] 
                           setLoginMessage:
                                [[[[LoginMessage builder] 
                                   setLogin:YES] 
                                  setDeviseId:[self getRadioId]] 
                                 build]]
                          build];
    
    [self send:envelope];
}

- (void) logout {
    Envelope *envelope = [[[[Envelope builder] 
                            setMsgType:Envelope_MsgTypeLogin] 
                           setLoginMessage:[[[[LoginMessage builder] setLogin:NO] setDeviseId:[self getRadioId]] build] ]
                          build];
    
    [self send:envelope];    
}

- (int) disable {
    [self logout];
    return [super disable];
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)remoteHost port:(UInt16)remotePort
{
	NSLog(@"Socket is connected!");
    NSLog(@"Remote Address: %@:%hu", remoteHost, remotePort);
	NSString *localHost = [sock localHost];
	UInt16 localPort = [sock localPort];
	NSLog(@"Local Address: %@:%hu", localHost, localPort); 
    
    [self login];
    
    [sock readDataToLength:HEADER_LENGTH withTimeout:TIME_OUT tag:READ_HEADER];
}

- (int) sendTo: (node_id_t)to withData: (block_data_t*)data withLenght: (size_t)len {
    
    @try {
        NSArray *cacheData = [NSKeyedUnarchiver unarchiveObjectWithData:[NSData dataWithBytes:data length:len]];
        NSMutableArray *arrayOfSensorMessages = [NSMutableArray arrayWithCapacity:[cacheData count]];
        
        for(id obj in cacheData) {
            if([obj isKindOfClass:[NSData class]]) {
                [arrayOfSensorMessages addObject:[SensorMessage parseFromData:obj]];
                //NSLog(@"%i",[[SensorMessage parseFromData:obj] id]);
            }
        }
       
        Envelope *envelope = [[[[[[Envelope builder]
                                  setMsgType:Envelope_MsgTypeOperationInvocationRequest] 
                                 setSource:0] 
                                setDestination:0] 
                               setSensorMessageArray:arrayOfSensorMessages]
                              build];
        
        return [self send:envelope];
    }
    @catch (NSException *exception) {
        NSLog(@"Error");
        return ERROR_UNSPEC;
    }    
    
}

- (int) send:(Envelope*) envelope {
    
    uint8_t len = [[envelope data] length]; 
    uint8_t header[4];
    
    header[0] = (len >> 24) & 0xff;
    header[1] = (len >> 16) & 0xff;
    header[2] = (len >> 8) & 0xff;
    header[3] = len & 0xff;
    
    NSMutableData *buffer = [NSMutableData dataWithBytes:header length:4];
    [buffer appendData:[envelope data]];
    
    return [super send:buffer];
}


- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag {
    
    uint8_t buf[4];
    
    [data getBytes:buf length:4];
    
    if(tag == READ_HEADER) {
        
        
        uint32_t rcvd_length = buf[0] << 24 | buf[1] << 16 |
        buf[2] << 8 | buf[3];
        
        // read data
        [sock readDataToLength:rcvd_length withTimeout:TIME_OUT tag:READ_BODY];
    }
    
    if(tag == READ_BODY) {
        
       
        Envelope *envelope = [Envelope parseFromData:data];
        
        // check destination!
        //
        // if((node_id_t)[envelope destination] == [self getRadioId]) {
        //    
        // }
        
        if([envelope msgType] == Envelope_MsgTypeListenerEvent) {
            for (SensorMessage* msg in envelope.sensorMessage) {
                
                NSData* data = [msg data];
                [self fireDelegateWithData:(uint8_t*)[data bytes] lenght:[data length] from:[msg id]];
            }
        }
            
        //[self fireDelegateWithData:binary lenght:[binDataWithoutHeader length] from: sourceId];
        
        
        // read header
        [sock readDataToLength:HEADER_LENGTH withTimeout:TIME_OUT tag:READ_HEADER];
    }
}


@end
