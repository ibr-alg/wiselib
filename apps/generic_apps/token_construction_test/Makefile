# ----------------------------------------
# Environment variable WISELIB_PATH needed
# ----------------------------------------

all: pc
# all: scw_msb
# all: contiki_msb
# all: contiki_micaz
# all: isense
# all: tinyos-tossim
# all: tinyos-micaz

export APP_SRC=app.cpp
export BIN_OUT=app

#export APP_SRC=example_app.cpp
#export BIN_OUT=example_app

#export APP_SRC=format_sd.cpp
#export BIN_OUT=format_sd
#export WISELIB_EXIT_MAIN=1


#export PC_COMPILE_DEBUG=1
export ADD_CXXFLAGS="-Wall"

export ARDUINO_USE_XBEE=1
#export ADD_CXXFLAGS="-Wno-format -Wno-unused-local-typedefs"

include ../Makefile
