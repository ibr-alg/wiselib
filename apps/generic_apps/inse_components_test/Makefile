# ----------------------------------------
# Environment variable WISELIB_PATH needed
# ----------------------------------------

all: shawn
# all: scw_msb
# all: contiki_msb
# all: contiki_micaz
#all: contiki_inga
# all: isense
# all: tinyos-tossim
# all: tinyos-micaz

export APP_SRC=app.cpp
export BIN_OUT=app
export PC_COMPILE_DEBUG=1
export ADD_CXXFLAGS="-DSHAWN_DEBUG_ADD_NODE_ID=1"

#export ADD_CXXFLAGS="-DWISELIB_ALLOW_SLEEP=1 -DWISELIB_ALLOW_DOZE=1"

include ../Makefile
