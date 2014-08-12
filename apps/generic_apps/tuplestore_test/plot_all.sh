#!/bin/bash

PYTHON=python3

cd experiments
ALL=$(echo 2*/)
cd -
#time nice $PYTHON ./plot_debug.py $ALL 2>&1|tee plot.log
#time nice $PYTHON -u ./plot_ng.py $ALL 2>&1|disgruntle plot_all_ng.log
until nice $PYTHON -u ./plot_ng.py $ALL 2>&1|disgruntle plot_all_ng.log; do
	echo $(date)
done

