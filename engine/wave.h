//
// C++ Interface: wave
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

/*
 *  CLASS WAVE
 */

#ifndef __WAVE_H__
#define __WAVE_H__


#include <samplerate.h>
#include <sndfile.h>
#include <string>

class Wave {

private:

	SNDFILE   *fileIn;
	SNDFILE   *fileOut;
	SF_INFO    inHeader;
	SF_INFO    outHeader;

public:

	Wave();
	~Wave();

	float     *data;
	int        size;		// wave size (size in stereo: size / 2)
	bool       isLogical;	// memory only (a take)
	bool       isEdited;	// edited via editor

	inline int  rate    () { return inHeader.samplerate; }
	inline int  channels() { return inHeader.channels; }
	inline int  frames  () { return inHeader.frames; }
	inline void rate    (int v) { inHeader.samplerate = v; }
	inline void channels(int v) { inHeader.channels = v; }
	inline void frames  (int v) { inHeader.frames = v; }

	int  open      (const char *f);
	int  readData  ();
	int	 writeData (const char *f);
	void clear     ();

	/* allocEmpty
	 * alloc an empty waveform. */

	int allocEmpty(unsigned size);

	/* resample
	 * simple algorithm for one-shot resampling. */

	int resample(int quality, int newRate);
};

/* STUFF from waveFx here */

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
