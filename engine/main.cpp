//
// C++ Implementation: main
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


#include "engine.h"


int main( int argc, char **argv ) {

	init::prepareParser();
	init::prepareKernelAudio();
	init::prepareKernelMIDI();

	if( --argc ) {
		gLog("[main] loading patch '%s'\n", *++argv );
		control::loadPatch( *argv, "~/", false );
	} else
	{
		gLog("[main] loading default patch 'my_patch'\n" );
		control::loadPatch( "my_patch.gptc", "~/", false );
	}

	init::startKernelAudio();

	printf(
	"gdEngine " VERSIONE " \n");

	char chr; bool run = true;

	while( run && (chr = getchar()) > 0 ) switch( chr ) {
		case 10: case 13: break;
		case 'q': run = false; break;
		case ' ': control::startStopSeq(); break;
		case 'p': control::startSeq(); break;
		case 's': control::stopSeq(); break;
		case 'r': control::rewindSeq(); break;
		case 'a': control::playAll(); break;
		case 'h': printf("q\tquit\np\tplay\ns\tstop\nspace\ttoggle\nr\trewind\na\tplay all\n"); break;
		default : printf("'%c' not recognized, try 'h'\n", chr );
	}

	init::shutdown();

	return 0;
}
