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

	CommandHandler *comms = new CommandHandler();
	char line[128];
	String *response;

	while( !control::quit ) {
		fgets( line, 128, stdin );
		response = comms->parse( line, strlen(line)-1 );
		fputs( response->gets(), stdout ); fputc( '\n', stdout );
	}

	init::shutdown();

	return 0;
}
