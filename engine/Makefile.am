AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -g -O2 -Wall -pedantic -Werror

bin_PROGRAMS = gdEngine

lib_LIBRARIES = libengine.a

gdEngine_SOURCES = main.cpp

libengine_a_SOURCES = channel.cpp config.cpp control.cpp interface.cpp \
	kernelaudio.cpp kernelmidi.cpp midichannel.cpp mixer.cpp patch.cpp recorder.cpp \
	samplechannel.cpp utility.cpp wave.cpp init.cpp

# Check for environment: these vars are defined via AM_CONDITIONAL
# inside configure.ac

if LINUX
gdEngine_LDADD = $(top_builddir)/engine/libengine.a \
    $(top_builddir)/midifile/lib/libmidifile.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	-lasound -ldl -ljack -lm -lpthread -lsamplerate -lsndfile
endif
if WINDOWS
								-lwininet -lgdi32 -lshell32 -lvfw32 -lrpcrt4 \
								-luuid -lcomctl32 -lole32 -lws2_32 -lsndfile \
								-lsamplerate -lrtmidi -lwinmm -lsetupapi -lksuser
endif
if OSX
								-lsamplerate
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
	mv gdEngine gdengine_lin
endif
if WINDOWS
rename:
	mv gdEngine gdengine_win.exe
endif
if OSX
rename:
	mv gdEngine gdengine_osx
endif

noinstHEADERS = patch.h channel.h config.h const.h kernel.h kernelaudio.h kernelmidi.h \
	midichannel.h samplechannel.h utility.h mixer.h control.h recorder.h \
	wave.h init.h interface.h
