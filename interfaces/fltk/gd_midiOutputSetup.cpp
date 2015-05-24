/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiOutputSetup
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


gdMidiOutputSetup::gdMidiOutputSetup(MidiChannel *ch)
	: gWindow(310, 80, "Midi Output Setup"), ch(ch)
{
	begin();
	enableOut      = new gCheck(x()+8, y()+8, 150, 20, "Enable MIDI output");
	chanListOut    = new gChoice(w()-108, y()+8, 100, 20);

	enableProgChg  = new gCheck(x()+8, h()+2-50, 20, 20);
	enableBankChg  = new gCheck(x()+8, h()+2-25, 20, 20);
	progChange     = new gValue(x()+100, h()-50, 20, 20, "Prog Change");
	bankChange     = new gValue(x()+100, h()-25, 20, 20, "Bank Change");

	save   = new gButton(w()-88, bankChange->y(), 80, 20, "Save");
	cancel = new gButton(w()-88-save->w()-8, save->y(), 80, 20, "Cancel");
	end();

	fillChanMenu(chanListOut);

	if (ch->midiOut) {
		enableOut->value(1);
		if( ch->midiOutProg ) enableProgChg->value(1);
		if( ch->midiOutBank ) enableBankChg->value(1);
	} else {
		chanListOut->deactivate();
		progChange->deactivate();
		bankChange->deactivate();
	}

	chanListOut->value(ch->midiOutChan);
	progChange->value(ch->midiProgChg);
	bankChange->value(ch->midiBankChg);

	enableOut->callback(cb_enableChanList, (void*)this);
	enableProgChg->callback(cb_enableProgChg, (void*)this);
	enableBankChg->callback(cb_enableBankChg, (void*)this);

	save->callback(cb_save, (void*)this);
	cancel->callback(cb_cancel, (void*)this);

	set_modal();
	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::cb_save          (Fl_Widget *w, void *p) { ((gdMidiOutputSetup*)p)->__cb_save(); }
void gdMidiOutputSetup::cb_cancel        (Fl_Widget *w, void *p) { ((gdMidiOutputSetup*)p)->__cb_cancel(); }
void gdMidiOutputSetup::cb_enableChanList(Fl_Widget *w, void *p) { ((gdMidiOutputSetup*)p)->__cb_enableChanList(); }
void gdMidiOutputSetup::cb_enableProgChg(Fl_Widget *w, void *p) { ((gdMidiOutputSetup*)p)->__cb_enableProgChg(); }
void gdMidiOutputSetup::cb_enableBankChg(Fl_Widget *w, void *p) { ((gdMidiOutputSetup*)p)->__cb_enableBankChg(); }


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::__cb_enableChanList() {
	enableOut->value() ? chanListOut->activate() : chanListOut->deactivate();
}


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::__cb_enableProgChg() {
	enableProgChg->value() ? progChange->activate() : progChange->deactivate();
}


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::__cb_enableBankChg() {
	enableBankChg->value() ? bankChange->activate() : bankChange->deactivate();
}


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::__cb_save() {
	ch->midiOut     = enableOut->value();
	ch->midiOutChan = chanListOut->value();
	ch->midiOutProg = enableProgChg->value();
	ch->midiOutBank = enableBankChg->value();
	ch->midiProgChg = progChange->value();
	ch->midiBankChg = bankChange->value();
	((gChannel*)ch->guiChannel)->update();
	do_callback();
}


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::__cb_cancel() { do_callback(); }


/* ------------------------------------------------------------------ */


void gdMidiOutputSetup::fillChanMenu(gChoice *m) {
	m->add("Channel 1");
	m->add("Channel 2");
	m->add("Channel 3");
	m->add("Channel 4");
	m->add("Channel 5");
	m->add("Channel 6");
	m->add("Channel 7");
	m->add("Channel 8");
	m->add("Channel 9");
	m->add("Channel 10");
	m->add("Channel 11");
	m->add("Channel 12");
	m->add("Channel 13");
	m->add("Channel 14");
	m->add("Channel 15");
	m->add("Channel 16");
	m->value(0);
}
