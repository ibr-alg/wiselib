#!/bin/bash

cd experiments
ALL=$(echo 2*/)
cd -
time python ./plot.py $ALL |tee plot.log

