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

gateway: isense.5148
server: isense.5148
client: isense

export APP_SRC=DPS_IPv6_test.cpp

ifeq (server,$(firstword $(MAKECMDGOALS)))
export BIN_OUT=DPS_IPv6_test_48
ADD_CXXFLAGS=-DDPS_IPv6_SKELETON
else ifeq (gateway,$(firstword $(MAKECMDGOALS)))
export BIN_OUT=DPS_IPv6_test_48_gw
ADD_CXXFLAGS='-DDPS_IPv6_SKELETON -DIPv6_SLIP'
else ifeq (client,$(firstword $(MAKECMDGOALS)))
export BIN_OUT=DPS_IPv6_test_39
ADD_CXXFLAGS='-DDPS_IPv6_STUB -DDPS_COMPILE_ONLY_CLIENT_CODES'
#  -DDPS_COMPILE_ONLY_CLIENT_CODES saves ~1KB 
endif

include ../Makefile
