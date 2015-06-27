/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_mainWindow
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

#define 	MAINWIN_W		GUI_WIDTH
#define		MAINWIN_H		165

#ifdef __linux__
	#include <sys/stat.h>			// for mkdir
#endif


#include "engine.h"
#include "fltk.h"
#include "gd_about.h"
#include "gui_utils.h"
#include "graphics.h"


#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


#if defined(WITH_VST)
extern PluginHost  	 G_PluginHost;
#endif



gdMainWindow::gdMainWindow(int W, int H, const char *title, int argc, char **argv)
	: gWindow(W, H, title)
{
	Fl::visible_focus(0);
	Fl::background(25, 25, 25);
	Fl::set_boxtype(G_BOX, gDrawBox, 1, 1, 2, 2);    // custom box G_BOX

	size_range(MAINWIN_W, MAINWIN_H);

	menu       = new gMenu(8, 8);
	inOut      = new gInOut(408, 8);
	controller = new gController(8, 37);
	timing     = new gTiming(/*632*/ 590, 37);
	songMeter  = new gSongMeter(100, 73, 609, 20);
	beatMeter  = new gBeatMeter(100, 93, 609, 20);
	launcher   = new gLauncher(8, 122, w()-16, 24);
	uniKeyboard = new gKeyboard(8, 146, w()-16, 356); 

	uniKeyboard->init(DEF_COLS);				
	launcher->setUniKeyboard( uniKeyboard );

	/* zone 1 - menus, and I/O tools */

	Fl_Group *zone1 = new Fl_Group(8, 8, W-16, 20);
	zone1->add(menu);
	zone1->resizable(new Fl_Box(300, 8, 80, 20));
	zone1->add(inOut);

	/* zone 2 - controller and timing tools */

	Fl_Group *zone2 = new Fl_Group(8, timing->y(), W-16, timing->h());
	zone2->add(controller);
	zone2->resizable(new Fl_Box(controller->x()+controller->w()+4, zone2->y(), 80, 20));
	zone2->add(timing);

	/* zone 3 - song meter and beat meter */

	Fl_Group *zone3 = new Fl_Group(8, songMeter->y(), W-16, songMeter->h()+beatMeter->h());
	zone3->add(songMeter);
	zone3->add(beatMeter);

	add(zone1);
	add(zone2);
	add(zone3);

	/* 
	 * zone 4,5 - the Launcher and Uni keyboard  
	 * 
	 * KBLaucher adds apparent columns as they are needed, and each one gets it's own top window.
	 * As required though, it really creates a gKeyboard instance in each new window it opens.
	 * Each keyboard gets a mbreak number assiged to it, and otherwise acts like the gKeyboard did. 
	 */

	//resizable( launcher );
	add(launcher);	
	resizable( uniKeyboard );
	add(uniKeyboard);
	
	callback(cb_endprogram);
	gu_setFavicon(this);

	show(argc, argv);
}

/*
gdMainWindow::~gdMainWindow() {}
*/

/* ------------------------------------------------------------------ */


void gdMainWindow::cb_endprogram(Fl_Widget *v, void *p) { mainWin->__cb_endprogram(); }


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_endprogram()
{
	if (!gdConfirmWin("Warning", "Quit Giada: are you sure?"))
		return;

	init::shutdown();  // I would like to move this to main()

	hide();
	delete this;
}


gdMainWindow *mainWin;


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gInOut::gInOut(int x, int y)
	: Fl_Group(x, y, 400, 20)
{
	begin();

#if defined(WITH_VST)
	masterFxIn  = new gButton    (x, y, 20, 20, "", fxOff_xpm, fxOn_xpm);
	inVol		    = new gDial      (masterFxIn->x()+masterFxIn->w()+4, y, 20, 20);
	inMeter     = new gSoundMeter(inVol->x()+inVol->w()+4, y+5, 140, 10);
	inToOut     = new gClick     (inMeter->x()+inMeter->w()+4, y+5, 10, 10, "");
	outMeter    = new gSoundMeter(inToOut->x()+inToOut->w()+4, y+5, 140, 10);
	outVol		  = new gDial      (outMeter->x()+outMeter->w()+4, y, 20, 20);
	masterFxOut = new gButton    (outVol->x()+outVol->w()+4, y, 20, 20, "", fxOff_xpm, fxOn_xpm);
#else
	outMeter    = new gSoundMeter(x, y+5, 140, 10);
	inMeter     = new gSoundMeter(outMeter->x()+outMeter->w()+4, y+5, 140, 10);
	outVol		  = new gDial      (inMeter->x()+inMeter->w()+4, y, 20, 20);
	inVol		    = new gDial      (outVol->x()+outVol->w()+4, y, 20, 20);
#endif

	end();

	resizable(NULL);   // don't resize any widget

	outVol->callback(cb_outVol, (void*)this);
	outVol->value(G_Mixer.outVol);
	inVol->callback(cb_inVol, (void*)this);
	inVol->value(G_Mixer.inVol);

#ifdef WITH_VST
	masterFxOut->callback(cb_masterFxOut, (void*)this);
	masterFxIn->callback(cb_masterFxIn, (void*)this);
	inToOut->callback(cb_inToOut, (void*)this);
	inToOut->type(FL_TOGGLE_BUTTON);
#endif

	G_Interface->inout = (void *) this;
	G_Interface->setInVol = cb_setInVol;
	G_Interface->setOutVol = cb_setOutVol;
	gLog("interface.inout:setInVol,setOutVol\n");
}


/* ------------------------------------------------------------------ */

void gInOut::cb_setOutVol(float v, void *p) {
	Fl::lock();
	((gInOut*)p)->setOutVol(v);
	Fl::unlock();
 }

void gInOut::cb_setInVol(float v, void *p) {
	Fl::lock();
	((gInOut*)p)->setInVol(v);
	Fl::unlock();
 }

void gInOut::cb_outVol     (Fl_Widget *v, void *p)  	{ ((gInOut*)p)->__cb_outVol(); }
void gInOut::cb_inVol      (Fl_Widget *v, void *p)  	{ ((gInOut*)p)->__cb_inVol(); }
#ifdef WITH_VST
void gInOut::cb_masterFxOut(Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_masterFxOut(); }
void gInOut::cb_masterFxIn (Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_masterFxIn(); }
void gInOut::cb_inToOut    (Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_inToOut(); }
#endif


/* ------------------------------------------------------------------ */


void gInOut::__cb_outVol()
{
	control::setOutVol(outVol->value());
}


/* ------------------------------------------------------------------ */


void gInOut::__cb_inVol()
{
	control::setInVol(inVol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gInOut::__cb_masterFxOut()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_OUT), WID_FX_LIST);
}

void gInOut::__cb_masterFxIn()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_IN), WID_FX_LIST);
}

void gInOut::__cb_inToOut()
{
	G_Mixer.inToOut = inToOut->value();
}
#endif


/* ------------------------------------------------------------------ */


void gInOut::refresh()
{
	outMeter->mixerPeak = G_Mixer.peakOut;
	inMeter->mixerPeak  = G_Mixer.peakIn;
	outMeter->redraw();
	inMeter->redraw();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gMenu::gMenu(int x, int y)
	: Fl_Group(x, y, 300, 20)
{
	begin();

	file   = new gClick(x, y, 70, 21, "file");
	edit   = new gClick(file->x()+file->w()+4,  y, 70, 21, "edit");
	config = new gClick(edit->x()+edit->w()+4, y, 70, 21, "config");
	about	 = new gClick(config->x()+config->w()+4, y, 70, 21, "about");

	end();

	resizable(NULL);   // don't resize any widget

	about->callback(cb_about, (void*)this);
	file->callback(cb_file, (void*)this);
	edit->callback(cb_edit, (void*)this);
	config->callback(cb_config, (void*)this);
}


/* ------------------------------------------------------------------ */


void gMenu::cb_about (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_about(); }
void gMenu::cb_config(Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_config(); }
void gMenu::cb_file  (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_file(); }
void gMenu::cb_edit  (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_edit(); }


/* ------------------------------------------------------------------ */


void gMenu::__cb_about()
{
	gu_openSubWindow(mainWin, new gdAbout(), WID_ABOUT);
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_config()
{
	gu_openSubWindow(mainWin, new gdConfig(380, 370), WID_CONFIG);
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_file()
{
	/* An Fl_Menu_Button is made of many Fl_Menu_Item */

	Fl_Menu_Item menu[] = {
		{"Open patch or project..."},
		{"Save patch..."},
		{"Save project..."},
		{"Quit Giada"},
		{0}
	};

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = menu->popup(Fl::event_x(),	Fl::event_y(), 0, 0, b);
	if (!m) return;


	if (strcmp(m->label(), "Open patch or project...") == 0) {
		gWindow *childWin = new gdBrowser("Load Patch", G_Conf.patchPath, 0, BROWSER_LOAD_PATCH);
		gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
		return;
	}
	if (strcmp(m->label(), "Save patch...") == 0) {
		if (G_Mixer.hasLogicalSamples() || G_Mixer.hasEditedSamples())
			if (!gdConfirmWin("Warning", "You should save a project in order to store\nyour takes and/or processed samples."))
				return;
		gWindow *childWin = new gdBrowser("Save Patch", G_Conf.patchPath, 0, BROWSER_SAVE_PATCH);
		gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
		return;
	}
	if (strcmp(m->label(), "Save project...") == 0) {
		gWindow *childWin = new gdBrowser("Save Project", G_Conf.patchPath, 0, BROWSER_SAVE_PROJECT);
		gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
		return;
	}
	if (strcmp(m->label(), "Quit Giada") == 0) {
		mainWin->do_callback();
		return;
	}
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_edit()
{
	Fl_Menu_Item menu[] = {
		{"Clear all samples"},
		{"Clear all actions"},
		{"Remove empty columns"},
		{"Reset to init state"},
		{"Setup global MIDI input..."},
		{0}
	};

	/* clear all actions disabled if no recs, clear all samples disabled
	 * if no samples. */

	menu[1].deactivate();

	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->hasActions) {
			menu[1].activate();
			break;
		}
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)G_Mixer.channels.at(i))->wave != NULL) {
				menu[0].activate();
				break;
			}

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = menu->popup(Fl::event_x(),	Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Clear all samples") == 0) {
		if (!gdConfirmWin("Warning", "Clear all samples: are you sure?"))
			return;
		mainWin->delSubWindow(WID_SAMPLE_EDITOR);
		control::clearAllSamples();
		gu_update_controls();
		return;
	}
	if (strcmp(m->label(), "Clear all actions") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		mainWin->delSubWindow(WID_ACTION_EDITOR);
		control::clearAllRecs();
		gu_update_controls();
		return;
	}
	if (strcmp(m->label(), "Reset to init state") == 0) {
		if (!gdConfirmWin("Warning", "Reset to init state: are you sure?"))
			return;
		gu_closeAllSubwindows();
		control::resetToInitState();
		gu_update_controls();
		return;
	}
	if (strcmp(m->label(), "Remove empty columns") == 0) {
		mainWin->launcher->organizeColumns();
		return;
	}
	if (strcmp(m->label(), "Setup global MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberMaster(), 0);
		return;
	}
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */

gController::gController(int x, int y)
	: Fl_Group(x, y, 131, 25)
{
	begin();

	rewind    = new gClick(x,  y, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play      = new gClick(rewind->x()+rewind->w()+4, y, 25, 25, "", play_xpm, pause_xpm);
	recAction = new gClick(play->x()+play->w()+4, y, 25, 25, "", recOff_xpm, recOn_xpm);
	recInput  = new gClick(recAction->x()+recAction->w()+4, y, 25, 25, "", inputRecOff_xpm, inputRecOn_xpm);
	metronome = new gClick(recInput->x()+recInput->w()+4, y+10, 15, 15, "", metronomeOff_xpm, metronomeOn_xpm);

	end();

	resizable(NULL);   // don't resize any widget

	rewind->callback(cb_rewind, (void*)this);

	play->callback(cb_play);
	play->type(FL_TOGGLE_BUTTON);

	recAction->callback(cb_recAction, (void*)this);
	recAction->type(FL_TOGGLE_BUTTON);

	recInput->callback(cb_recInput, (void*)this);
	recInput->type(FL_TOGGLE_BUTTON);

	metronome->callback(cb_metronome, (void*)this);
	metronome->type(FL_TOGGLE_BUTTON);

	/* setup the callbacks in G_Interface */
	G_Interface->controller = (void *)this;
	G_Interface->updatePlay = cb_updatePlay;
	G_Interface->updateMetronome = cb_updateMetronome;
	G_Interface->updateRecInput = cb_updateRecInput;
	G_Interface->updateRecAction = cb_updateRecAction;

	/* set up the channel updateActionButton as well */
	G_Interface->updateActionButton = &(gSampleChannel::cb_updateActionButton);

	gLog("controller.updatePlay,updateMetronome,updateRecInput,updateRecAction\n");
}


/* ------------------------------------------------------------------ */


void gController::cb_rewind   (Fl_Widget *v, void *p) { ((gController*)p)->__cb_rewind(); }
void gController::cb_play     (Fl_Widget *v, void *p) { ((gController*)p)->__cb_play(); }
void gController::cb_recAction(Fl_Widget *v, void *p) { ((gController*)p)->__cb_recAction(); }
void gController::cb_recInput (Fl_Widget *v, void *p) { ((gController*)p)->__cb_recInput(); }
void gController::cb_metronome(Fl_Widget *v, void *p) { ((gController*)p)->__cb_metronome(); }

//
// these callbacks are called from main thread, so use locking
//

void gController::cb_updatePlay(int n, void *p) { 
	Fl::lock();
	((gController*)p)->updatePlay(n);
	Fl::unlock();
}
void gController::cb_updateMetronome(int n, void *p) {
	Fl::lock();
	((gController*)p)->updateMetronome(n); 
	Fl::unlock();
}
void gController::cb_updateRecInput(int n, void *p) {
	Fl::lock();
	((gController*)p)->updateRecInput(n);
	Fl::unlock();
}
void gController::cb_updateRecAction(int n, void *p) {
	Fl::lock();
	((gController*)p)->updateRecAction(n);
	Fl::unlock();
}


/* ------------------------------------------------------------------ */


void gController::__cb_rewind()
{
	control::rewindSeq();
}


/* ------------------------------------------------------------------ */


void gController::__cb_play()
{
	control::startStopSeq();
}


/* ------------------------------------------------------------------ */


void gController::__cb_recAction()
{
	control::startStopActionRec();
}


/* ------------------------------------------------------------------ */


void gController::__cb_recInput()
{
	control::startStopInputRec();
}


/* ------------------------------------------------------------------ */


void gController::__cb_metronome()
{
	control::startStopMetronome();
}


/* ------------------------------------------------------------------ */


void gController::updatePlay(int v)
{
	play->value(v);
	play->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateMetronome(int v)
{
	metronome->value(v);
	metronome->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateRecInput(int v)
{
	recInput->value(v);
	recInput->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateRecAction(int v)
{
	recAction->value(v);
	recAction->redraw();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gTiming::gTiming(int x, int y)
	: Fl_Group(x, y, 170+40, 50)
{
	begin();

	quantizer  = new gChoice(x, y, 40, 15, "", false);
	incrBpm    = new gClick (quantizer->x()+quantizer->w()+4,  y, 15, 15, "+");
	bpm        = new gClick (incrBpm->x()+incrBpm->w()+4,  y, 40, 15);
	decrBpm    = new gClick (bpm->x()+bpm->w()+4, y, 15, 15, "-");
	meter      = new gClick (decrBpm->x()+decrBpm->w()+8, y+20, 40, 15, "4/1");
	multiplier = new gClick (meter->x()+meter->w()+4, y+20, 15, 15, "×");
	divider    = new gClick (multiplier->x()+multiplier->w()+4, y+20, 15, 15, "÷");
	
	suspend    = new gCheck (decrBpm->x()+decrBpm->w()+8, y, 15, 15 );
	advance    = new gClick (suspend->x()+suspend->w()+5, y, 20, 15, ">>");
	songmultiplier = new gClick (advance->x()+advance->w()+4, y, 15, 15, "X");
	songdivider    = new gClick (songmultiplier->x()+songmultiplier->w()+4, y, 15, 15, "/");

	end();

	resizable(NULL);   // don't resize any widget

	char buf[6]; snprintf(buf, 6, "%f", G_Mixer.bpm);
	bpm->copy_label(buf);

	bpm->callback(cb_bpm, (void*)this);
	incrBpm->callback(cb_incrBpm, (void*)this);
	decrBpm->callback(cb_decrBpm, (void*)this);
	meter->callback(cb_meter, (void*)this);
	multiplier->callback(cb_multiplier, (void*)this);
	divider->callback(cb_divider, (void*)this);
	
	songmultiplier->callback(cb_songmultiplier, (void*)this);
 	songdivider->callback(cb_songdivider, (void*)this);
 	advance->callback(cb_advance, (void*)this);
 	suspend->callback(cb_suspend, (void*)this);
 	suspend->value(1);

	quantizer->add("off", 0, cb_quantizer, (void*)this);
	quantizer->add("1b",  0, cb_quantizer, (void*)this);
	quantizer->add("2b",  0, cb_quantizer, (void*)this);
	quantizer->add("3b",  0, cb_quantizer, (void*)this);
	quantizer->add("4b",  0, cb_quantizer, (void*)this);
	quantizer->add("6b",  0, cb_quantizer, (void*)this);
	quantizer->add("8b",  0, cb_quantizer, (void*)this);
	quantizer->value(0); //  "off" by default

	G_Interface->timing = (void *) this;
	G_Interface->setBeatMeter = cb_setBeatMeter;

	gLog("interface.timing:setMeter\n");
}


/* ------------------------------------------------------------------ */



void gTiming::cb_setBeatMeter  (int beats, int bars, void *p) { 

	((gTiming*)p)->setMeter(beats, bars);
}



void gTiming::cb_bpm       (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_bpm(); }
void gTiming::cb_incrBpm   (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_incrBpm(); }
void gTiming::cb_decrBpm   (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_decrBpm(); }
void gTiming::cb_meter     (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_meter(); }
void gTiming::cb_quantizer (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_quantizer(); }
void gTiming::cb_multiplier(Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_multiplier(); }
void gTiming::cb_divider   (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_divider(); }
void gTiming::cb_songmultiplier(Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_songmultiplier(); }
void gTiming::cb_songdivider   (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_songdivider(); }
void gTiming::cb_advance       (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_advance(); }
void gTiming::cb_suspend       (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_suspend(); }


/* ------------------------------------------------------------------ */


void gTiming::__cb_bpm()
{
	gu_openSubWindow(mainWin, new gdBpmInput(bpm->label()), WID_BPM);
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_incrBpm()
{
	adjustBpm( 1.0f );
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_decrBpm()
{
	adjustBpm( -1.0f );
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_meter()
{
	gu_openSubWindow(mainWin, new gdBeatsInput(), WID_BEATS);
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_quantizer()
{
	control::quantize(quantizer->value());
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_multiplier()
{
	control::beatsMultiply();
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_divider()
{
	control::beatsDivide();
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_songmultiplier()
{
	control::breaksMultiply();
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_songdivider()
{
	control::breaksDivide();
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_advance()
{
	G_Mixer.advance();
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_suspend()
{
	G_Mixer.suspend = !G_Mixer.suspend;
}


/* ------------------------------------------------------------------ */


void gTiming::setBpm(const char *v)
{
	bpm->copy_label(v);
}


/* ------------------------------------------------------------------ */


void gTiming::setMeter(int beats, int bars)
{
	char buf[8];
	sprintf(buf, "%d/%d", beats, bars);
	meter->copy_label(buf);
}

