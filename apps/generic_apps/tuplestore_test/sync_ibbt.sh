#!/bin/bash
EXP_DIR=experiments/
HOST=wilabfs.atlantis.ugent.be
USER=hasemann
PLATFORM_DIR=iPlatform/tuplestore_platform/
APP_DIR=upload


#FILENAME_DATABASE=app_20140107091518.exe
#FILENAME_GATEWAY=app_ackto2000_r3_20131003082019.exe

rsync -P ./out/pc/app_host start_mount_code $EXP_DIR/inode*.vars *.rdf $USER@$HOST:$PLATFORM_DIR

#echo rsync ./out/contik-sky/app_database.exe $USER@$HOST:$APP_DIR/app_database_20140116235959.exe
#rsync -P ./out/contiki-sky/app_database.exe $USER@$HOST:$APP_DIR/$FILENAME_DATABASE
#rsync -P ./out/contiki-sky/app_gateway.exe $USER@$HOST:$APP_DIR/$FILENAME_GATEWAY
rsync -P $EXP_DIR/*.exe $USER@$HOST:$APP_DIR/

