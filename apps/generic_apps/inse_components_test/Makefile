# ----------------------------------------
# Environment variable WISELIB_PATH needed
# ----------------------------------------

all: shawn

export APP_SRC=app.cpp
export BIN_OUT=app

# When on PC, compile with debug flags on
#export PC_COMPILE_DEBUG=1
#export ADD_CXXFLAGS="-DSHAWN_DEBUG_ADD_NODE_ID=1"

# But eg. for contiki, no debug stuff please!
export ADD_CXXFLAGS="-DWISELIB_ALLOW_SLEEP=1 -DWISELIB_ALLOW_DOZE=1 -DNDEBUG -Wl,-lc -Wl,-L/opt/mspgcc/lib -DUSE_LEDS"

include ../Makefile
