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


#ifndef __PATCH_H__
#define __PATCH_H__


/* ------  a BUNCH OF DEFINES are here in patch ---------- */

#define DEFAULT_BPM				   120.0f
#define DEFAULT_BEATS			   4
#define DEFAULT_BARS			   1
#define DEFAULT_BREAKS			   4
#define DEFAULT_QUANTIZE     0		  // quantizer off
#define DEFAULT_FADEOUT_STEP 0.01f  // micro-fadeout speed

/* -- patch signals ------------------------------------------------- */
#define PATCH_UNREADABLE  0
#define PATCH_INVALID    -1
#define PATCH_OPEN_OK     1

/* -- channel types ------------------------------------------------- */
#define CHANNEL_SAMPLE 0x01
#define CHANNEL_MIDI   0x02

/* -- application version ------------------------------------------- */
#define VERSIONE  "0.1"
#define VERSIONE_FLOAT  0.1
#define VERSIONE_STR	"GiadaEngine"

#include <stdio.h>
#include <string>
#include <stdint.h>

#define MAX_PATCHNAME_LEN	 32

class Patch  {

private:
	int  readMasterPlugins(int type);
	void writeMasterPlugins(int type);

public:

	FILE *fp;
	std::string getValue(const char *in);
	const char *getValueC(const char *in);

	char  name[MAX_PATCHNAME_LEN];
	float version;
	int   lastTakeId;
	int   lastMidiId;
	int   samplerate;

	int         open(const char *file);
	void        setDefault();
	int         close();

	void		getName       ();
	int         getNumChans   ();
	std::string getSamplePath (int i);
	float       getVol        (int i);
	int         getMode       (int i);
	int         getMute       (int i);
	int         getMute_s     (int i);
	int         getSolo       (int i);
	int         getBegin      (int i);
	int         getEnd        (int i, unsigned sampleSize);
	float       getBoost      (int i);
	float       getPanLeft    (int i);
	float       getPanRight   (int i);
	float       getPitch      (int i);
	bool        getRecActive  (int i);
	int         getColumn     (int i);
	int         getIndex      (int i);
	int         getType       (int i);
	int         getKey        (int i);
	uint32_t    getMidiValue  (int i, const char *c);
	float       getOutVol     ();
	float       getInVol      ();
	float       getBpm        ();
	int         getBars       ();
	int         getBeats      ();
	int         getBreaks     ();
	int         getQuantize   ();
	bool        getMetronome  ();
	int         getLastTakeId ();
	int         getLastMidiId ();
	int         getSamplerate ();

	int         write(const char *file, const char *name, bool isProject);
	int         readRecs();
#ifdef WITH_VST
	int         readPlugins();
#endif
};

extern class Patch G_Patch;

#endif
