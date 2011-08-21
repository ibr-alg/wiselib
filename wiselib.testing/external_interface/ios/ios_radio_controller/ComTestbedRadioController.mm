//
//  ComTestbedRadioController.m
//  Network
//
//  Created by Marcus on 21.12.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ComTestbedRadioController.h"

@implementation ComTestbedRadioController

- (int) sendTo: (node_id_t)to withData: (block_data_t*)data withLenght: (size_t)len {
    
    NSString* device_id = [NSString stringWithFormat:@"urn:wisebed:tubs:%i",[self getRadioId]];
    NSString* urn = [NSString stringWithFormat:@"urn:wisebed:tubs:%i",to];
    
    uint8_t binData[1];
    binData[0] = 10;
    
    NSMutableData *payload = [NSMutableData dataWithBytes:binData length:1];
    [payload appendData:[NSData dataWithBytes:data length:len]];
    
    Message *message = [[[[[Message builder]
                           setSourceNodeId:device_id]
                          setTimestamp:[[NSDate date] description]]
                         setBinaryData:payload]
                        build];       

    OperationInvocation *operation = [[[[OperationInvocation builder]
                                        setOperation:OperationInvocation_OperationSend]
                                       setArguments:[message data]]
                                      build];
    
    Msg *trmessage = [[[[[[[[Msg builder]
                            setMsgType:@"de.uniluebeck.itm.tr.runtime.wsnapp.WSNApp/OPERATION_INVOCATION_REQUEST"]
                           setFrom:device_id]
                          setTo:urn]
                         setPayload:[operation data]]
                        setPriority:2]
                       setValidUntil:([[NSDate date] timeIntervalSince1970]*1000) + 60000]
                      build];
    
    NSUInteger trmessageSize = [[trmessage data] length];
    
    uint8_t header[4];
    
    header[0] = (trmessageSize >> 24) & 0xff;
    header[1] = (trmessageSize >> 16) & 0xff;
    header[2] = (trmessageSize >> 8) & 0xff;
    header[3] = trmessageSize & 0xff;
    
    NSMutableData *outputBuffer = [NSMutableData dataWithBytes:header length:4];
    [outputBuffer appendData:[trmessage data]];
    
    return [super send:outputBuffer];
}

- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag {
    
    uint8_t buf[512];
    
    [data getBytes:buf length:[data length]];
    
    if(tag == READ_HEADER) {
        
        
        uint32_t rcvd_length = buf[0] << 24 | buf[1] << 16 |
        buf[2] << 8 | buf[3];
        
        // read data
        [sock readDataToLength:rcvd_length withTimeout:TIME_OUT tag:READ_BODY];
    }
    
    if(tag == READ_BODY) {
        
        Msg *trmessage = [Msg parseFromData:data];
        
        if ([[trmessage msgType] isEqualToString:@"de.uniluebeck.itm.tr.runtime.wsnapp.WSNApp/LISTENER_MESSAGE"]) {

#ifdef IOS_RADIO_CONTROLLER_DEBUG
            //NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: got LISTENER_MESSAGE");
#endif
            
            Message *message = [Message parseFromData:[trmessage payload]];
            [message sourceNodeId];
            
            NSRange range = NSMakeRange([[message sourceNodeId] length]-3, 3);
            node_id_t sourceId = (node_id_t)[[[message sourceNodeId] substringWithRange:range] intValue];
            
            ///////////////
            //uint8_t old_msg[[[message binaryData] length]];
            //[[message binaryData] getBytes:old_msg];
            //[self fireDelegateWithData:old_msg  lenght:[[message binaryData] length] from: sourceId];
            /////////////
            
            uint8_t messageHeader[1];
            [[message binaryData] getBytes:messageHeader range:NSMakeRange(0, 1)];
            
            
            //NSLog(@"%i", messageHeader[0]);
            //NSLog(@"%@", [[[NSString alloc] initWithBytes:[message binaryData] length:[[message binaryData] length] encoding:NSUTF8StringEncoding] autorelease] );
            //NSLog(@"%@", [message binaryData]);
            
            
            if( (messageHeader[0] & 0xff) == 0x6a || (messageHeader[0] & 0xff) == 0x69) {
                NSMutableData *binDataWithoutHeader = [NSMutableData dataWithData:[message binaryData]];
                [binDataWithoutHeader replaceBytesInRange:NSMakeRange(0, 1) withBytes:NULL length:0];
                
                
                uint8_t binary[ [binDataWithoutHeader length] ];
                [binDataWithoutHeader getBytes:binary];
                
#ifdef IOS_RADIO_CONTROLLER_DEBUG
    //NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: Message from: %i", sourceId);
#endif
                
                [self fireDelegateWithData:binary lenght:[binDataWithoutHeader length] from: sourceId];
            }
            
            
    
        } else {
#ifdef IOS_RADIO_CONTROLLER_DEBUG
    NSLog(@"IOS_RADIO_CONTROLLER_DEBUG: Incomming Message is not a LISTENER MESSAGE");
#endif
        }
        // read header
        [sock readDataToLength:HEADER_LENGTH withTimeout:TIME_OUT tag:READ_HEADER];
    }
}


@end
