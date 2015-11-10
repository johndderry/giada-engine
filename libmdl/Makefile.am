AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -g -O2 -Wall -pedantic -Werror

lib_LIBRARIES = libmdl.a

libmdl_a_SOURCES = midibase.cpp midistream.cpp seqcontext.cpp

noinstHEADERS = midibase.h midistream.h seqcontext.h

gitclean:
	(make clean)
	rm -r .deps
	rm Makefile Makefile.in


