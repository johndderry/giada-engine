/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_launcher
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2015 John D. Derry
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
#include "fltk.h"
#include "gui_utils.h"
#include "graphics.h"


#ifdef WITH_VST
#include "gd_pluginList.h"
#endif



gdSubWindow::gdSubWindow(int W, int H, const char *title, int argc, char **argv)
	: gWindow(W, H, title)
{
	Fl::visible_focus(0);
	Fl::background(25, 25, 25);
	Fl::set_boxtype(G_BOX, gDrawBox, 1, 1, 2, 2);    // custom box G_BOX

	size_range(SUBWIN_W, SUBWIN_H);

	keyboard   = new gKeyboard(0, 0, W, H);
	resizable( keyboard );
	add(keyboard);

	//callback(cb_close, (void *)this );
	gu_setFavicon(this);

	show(argc, argv);
}


void gdSubWindow::cb_close(Fl_Widget *v, void *p) { ((gdSubWindow *)p)->__cb_close(); }


void gdSubWindow::__cb_close() { hide(); }


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */

const char *gLauncher::winLabels[MAX_BREAKS] = {
	"Sect A", "Sect B",	"Sect C", "Sect D",	"Sect E", "Sect F",
	"Sect G", "Sect H",	"Sect I", "Sect J",	"Sect K", "Sect L",
	"Sect M", "Sect N",	"Sect O", "Sect P",	"Sect Q", "Sect R",
	"Sect S", "Sect T",	"Sect U", "Sect V",	"Sect W", "Sect X",
	"Sect Y", "Sect Z",	"Sect a", "Sect B",	"Sect c", "Sect d",
	"Sect e", "Sect f" };

const char *gLauncher::winSect[MAX_BREAKS] = {
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J",	"K", "L",
	"M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",	"W", "X",
	"Y", "Z", "a", "B", "c", "d", "e", "f" };

gLauncher::gLauncher(int X, int Y, int W, int H)
: Fl_Group    (X, Y, W, H), addSampleChanBtn(NULL), addMidiChanBtn(NULL) {

#if 0
	addSampleChanBtn = new gClick( x(), y(), 90, 20, "NewSampChan");
	addSampleChanBtn->callback( cb_addSampleChan, (void *)this );
	addMidiChanBtn = new gClick( x()+100, y(), 90, 20, "NewMidiChan");
	addMidiChanBtn->callback( cb_addMidiChan, (void *)this );
	int xpos = 204;
#endif
	int xpos = 40;

	for( int i=0; i < MAX_BREAKS; i++, xpos+=22 ) {
		keyboards[i] = NULL;
		subwindows[i] = NULL;
//		active[i] = new gCheck( xpos, y()+4, 20, 20, winSect[i]);
		active[i] = new gCheck( xpos, y()+4, 20, 20, "");
		active[i]->callback( cb_checkwindows, (void *)this );
	}
	
	uniKeyboard = NULL;

// this is broken, bad crash	
#if 0
	refreshBreakButtons( G_Mixer.breaks );
#endif

	G_Interface->launcher = (void *) this;
	G_Interface->setBreakMeter = cb_setBreakMeter;

}

gLauncher::~gLauncher() {
	
	for( int i=0; i < MAX_BREAKS; i++ ) 
		if( subwindows[i] )  subwindows[i]->hide();
	
}

void gLauncher::refreshBreakButtons( int breaks ) {

	for( int i=0; i < MAX_BREAKS; i++ ) {
		if( i < breaks ) 
			active[i]->show();
		else 
			active[i]->hide();
	}
}

void gLauncher::cb_checkwindows  (Fl_Widget *v, void *p ) {

	Fl::lock();
	((gLauncher *)p)->__cb_checkwindows();
	Fl::unlock();
		
}

void gLauncher::__cb_checkwindows () {

	
	for( int pos = 0; pos < G_Mixer.breaks; pos++ ) {

		if( active[pos]->value() ) {
			// indicates that there should be a keyboard scd howing
			if( keyboards[pos] ) { 
				if( subwindows[pos]  == NULL) {
					// keyboard without a subwindow, create it
					subwindows[pos] = new gdSubWindow(GUI_WIDTH, GUI_HEIGHT, winLabels[pos], 0, NULL );
					subwindows[pos]->add( keyboards[pos] );
				}
				else
					// make sure it's showing
					subwindows[pos]->show();
			}
			else {
				// create both, since there would never be a subwindow without a keyboard
				subwindows[pos] = new gdSubWindow(GUI_WIDTH, GUI_HEIGHT, winLabels[pos], 0, NULL );
				keyboards[pos] = new gKeyboard(0, 0, subwindows[pos]->w(), subwindows[pos]->h() );
				keyboards[pos]->mbreak = pos;
				keyboards[pos]->init(DEF_COLS);				
				subwindows[pos]->add( keyboards[pos] );
			}
		}
		else
			// hide the subwindow if it exits
			if( subwindows[pos] ) subwindows[pos]->hide();
	}
}


void gLauncher::cb_setBreakMeter  (int breaks, void *p) { 

#if 0
	Fl::lock();
	((gLauncher*)p)->refreshBreakButtons( breaks );
	Fl::unlock();
#endif
}


void gLauncher::cb_addSampleChan(Fl_Widget *v, void *p) {
	Fl::lock();
	((gLauncher*)p)->__cb_addSampleChan();
	Fl::unlock();
}


void gLauncher::cb_addMidiChan(Fl_Widget *v, void *p) {
	Fl::lock();
	((gLauncher*)p)->__cb_addMidiChan();
	Fl::unlock();
 }



void gLauncher::__cb_addSampleChan()
{
	
	gLog("[gLauncher] new sample channel added\n");
}


void gLauncher::__cb_addMidiChan()
{
	gLog("[gLauncher] new midi channel added\n");
}


void gLauncher::clear() 
{
	if( uniKeyboard ) uniKeyboard->clear();
	
	for(unsigned int pos =0; pos < MAX_BREAKS; pos++ )

		if( keyboards[pos] ) (keyboards[pos])->clear();

}


/* ------------------------------------------------------------------ */


void gLauncher::freeChannel(gChannel *gch)
{
	gch->reset();
}


/* ------------------------------------------------------------------ */


void gLauncher::deleteChannel(gChannel *gch)
{
	if ( uniKeyboard ) uniKeyboard->deleteChannel(gch);
	
	for(unsigned int pos =0; pos < MAX_BREAKS; pos++ )

		if( keyboards[pos] ) (keyboards[pos])->deleteChannel(gch);

}



/* ------------------------------------------------------------------ */


void gLauncher::refreshColumns()
{
	
	if( uniKeyboard ) uniKeyboard->refreshColumns();
	
	for(unsigned int pos =0; pos < MAX_BREAKS; pos++ )

		if( keyboards[pos] ) (keyboards[pos])->refreshColumns();

}



/* ------------------------------------------------------------------ */


void gLauncher::printChannelMessage(int res)
{
	if      (res == SAMPLE_NOT_VALID)
		gdAlert("This is not a valid WAVE file.");
	else if (res == SAMPLE_MULTICHANNEL)
		gdAlert("Multichannel samples not supported.");
	else if (res == SAMPLE_WRONG_BIT)
		gdAlert("This sample has an\nunsupported bit-depth (> 32 bit).");
	else if (res == SAMPLE_WRONG_ENDIAN)
		gdAlert("This sample has a wrong\nbyte order (not little-endian).");
	else if (res == SAMPLE_WRONG_FORMAT)
		gdAlert("This sample is encoded in\nan unsupported audio format.");
	else if (res == SAMPLE_READ_ERROR)
		gdAlert("Unable to read this sample.");
	else if (res == SAMPLE_PATH_TOO_LONG)
		gdAlert("File path too long.");
	else
		gdAlert("Unknown error.");
}


/* ------------------------------------------------------------------ */


void gLauncher::organizeColumns()
{
	if( uniKeyboard ) uniKeyboard->organizeColumns();

	for(unsigned int pos =0; pos < MAX_BREAKS; pos++ )

		if( keyboards[pos] ) (keyboards[pos])->organizeColumns();

}


/* ------------------------------------------------------------------ */


void gLauncher::redraw()
{
	if( uniKeyboard ) uniKeyboard->redraw();
	
	for( int pos = 0; pos < G_Mixer.breaks; pos++ )

		if( active[pos]->value() && keyboards[pos] )  

			(keyboards[pos])->redraw();
}


/* ------------------------------------------------------------------ */


void gLauncher::setChannelWithActions(gSampleChannel *gch)
{
	if (gch->ch->hasActions)
		gch->addActionButton();
	else
		gch->delActionButton();
}


/* ------------------------------------------------------------------ */


void gLauncher::updateChannel(gChannel *gch)
{
	gch->update();
}

void gLauncher::setUniKeyboard( gKeyboard *kb ) {
	
	uniKeyboard = kb;
}

