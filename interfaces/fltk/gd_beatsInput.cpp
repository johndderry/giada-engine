/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_beatsInput
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

gdBeatsInput::gdBeatsInput()
	: gWindow(164, 60, "Beats")
{
	if (G_Conf.beatsX)
		resize(G_Conf.beatsX, G_Conf.beatsY, w(), h());

	set_modal();

	beats   = new gInput(8,  8,  35, 20);
	bars    = new gInput(47, 8,  35, 20);
	ok 		  = new gClick(86, 8,  70, 20, "Ok");
	exp 	  = new gCheck(8,  40, 12, 12, "resize recordings");
	end();

	char buf_bars[3]; sprintf(buf_bars, "%d", G_Mixer.bars);
	char buf_beats[3]; sprintf(buf_beats, "%d", G_Mixer.beats);
	beats->maximum_size(2);
	beats->value(buf_beats);
	beats->type(FL_INT_INPUT);
	bars->maximum_size(2);
	bars->value(buf_bars);
	bars->type(FL_INT_INPUT);
	ok->shortcut(FL_Enter);
	ok->callback(cb_update_batt, (void*)this);
	exp->value(1); // checked by default

	gu_setFavicon(this);
	setId(WID_BEATS);
	show();
}


/* ------------------------------------------------------------------ */


gdBeatsInput::~gdBeatsInput() {
	G_Conf.beatsX = x();
	G_Conf.beatsY = y();
}


/* ------------------------------------------------------------------ */


void gdBeatsInput::cb_update_batt(Fl_Widget *w, void *p) { ((gdBeatsInput*)p)->__cb_update_batt(); }


/* ------------------------------------------------------------------ */


void gdBeatsInput::__cb_update_batt() {
	if (!strcmp(beats->value(), "") || !strcmp(bars->value(), ""))
		return;
	control::setBeats(atoi(beats->value()), atoi(bars->value()), exp->value());
	do_callback();
}
