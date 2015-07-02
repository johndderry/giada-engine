AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = --std=c++11 -g -O2 -Wall -pedantic -Werror

bin_PROGRAMS = giadaNet

giadaNet_SOURCES = main.cpp lex.cc

# Check for environment: these vars are defined via AM_CONDITIONAL
# inside configure.ac

if LINUX
giadaNet_LDADD = $(top_builddir)/engine/libengine.a \
    $(top_builddir)/midifile/lib/libmidifile.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	$(top_builddir)/netLink/src/libnetlink.a \
	-lasound -ldl -ljack -lm -lpthread -lsamplerate -lsndfile
endif
if WINDOWS
giadaNet_LDADD = $(top_builddir)/engine/libengine.a \
    $(top_builddir)/midifile/lib/libmidifile.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	$(top_builddir)/netLink/src/libnetlink.a \
	-ldsound -lwsock32 -lm -lpthread \
	-lwininet -lgdi32 -lshell32 -lvfw32 -lrpcrt4 \
	-luuid -lcomctl32 -lole32 -lws2_32 -lsndfile \
	-lsamplerate -lwinmm -lsetupapi -lksuser
endif
if OSX
giadaNet_LDADD = $(top_builddir)/engine/libengine.a \
    $(top_builddir)/midifile/lib/libmidifile.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	$(top_builddir)/netLink/src/libnetlink.a \
	-lm -lpthread -lsamplerate -lsndfile
giadaNet_LDFLAGS = -framework CoreAudio -framework Cocoa -framework Carbon \
    -framework CoreMIDI -framework CoreFoundation
endif

# create the flexc++ lexer file
lex.cc:
	flexc++ giadaNetLexer

Scanner.h:
	flexc++ giadaNetLexer

# used only under MinGW to compile the resource.rc file (program icon)

.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@


# rename the binaries

if LINUX
rename:
	mv giadaNet giadaNet_lin
endif
if WINDOWS
rename:
	mv giadaNet giadaNet_win.exe
endif
if OSX
rename:
	mv giadaNet giadaNet_osx
endif

# clean up for git add
gitclean:
	(make clean)
	rm -r .deps
	rm giadaNetLexer.c
	rm Makefile Makefile.in

noinstHEADERS = Scanner.h

