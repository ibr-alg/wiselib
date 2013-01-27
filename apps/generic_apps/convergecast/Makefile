# ------------------------------------------------
# Environment variable WISELIB_PATH_TESTING needed
# ------------------------------------------------

#all: isense
all: shawn
# all: scw_msb
# all: contiki_msb
# all: contiki_micaz
# all: contiki_sky
# all: isense
# all: tinyos-tossim
# all: tinyos-micaz
# all: tinyos-telosb

export APP_SRC=convergecast_example_app.cpp
export BIN_OUT=convergecast_example_app

include $(WISELIB_PATH)/../apps/generic_apps/Makefile.local
include $(WISELIB_GENERIC_APPS)/Makefile

tubs-hallway:
	@echo "----- Building Tubs Hallway Application ------"
	@$(MAKE) TARGET=$(BIN_OUT) -f $(FLOOR13_PATH)/isense/Makefile.build

tubs-hallway-clean:
	@echo "----- Cleaning Applications ------"
	@$(MAKE) TARGET=$(BIN_OUT) --no-print-directory -f $(FLOOR13_PATH)/isense/Makefile.build clean