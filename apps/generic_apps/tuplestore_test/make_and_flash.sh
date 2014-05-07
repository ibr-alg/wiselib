#!/bin/bash

make -f Makefile.gateway &&
make -f Makefile.database &&
sudo make -f Makefile.gateway flash-sky &&
sudo make -f Makefile.database PORT=/dev/ttyUSB1 flash-sky

