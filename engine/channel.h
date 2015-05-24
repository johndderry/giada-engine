//
// C++ Interface: channel
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


#ifndef CHANNEL_H
#define CHANNEL_H


/* LOTS of DEFINES that are needed by channel are here */

#define DEFAULT_MIDI_SYSTEM		0
#define DEFAULT_MIDI_PORT_IN	-1
#define DEFAULT_MIDI_PORT_OUT	-1
#define DEFAULT_SAMPLERATE		44100
#define DEFAULT_BUFSIZE			1024
#define DEFAULT_DELAYCOMP		0
#define DEFAULT_VOL			0.0f
#define DEFAULT_BOOST		0.0f
#define gDEFAULT_PITCH		1.0f   // ugly and temporary fix to avoid conflicts with wingdi.h (Windows only).
#define DEFAULT_OUT_VOL		1.0f
#define DEFAULT_IN_VOL		0.0f
#define DEFAULT_CHANMODE	SINGLE_BASIC

#define REC_WAITING		0x01 // 0000 0001
#define REC_ENDING		0x02 // 0000 0010
#define REC_READING		0x04 // 0000 0100
#define REC_STOPPED		0x08 // 0000 1000

/* ---------- as well as mixer statuses and modes relating to channels -------- */
//
// bitflags not implememted in code!
//
#define LOOP_BASIC		0x01 // 0000 0001  chanMode
#define LOOP_ONCE		0x02 // 0000 0010  chanMode
#define	SINGLE_BASIC	0x04 // 0000 0100  chanMode
#define SINGLE_PRESS	0x08 // 0000 1000  chanMode
#define SINGLE_RETRIG	0x10 // 0001 0000  chanMode
#define LOOP_REPEAT		0x20 // 0010 0000  chanMode
#define SINGLE_ENDLESS	0x40 // 0100 0000  chanMode
#define LOOP_ONCE_BAR	0x80 // 1000 0000  chanMode

#define LOOP_ANY		0xA3 // 1010 0011  chanMode - any loop mode
#define SINGLE_ANY		0x5C // 0101 1100  chanMode - any single mode

#define	STATUS_ENDING	0x01 // 0000 0001  chanStatus - ending            (loop mode only)
#define	STATUS_WAIT		0x02 // 0000 0010  chanStatus - waiting for start (loop mode only)
#define	STATUS_PLAY		0x04 // 0000 0100  chanStatus - playing
#define STATUS_OFF		0x08 // 0000 1000  chanStatus - off
#define STATUS_EMPTY	0x10 // 0001 0000  chanStatus - not loaded (empty chan)
#define STATUS_MISSING	0x20 // 0010 0000  chanStatus - not found
#define STATUS_WRONG	0x40 // 0100 0000  chanStatus - something wrong (freq, bitrate, ...)


#ifdef WITH_VST


/* before including aeffetx(x).h we must define __cdecl, otherwise VST
 * headers can't be compiled correctly. In windows __cdecl is already
 * defined. */

	#ifdef __GNUC__
		#ifndef _WIN32
			#define __cdecl
		#endif
	#endif
	#include "vst/aeffectx.h"

#endif


class Channel {

protected:

	/* bufferSize
	 * size of every buffer in this channel (vChan, pChan) */

	int bufferSize;

public:

	Channel(int type, int status, int bufferSize);
	virtual ~Channel();

	/* writePatch
	 * store values in patch, writing to *fp. */

	virtual void writePatch(FILE *fp, int i, bool isProject);

	/* loadByPatch
	 * load a sample inside a patch. */

	virtual int loadByPatch(const char *file, int i) = 0;

	/* process
	 * merge vChannels into buffer, plus plugin processing (if any). */

	virtual void process(float *buffer) = 0;

	/* start
	 * action to do when channel starts. doQuantize = false (don't
	 * quantize) when Mixer is reading actions from Recorder::. */

	virtual void start(int frame, bool doQuantize) = 0;

	/* stop
	 * action to do when channel is stopped normally (via key or MIDI). */

	virtual void stop() = 0;

	/* kill
	 * action to do when channel stops abruptly. */

	virtual void kill(int frame) = 0;

	/* mute
	 * action to do when channel is muted. If internal == true, set
	 * internal mute without altering main mute. */

	virtual void setMute  (bool internal) = 0;
	virtual void unsetMute(bool internal) = 0;

	/* empty
	 * free any associated resources (e.g. waveform for SAMPLE). */

	virtual void empty() = 0;

	/* stopBySeq
	 * action to do when channel is stopped by sequencer. */

	virtual void stopBySeq() = 0;

	/* quantize
	 * start channel according to quantizer. Index = array index of
	 * mixer::channels, used by recorder. LocalFrame = frame within buffer.
	 * GloalFrame = actual frame from mixer. */

	virtual void quantize(int index, int localFrame, int globalFrame) = 0;

	/* onZero
	 * action to do when frame goes to zero, i.e. sequencer restart. */

	virtual void onZero(int frame) = 0;

	/* onBar
	 * action to do when a bar has passed. */

	virtual void onBar(int frame) = 0;

	/* parseAction
	 * do something on a recorded action. Parameters:
	 * action *a   - action to parse
	 * localFrame  - frame number of the processed buffer
	 * globalFrame - actual frame in Mixer */

	virtual void parseAction(void *a, int localFrame, int globalFrame) = 0;

	/* rewind
	 * rewind channel when rewind button is pressed. */

	virtual void rewind() = 0;

	/* ---------------------------------------------------------------- */

	/* isPlaying
	 * tell wether the channel is playing or is stopped. */

	bool isPlaying();

	/* readPatchMidiIn
	 * read from patch all midi-related parameters such as keypress, mute 
	 * and so on. */

	void readPatchMidiIn(int i);

	/* update the sample name. This code moved from Wave
	 */	 
	 
	void updateName(const char *n);

	/* ---------------------------------------------------------------- */

	std::string pathfile; 			// full path + sample name
	std::string name;				// sample name (changeable)

	std::string basename ();
	std::string extension();

	int     index;                // unique id
	int     type;                 // midi or sample
	int     mbreak;               // musical break number
	int     status;	              // status
	
	float   volume;               // global volume
	float   volume_i;             // internal volume
	float   volume_d;             // delta volume (for envelope)
	float   panLeft;
	float   panRight;
	
	bool    mute_i;               // internal mute
	bool	mute_s;               // previous mute status after being solo'd
	bool    mute;                 // global mute
	bool    solo;
	bool    hasActions;           // has something recorded

	int     recStatus;            // status of recordings (waiting, ending, ...)
	void    *guiChannel;          // pointer to a gChannel object, part of the GUI

	bool     tunnelIn;            // enable midi tunneling
	bool     midiIn;              // enable midi input
	
	uint32_t midiInKeyPress;
	uint32_t midiInKeyRel;
	uint32_t midiInKill;
	uint32_t midiInVolume;
	uint32_t midiInMute;
	uint32_t midiInSolo;

#ifdef WITH_VST
	gVector <class Plugin *> plugins;
#endif

};

#endif

