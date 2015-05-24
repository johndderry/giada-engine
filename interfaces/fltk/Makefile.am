AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -g -O0 -Wall -pedantic -Werror

bin_PROGRAMS = giada

giada_SOURCES = main.cpp \
gd_about.cpp gd_actionEditor.cpp gd_beatsInput.cpp gd_bpmInput.cpp \
gd_browser.cpp gd_config.cpp gd_devInfo.cpp gd_editor.cpp gd_keyGrabber.cpp gd_mainWindow.cpp \
gd_midiGrabber.cpp gd_midiOutputSetup.cpp gd_warnings.cpp ge_actionChannel.cpp ge_actionWidget.cpp \
ge_browser.cpp ge_channel.cpp ge_envelopeChannel.cpp ge_mixed.cpp ge_muteChannel.cpp \
ge_pianoRoll.cpp ge_waveform.cpp ge_window.cpp gg_keyboard.cpp gg_launcher.cpp gg_waveTools.cpp \
graphics.cpp gui_utils.cpp 


if LINUX
giada_LDADD = $(top_builddir)/engine/libengine.a \
	$(top_builddir)/midifile/lib/libmidifile.a \
	$(top_builddir)/rtaudio/librtaudio.a \
	$(top_builddir)/rtmidi/librtmidi.a \
	-lfltk -lX11 -lXext -lXft -lXpm \
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

noinstHEADERS = gd_about.h gd_actionEditor.h gd_beatsInput.h gd_bpmInput.h \
gd_browser.h gd_config.h gd_devInfo.h gd_editor.h gd_keyGrabber.h gd_mainWindow.h \
gd_midiGrabber.h gd_midiOutputSetup.h gd_warnings.h ge_actionChannel.h ge_actionWidget.h \
ge_browser.h ge_channel.h ge_envelopeChannel.h ge_mixed.h ge_muteChannel.h \
ge_pianoRoll.h ge_waveform.h ge_window.h gg_keyboard.h gg_launcher.h gg_waveTools.h \
graphics.h gui_utils.h utils.h
