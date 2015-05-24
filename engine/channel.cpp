//
// C++ Implementation: channel
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

#include "config.h"
#include "utility.h"
#include "patch.h"
#include "channel.h"

#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Channel::Channel(int type, int status, int bufferSize)
	: bufferSize(bufferSize),
	  type      (type),
	  mbreak    (-1),
	  status    (status),
	  volume    (DEFAULT_VOL),
	  volume_i  (1.0f),
	  volume_d  (0.0f),
	  panLeft   (1.0f),
	  panRight  (1.0f),
	  mute_i    (false),
	  mute_s    (false),
	  mute      (false),
	  solo      (false),
	  hasActions(false),
	  recStatus (REC_STOPPED),
	  guiChannel(NULL),
	  tunnelIn      (true),
	  midiIn        (false),
	  midiInKeyPress(0x0),
	  midiInKeyRel  (0x0),
	  midiInKill    (0x0),
	  midiInVolume  (0x0),
	  midiInMute    (0x0),
	  midiInSolo    (0x0)
{
}


/* ------------------------------------------------------------------ */


Channel::~Channel()
{
	status = STATUS_OFF;
	//if (vChan) delete[] vChan;
}


/* ------------------------------------------------------------------ */


void Channel::readPatchMidiIn(int i)
{
	tunnelIn       = G_Patch.getMidiValue(i, "Tunnel");
	midiIn         = G_Patch.getMidiValue(i, "In");
	midiInKeyPress = G_Patch.getMidiValue(i, "InKeyPress");
	midiInKeyRel   = G_Patch.getMidiValue(i, "InKeyRel");
	midiInKill     = G_Patch.getMidiValue(i, "InKill");
	midiInVolume   = G_Patch.getMidiValue(i, "InVolume");
	midiInMute     = G_Patch.getMidiValue(i, "InMute");
	midiInSolo     = G_Patch.getMidiValue(i, "InSolo");
}


/* ------------------------------------------------------------------ */


bool Channel::isPlaying()
{
	return status & (STATUS_PLAY | STATUS_ENDING);
}


/* ------------------------------------------------------------------ */


void Channel::writePatch(FILE *fp, int i, bool isProject)
{
	fprintf(fp, "chanType%d=%d\n",     i, type);
	fprintf(fp, "chanIndex%d=%d\n",    i, index);
	//
	// !! can't use guiChannel from here !! 
	//
	//fprintf(fp, "chanColumn%d=%d\n",   i, guiChannel->getColumnIndex());
	fprintf(fp, "chanMute%d=%d\n",     i, mute);
	fprintf(fp, "chanMute_s%d=%d\n",   i, mute_s);
	fprintf(fp, "chanSolo%d=%d\n",     i, solo);
	fprintf(fp, "chanvol%d=%f\n",      i, volume);
	fprintf(fp, "chanPanLeft%d=%f\n",  i, panLeft);
	fprintf(fp, "chanPanRight%d=%f\n", i, panRight);

	fprintf(fp, "chanMidiTunnel%d=%u\n",     i, tunnelIn);
	fprintf(fp, "chanMidiIn%d=%u\n",         i, midiIn);
	fprintf(fp, "chanMidiInKeyPress%d=%u\n", i, midiInKeyPress);
	fprintf(fp, "chanMidiInKeyRel%d=%u\n",   i, midiInKeyRel);
	fprintf(fp, "chanMidiInKill%d=%u\n",     i, midiInKill);
	fprintf(fp, "chanMidiInVolume%d=%u\n",   i, midiInVolume);
	fprintf(fp, "chanMidiInMute%d=%u\n",     i, midiInMute);
	fprintf(fp, "chanMidiInSolo%d=%u\n",     i, midiInSolo);
}

/* ------------------------------------------------------------------ */


std::string Channel::basename() {
	return gStripExt(gBasename(pathfile.c_str()).c_str());
}

std::string Channel::extension() {
	return gGetExt(pathfile.c_str());
}


/* ------------------------------------------------------------------ */


void Channel::updateName(const char *n) {
	std::string ext = gGetExt(pathfile.c_str());
	name      = gStripExt(gBasename(n).c_str());
	pathfile  = gDirname(pathfile.c_str()) + gGetSlash() + name + "." + ext;

	// isLogical = true;

	/* a wave with updated name must become logical, since the underlying
	 * file does not exist yet. */
}

