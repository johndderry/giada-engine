//
// C++ Interface: samplechannel
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


#ifndef SAMPLE_CHANNEL_H
#define SAMPLE_CHANNEL_H

#include <math.h>
#include <samplerate.h>

class SampleChannel : public Channel {

private:

	/* rsmp_state, rsmp_data
	 * structs from libsamplerate */

	SRC_STATE *rsmp_state;
	SRC_DATA   rsmp_data;

	/* vChan, pChan
	 * virtual channel and extra virtual channel 
         * for processing resampled data.  */

	float   *vChan;               // virtual channel
	float   *pChan;               // extra virtual channel

	/* frameRewind
	 * exact frame in which a rewind occurs */

	int frameRewind;

	/* fillChan
	 * copy from wave to *dest and resample data from wave, if necessary.
	 * Start to fill pChan from byte 'offset'. If rewind=false don't
	 * rewind internal tracker. Returns new sample position, in frames */

	int fillChan(float *dest, int start, int offset, bool rewind=true);

	/* clearChan
	 * set data to zero from start to bufferSize-1. */

	void clearChan(float *dest, int start);

	/* calcFadeoutStep
	 * how many frames are left before the end of the sample? Is there
	 * enough room for a complete fadeout? Should we shorten it? */

	void calcFadeoutStep();

	/* calcVolumeEnv
	 * compute any changes in volume done via envelope tool */

	void calcVolumeEnv(int frame);

public:

	SampleChannel(int bufferSize);
	~SampleChannel();

	void  clear      ();
	void  process    (float *buffer);
	void  start      (int frame, bool doQuantize);
	void  kill       (int frame);
	void  empty      ();
	void  stopBySeq  ();
	void  stop       ();
	void  rewind     ();
	void  setMute    (bool internal);
	void  unsetMute  (bool internal);
	void  reset      (int frame);
	int   load       (const char *file);
	int   loadByPatch(const char *file, int i);
	void  writePatch (FILE *fp, int i, bool isProject);
	void  quantize   (int index, int localFrame, int globalFrame);
	void  onZero     (int frame);
	void  onBar      (int frame);

	/* can't refererence recorder from samplechannel.h */
	//void  parseAction(recorder::action *a, int localFrame, int globalFrame);
	void  parseAction(void *a, int localFrame, int globalFrame);

	/* fade methods
	 * prepare channel for fade, mixer will take care of the process
	 * during master play. */

	void  setFadeIn  (bool internal);
	void  setFadeOut (int actionPostFadeout);
	void  setXFade   (int frame);

	/* pushWave
	 * add a new wave to an existing channel. */

	void pushWave(class Wave *w);

	/* getPosition
	 * returns the position of an active sample. If EMPTY o MISSING
	 * returns -1. */

	int getPosition();

	/* sum
	 * add sample frames to virtual channel. Frame = processed frame in
	 * Mixer. Running = is Mixer in play? */

	void sum(int frame, bool running);

	/* setPitch
	 * updates the pitch value and chanStart+chanEnd accordingly. */

	void setPitch(float v);

	/* setStart/end
	 * change begin/end read points in sample. */

	void setBegin(unsigned v);
	void setEnd  (unsigned v);

	/* save
	 * save sample to file. */

	int save(const char *path);

	/* hardStop
	 * stop the channel immediately, no further checks. */

	void hardStop(int frame);

	/* allocEmpty
	 * alloc an empty wave used in input recordings. */

	bool allocEmpty(int frames, int takeId);

	/* canInputRec
	 * true if channel can host a new wave from input recording. */

	bool  canInputRec();

	/* setReadActions
	 * if enabled, recorder will read actions from this channel */

	void setReadActions(bool v);

	/* ---------------------------------------------------------------- */

	class  Wave *wave;

	int    tracker;         // chan position
	int    begin;
	int    end;
	float  pitch;
	float  boost;
	int    mode;            // mode
	bool   qWait;           // quantizer wait
	bool   fadeinOn;
	float  fadeinVol;
	bool   fadeoutOn;
	float  fadeoutVol;      // fadeout volume
	int    fadeoutTracker;  // tracker fadeout, xfade only
	float  fadeoutStep;     // fadeout decrease
	int    fadeoutType;     // xfade or fadeout
	int    fadeoutEnd;      // what to do when fadeout ends
	int    key;
	int    endmode;          // record end mode: 0(normal) no end, 1 end, 2 new sample 

	/* recorder:: stuff */

	bool   readActions;     // read actions or not

	/* midi stuff */

	uint32_t midiInReadActions;
	uint32_t midiInPitch;

	/* const - what to do when a fadeout ends */

	enum {
		DO_STOP   = 0x01,
		DO_MUTE   = 0x02,
		DO_MUTE_I = 0x04
	};

	/*  const - fade types */

	enum {
		FADEOUT = 0x01,
		XFADE   = 0x02
	};
};

#endif
