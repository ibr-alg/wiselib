#!/bin/bash

PYTHON=python3

cd experiments
ALL=$(echo 2*/)
cd -
time $PYTHON ./plot.py $ALL |tee plot.log

