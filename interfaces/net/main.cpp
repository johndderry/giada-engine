//
// C++ Implementation: main
//
// Description: 
//
//	giada-net is a derivative of Giada
//
// Copyright (C) 2015 John D. Derry
//
//
#include "../../engine/engine.h"
#include "../../netLink/src/socket.h"
#include "../../netLink/src/socket_group.h"
#include "../../netLink/src/smart_buffer.h"

#include <iostream>

class OnRead: public NL::SocketGroupCmd {

	void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

		((NL::SmartBuffer*)reference)->read(socket);
	}

};


class OnDisconnect: public NL::SocketGroupCmd {

	void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

		group->remove(socket);
	}

};


int main( int argc, char **argv ) {

#if RUNIT
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
#endif

	printf(
	"giada-net " VERSIONE " \n");

    while (int token = scanner.lex())   // get all tokens
    {
		string const &text = scanner.matched();
		switch (token)
		{
			case control::START_SEQ:
				printf("start seq\n");
			break;
			case control::STOP_SEQ:
				printf("stop seq\n");
			break;
			case control::REWIND_SEQ:
				printf("rewind seq\n");
			break;
			case control::PLAY_ALL:
				printf("play all\n");
			break;
	    }
    }

#if RUNIT
	init::shutdown();
#endif

	return 0;
}
