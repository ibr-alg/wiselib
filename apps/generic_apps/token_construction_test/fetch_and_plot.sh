#!/bin/sh

JOBID=$1

rsync -CHaPx hasemann@wilabfs.atlantis.ugent.be:iPlatform/contiki_platform/log/$JOBID ./ &&
	cat $JOBID/*/output.txt | sort > log.txt &&
	python2 ./plot.py


