PACKAGE = wavetools

SRC_ALL    = rawcut_autozcr.cpp rawcut_label.cpp common/detect_zcr.cpp common/label_utils.cpp
HEADER_ALL = common/detect_zcr.hpp common/label_utils.hpp

FILES	= Makefile $(SRC_ALL) $(HEADER_ALL)
VER	= `date +%Y%m%d`


### command and flags ###
LD	= g++
LDFLAGS	= $(DEBUG)
LDLIBS	= -lm
CC	= g++
CFLAGS	= -O2 -Wall $(DEBUG)
CPPFLAGS= -I.

### Default rules
.SUFFIXES: .o .cpp
.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

### Make application
all: rawcut_autozcr rawcut_label

rawcut_autozcr.o: $(HEADER_ALL)
rawcut_autozcr: rawcut_autozcr.o common/detect_zcr.o common/label_utils.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

rawcut_label.o: $(HEADER_ALL)
rawcut_label: rawcut_label.o common/detect_zcr.o common/label_utils.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

### Utilities
clean:
	rm -f *.o common/*.o
	rm -f rawcut_label.exe rawcut_autozcr.exe
