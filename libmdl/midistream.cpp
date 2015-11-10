// midistream.cc  *** MIDIMAKER - Basic Classes ***
// c++ classes for midi file and stream support
// (c) 2015 John Derry 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <alsa/asoundlib.h>
#include "seqcontext.h"
#include "midibase.h"
#include "midistream.h"

/******************************************************/
/****************** MIDIFILE CLASS ********************/
/******************************************************/

// midifile::midifile - create, initialize the file
MidiFile::MidiFile( const char *filename, const char *access ) 
{
	if( strchr( access, 'r' ) )
	{
		accessmode = 0;
		midiin = fopen( filename, "r" );
		if( !midiin )
		{
			fprintf(stderr, "error opening midifile %s for input\n", filename );
			exit(1);
		}
	}	
	else if( strchr( access, 'w' ) ) 
	{
		accessmode = 1;
		midiout = fopen( filename, "w");
		if( !midiout  ) 
		{
			fprintf(stderr, "error opening midifile %s for output\n", filename );
			exit(2);
		}
	}
	else 
	{
		fprintf(stderr, "invalid accessmode %s\n", access );
		exit(2);
	}
				
	if( accessmode )
	{	
		// initialize the actual header storage with defaults
		hdrbytes[0] = hdrbytes[2] = hdrbytes[4] = 0; // upper byte zeroed
		// presently, only do midi type 1
		hdrbytes[1] = hdrbytes[3] = 1; 	// type 1, 1 track */
		//hdrbytes[5] = 0x78;  /* = 120 ppqn */
		hdrbytes[5] = 0x60;    /* = 96 ppqn */
	}
	else 
	{
		readheader();
#if DEBUG
		fprintf(stderr,"Header is read: type=%x tracks=%x timing=%x\n",
			type(), tracks(), timing() );
#endif			
	}	
}
	
// these allow modification of the defaults
void MidiFile::type( unsigned char v ) 
{
	hdrbytes[1] = v;
}
	
void MidiFile::tracks( unsigned char v )
{
	hdrbytes[3] = v;
}
	
void MidiFile::timing( unsigned char v ) 
{
	hdrbytes[5] = v;
}
	
// these return the current header values
unsigned char MidiFile::type( )
{
	return hdrbytes[1];
}
	
unsigned char MidiFile::tracks( ) {
	return hdrbytes[3];
	}
	
unsigned char MidiFile::timing( )
{
	return hdrbytes[5];
}
	
int MidiFile::access( ) {
	return accessmode;
	}
	
// midifile::writeheader - chunk and flush the header 
void MidiFile::writeheader() 
{
	Chunk header(ct_header,6);
	header.load(hdrbytes, 6);
	header.write(midiout);
}

// midifile::readheader - get the header chunk from file 
void MidiFile::readheader() 
{
	Chunk header(ct_header,6);
	header.read(midiin);
	header.unload(hdrbytes, 6);
}

// midifile::~midifile - close the file stream
MidiFile::~MidiFile() 
{
	if( accessmode )
	{
		fclose( midiout );
	}
	else
	{
		fclose( midiin );
	}
}

FILE *MidiFile::getostream() 
{
	// return a stream reference to use with chunk::write()
	return( midiout );
}

FILE *MidiFile::getistream() 
{
	// return a stream reference to use with chunk::read()
	return( midiin );
}
