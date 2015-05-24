/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_browser
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


#ifndef GD_BROWSER_H
#define GD_BROWSER_H

/* -- browser types ------------------------------------------------- */
#define BROWSER_LOAD_PATCH   0x00
#define BROWSER_LOAD_SAMPLE  0x01
#define BROWSER_SAVE_PATCH   0x02
#define BROWSER_SAVE_SAMPLE  0x04
#define BROWSER_SAVE_PROJECT 0x08
#define BROWSER_LOAD_PLUGIN  0x10
#define BROWSER_LOAD_MIDI    0x20
#define BROWSER_SAVE_MIDI    0x40


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

// these used to be in glue.h, now they are here - 
// except control:: still has it's own loadChannel
// consider moving them to be static methods of gdBrowser

void adjustBpm(float adjust);
void setBpm(const char *v1, const char *v2);
int loadSampChannel(SampleChannel *ch, const char *fname);
int loadMidiChannel(MidiChannel *ch, const char *fname);
void freeChannel(Channel *ch);
void deleteChannel(Channel *ch);
void resetToInitState(bool resetGui);
int loadPatch(const char *fname, const char *fpath, gProgress *status, bool isProject);


class gdBrowser : public gWindow {

private:
	static void cb_down(Fl_Widget *v, void *p);
	static void cb_up  (Fl_Widget *v, void *p);
	static void cb_load_sample (Fl_Widget *v, void *p);
	static void cb_save_sample (Fl_Widget *v, void *p);
	static void cb_load_midi   (Fl_Widget *v, void *p);
	static void cb_save_midi   (Fl_Widget *v, void *p);
	static void cb_load_patch  (Fl_Widget *v, void *p);
	static void cb_save_patch  (Fl_Widget *v, void *p);
	static void cb_save_project(Fl_Widget *v, void *p);
	static void cb_close       (Fl_Widget *w, void *p);
#ifdef WITH_VST
	static void cb_loadPlugin  (Fl_Widget *v, void *p);
#endif

	inline void __cb_down();
	inline void __cb_up();
	inline void __cb_load_sample();
	inline void __cb_save_sample();
	inline void __cb_load_midi();
	inline void __cb_save_midi();
	inline void __cb_save_project();
	inline void __cb_load_patch();
	inline void __cb_save_patch();
	inline void __cb_close();
#ifdef WITH_VST
	inline void __cb_loadPlugin();
#endif

	class gBrowser  *browser;
	class gClick    *ok;
	class gClick    *cancel;
	class gInput    *where;
	class gInput    *name;
 	class gClick    *updir;
 	class gProgress *status;

	class Channel *ch;

	/* browser type,  uses BROWSER_  defines */

	int type;

	/* PluginHost stack type. Used only when loading plugins */

	int stackType;

public:
	gdBrowser(const char *title, const char *initPath, class Channel *ch, int type, int stackType=0);
	~gdBrowser();
};

#endif
