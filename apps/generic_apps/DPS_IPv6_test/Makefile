# ----------------------------------------
# Environment variable WISELIB_PATH needed
# ----------------------------------------

all: shawn
# all: scw_msb
# all: contiki_msb
# all: contiki_micaz
# all: isense
# all: tinyos-tossim
# all: tinyos-micaz

server: isense.5148
client: isense

export APP_SRC=DPS_IPv6_test.cpp

ifeq (server,$(firstword $(MAKECMDGOALS)))
export BIN_OUT=DPS_IPv6_test_48
ADD_CXXFLAGS=-DDPS_IPv6_SKELETON
else ifeq (client,$(firstword $(MAKECMDGOALS)))
export BIN_OUT=DPS_IPv6_test_39
ADD_CXXFLAGS=-DDPS_IPv6_STUB
endif

include ../Makefile
