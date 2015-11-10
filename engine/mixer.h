//
// C++ Interface: mixer
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


#ifndef MIXER_H
#define MIXER_H

/* -- mixerHandler signals ------------------------------------------ */
#define MIDI_LOADED_OK        1
#define SAMPLE_LOADED_OK      1
#define SAMPLE_LEFT_EMPTY     0
#define SAMPLE_NOT_VALID     -1
#define SAMPLE_MULTICHANNEL  -2
#define SAMPLE_WRONG_BIT     -4
#define SAMPLE_WRONG_ENDIAN  -5
#define SAMPLE_WRONG_FORMAT  -6
#define SAMPLE_READ_ERROR    -8
#define SAMPLE_PATH_TOO_LONG -9

/* -- MIN/MAX values ------------------------------------------------ */
#define MAX_BEATS		32
#define MAX_BARS		32
#define MAX_BREAKS		32
#define DB_MIN_SCALE		60.0f
#define MAX_VST_EVENTS		32

#include <stdlib.h>
#include <pthread.h>
#include <vector>

//#include <RtAudio.h>
#include "../rtaudio/RtAudio.h"


class Mixer {

public:

	Mixer();
	~Mixer();

	void init();
	int  close();

	/* addChannel
	 * add a new channel without any wave inside of it. */

	class Channel *addChannel(int type);

	/* deleteChannel
	 * completely remove a channel from the stack. */

	int deleteChannel(class Channel *ch);

	/* masterPlay
	 * core method (callback) */

	static int masterPlay(
		void *out_buf, void *in_buf, unsigned n_frames,
		double streamTime, RtAudioStreamStatus status, void *userData
	);
	int __masterPlay(void *out_buf, void *in_buf, unsigned n_frames);

	/* updateFrameBars
	 * updates bpm, frames, beats and so on. */

	void updateFrameBars();

	/* isSilent
	 * is mixer silent? */

	bool isSilent();

	/* rewind
	 * rewind sequencer to sample 0. */

	void rewind();

	/* updateQuanto
	 * recomputes the quanto between two quantizations */

	void updateQuanto();

	/* hasLogicalSamples
	 * true if 1 or more samples are logical (memory only, such as takes) */

	bool hasLogicalSamples();

	/* hasEditedSamples
	 * true if 1 or more samples was edited via gEditor */

	bool hasEditedSamples();

	/* mergeVirtualInput
	 * memcpy the virtual channel input in the channel designed for input
	 * recording. Called by mixerHandler on stopInputRec() */

	bool mergeVirtualInput();

	/* getChannelByIndex
	 * return channel with given index 'i'. */

	Channel *getChannelByIndex(int i);

	inline Channel* getLastChannel() { return channels.at(channels.size-1); }

	/* enterMBreak
	 * enter a new music break (section) by number */

	void enterMBreak(int n);

	/* advance
	 * advance the actualBreak by one and check for break changes */

	void advance();

	/* swapMidiIn
	 * switch midi recording channel */

	void swapMidiIn(int n);


	/* ---------------------------------------------------------------- */


	enum {    // const - what to do when a fadeout ends
		DO_STOP   = 0x01,
		DO_MUTE   = 0x02,
		DO_MUTE_I = 0x04
	};

	enum {    // const - fade types
		FADEOUT = 0x01,
		XFADE   = 0x02
	};

	gVector<class Channel*> channels;

	bool   running;
	bool   ready;
	float *vChanInput;        // virtual channel for recording
	float *vChanInToOut;      // virtual channel in->out bridge (hear what you're playin)
	int    frameSize;
	float  outVol;
	float  inVol;
	float  peakOut;
	float  peakIn;
	int    quanto;
	char   quantize;
	bool   metronome;
	float  bpm;
	int    bars;
	int    beats;
	int    breaks;
	int    waitRec;        // delayComp guard

	bool   docross;	       // crossfade guard
	bool   rewindWait;       // rewind guard, if quantized
	bool   recordWait;		// recording starts at Frame 0 for delay recrding

	int    framesPerBar;      // frames in one bar
	int    framesPerBeat;     // frames in one beat
	int    framesInSequencer; // frames in the whole sequencer
	int    totalFrames;       // frames in the selected range (e.g. 4/4)
	int    actualFrame;
	int    actualBeat;
	int    actualBreak;
	int    breakId[MAX_BREAKS];
	bool   suspend;
	int    nextMidiChan;

	/*
	 * Metronome
	 *
	 * Tocks are on the beat
	 * Ticks are on the downbeat at beginning of each measure in the default config;
	 *   but alternate usage buts them on the last beat, like a fill.
	 * Tacks are used in alternate to become the downbeat again, but only for the midi.
	 *
	 * In the configuration, Tick, Tock and Tack messages are:
     *   midi-note-byte << 8 | volume byte
	 */
   
#define TICKSIZE 38

	static float tock[TICKSIZE];
	static float tick[TICKSIZE];
	int  tickTracker, tockTracker, tackTracker;
	bool tickPlay, tockPlay, tackPlay; // 1 = play, 0 = stop

	/* chanInput
	 * the active channel during a recording. NULL = no channels active */

	class SampleChannel *chanInput;
	class SampleChannel  *waitChan;		// chan for delay recording

	/* inputTracker
	 * position of the sample in the input side (recording) */

	int inputTracker;

	/* inToOut
	 * copy, process and paste the input into the output, in order to
	 * obtain a "hear what you're playing" feature. */

	bool inToOut;

	pthread_mutex_t mutex_recs;
	pthread_mutex_t mutex_chans;
	pthread_mutex_t mutex_plugins;


private:

	int midiTCstep;      // part of MTC to send (0 to 7)
	int midiTCrate;      // send MTC data every midiTCrate frames
	int midiTCframes;
	int midiTCseconds;
	int midiTCminutes;
	int midiTChours;

	/* getNewIndex
	 * compute new index value for new channels */

	int getNewIndex();

	/* sendMIDIsync
	 * generate MIDI sync output data */

	void sendMIDIsync();

	/* sendMIDIrewind
	 * rewind timecode to beat 0 and also send a MTC full frame to cue
	 * the slave */

	void sendMIDIrewind();
};

extern class Mixer G_Mixer;

/* ------------------------------------------------------------------------------ */

/*
 * Mixer Handler STUFF here
 */

/* stopSequencer
 * stop the sequencer, with special case if samplesStopOnSeqHalt is
 * true. */

void mh_stopSequencer();

void mh_rewindSequencer();

/* clear
 * stop everything and clear all channels. */
 
void mh_clear();

/* uniqueSolo
 * true if ch is the only solo'd channel in mixer. */

bool mh_uniqueSolo(class Channel *ch);

/* loadPatch
 * load a path or a project (if isProject) into Mixer. If isProject, path
 * must contain the address of the project folder. */

void mh_loadPatch(bool isProject, const char *projPath=0);

/* startInputRec - record from line in
 * creates a new empty wave in the first available channels and returns
 * the chan number chosen, otherwise -1 if there are no more empty
 * channels available. */

SampleChannel *mh_startInputRec();

SampleChannel *mh_stopInputRec();

/* uniqueSamplename
 * return true if samplename 'n' is unique. Requires SampleChannel *ch
 * in order to skip check against itself. */

bool mh_uniqueSamplename(class SampleChannel *ch, const char *name);


/* normalizeSoft
 * normalize the wave by returning the dB value for the boost volume. It
 * doesn't deal with data in memory. */

float wfx_normalizeSoft(class Wave *w);

bool wfx_monoToStereo(class Wave *w);

void wfx_silence(class Wave *w, int a, int b);

int wfx_cut(class Wave *w, int a, int b);

int wfx_trim(class Wave *w, int a, int b);

/* fade
 * fade in or fade out selection. Fade In = type 0, Fade Out = type 1 */

void wfx_fade(class Wave *w, int a, int b, int type);

/* smooth
 * smooth edges of selection. */

void wfx_smooth(class Wave *w, int a, int b);


#endif
