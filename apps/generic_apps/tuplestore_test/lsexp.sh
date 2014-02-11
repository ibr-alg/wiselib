#!/bin/bash
EXP_DIR=experiments

for f in $EXP_DIR/248*; do
	if [ -d "$f" ]; then
		echo $f
		for ff in $f/*/*.vars; do
			. $ff
			echo "  "$INODE_DB $DATABASE/$MODE
		done
	fi
done
	

