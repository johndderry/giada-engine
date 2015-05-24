/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ---------------------------------------------------------------------
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


#include <pthread.h>
#if defined(__linux__) || defined(__APPLE__)
	#include <unistd.h>
#endif


#include "engine.h"
#include "fltk.h"
#include "gui_utils.h"


#ifdef WITH_VST
PluginHost    G_PluginHost;
#endif

pthread_t     t_video;

void main_startGUI(int argc, char **argv)
{
	char win_label[32];
	sprintf(win_label, "%s - %s",
		VERSIONE_STR,
		!strcmp(G_Patch.name, "") ? "(default patch)" : G_Patch.name);

	G_Interface = new Interface();

	if( G_Conf.mainWindowW < 0 ) G_Conf.mainWindowW = GUI_WIDTH;
	if( G_Conf.mainWindowH < 0 ) G_Conf.mainWindowH = GUI_HEIGHT;

	mainWin = new gdMainWindow(GUI_WIDTH, GUI_HEIGHT, win_label, argc, argv);
	mainWin->resize(G_Conf.mainWindowX, G_Conf.mainWindowY, G_Conf.mainWindowW, G_Conf.mainWindowH);

	/* never update the GUI elements if kernelAudio::audioStatus is bad, segfaults
	 * are around the corner */

	if (kernelAudio::audioStatus) 
		gu_update_controls();
	else
		gdAlert(
			"Your soundcard isn't configured correctly.\n"
			"Check the configuration and restart Giada."
		);
}


/* ------------------------------------------------------------------ */


void *thread_video(void *arg) {
	if (kernelAudio::audioStatus)
		while (!control::quit)	{
			gu_refresh();
#ifdef _WIN32
			Sleep(GUI_SLEEP);
#else
			usleep(GUI_SLEEP);
#endif
		}
	pthread_exit(NULL);
	return 0;
}



int main(int argc, char **argv) {

	int ret;
	bool runonce = true;
	
	while( runonce ) 
	{
		runonce = false;

		init::prepareParser();
		init::prepareKernelAudio();
		init::prepareKernelMIDI();

		main_startGUI(argc, argv);
	
		Fl::lock();
		pthread_create(&t_video, NULL, thread_video, NULL);
		init::startKernelAudio();
	
		ret = Fl::run();
	
		pthread_join(t_video, NULL);
		//Fl::unlock();

		/*  
			like to move init::shutdown(); here then
			the next section could conceivable work
		*/
		/*
			this doesn't work right - fltk can't be restarted! 
		*/
		if( control::restart ) 
		{
			runonce = true;
			control::restart = false;
			control::quit = false;
		}
	}

	return ret;
}
