//
// C++ Implementation: init
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


#include "engine.h"


namespace init {

void updateConfig() 
{
	if (!G_Conf.write())
		gLog("[INIT] error while saving configuration file!\n");
	else {
		G_Conf.read();
		gLog("[INIT] configuration refreshed\n");
	}
}

void prepareParser()
{
	G_Conf.read();
	G_Patch.setDefault();
	if (!gLog_init(G_Conf.logMode))
		gLog("[INIT] log init failed! Using default stdout\n");
	time_t t;
	time (&t);
	gLog("[INIT] "VERSIONE_STR" "VERSIONE" - %s", ctime(&t));
	gLog("[INIT] configuration file ready\n");
}


/* ------------------------------------------------------------------ */


void prepareKernelAudio()
{
	kernelAudio::openDevice(
		G_Conf.soundSystem,
		G_Conf.soundDeviceOut,
		G_Conf.soundDeviceIn,
		G_Conf.channelsOut,
		G_Conf.channelsIn,
		G_Conf.samplerate,
		G_Conf.buffersize);
	G_Mixer.init();
	recorder::init();
}


/* ------------------------------------------------------------------ */


void prepareKernelMIDI()
{
	kernelMidi::setApi(G_Conf.midiSystem);
	kernelMidi::openOutDevice(G_Conf.midiPortOut);
	kernelMidi::openInDevice(G_Conf.midiPortIn);
}


/* ------------------------------------------------------------------ */


void startKernelAudio()
{
	if (kernelAudio::audioStatus)
		kernelAudio::startStream();

#ifdef WITH_VST
	G_PluginHost.allocBuffers();
#endif
}


/* ------------------------------------------------------------------ */



void shutdown()
{
	control::quit = true;
	
#if 0
	/* store position and size of the main window for the next startup */
	
	G_Conf.mainWindowX = mainWin->x();
	G_Conf.mainWindowY = mainWin->y();
	G_Conf.mainWindowW = mainWin->w();
	G_Conf.mainWindowH = mainWin->h();

	/* close any open subwindow, especially before cleaning PluginHost to
	 * avoid mess */

	gu_closeAllSubwindows();
	gLog("[INIT] all subwindows closed\n");
#endif

	/* write configuration file */

	if (!G_Conf.write())
		gLog("[INIT] error while saving configuration file!\n");
	else
		gLog("[INIT] configuration saved\n");

	/* if kernelAudio::audioStatus we close the kernelAudio FIRST, THEN the mixer.
	 * The opposite could cause random segfaults (even now with RtAudio?). */

	if (kernelAudio::audioStatus) {
		kernelAudio::closeDevice();
		G_Mixer.close();
		gLog("[INIT] Mixer closed\n");
	}

	recorder::clearAll();
	gLog("[INIT] Recorder cleaned up\n");

#ifdef WITH_VST
	G_PluginHost.freeAllStacks();
	gLog("[INIT] Plugin Host cleaned up\n");
#endif

	gLog("[INIT] "VERSIONE_STR" "VERSIONE" closed\n\n");
	gLog_close();
}

} // end namespace
