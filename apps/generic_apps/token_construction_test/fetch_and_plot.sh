#!/bin/sh

JOBID=$1

rsync -CHaPx hasemann@wilabfs.atlantis.ugent.be:iPlatform/contiki_platform/log/$JOBID ./ &&
	
rm log_unsorted.txt
for file in $JOBID/*/output.txt; do
	addr=$(grep '|@' $file|head -n 1|sed 's/.*|@\([0-9]\+\) .*/\1/')
	if [ -z "$addr" ]; then
		addr=$(basename $(dirname $file))
	fi
	echo $file '-> '$addr
	sed 's/^\(T[0-9]\+\.[0-9]\+\)|\(.*\)$/\1|@'$addr' \2/' $file >> log_unsorted.txt
done	

sort log_unsorted.txt > log.txt
python2 ./plot.py


