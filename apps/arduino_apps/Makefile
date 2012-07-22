WISELIB_PATH=../..

WISELIB_STABLE=$(WISELIB_PATH)/wiselib.stable
WISELIB_TESTING=$(WISELIB_PATH)/wiselib.testing

CXX=avr-gcc
INCLUDE=-I ~/work/arduino/include/arduino -I$(WISELIB_STABLE) -I$(WISELIB_TESTING)
LIBS=-L ~/work/arduino/lib -lm -larduino
MCU=-mmcu=atmega328p
CPU_SPEED=-DF_CPU=16000000UL
CFLAGS=$(MCU) $(CPU_SPEED) -Os -w -Wl,--gc-sections -ffunction-sections -fdata-sections
PORT=/dev/ttyACM0

default: build upload

build: example.hex

example.hex: example.elf
	avr-objcopy -O ihex $< $@

example.elf: example_app.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LIBS)

upload:
	avrdude -V -F -p m328p -c arduino -b 115200 -Uflash:w:example.hex -P$(PORT)

clean:
	@echo -n Cleaning ...
	$(shell rm example.elf 2> /dev/null)
	$(shell rm example.hex 2> /dev/null)
	$(shell rm *.o 2> /dev/null)
	@echo " done"

%.o: %.cpp
	$(CXX) $< $(CFLAGS) $(INCLUDE) -c -o $@
