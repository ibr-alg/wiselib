#!/bin/bash

cd experiments
ALL=$(echo 2*/)
cd -
python ./plot.py $ALL |tee plot.log

