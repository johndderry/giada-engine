/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_launcher
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


#ifndef GG_LAUNCHER_H
#define GG_LAUNCHER_H


#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Check_Button.H>


#define SUBWIN_W	GUI_WIDTH
#define SUBWIN_H	GUI_HEIGHT
#define DEF_COLS	2


/* ------------------------------------------------------------------ */


class gdSubWindow : public gWindow
{
private:

	static void cb_close (Fl_Widget *v, void *p);
	inline void __cb_close();

public:

	class gKeyboard   *keyboard;

	gdSubWindow(int w, int h, const char *title, int argc, char **argv);
};


/* ------------------------------------------------------------------ */


class gLauncher : public Fl_Group {


	static void cb_addSampleChan  (Fl_Widget *v, void *p);
	inline void __cb_addSampleChan();
	static void cb_addMidiChan  (Fl_Widget *v, void *p);
	inline void __cb_addMidiChan();

	static void cb_checkwindows (Fl_Widget *v, void *p);
	void __cb_checkwindows();

	class gClick      *addSampleChanBtn;
	class gClick      *addMidiChanBtn;
	class gCheck      *active[MAX_BREAKS];		

	gKeyboard         *uniKeyboard;
	gKeyboard         *keyboards[MAX_BREAKS];	

	gdSubWindow       *subwindows[MAX_BREAKS];

public:

	gLauncher( int X, int Y, int W, int H);
	~gLauncher( );

	static const char *winLabels[MAX_BREAKS];
	static const char *winSect[MAX_BREAKS];

	static void cb_setBreakMeter( int, void * );

	void clear();	// call clear() on all keyboards

	void setUniKeyboard(gKeyboard *);
	void refreshBreakButtons( int );

	void updateChannel( gChannel * );
	void deleteChannel( gChannel * );
    void freeChannel( gChannel * );

	void printChannelMessage( int ) ;
	void setChannelWithActions( class gSampleChannel *gch );
	void redraw();

	void organizeColumns();
	void refreshColumns();

};

/* ------------------------------------------------------------------ */


#endif
