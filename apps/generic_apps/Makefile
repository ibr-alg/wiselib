TOP := $(dir $(lastword $(MAKEFILE_LIST)))
include $(TOP)/Makefile.local

###############################################################
##
## Parameters
#####

###
# PORT is used for flash-targets. E.g., type
#   flash-sky TARGET=/dev/ttyUSB1
# 
ifndef PORT
   export PORT=/dev/ttyUSB0
endif

###
# PORT change node id in tinyos hex. E.g., type
#   make tinyos-nodeid-{micaz|gnode|telosb} TOS_NODE_ID=77
#
ifndef TOS_NODE_ID
   TOS_NODE_ID=1
endif

ifndef WISELIB_EXIT_MAIN
   WISELIB_EXIT_MAIN=0
endif


###############################################################
##
## Compile Application for specific OSs
#####

arduino:
	$(MAKE) -f $(WISELIB_BASE)/apps/generic_apps/Makefile.arduino arduino ADD_CXXFLAGS=$(ADD_CXXFLAGS)

contiki_msb:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.contiki contiki_msp TARGET=msb430 ADD_CXXFLAGS=$(ADD_CXXFLAGS)

contiki_sky:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.contiki contiki_msp TARGET=sky ADD_CXXFLAGS=$(ADD_CXXFLAGS)

contiki_native:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.contiki contiki TARGET=native ADD_CXXFLAGS=$(ADD_CXXFLAGS)

contiki_micaz:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.contiki contiki_atmel TARGET=micaz ADD_CXXFLAGS=$(ADD_CXXFLAGS)

contiki_inga:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.contiki contiki_atmel TARGET=inga ADD_CXXFLAGS=$(ADD_CXXFLAGS)

feuerware:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.feuerware ADD_CXXFLAGS=$(ADD_CXXFLAGS)

isense:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.isense JENNIC_CHIP=JN5139R1 ADD_CXXFLAGS=$(ADD_CXXFLAGS)

isense.5148:
	#make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.isense JENNIC_CHIP=JN5148 ADD_CXXFLAGS="$(ADD_CXXFLAGS)"
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.isense JENNIC_CHIP=JN5148 ADD_CXXFLAGS=$(ADD_CXXFLAGS)

lorien_sky:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.lorien ADD_CXXFLAGS=$(ADD_CXXFLAGS)

pc:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.pc WISELIB_EXIT_MAIN=$(WISELIB_EXIT_MAIN) ADD_CXXFLAGS=$(ADD_CXXFLAGS) PC_CXX_FLAGS=$(PC_CXX_FLAGS)

scw_msb:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.scw scw_msb ADD_CXXFLAGS=$(ADD_CXXFLAGS)

shawn:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.shawn ADD_CXXFLAGS=$(ADD_CXXFLAGS)

trisos:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.trisos ADD_CXXFLAGS=$(ADD_CXXFLAGS)

tinyos-telosb:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.tinyos.telosb telosb tinyos-telosb ADD_CXXFLAGS=$(ADD_CXXFLAGS)

tinyos-micaz:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.tinyos.micaz micaz tinyos-micaz ADD_CXXFLAGS=$(ADD_CXXFLAGS)

tinyos-inga:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.tinyos.inga inga tinyos-inga ADD_CXXFLAGS=$(ADD_CXXFLAGS)

tinyos-tossim:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.tinyos.tossim micaz sim tinyos-tossim ADD_CXXFLAGS=$(ADD_CXXFLAGS)

tinyos-gnode:
	make -f $(WISELIB_BASE)/apps/generic_apps/Makefile.tinyos.gnode gnode tinyos-gnode ADD_CXXFLAGS=$(ADD_CXXFLAGS)


###############################################################
##
## Flash hex file to node
#####
flash-jtag:
	$(MSP430_JTAG) -mr -p out/$(BIN_OUT).hex

flash-sky:
	$(CONTIKI_PATH)/tools/sky/msp430-bsl-linux --telosb -D \
		-c $(PORT) -r -e -I -p out/$(BIN_OUT).hex

flash-micaz:
	uisp -dprog=mib510 -dserial=$(PORT) -dpart=ATmega128 --wr_fuse_h=0xd1 \
		--wr_fuse_e=ff --erase --upload if=out/$(BIN_OUT).hex --verify

flash-tinyos-sky:
	tos-bsl --telosb -c $(PORT) -r -e -I -p out/$(BIN_OUT).hex

flash-gnode:
	lnode-bsl -c $(PORT) -r -e -I -p out/$(BIN_OUT).hex

flash-inga:
	inga_tool -d $(PORT)
	avrdude -v -b 230400 -P $(PORT) -c avr109 -p atmega1284p -U flash:w:out/$(BIN_OUT).hex
	
flash-arduino:
	$(MAKE) -f $(WISELIB_BASE)/apps/generic_apps/Makefile.arduino flash ADD_CXXFLAGS=$(ADD_CXXFLAGS)

monitor-arduino:
	$(MAKE) -f $(WISELIB_BASE)/apps/generic_apps/Makefile.arduino monitor ADD_CXXFLAGS=$(ADD_CXXFLAGS)


###############################################################
##
## Change TinyOS node ids
#####

tinyos-nodeid-micaz:
	cp build/micaz/main.exe out/$(BIN_OUT).exe
	tos-set-symbols out/$(BIN_OUT).srec out/$(BIN_OUT).srec-$(TOS_NODE_ID) \
	  TOS_NODE_ID=$(TOS_NODE_ID) ActiveMessageAddressC__addr=$(TOS_NODE_ID)
	avr-objcopy out/$(BIN_OUT).srec-$(TOS_NODE_ID) out/$(BIN_OUT).hex

# Alternatively possible:
#   make -f $MAKERULES telosb reinstall,22
tinyos-nodeid-telosb:
	tos-set-symbols --objcopy msp430-objcopy --objdump msp430-objdump \
	  --target ihex  build/telosb/main.exe out/$(BIN_OUT).hex-$(TOS_NODE_ID) \
	  TOS_NODE_ID=$(TOS_NODE_ID) ActiveMessageAddressC__addr=$(TOS_NODE_ID)
	cp out/$(BIN_OUT).hex-$(TOS_NODE_ID) out/$(BIN_OUT).hex

tinyos-nodeid-gnode:
	tos-set-symbols --objcopy msp430-objcopy --objdump msp430-objdump \
	  --target ihex  build/gnode/main.exe out/$(BIN_OUT).hex-$(TOS_NODE_ID) \
	  TOS_NODE_ID=$(TOS_NODE_ID) ActiveMessageAddressC__addr=$(TOS_NODE_ID)
	cp out/$(BIN_OUT).hex-$(TOS_NODE_ID) out/$(BIN_OUT).hex


###############################################################
##
## Clean
#####
clean:
	rm -Rf out/contiki-* out/feuerware out/isense out/pc out/scw out/shawn out/tinyos-* out/lorien-sky out/arduino \
		obj_* symbols.* contiki-* \
		_TOSSIMmodule.so TOSSIM.* build/ simbuild/ app.xml
	rm -f out/*
	find . -executable -type f -delete
	rm *.d *.hex
	rmdir out
