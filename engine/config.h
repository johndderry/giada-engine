//
// C++ Interface: config
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


#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_LINE_LEN			1024
#define CONF_FILENAME			"giada.conf"

/* -- default system ------------------------------------------------ */
#if defined(__linux__)
	#define DEFAULT_SOUNDSYS	SYS_API_ALSA
#elif defined(_WIN32)
	#define DEFAULT_SOUNDSYS 	SYS_API_DS
#elif defined(__APPLE__)
	#define DEFAULT_SOUNDSYS 	SYS_API_CORE
#endif

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>

#if defined(__APPLE__)
	#include <pwd.h>
#endif


class Conf 
{
private:

	int openFileForReading();
	int openFileForWriting();
	int createConfigFolder(const char *path);

public:

	FILE *fp;
	std::string getValue(const char *in);

	int  logMode;

	int  soundSystem;
	int  soundDeviceOut;
	int  soundDeviceIn;
	int  channelsOut;
	int  channelsIn;
	int  samplerate;
	int  buffersize;
	int  delayComp;
	bool limitOutput;
	int  rsmpQuality;

	int   midiSystem;
	int   midiPortOut;
	int   midiPortIn;
	int   midiSync;  // see const.h
	float midiTCfps;

	uint32_t midiInRewind;
	uint32_t midiInStartStop;
	uint32_t midiInActionRec;
	uint32_t midiInInputRec;
	uint32_t midiInMetronome;
	uint32_t midiInVolumeIn;
	uint32_t midiInVolumeOut;
	uint32_t midiInBeatDouble;
	uint32_t midiInBeatHalf;

	uint16_t midiTackMesg;
	uint16_t midiTickMesg;
	uint16_t midiTockMesg;

	bool recsStopOnChanHalt;
	bool chansStopOnSeqHalt;
	bool treatRecsAsLoops;
	bool useVerifyDialog;
	bool useAltClick;
	bool useMidiClick;
	bool incrMidiChan;
	bool autoPlayMidi;

	char pluginPath[FILENAME_MAX];
	char patchPath [FILENAME_MAX];
	char samplePath[FILENAME_MAX];

	int  mainWindowX, mainWindowY, mainWindowW, mainWindowH;
	int  browserX, browserY, browserW, browserH;
	int  actionEditorX, actionEditorY, actionEditorW, actionEditorH, actionEditorZoom;
	int  actionEditorGridVal;
	int  actionEditorGridOn;
	int  sampleEditorX, sampleEditorY, sampleEditorW, sampleEditorH;
	int  sampleEditorGridVal;
	int  sampleEditorGridOn;
	int  pianoRollY, pianoRollH;
	int  pluginListX, pluginListY;
	int  configX, configY;
	int  bpmX, bpmY;
	int  beatsX, beatsY;
	int  aboutX, aboutY;

	int  read();
	int  write();
	void setDefault();

	/* setPath
	 * updates one of the following values: plugin, patch or sample.
	 * Pass it a pointer to one of these (path) and the string to save (p). */

	void setPath(char *path, const char *p);

	void close();
};

extern class Conf G_Conf;

#endif




