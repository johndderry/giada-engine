AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -g -O0 -Wall -pedantic -Werror

bin_PROGRAMS = giada

giada_SOURCES = main.cpp

if LINUX
giada_LDADD = $(top_builddir)/engine/libengine.a \
	$(top_builddir)/libmdl/libmdl.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	-lfltk -lX11 -lXpm \
	-lasound -ldl -ljack -lm -lpthread -lsamplerate -lsndfile
endif
if WINDOWS
giada_LDADD = $(top_builddir)/engine/libengine.a \
	$(top_builddir)/libmdl/libmdl.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	-ldsound -lwsock32 -lm -lpthread \
	-lfltk -lwininet -lgdi32 -lshell32 -lvfw32 -lrpcrt4 \
	-luuid -lcomctl32 -lole32 -lws2_32 -lsndfile \
	-lsamplerate -lwinmm -lsetupapi -lksuser
giada_LDFLAGS = -mwindows -mno-cygwin -static
giada_SOURCES += resource.rc
endif
if OSX
giada_LDADD = $(top_builddir)/engine/libengine.a \
	$(top_builddir)/libmdl/libmdl.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	-lfltk -lm -lpthread -lsamplerate -lsndfile
giada_LDFLAGS = -framework CoreAudio -framework Cocoa -framework Carbon \
    -framework CoreMIDI -framework CoreFoundation
endif


# used only under MinGW to compile the resource.rc file (program icon)

.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@


# rename the binaries

if LINUX
rename:
	mv giada giada_lin
endif
if WINDOWS
rename:
	mv giada giada_win.exe
endif
if OSX
rename:
	mv giada giada_osx
endif

# clean up for git add
gitclean:
	(make clean)
	rm -r .deps
	rm Makefile Makefile.in

noinstHEADERS = 
