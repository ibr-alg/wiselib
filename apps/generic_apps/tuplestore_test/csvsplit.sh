#!/bin/zsh

#
# Usage:
# configure the parameters below such that splitting will occur between
# different experiment runs (use plot.py to find out what the paramaters
# should be).
# Then call with the .csv.gz file as parameter.
#
# NOTE: Hard-coded assumption that its called with a .csv.gz (i.e. not a plain
# .csv!)


MEASUREMENT_INTERVAL=$((64.0 / 3000.0))
EXP_OFFSET=0
EXP_DELTA=2000

FNAME=$1
if [ ! -e "$FNAME" ]; then
	echo "syntax: $0 [filename]"
	exit 1
fi

echo Counting lines...
LINES=$(zcat $FNAME|wc -l)

echo Counting motes...
MOTES=$(($(zcat $FNAME|awk '{ print $87 }'|sort|uniq -c|wc -l) - 1))

LINES_PER_CHUNK=$(printf '%.0f' $(($MOTES * $EXP_DELTA / $MEASUREMENT_INTERVAL)))
BASENAME=${FNAME:r:r}

echo
echo file: $FNAME
echo basename: $BASENAME
echo size: $(stat -c %s $FNAME)
echo lines: $LINES
echo lines per chunk: $LINES_PER_CHUNK
echo chunks: $(($LINES / $LINES_PER_CHUNK))
echo motes: $MOTES
echo
echo press return to do the split
read

#zcat $FNAME|split -d -C $LINES_PER_CHUNK - ${BASENAME}_

rm ${BASENAME}_*.csv

L=0
O=0
HEAD=""
echo starting chunk 00
zcat $FNAME|while read line; do
	OFILE=${BASENAME}_$(printf '%02d' "$O").csv

	if [ -z "$HEAD" ]; then
		export HEAD="$line"
		continue
	fi
	if [ "$L" -eq "0" ]; then
		#rm $OFILE;
		echo "$HEAD" > $OFILE
	fi
	echo $line >> $OFILE 
	export L=$(($L + 1))
	if [ "$L" -ge "$LINES_PER_CHUNK" ]; then
		export L=0
		export O=$(($O + 1))
		printf "starting chunk %02d\n" $O
	fi
done


