### Do not edit -- Generated by 'configure --with-whatever' from Makefile.in
### RtMidi library Makefile

RM = /bin/rm
LN = /bin/ln

OBJECTS	= RtMidi.o

LIBNAME = librtmidi
STATIC = $(LIBNAME).a
SHARED = librtmidi.so
RELEASE = 2.1.0
MAJOR = 2
LIBRARIES = $(STATIC) $(SHARED)

CC       = g++
AR       = /usr/bin/ar
RANLIB   = ranlib

DEFS     =  -I/usr/local/include  -D__LINUX_ALSA__
CFLAGS   = -O3 -Wall -Wextra -Iinclude -fPIC

PREFIX   = /usr/local

all : $(LIBRARIES)

tests:
	cd tests && $(MAKE) all

$(LIBRARIES): $(OBJECTS)
	$(AR) ruv $(STATIC) $(OBJECTS)
	ranlib $(STATIC)
	$(CC) -fPIC -shared -Wl,-soname,$(SHARED).$(MAJOR) -o $(SHARED).$(RELEASE) $(OBJECTS) -lpthread -lasound  -L/usr/local/lib
	$(LN) -sf librtmidi.so.$(RELEASE) $(SHARED)
	$(LN) -sf librtmidi.so.$(RELEASE) $(SHARED).$(MAJOR)

#	$(CC) -shared $(OBJECTS) -o $(SHARED) -lpthread -lasound  -L/usr/local/lib

%.o : %.cpp
	$(CC) $(CFLAGS) $(DEFS) -c $(<) -o $@

%.o : include/%.cpp
	$(CC) $(CFLAGS) $(DEFS) -c $(<) -o $@

install: all
	install --mode=755 $(STATIC) $(PREFIX)/lib/
	install --mode=755 librtmidi.so.$(RELEASE) $(PREFIX)/lib/
	$(LN) -sf librtmidi.so.$(RELEASE) $(PREFIX)/lib/$(SHARED)
	$(LN) -sf librtmidi.so.$(RELEASE) $(PREFIX)/lib/$(SHARED).$(MAJOR)
	install --mode=644 $(LIBNAME).pc $(PREFIX)/lib/pkgconfig
	install --mode=755 rtmidi-config $(PREFIX)/bin/
	install --mode=644 RtMidi.h RtError.h $(PREFIX)/include

uninstall:
	-@rm -vf $(patsubst %,$(PREFIX)/lib/%, $(LIBRARIES) $(SHARED).$(MAJOR) $(SHARED).$(RELEASE))
	-@rm -vf $(PREFIX)/lib/pkgconfig/$(LIBNAME).pc
	-@rm -vf $(PREFIX)/bin/rtmidi-config

clean : 
	$(RM) -f $(LIBRARIES) librtmidi.so.$(RELEASE) $(SHARED)*
	$(RM) -f $(OBJECTS)
	$(RM) -f *~
	cd tests && $(MAKE) clean

distclean:
	$(RM) -f $(LIBRARIES) librtmidi.so.$(RELEASE) $(SHARED)*
	$(RM) -f $(OBJECTS)
	$(RM) -f *~
	$(RM) -rf config.log config.status autom4te.cache Makefile rtmidi-config $(LIBNAME).pc
	cd tests && $(MAKE) distclean

strip : 
	strip $(LIBRARIES)
	ranlib $(LIBRARIES)
	cd tests && $(MAKE) strip

.PHONY: strip distclean clean uninstall
