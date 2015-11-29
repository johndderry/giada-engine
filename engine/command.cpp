//
// C++ Interface: command
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

#include <string.h>
#include "engine.h"

String::String() { base = NULL; length = 0; }

String::~String() { if( base != NULL ) delete[] base; }

String::String(char *s, int l) {

	base = new char[l+1];
	length = l;
	strncpy(base, s, l);
	base[length] = '\0'; 
}

String::String(const char *s, int l) {

	base = new char[l+1];
	length = l;
	strncpy(base, s, l);
	base[length] = '\0'; 
}

char *String::gets() 	{ return base; }

int String::len()		{ return length; }

int String::append(char c) {
	char *base2 = base;
	base = new char[length+2];

	if( base2 != NULL ) {
		strcpy(base, base2);
		delete[] base2;
	}

	base[length++] = c;
	base[length] = '\0';

	return length;
}

int String::append(char *s) {
	int alen = strlen(s);
	char *base2 = base;
	base = new char[length+alen+1];

	if( base2 != NULL ) {
		strcpy(base, base2);
		delete[] base2;
	}

	strcpy(base, s);
	length += alen;

	return length;
}

int String::compare(const char *s)	{ return strcmp(base, s); }
int String::compare(String *s)		{ return strcmp(base, s->base); }
bool String::match(const char *s) {

	char *bp;
	const char *sp;
	bp = base;	sp = s;
	while( *sp && *bp && *bp == *sp ) { sp++; bp++; }
	if( *bp == '\0' ) return true;
	return false;
}

String **String::split(const char *separators) {

	String **ary = NULL;
	char *ptr;
	const char *ptr2;

	/* get a count of the resulting split first */
	int count = 1;
	ptr = base;

	while( ptr < base + length ) {
		for(ptr2=separators; *ptr2; ptr2++)
			if( *ptr == *ptr2 ) {
				count++;
				break;
			}
		ptr++;
	}

	//fprintf(stderr,"split count=%d\n", count );
	ary = new String*[count+1];
	ary[count] = NULL;

	count = 0;

	/* now split up the string */
	String *current = new String();
	ptr = base;
	while( ptr < base + length ) {
		for(ptr2=separators; *ptr2; ptr2++)
			if( *ptr == *ptr2 ) {
				//count++;
				break;
			}
		if( *ptr2 == '\0' )
			current->append( *ptr++ );
		else {
			//fprintf(stderr, "assigning ary[%d]\n", count);
			ary[count++] = current;
			current = new String();
			ptr++;
		}
	}

	if( current->len() ) {
		//fprintf(stderr, "assigning ary[%d]\n", count);
		ary[count++] = current;
	}

	return ary;
}

/****************************************************************/

Unsigned::Unsigned(char *s) {

	sscanf(s, "%x", &val );

}

unsigned Unsigned::value() {

	return val;

}

/****************************************************************/


CommandHandler::CommandHandler() {

}

Channel *CommandHandler::locate( unsigned val ) {

	Channel *ch;
	for (unsigned i=0; i < G_Mixer.channels.size; i++) {
		ch = G_Mixer.channels.at(i);
		if( ( (unsigned long)ch & 0xffffffff) == val ) 
			return ch;
	}
	return NULL;
}


void CommandHandler::parseSampleArguments( Unsigned *ch_addr, String **arguments ) {

	fprintf(stderr,"sample %x:", ch_addr->value() );
}


void CommandHandler::parseMidiArguments( Unsigned *ch_addr, String **arguments ) {

	fprintf(stderr,"midi %x:", ch_addr->value() );
	MidiChannel *channel = (MidiChannel *) locate( ch_addr->value() );

	for( String **arg = arguments; *arg != NULL; arg++ ) {
		fprintf(stderr,"arg=%s ", (*arg)->gets() );
		if( (*arg)->match("start") )	channel->start(0, false );
		else if( (*arg)->match("stop") )	channel->stop( );
		else if( (*arg)->match("rewind") )	channel->rewind( );
		else if( (*arg)->match("empty") )	channel->empty( );
		else if( (*arg)->match("setmute") )	channel->setMute( false );
		else if( (*arg)->match("unsetmute") )	channel->unsetMute( false );
		else if( (*arg)->match("load") )		channel->load((*++arg)->gets());	
		else if( (*arg)->match("save") )	channel->save((*++arg)->gets());
	}

}


String *CommandHandler::parse(char *s, int l) {

	String *result = new String("OK", 2 ),
		   *comstr = new String(s,l);
	String **comary; 
	Unsigned *uns;

	Channel *ch;
	char buf[64];

	//fprintf(stderr, "parser sees: %s\n", comstr->gets() );

	comary = comstr->split(" \t");

	if( 	 comary[0]->match("quit") )		control::quit = true;
	else if( comary[0]->match("play") )		control::startSeq();
	else if( comary[0]->match("stop") )		control::stopSeq();
	else if( comary[0]->match("record") )	control::startStopActionRec();
	else if( comary[0]->match("sample") )	control::startStopInputRec();
	else if( comary[0]->match("rewind") )	control::rewindSeq();
	else if( comary[0]->match("all") )		control::playAll();
	else if( comary[0]->match("metro") )	control::startStopMetronome(false);
	else if( comary[0]->match("list") ) {
		if( comary[1]->match("sample") ) {
			delete result; result = new String();
			for (unsigned i=0; i < G_Mixer.channels.size; i++) {
				ch = G_Mixer.channels.at(i);
				if( ch->type == CHANNEL_SAMPLE ) { 
					sprintf(buf,"0x%x,",(unsigned)((unsigned long)ch & 0xffffffff) );
					result->append(buf);
				}
			}
		}
		else if( comary[1]->match("midi") ) {
			delete result; result = new String();
			for (unsigned i=0; i < G_Mixer.channels.size; i++) {
				ch = G_Mixer.channels.at(i);
				if( ch->type == CHANNEL_MIDI ) { 
					sprintf(buf,"0x%x,",(unsigned)((unsigned long)ch & 0xffffffff) );
					result->append(buf);
				}
			}
		}
	}
	else if( comary[0]->match("create") ) {
		if( comary[1]->match("sample") ) {
			ch = control::addChannel(0,CHANNEL_SAMPLE);
			sprintf(buf,"0x%x",(unsigned)((unsigned long)ch & 0xffffffff) );
			delete result; result = new String(buf,strlen(buf));
		}
		else if( comary[1]->match("midi") ) {
			ch = control::addChannel(0,CHANNEL_MIDI);
			sprintf(buf,"0x%x",(unsigned)((unsigned long)ch & 0xffffffff) );
			delete result; result = new String(buf,strlen(buf));
		}
	}
	else if( comary[0]->match("load") ) {
		if( comary[1]->match("patch") ) {
			int i = control::loadPatch((const char *)(comary[2]->gets()), (const char *)(comary[3]->gets()), false);
			if( i != PATCH_OPEN_OK ) {
				delete result; 
				result = new String( "FAILURE", 7 );
			}
		}
	}
	else if( comary[0]->match("save") ) {
		if( comary[1]->match("patch") ) {
			int i = control::savePatch((const char *)(comary[2]->gets()), (const char *)(comary[3]->gets()), false);
			if( i != PATCH_OPEN_OK ) {
				delete result; 
				result = new String( "FAILURE", 7 );
			}
		}
	}
	else if( comary[0]->match("sample") ) {
		uns = new Unsigned( comary[1]->gets() );
		parseSampleArguments( uns, &comary[2] );
	}
	else if( comary[0]->match("midi") ) {
		uns = new Unsigned( comary[1]->gets() );
		parseMidiArguments( uns, &comary[2] );
	}
	else {
		delete result; 
		result = new String( "BAD_COMMAND", 11 );
	}

	return result;
}

