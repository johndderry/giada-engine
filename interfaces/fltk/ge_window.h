/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_window
 * A custom window.
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


#ifndef __GE_WINDOW_H__
#define __GE_WINDOW_H__

/* -- unique IDs of mainWin's subwindows ---------------------------- */
/* -- wid > 0 are reserved by gg_keyboard --------------------------- */
#define WID_BEATS         -1
#define WID_BPM           -2
#define WID_ABOUT         -3
#define WID_FILE_BROWSER  -4
#define WID_CONFIG        -5
#define WID_FX_LIST       -6
#define WID_ACTION_EDITOR -7
#define WID_SAMPLE_EDITOR -8
#define WID_FX            -9


#ifdef _WIN32
	#define GUI_SLEEP			1000/24
#else
	#define GUI_SLEEP			1000000/24 // == 1.000.000 / 24 == 1/24 sec == 24 Hz
#endif
#define GUI_WIDTH			810
#define GUI_HEIGHT			510

#define COLOR_BD_0     fl_rgb_color(78,  78,  78)			// border off
#define COLOR_BD_1     fl_rgb_color(188, 188, 188)    // border on
#define COLOR_BG_0     fl_rgb_color(37,  37,  37)     // bg off
#define COLOR_BG_1     fl_rgb_color(78,  78,  78)     // bg on (clicked)
#define COLOR_BG_2     fl_rgb_color(177, 142, 142)    // bg active (play, for some widgets)
#define COLOR_BG_3     fl_rgb_color(28,  32,  80)     // bg input rec
#define COLOR_BG_4     fl_rgb_color(113, 31,  31)     // bg action rec
#define COLOR_ALERT    fl_rgb_color(239, 75,  53)     // peak meter alert
#define COLOR_TEXT_0   fl_rgb_color(200, 200, 200)
#define COLOR_TEXT_1   fl_rgb_color(25,  25,  25)
#define COLOR_BG_MAIN  fl_rgb_color(25,  25,  25)		   // windows background
#define COLOR_BG_DARK  fl_rgb_color(0,   0,   0)		   // inputs background


#include <FL/Fl_Double_Window.H>


class gWindow : public Fl_Double_Window {

protected:
	gVector <gWindow *> subWindows;
	int id;
	gWindow *parent;

public:
	gWindow(int x, int y, int w, int h, const char *title=0, int id=0);
	gWindow(int w, int h, const char *title=0, int id=0);
	~gWindow();

	static void cb_closeChild(Fl_Widget *v, void *p);

	void addSubWindow(gWindow *w);
	void delSubWindow(gWindow *w);
	void delSubWindow(int id);

	int  getId();
	void setId(int id);
	void debug();

	void     setParent(gWindow *);
	gWindow *getParent();
	gWindow *getChild(int id);

	/* hasWindow
	 * true if the window with id 'id' exists in the stack. */

	bool hasWindow(int id);

};


#endif
