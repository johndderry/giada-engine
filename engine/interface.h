//
// C++ Interface: interface
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

/*
 * register the callbacks for an internal interface. This is how
 * we have separated the interface from the engine code.
 */

class Interface {

public:
	void *controller, *keyboard, *inout, *timing, *launcher;

	void (*updatePlay)(int, void *);

	void (*updateRecAction)(int, void *);

	void (*updateRecInput)(int, void *);

	void (*updateMetronome)(int, void *);

	void (*setChannelWithActions)(void *, void *);

	void (*setChanVol)(void *, float, void *);

	void (*setChanMute)(void *, bool, void *);

	void (*setChanSolo)(void *, bool, void *);

	void (*setInVol)(float, void *);

	void (*setOutVol)(float, void *);

	void (*clear)(void *);

	void (*reset)(void *, void *);

	void (*alert)(const char *, void *);

	void (*labelWave)(void *, const char *, void *);

	void *(*addChannel)(int, void *, void *);

	void (*setBeatMeter)(int, int, void *);

	void (*setBreakMeter)(int, void *);

};

extern Interface *G_Interface;
