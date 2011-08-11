#!/bin/sh
# generates obj-c protobuffer files

TESTBED_RUNTIME=/Developer/testbed-runtime/

WSNAPP_PROTO_PATH=$TESTBED_RUNTIME/iwsn/runtime.wsn-app/src/main/resources/
MESSAGES_PROTO_PATH=$TESTBED_RUNTIME/iwsn/runtime/src/main/resources/

WSNAPP_PROTO_FILE=WSNAppMessages.proto
MESSAGES_PROTO_FILE=Messages.proto
IOSCOCOS_PROTO_FILE=IOSAppMessage.proto

cp $WSNAPP_PROTO_PATH/$WSNAPP_PROTO_FILE \
	 $MESSAGES_PROTO_PATH/$MESSAGES_PROTO_FILE ./

protoc --objc_out=./ ./$WSNAPP_PROTO_FILE
protoc --objc_out=./ ./$MESSAGES_PROTO_FILE
protoc --objc_out=./ ./$IOSCOCOS_PROTO_FILE

#mv Messages.pb.cc Messages.pb.cpp
#mv WSNAppMessages.pb.cc WSNAppMessages.pb.cpp

#rm $WSNAPP_PROTO_FILE $MESSAGES_PROTO_FILE


sed -i '' -e 's,<ProtocolBuffers/ProtocolBuffers.h>,"ProtocolBuffers.h",' Messages.pb.h
sed -i '' -e 's,<ProtocolBuffers/ProtocolBuffers.h>,"ProtocolBuffers.h",' WSNAppMessages.pb.h
sed -i '' -e 's,<ProtocolBuffers/ProtocolBuffers.h>,"ProtocolBuffers.h",' IOSAppMessage.pb.h