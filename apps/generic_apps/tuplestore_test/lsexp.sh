#!/bin/bash
EXP_DIR=experiments

for f in $EXP_DIR/2*; do
	if [ -d "$f" ]; then
		#echo $f
		for ff in $f/*/*.vars; do
			. $ff
			if [ "$DEBUG" == "1" ]; then
				DBG="DEBUG"
			else
				DBG=""
			fi
			if [ "${DATABASE}" == "tuplestore" ]; then
				echo $f ${INODE_DB} ${DATABASE}.${TS_DICT} $MODE $DBG
			else
				echo $f ${INODE_DB} ${DATABASE} $MODE $DBG
			fi
		done
	fi
done
	

