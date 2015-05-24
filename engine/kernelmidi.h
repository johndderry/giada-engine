//
// C++ Interface: kernelmidi
//
// Description: 
//
//	GiadaEngine is a derivative of Giada
//
// Copyright (C) 2015 John D. Derry
//
//
/* ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#ifndef KERNELMIDI_H
#define KERNELMIDI_H


#include <stdint.h>
//#include <RtMidi.h>
#include "../rtmidi/RtMidi.h"

/* -- kernel midi --------------------------------------------------- */
#define MIDI_API_JACK		0x01  // 0000 0001
#define MIDI_API_ALSA		0x02  // 0000 0010

/* -- MIDI signals -----------------------------------------------------
 * all signals are set to channel 0 (where channels are considered).
 * It's up to the caller to bitmask them with the proper channel number. */

/* channel voices messages - controller (0xB0) is a special subset of
 * this family: it drives knobs, volume, faders and such. */

#define MIDI_PROGRAM        0xC0 << 24
#define MIDI_CONTROLLER     0xB0 << 24
#define MIDI_NOTE_ON        0x90 << 24
#define MIDI_NOTE_OFF       0x80 << 24
#define MIDI_ALL_NOTES_OFF (MIDI_CONTROLLER) | (0x7B << 16)
#define MIDI_VOLUME        (MIDI_CONTROLLER) | (0x07 << 16)
#define MIDI_CHANNEL_MASK   0xF0FFFFFF

/* system common / real-time messages. Single bytes */

#define MIDI_SYSEX          0xF0
#define MIDI_MTC_QUARTER    0xF1
#define MIDI_POSITION_PTR   0xF2
#define MIDI_CLOCK          0xF8
#define MIDI_START          0xFA
#define MIDI_CONTINUE       0xFB
#define MIDI_STOP           0xFC
#define MIDI_EOX            0xF7  // end of sysex

/* channels */

#define MIDI_CHAN_0         0x00 << 24
#define MIDI_CHAN_1         0x01 << 24
#define MIDI_CHAN_2         0x02 << 24
#define MIDI_CHAN_3         0x03 << 24
#define MIDI_CHAN_4         0x04 << 24
#define MIDI_CHAN_5         0x05 << 24
#define MIDI_CHAN_6         0x06 << 24
#define MIDI_CHAN_7         0x07 << 24
#define MIDI_CHAN_8         0x08 << 24
#define MIDI_CHAN_9         0x09 << 24
#define MIDI_CHAN_10        0x0A << 24
#define MIDI_CHAN_11        0x0B << 24
#define MIDI_CHAN_12        0x0C << 24
#define MIDI_CHAN_13        0x0D << 24
#define MIDI_CHAN_14        0x0E << 24
#define MIDI_CHAN_15        0x0F << 24

const int MIDI_CHANS[16] = {
	MIDI_CHAN_0,  MIDI_CHAN_1,	MIDI_CHAN_2,  MIDI_CHAN_3,
	MIDI_CHAN_4,  MIDI_CHAN_5,	MIDI_CHAN_6,  MIDI_CHAN_7,
	MIDI_CHAN_8,  MIDI_CHAN_9,	MIDI_CHAN_10, MIDI_CHAN_11,
	MIDI_CHAN_12, MIDI_CHAN_13,	MIDI_CHAN_14, MIDI_CHAN_15
};

/* midi sync constants */

#define MIDI_SYNC_NONE      0x00
#define MIDI_SYNC_CLOCK_M   0x01
#define MIDI_SYNC_CLOCK_S   0x02
#define MIDI_SYNC_MTC_M     0x04
#define MIDI_SYNC_MTC_S     0x08

namespace kernelMidi {

	/* namespace variables declared as extern in header file */

	extern int      api;      // one api for both in & out
	extern unsigned numOutPorts;
	extern unsigned numInPorts;
	extern bool	midiStatus;

	typedef void (cb_midiLearn) (uint32_t, void *);

	/* cb_learn
	 * callback prepared by the gdMidiGrabber window and called by
	 * kernelMidi. It contains things to do once the midi message has been
	 * stored. */

	extern cb_midiLearn *cb_learn;
	extern void         *cb_data;

	void startMidiLearn(cb_midiLearn *cb, void *data);
	void stopMidiLearn();

	inline int getB1(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
	inline int getB2(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
	inline int getB3(uint32_t iValue) { return (iValue >> 8)  & 0xFF; }

	inline uint32_t getIValue(int b1, int b2, int b3) {
		return (b1 << 24) | (b2 << 16) | (b3 << 8) | (0x00);
	}

	/* send
	 * send a MIDI message 's' (uint32_t). */

	void send(uint32_t s);

	/* send (2)
	 * send separate bytes of MIDI message. */

	void send(int b1, int b2=-1, int b3=-1);

	/* setApi
	 * set the Api in use for both in & out messages. */

	void setApi(int api);

	/* open/close/in/outDevice */

	int openOutDevice(int port);
	int openInDevice(int port);
	int closeInDevice();
	int closeOutDevice();

	/* getIn/OutPortName
	 * return the name of the port 'p'. */

	const char *getInPortName(unsigned p);
	const char *getOutPortName(unsigned p);

	bool hasAPI(int API);

	/* callback
	 * master callback for input events. */

	void callback(double t, std::vector<unsigned char> *msg, void *data);
	
	std::string getRtMidiVersion();
	
	std::string getMidiFileVersion();
	
}

#endif
