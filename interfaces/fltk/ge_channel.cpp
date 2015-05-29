/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channel
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


#include "engine.h"
#include "fltk.h"
#include "gui_utils.h"
#include "graphics.h"


#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


gChannel::gChannel(int X, int Y, int W, int H)
 : Fl_Group(X, Y, W, H, NULL)
{
}


/* ------------------------------------------------------------------ */


int gChannel::getColumnIndex()
{
	return ((gColumn*)parent())->getIndex();
}


/* ------------------------------------------------------------------ */


void gChannel::cb_setChanVol(void *ch, int v, void *p )
{
	((gChannel*)ch)->vol->value(v);
}

void gChannel::cb_reset(void *ch, void *p )
{
	((gChannel*)ch)->reset();
}


/* ------------------------------------------------------------------ */


gSampleChannel::gSampleChannel(int X, int Y, int W, int H, class SampleChannel *ch)
	: gChannel(X, Y, W, H), ch(ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 168; // (7 widgets * 20) + (7 paddings * 4)
#else
	int delta = 144; // (6 widgets * 20) + (6 paddings * 4)
#endif

	button       = new gButton (x(), y(), 20, 20);
	status       = new gStatus (button->x()+button->w()+4, y(), 20, 20, ch);
	sampleButton = new gClick  (status->x()+status->w()+4, y(), w() - delta, 20, "-- no sample --");
	modeBox      = new gModeBox(sampleButton->x()+sampleButton->w()+4, y(), 20, 20, ch);
	mute         = new gClick  (modeBox->x()+modeBox->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick  (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);

#if defined(WITH_VST)
	fx           = new gButton (solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial   (fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol          = new gDial   (solo->x()+solo->w()+4, y(), 20, 20);
#endif

	readActions  = NULL; // no 'R' button

	end();

	resizable(sampleButton);

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	sampleButton->callback(cb_openMenu, (void*)this);
	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* ------------------------------------------------------------------ */


void gSampleChannel::cb_button      (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_button(); }
void gSampleChannel::cb_mute        (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_mute(); }
void gSampleChannel::cb_solo        (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_solo(); }
void gSampleChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_openMenu(); }
void gSampleChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_changeVol(); }
void gSampleChannel::cb_readActions (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_readActions(); }
#ifdef WITH_VST
void gSampleChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_mute()
{
	control::setMute(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_solo()
{
	solo->value() ? control::setSoloOn(ch) : control::setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_changeVol()
{
	control::setChanVol(ch, vol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gSampleChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif


/* ------------------------------------------------------------------ */



void gSampleChannel::__cb_button()
{
	if (button->value())    // pushed
		control::keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		control::keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_openMenu()
{
	/* if you're recording (actions or input) no menu is allowed; you can't
	 * do anything, especially deallocate the channel */

	if (G_Mixer.chanInput == ch || recorder::active)
		return;

	/* the following is a trash workaround for a FLTK menu. We need a gMenu
	 * widget asap */

	Fl_Menu_Item rclick_menu[] = {
		{"Load new sample..."},                     // 0
		{"Export sample to file..."},               // 1
		{"Setup keyboard input..."},                // 2
		{"Setup MIDI input..."},                    // 3
		{"Edit sample..."},                         // 4
		{"Edit actions..."},                        // 5
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 6
			{"All"},                                  // 7
			{"Mute"},                                 // 8
			{"Volume"},                               // 9
			{"Start/Stop"},                           // 10
			{0},                                      // 11
		{"Free channel"},                           // 12
		{"Delete channel"},                         // 13
		{0}
	};

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		rclick_menu[1].deactivate();
		rclick_menu[4].deactivate();
		rclick_menu[12].deactivate();
	}

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[6].deactivate();

	/* no 'clear start/stop actions' for those channels in loop mode:
	 * they cannot have start/stop actions. */

	if (ch->mode & LOOP_ANY)
		rclick_menu[10].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Load new sample...") == 0) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Setup keyboard input...") == 0) {
		new gdKeyGrabber(ch); /// FIXME - use gu_openSubWindow
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Edit sample...") == 0) {
		gu_openSubWindow(mainWin, new gdEditor(ch), WID_SAMPLE_EDITOR); /// FIXME title it's up to gdEditor
		return;
	}

	if (strcmp(m->label(), "Export sample to file...") == 0) {
		openBrowser(BROWSER_SAVE_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Delete channel") == 0) {
		if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
			return;
		deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Free channel") == 0) {
		if (ch->status == STATUS_PLAY) {
			if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
				return;
		}
		else if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
			return;

		freeChannel(ch);

		/* delete any related subwindow */

		/** FIXME - use gu_closeAllSubwindows() */

		mainWin->delSubWindow(WID_FILE_BROWSER);
		mainWin->delSubWindow(WID_ACTION_EDITOR);
		mainWin->delSubWindow(WID_SAMPLE_EDITOR);
		mainWin->delSubWindow(WID_FX_LIST);

		return;
	}

	if (strcmp(m->label(), "Mute") == 0) {
		if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_MUTEON | ACTION_MUTEOFF);
		if (!ch->hasActions)
			delActionButton();

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
		if (!ch->hasActions)
			delActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Volume") == 0) {
		if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_VOLUME);
		if (!ch->hasActions)
			delActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		delActionButton();
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_readActions()
{
	ch->readActions ? control::stopReadingRecs(ch) : control::startReadingRecs(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::openBrowser(int type)
{
	const char *title = "";
	switch (type) {
		case BROWSER_LOAD_SAMPLE:
			title = "Browse Sample";
			break;
		case BROWSER_SAVE_SAMPLE:
			title = "Save Sample";
			break;
		case -1:
			title = "Edit Sample";
			break;
	}
	gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, ch, type);
	gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::refresh()
{
  if (!sampleButton->visible()) // sampleButton invisible? status too (see below)
    return;

	if (ch->status == STATUS_OFF) {
		sampleButton->bgColor0 = COLOR_BG_0;
		sampleButton->bdColor  = COLOR_BD_0;
		sampleButton->txtColor = COLOR_TEXT_0;
	}
	else
	if (ch->status == STATUS_PLAY) {
		sampleButton->bgColor0 = COLOR_BG_2;
		sampleButton->bdColor  = COLOR_BD_1;
		sampleButton->txtColor = COLOR_TEXT_1;
	}
	else
	if (ch->status & (STATUS_WAIT | STATUS_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->recStatus & (REC_WAITING | REC_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->wave != NULL) {

		if (G_Mixer.chanInput == ch)
			sampleButton->bgColor0 = COLOR_BG_3;

		if (recorder::active) {
			if (recorder::canRec(ch)) {
				sampleButton->bgColor0 = COLOR_BG_4;
				sampleButton->txtColor = COLOR_TEXT_0;
			}
		}
		status->redraw(); // status invisible? sampleButton too (see below)
	}
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::reset()
{
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	//sampleButton->label("-- no sample --");
	sampleButton->label(ch->name.c_str());
	delActionButton(true); // force==true, don't check, just remove it
 	sampleButton->redraw();
	status->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::update()
{
	/* update sample button's label */

	switch (ch->status) {
		case STATUS_EMPTY:
			sampleButton->label("-- empty sample --");
			break;
		case STATUS_MISSING:
		case STATUS_WRONG:
			sampleButton->label("* file not found! *");
			break;
		default:
			sampleButton->label(ch->name.c_str());
			break;
	}

	/* update channels. If you load a patch with recorded actions, the 'R'
	 * button must be shown. Moreover if the actions are active, the 'R'
	 * button must be activated accordingly. */

	if (ch->hasActions)
		addActionButton();
	else
		delActionButton();

	/* update key box */

	char k[4];
	sprintf(k, "%c", ch->key);
	button->copy_label(k);
	button->redraw();

	/* updates modebox */

	modeBox->value(ch->mode);
	modeBox->redraw();

	/* update volumes+mute+solo */

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);
}


/* ------------------------------------------------------------------ */


int gSampleChannel::keyPress(int e)
{
	int ret;
	if (e == FL_KEYDOWN && button->value())                              // key already pressed! skip it
		ret = 1;
	else
	if (Fl::event_key() == ch->key && !button->value()) {
		button->take_focus();                                              // move focus to this button
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
		button->do_callback();                                             // invoke the button's callback
		ret = 1;
	}
	else
		ret = 0;

	if (Fl::event_key() == ch->key)
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state

	return ret;
}


/* ------------------------------------------------------------------ */


void gSampleChannel::addActionButton()
{
	/* quit if 'R' exists yet. */

	if (readActions != NULL)
		return;

	sampleButton->size(sampleButton->w()-24, sampleButton->h());

	redraw();

	readActions = new gClick(sampleButton->x() + sampleButton->w() + 4,
                           sampleButton->y(), 20, 20, "", readActionOff_xpm,
                           readActionOn_xpm);
	readActions->type(FL_TOGGLE_BUTTON);
	readActions->value(ch->readActions);
	readActions->callback(cb_readActions, (void*)this);
	add(readActions);

	/* hard redraw: there's no other way to avoid glitches when moving
	 * the 'R' button */

	mainWin->launcher->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::delActionButton(bool force)
{
	if (readActions == NULL)
		return;

	/* TODO - readActions check is useless here */

	if (!force && (readActions == NULL || ch->hasActions))
		return;

	remove(readActions);		// delete from Keyboard group (FLTK)
	delete readActions;     // delete (C++)
	readActions = NULL;

	sampleButton->size(sampleButton->w()+24, sampleButton->h());
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::resize(int X, int Y, int W, int H)
{
  w() < 164 ? status->hide()  : status->show();
  w() < 140 ? modeBox->hide() : modeBox->show();
  sampleButton->w() < 20 ? sampleButton->hide() : sampleButton->show();
  gChannel::resize(X, Y, W, H);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gMidiChannel::gMidiChannel(int X, int Y, int W, int H, class MidiChannel *ch)
	: gChannel(X, Y, W, H), ch(ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 120; // (5 widgets * 20) + (5 paddings * 4)
#else
	int delta = 96; // (4 widgets * 20) + (4 paddings * 4)
#endif

	button       = new gButton (x(), y(), 20, 20);
	recordButton = new gClick (button->x()+button->w()+4, y(), 20, 20);
	sampleButton = new gClick (recordButton->x()+recordButton->w()+4, y(), w() - delta - 20, 20, "-- MIDI --");
	mute         = new gClick (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx           = new gButton(solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial  (fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol          = new gDial  (solo->x()+solo->w()+4, y(), 20, 20);
#endif

	end();

	resizable(sampleButton);

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	recordButton->callback(cb_record, (void*)this);
	sampleButton->callback(cb_openMenu, (void*)this);
	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* ------------------------------------------------------------------ */


void gMidiChannel::cb_button      (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_button(); }
void gMidiChannel::cb_record      (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_record(); }
void gMidiChannel::cb_mute        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_mute(); }
void gMidiChannel::cb_solo        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_solo(); }
void gMidiChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openMenu(); }
void gMidiChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_changeVol(); }
#ifdef WITH_VST
void gMidiChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_mute()
{
	control::setMute(ch);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_solo()
{
	solo->value() ? control::setSoloOn(ch) : control::setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_changeVol()
{
	control::setChanVol(ch, vol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gMidiChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_button()
{
	if (button->value())
		control::keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_record()
{
	G_Mixer.swapMidiIn ( ch->index );
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_openMenu()
{
	Fl_Menu_Item rclick_menu[] = {
		{"Load MIDI from file..."},                        	// 0
		{"Edit actions..."},                        // 1
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 2
			{"All"},                                  // 3
			{0},                                      // 4
		{"Setup MIDI output..."},                   // 5
		{"Setup MIDI input..."},                    // 6
		{"Export MIDI to file..."},                 // 7
		{"Delete channel"},                         // 8
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[1].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Delete channel") == 0) {
		if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
			return;
		deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Load MIDI from file...") == 0) {
		openBrowser(BROWSER_LOAD_MIDI);
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiOutputSetup(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, true), 0);
		return;
	}
	
	if (strcmp(m->label(), "Export MIDI to file...") == 0) {
		openBrowser(BROWSER_SAVE_MIDI);
		return;
	}

}


/* ------------------------------------------------------------------ */


void gMidiChannel::refresh()
{
	if (ch->status == STATUS_OFF) {
		sampleButton->bgColor0 = COLOR_BG_0;
		sampleButton->bdColor  = COLOR_BD_0;
		sampleButton->txtColor = COLOR_TEXT_0;
	}
	else
	if (ch->status == STATUS_PLAY) {
		sampleButton->bgColor0 = COLOR_BG_2;
		sampleButton->bdColor  = COLOR_BD_1;
		sampleButton->txtColor = COLOR_TEXT_1;
	}
	else
	if (ch->status & (STATUS_WAIT | STATUS_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->recStatus & (REC_WAITING | REC_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (recorder::active) {
		if (recorder::canRec(ch)) {
			sampleButton->bgColor0 = COLOR_BG_4;
			sampleButton->txtColor = COLOR_TEXT_0;
		}
	}

	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gMidiChannel::reset()
{
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	//sampleButton->label("-- MIDI --");
	sampleButton->label(ch->name.c_str());
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


bool gMidiChannel::uniqueMidiname(const char *name)
{
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		if (ch != G_Mixer.channels.at(i)) {
			if (G_Mixer.channels.at(i)->type == CHANNEL_MIDI ) {
				MidiChannel *other = (MidiChannel*) G_Mixer.channels.at(i);
				if (!strcmp(name, other->name.c_str()))
					return false;
			}
		}
	}
	return true;
}


/* ------------------------------------------------------------------ */


std::string gMidiChannel::setName() {

	/* increase lastMidiId until the midi name MIDI-[n] is unique */

	char wname[64] = "";
	sprintf(wname, "MIDI-%d", G_Patch.lastMidiId);
	while (!uniqueMidiname( wname )) {
		G_Patch.lastMidiId++;
		sprintf(wname, "MIDI-%d", G_Patch.lastMidiId);
	}
	
	gLog("midichannel name set to '%s'\n", wname);

	ch->name     = wname;
	ch->pathfile = gGetCurrentPath()+"/"+wname;

	return ch->name;
}

/* ------------------------------------------------------------------ */


void gMidiChannel::update()
{
		 
	if (ch->midiOut) {
		std::string tmpstr;
		char tmp[64];
		if( ch->hasActions )
			if( ch->name.length() )
				tmpstr = ch->name;
			else
				tmpstr = setName();
		else
			tmpstr = "-- midi --";
			
		sprintf(tmp, " (channel %d)", ch->midiOutChan+1);
		tmpstr.append(tmp);
		sampleButton->label(tmpstr.c_str());
	}
	else
	if( ch->hasActions ) 
		if( ch->name.length() )
			sampleButton->label(ch->name.c_str());
		else
			sampleButton->label(setName().c_str());
	else
		sampleButton->label("-- midi --");

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::resize(int X, int Y, int W, int H)
{
  sampleButton->w() < 20 ? sampleButton->hide() : sampleButton->show();
  gChannel::resize(X, Y, W, H);
}


/* ------------------------------------------------------------------ */


int gMidiChannel::keyPress(int e)
{
	return 1; // does nothing for MidiChannel
}

void gMidiChannel::openBrowser(int type)
{
	const char *title = "";
	switch (type) {
		case BROWSER_LOAD_MIDI:
			title = "Browse Midi";
			break;
		case BROWSER_SAVE_MIDI:
			title = "Save Midi";
			break;
		case -1:
			title = "Edit Midi";
			break;
	}
	gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, ch, type);
	gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
}

