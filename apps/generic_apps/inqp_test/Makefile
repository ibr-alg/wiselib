# ----------------------------------------
# Environment variable WISELIB_PATH needed
# ----------------------------------------
all: contiki_sky
#all: pc
# all: scw_msb
# all: contiki_msb
# all: contiki_micaz
# all: isense
# all: tinyos-tossim
# all: tinyos-micaz

export APP_SRC=app.cpp
export BIN_OUT=example_app
export PC_COMPILE_DEBUG=1
#export WISELIB_DISABLE_DEBUG_MESSAGES=1
#export WISELIB_DISABLE_DEBUG=1
export ADD_CXXFLAGS="-DWISELIB_ALLOW_SLEEP=1 -DWISELIB_ALLOW_DOZE=1"

include ../Makefile
