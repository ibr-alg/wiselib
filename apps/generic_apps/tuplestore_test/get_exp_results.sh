#!/bin/bash

HOST=wilabfs.atlantis.ugent.be
USER=hasemann
LOGS_DIR=iPlatform/tuplestore_platform/log

EXP_ID=$1
EXP_DIR=experiments

if [ -z "$EXP_ID" ]; then
	EXP_ID=$(ssh $USER@$HOST "cd $LOGS_DIR; ls -1d 2*"|sort|tail -n 1)
fi

echo Getting logs for experiment $EXP_ID ....
rsync -CHaPx $USER@$HOST:$LOGS_DIR/$EXP_ID $EXP_DIR
#rsync -CHaPx $USER@$HOST:$LOGS_DIR/ ./

echo Dumping SQL databases for experiment $EXP_ID ....

echo "SELECT * FROM tuplestore_${EXP_ID}_1242" | ssh $USER@$HOST "mysql -B -h www.wilab.atlantis.ugent.be -uhasemann -preude123 hasemann > /tmp/${EXP_ID}.csv"

echo 'Compressing log data (remote) ....'
ssh $USER@$HOST "rm /tmp/${EXP_ID}.csv.gz; gzip /tmp/${EXP_ID}.csv"

echo 'Copying log data from remote ....'
mv $EXP_DIR/${EXP_ID}.csv.gz  $EXP_DIR/${EXP_ID}.csv.gz.bak
rsync -P $USER@$HOST:/tmp/${EXP_ID}.csv.gz $EXP_DIR/${EXP_ID}.csv.gz
#  ${EXP_ID}.csv

