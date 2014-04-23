
all: pc

# For PC Os, end process after main method exited
# (you usually dont want that for a sensor node but for PC it can be useful
# for apps that are not supposed to run continously)
export WISELIB_EXIT_MAIN=1

export APP_SRC=block_example.cpp
export BIN_OUT=block_example

include ../Makefile

