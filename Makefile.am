SUBDIRS = rtaudio rtmidi midifile netLink/src engine interfaces

gitclean:
	(cd engine; make gitclean)
	(cd interfaces; make gitclean)
	(cd netLink/src; make clean)
	rm Makefile
	rm Makefile.in

