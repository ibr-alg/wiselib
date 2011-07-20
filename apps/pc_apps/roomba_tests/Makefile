-include Makefile.properties

WISELIB_PATH?=../..
WISELIB_STABLE=$(WISELIB_PATH)/wiselib.stable
WISELIB_TESTING=$(WISELIB_PATH)/wiselib.testing
QT_INC_PATH?=/usr/include/qt4
QT_LIB_PATH?=/usr/lib

INCLUDES=-I$(QT_INC_PATH) -I$(QT_INC_PATH)/QtCore -I$(QT_INC_PATH)/QtGui \
  -I$(WISELIB_STABLE) -I$(WISELIB_TESTING)
LDFLAGS=-L$(QT_LIB_PATH) -lpthread -lQtGui
MOC=moc
CXX=g++
LD=ld
CXXFLAGS=-g -O0 $(INCLUDES)

all: roomba_test soft_start_test mean_correction_test
clean:
	rm -f roomba_test battery_test svnrevision.h *_moc.cc *.o

svnrevision.h:
	./svnrevision.sh

%.o : %.cc %.h svnrevision.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<
%_moc.o : %.h
	$(MOC) $< -o $(@:.o=.cc)
	$(CXX) -c $(CXXFLAGS) -o $@ $(@:.o=.cc)

roomba_test: main.o stuff.o target_value_input_dialog.o target_value_input_dialog_moc.o
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

battery_test: battery_test.cc
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

soft_start_test: soft_start.o stuff.o target_value_input_dialog.o target_value_input_dialog_moc.o
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

mean_correction_test: mean_correction.o stuff.o target_value_input_dialog.o target_value_input_dialog_moc.o
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

.PHONY: svnrevision.h
# vim: noexpandtab:
