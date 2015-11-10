// midibase.cc  *** MIDIMAKER - Basic Classes ***
// c++ classes to create a standard midi file
// (c) 2015 John Derry

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "seqcontext.h"
#include "midibase.h"
#include "midistream.h"

inline void Chunk::copybytes( unsigned char *d, unsigned char *s,  unsigned int len )
{
	// copy characters, no null terminator
	while( len-- ) *d++ = *s++;
}

inline void Chunk::copybytes( unsigned char *d, const unsigned char *s,  unsigned int len )
{
	// copy characters, no null terminator
	while( len-- ) *d++ = *s++;
}

void Chunk::headerlen( unsigned char *s, unsigned len ) 
{
	// build a four byte header length at address
	s += 3;
	*s-- = (char)(len & 0xff);
	len = len >> 8;
	*s-- = (char)(len & 0xff);
	len = len >> 8;
	*s-- = (char)(len & 0xff);
	len = len >> 8;
	*s   = (char)(len & 0xff);
}

//
// chunk::chunk - initialize the chunk header, get ready for loading
//
Chunk::Chunk(	enum chunktype type, 		// pass the type
				unsigned len  				// and the overall (max)chunk length 
				) 
{
	// two cases to consider: midifile is reading, chunk storage space
	// is allocated after reading the header; or midifile is writing
	// and chuck space is created here because size is known

	if( type == ct_header )			copybytes(header.ID, (const unsigned char *) "MThd", 4);
	else if ( type == ct_track )	copybytes(header.ID, (const unsigned char *) "MTrk", 4);
	// else we don't know yet, do nothing
	clength = len;	
	start = current = NULL;
	// should be 0 for reading case
	if( clength ) {
		headerlen(header.length, clength );
		start = current = new unsigned char [clength];
		if( start == NULL ) exit(3);
		}
	return;
	}

//
// chuck::~chuck - free up dynamic storage
//
Chunk::~Chunk() 
{
	delete start;
}

//
// chunk::load - load the chunk with data from somewhere, updating pointers
//
unsigned Chunk::load(	unsigned char *p, 	// "load from" pointer
						unsigned l 			// length of load data
					 	) 
{
	// make sure we don't overwrite buffer
	if( current + l > start + clength ) 	return(0);

	copybytes( current, p, l );
#if DEBUG
		int l2=l; unsigned char *p2=p;
		fprintf(stderr, "ChnkLoad(a=%x,l=%x):", current - start, l );
		while (l2--) fprintf(stderr, "%x ", 0xff & *p2++ );
		fprintf(stderr, "\n");
#endif
	current += l;	
	return(l);
	}

//
// chunk::unload - unload the chunk data to another area
//
unsigned Chunk::unload(	unsigned char *p, 	// "load to" pointer
						unsigned l 			// length of load data
					 	) 
{
	if( current + l > start + clength ) // can't overread buffer
		return(0);
	copybytes( p, current, l );
#if DEBUG
		int l2=l; unsigned char *p2=p;
		fprintf(stderr, "ChnkUnload(a=%x,l=%x):", current - start, l );
		while (l2--) fprintf(stderr, "%x ", 0xff & *p2++ );
		fprintf(stderr, "\n");
#endif
	current += l;	
	return(l);
}

unsigned Chunk::length()
{
	return clength;
}

//
// chunk::adjustlen - fix the length of the chunk, return old length
//
int Chunk::length( unsigned len )
{
	int oldlen = clength;
	clength = len;
	headerlen(header.length, clength );
	return(oldlen);
}

//
// chunk::write - flush the chunk to output
//
int Chunk::write( FILE *s )
{
#if DEBUG
	fprintf(stderr, "ChnkWrite(to file) len of %d(%x)\n", clength, clength );
#endif
	if(	fwrite( (char *) &header, 1, 8, s ) < 8 ||
		fwrite( (char *) start, 1, clength, s ) < clength ) 
		return 1;
	return(0);
}

int Chunk::write( SeqContext *c, snd_seq_addr_t *s )
{
#if DEBUG
	fprintf(stderr, "ChnkWrite(to port) len of %d(%x)\n", clength, clength );
#endif
//	if(	al_fwrite( s,  (char *) &header, 8) < 8 ||
//		al_fwrite( s,  (char *) start, clength ) < clength ) 
//		return 1;
	fprintf(stderr, "**Not Supported: ChnkWrite(to port) len of %d(%x)\n",(int)clength, (unsigned)clength );
	return(0);
}

// chunk::read - fetch the chunk from source
int Chunk::read( FILE *s ) 
{
	unsigned oldlen;
	oldlen = clength; // sometimes we know read length, use this to verify
	fread( (char *) &header, 1, 8, s );
	clength = (unsigned)(header.length[3]) + 
		((unsigned)(header.length[2])<<8) +
		((unsigned)(header.length[1])<<16) + 
		((unsigned)(header.length[0])<<24) ; 
#if DEBUG
	fprintf(stderr, "ChnkRead(from file) oldlen %x new %x\n", oldlen, clength );
#endif
	if( oldlen > 0 && oldlen != clength )
	{
		fprintf(stderr,
			"Consistency error:read chunk oldlen=%d newlen=%d, quitting\n",
			(unsigned)oldlen, (unsigned)clength );
		exit(2);
	}
	// now we can allocate the space
	if( oldlen == 0 ) 
	{
		start = current = new unsigned char [clength];
		if( start == NULL ) exit(3);
	}
	// and read it in		
	if( fread( (char *)start, 1, clength, s ) < clength )
		return  1;
	return(0);
}
	
int Chunk::read( SeqContext *c, snd_seq_addr_t *s ) 
{
	unsigned oldlen;
	oldlen = clength; // sometimes we know read length, use this to verify
//	al_fread( s, (char *) &header, 8 );
	clength = (unsigned)(header.length[3]) + 
		((unsigned)(header.length[2])<<8) +
		((unsigned)(header.length[1])<<16) + 
		((unsigned)(header.length[0])<<24) ; 
#if DEBUG
	fprintf(stderr, "ChnkRead(from port) oldlen %x new %x\n", oldlen, clength );
#endif
	if( oldlen > 0 && oldlen != clength )
	{
		fprintf(stderr,
			"Consistency error:read chunk oldlen=%d newlen=%d, quitting\n",
			(unsigned)oldlen, (unsigned)clength );
		exit(2);
	}
	// now we can allocate the space
	if( oldlen == 0 ) 
	{
		start = current = new unsigned char [clength];
		if( start == NULL ) exit(3);
	}
	// and read it in		
//	if( al_fread( s, (char *)start, clength ) < clength )
//		return  1;
	return(0);
}
	
unsigned char *Chunk::currentdata() 
{
	return current;
}	

bool Chunk::complete() {

	if( current >= start + clength ) return true;
	return false;
}

enum chunktype Chunk::gettype()
{
	if( !strncmp( (char *)header.ID, "MThd", 4) ) return( ct_header );
	if( !strncmp( (char *)header.ID, "MTrk", 4) ) return( ct_track );
	return ct_undef;
}	

/******************************************************/
/***************** VARQUANTITY CLASS ******************/
/******************************************************/
VarQuantity::VarQuantity(unsigned long val )
{
	// VarQuantity::VarQuantity - 1st form converts passed value to midi variable-len value
	unsigned long buffer = val & 0x7f;	// first 7-bit value can be had immediately

	varquantitylen = 0;
	// stuff off each 7-bit value from input into unsigned long word (buffer)
	while( (val >>= 7 ) ) 
	{	// while more 7-bitters
		buffer <<= 8;
		buffer |= ((val & 0x7f) | 0x80 );
	}
	while( 1 )
	{	// now count the length as we copy back to varquantity
		varquantity[varquantitylen] = (char) (0xff & buffer);
		varquantitylen++;
		if( buffer & 0x80)	buffer >>=8;
		else				break;
	}
}

// 2nd form converts converts variable-len value in stream to a binary value
VarQuantity::VarQuantity()
{
	varquantitylen = 0;
}

unsigned long VarQuantity::getvalue()
{
	// this resembles MostSignicantByteFirst
	unsigned long buffer = 0;	
	int x = 0;
	// extract true byte, throw away the MSBit, shift up
	while( x < varquantitylen ) 
	{	// while more bytes
		buffer <<= 7;
		buffer |= (varquantity[x] & 0x7f);
		x++;
	}
	if( varquantity[--x] & 0x80 )
	{
		fprintf(stderr, "variable-length value inconstency found: ");
		for( x=0; x < varquantitylen; x++ ) fprintf(stderr, "%x ", (unsigned) varquantity[x] );
		fprintf(stderr, "\n");
	}  
	return( buffer );
}
		  
VarQuantity::~VarQuantity() { }

// length - set the length after copy stream into buffer
void VarQuantity::length( int len )
{
	varquantitylen = len;
}

// VarQuantity::getaddr ::length ::showtime - return various stuff
unsigned char *VarQuantity::getaddr()
{
	return(  varquantity );
}

int VarQuantity::length()
{
	return(  varquantitylen );
}

void VarQuantity::showtime()
{
	unsigned char *p = varquantity;
	int i = varquantitylen;
	fprintf(stderr,"(%x)", varquantitylen );
	while( i-- ) fprintf(stderr,"%x ", 0xff & *p++);
}

// note here
// we have no dump method, instead raw copy is used with getaddr()

/******************************************************/
/******************* MIDIEVENT CLASS ******************/
/******************************************************/

// MidiEvent::MidiEvent - start a midi event with deltatime supplied
MidiEvent::MidiEvent(unsigned long dt)
{ 
	deltime = new VarQuantity( dt );	// create a delta time with the value
}

MidiEvent::~MidiEvent() 
{
	delete deltime;
}
		
// MidiEvent::event - indicate the status, and tranfer midi event data
void MidiEvent::event( unsigned char stat, unsigned char *data, int datalen )
{
	unsigned char *p = databytes;		// pointer to destination in our class
	status = stat;						// update status
	if( datalen > 2 ) exit(9);  		// sanity check 
	databyteslen = datalen;					// update length
	while( datalen-- ) *p++ = *data++;		// tranfer bytes
}

// MidiEvent::print
void MidiEvent::print()
{
#if DEBUG
	fprintf(stderr,"MidiEvent Print\n");
#endif
}

// MidiEvent::chunkevent - add (this) event to the chunk supplied
int MidiEvent::chunkevent( Chunk *ch )
{
	// give some verbage
#if DEBUG
	fprintf(stderr, "ChunkingEvent:DeltaTime=");
    deltime->showtime();
	fprintf(stderr, " Status=%x %x:%x\n", 0xff & status, 
		0xff & databytes[0], 0xff & databytes[1] );
#endif
	// now make the tranfer
	if( !ch->load( deltime->getaddr(), deltime->length() ) ||
		!ch->load( &status, 1) || !ch->load( databytes, databyteslen ) )
		return(1);
	// normal return
	return(0);
}

// MidiEvent::getdtlen - return the delta time length 
int MidiEvent::getdtlen()
{
	return( deltime->length() );
}

long MidiEvent::getdtime() { return deltime->getvalue(); }

unsigned char MidiEvent::eventstatus() 	{ return status; }
unsigned char MidiEvent::byte0()		{ return databytes[0]; }
unsigned char MidiEvent::byte1()		{ return databytes[1]; }

/******************************************************/
/**************** NON-MIDIEVENT CLASS *****************/
/******************************************************/

// nonMidiEvent::nonMidiEvent - start a non midi event with deltatime supplied
nonMidiEvent::nonMidiEvent(unsigned long dt)
{ 
	deltime = new VarQuantity( dt );	// create a delta time with the value
	databyteslen = statusbyteslen = 0;	// zero the storage lengths
	textlen = NULL;					// indicates no text length present
}

nonMidiEvent::~nonMidiEvent()
{ 
	if( textlen ) delete textlen;
	delete deltime;
}
		
// nonMidiEvent::eventstat - supply the status and status length
void nonMidiEvent::eventstat( unsigned char *stat, int statlen )
{
	unsigned char *p = status;	// destination in our class
	statusbyteslen = statlen;
	// make the transfer 
	while( statlen-- ) *p++ = *stat++;
	}

// nonMidiEvent::eventstat - return the status and status length
int nonMidiEvent::eventstat( unsigned char *stat ) 
{
	int statlen = statusbyteslen;
	unsigned char *p = status;	// source in our class
	// make the transfer 
	while( statlen-- ) *stat++ = *p++;
	return( statusbyteslen );
	}

// nonMidiEvent::eventdata - supply the event data and data length
void nonMidiEvent::eventdata( unsigned char *data, int datalen ) 
{
	unsigned char *p;		// our local copy
	databytes = p = new unsigned char [datalen];	// get dynamic storage for this
	databyteslen = datalen;
	// make the transfer
	while( datalen-- ) *p++ = *data++;
}

// nonMidiEvent::eventdata - return the event data and data length
int nonMidiEvent::eventdata( unsigned char *data ) 
{
	int datalen = databyteslen;
	unsigned char *p = databytes;		// from our local copy
	// make the transfer
	while( datalen-- ) *data++ = *p++;
	return( databyteslen );
}

// nonMidiEvent::eventlen - supply the event text length
void nonMidiEvent::eventlen( unsigned long v ) 
{
	textlen = new VarQuantity( v );	// create a variable length value
}

// nonMidiEvent::eventlen - return the event text length
unsigned long nonMidiEvent::eventlen( )
{
	if( textlen ) return( textlen->getvalue() );
	else return 0;
}

// nonMidiEvent::print 
void nonMidiEvent::print() 
{
	fprintf(stderr,"nonMidiEvent Print\n");
}

// nonMidiEvent::chunkevent - add (this) event to the chunk supplied
int nonMidiEvent::chunkevent( Chunk *ch )
{
	if( !ch->load( deltime->getaddr(), deltime->length() ) ||
		!ch->load( status, statusbyteslen ) ) return(1);
	if( textlen && !ch->load( textlen->getaddr(), textlen->length() ))
		return(1);
	if( databyteslen && !ch->load( databytes, databyteslen )) return(1);
	return(0);
} 
		
// nonMidiEvent::getdtlen - return the delta time length 
int nonMidiEvent::getdtlen() 
{
	return( deltime->length() );
}

// nonMidiEvent::getlenlen - return the text length 
int nonMidiEvent::getlenlen() 
{
	if( textlen ) return( textlen->length() );
	else return( 0 );
}

/******************************************************/
/******************* TRACK CLASS **********************/
/******************************************************/

// Track::Track - initialize the track which can be used
//   for creating or storing track data
// Supply midi file or stream to attach to and
//   initial length if known or possibly
//   after loading track from stream

Track::Track( MidiFile *mf, unsigned long startlen ) 
{
	// could be assembling track or reading from file, 
	// both are supported together
	runninglen = 0;
	trackfinished = 0;
	abstime = 0L;
	mfile = mf;
	if( mf->access() ) 
	{  // for writing
		mf->writeheader(); 
	}
	bigchunk = new Chunk(ct_track, startlen );  // OK to pass startlen=0
}	

// Track::~Track - finish up and flush the track if necessary, 
//   return chunk memory storage
Track::~Track() 
{
	if( runninglen && !trackfinished ) finish();
	if (bigchunk) delete bigchunk;
}

//
// Track::recordMevent - record a midi event in the track
//
int Track::recordMevent( unsigned long dtime, unsigned char status, 
		unsigned char *data, int datalen )
{
	int ret;
	MidiEvent mtemp( dtime );
	mtemp.event( status, data, datalen );
	mtemp.print();
	if( (ret = mtemp.chunkevent( bigchunk ) )) return(ret);
	runninglen += mtemp.getdtlen() + 1 + datalen;
	return(0);
}		

//
// Track::recordMevent - record a midi event in the track
//
int Track::recordAbsMevent( unsigned long time, unsigned char status, 
		unsigned char data1, unsigned char data2 )
{
	int ret;
	unsigned char db[2];
	db[0] = data1;
	db[1] = data2;
	MidiEvent mtemp( time - abstime );
	abstime = time;
	mtemp.event( status, db, 2 );
	mtemp.print();
	if( (ret = mtemp.chunkevent( bigchunk ) )) return(ret);
	runninglen += mtemp.getdtlen() + 3;
	return(0);
}		

//
// Track::recordNONMevent - record a non midi event in the track
//
int Track::recordNONMevent( unsigned long dtime, unsigned char *status, int statlen, 
		unsigned long eventlen, unsigned char *data, int datalen )
{
	int ret;	
	nonMidiEvent mtemp( dtime );
	mtemp.eventstat( status, statlen );
	if( eventlen ) mtemp.eventlen( eventlen );
	if( datalen ) mtemp.eventdata( data, datalen );
	//mtemp.print();
	if( (ret = mtemp.chunkevent( bigchunk ) )) return(ret);
	runninglen += mtemp.getdtlen() + statlen + datalen 
			+ mtemp.getlenlen();
	return(0);
}		

void Track::finish(long unsigned end, SeqContext *seqcontext)
{
	snd_seq_event_t *ep;

	seqcontext->seq_midi_echo( end );
	/* else alsa stream close */

	snd_seq_drain_output( seqcontext->seq_handle() );

	snd_seq_event_input( seqcontext->seq_handle(), &ep );

	sleep(1);
	seqcontext->seq_stop_queue();
}

// Track::finish - finish up the track, flush it out
void Track::finish()
{

	if( mfile )
	{
		// we create a end-of-track marker
		unsigned char eotstatus[3] = { 0xff, 0x2F, 0x00 };
#if DEBUG
		fprintf(stderr, "Track:Creating EndOfTrack marker\n");
#endif
		// end-of-track is a non-midi event
		nonMidiEvent eot( 0 );
		eot.eventstat( eotstatus, 3 );
		eot.chunkevent( bigchunk );
	// adjust our track running length
		runninglen += eot.getdtlen() + 3;
		bigchunk->length( runninglen );
	// now we can flush the chunk out
#if DEBUG
		fprintf(stderr, "Track writing out chunk of len %d(%x)\n", 
			runninglen, runninglen );
#endif
		bigchunk->write( mfile->getostream() );
		trackfinished = 1;	// we are done
	}
	/* else alsa stream close */
}

int Track::varQtyLen( unsigned char *p )
{
	int n = 1;
	while( *p & 0x80 ) p++,n++;
	return n;
}

// nextEvent() determine next event in the chunk
int Track::nextEvent() 
{
	unsigned char status, *current;

	if( bigchunk->currentdata() == NULL ) 
		if( bigchunk->read(mfile->getistream()) ) return -1;
	if( bigchunk->complete() ) {
		fprintf(stderr, "nextEvent: chunk complete\n" );
		return -1;
	}

	current = bigchunk->currentdata();

#if DEBUG
	fprintf(stderr, "nextEvent: current=%x ", (unsigned)current );
#endif
	int v = varQtyLen(current);
	status = *( current + varQtyLen(current) );
	status = 0xff & status;
	fprintf(stderr, "varQLen=%d status=%x\n", v, status );
#if DEBUG
	fprintf(stderr, "nextEvent looking at status %x\n", status );
#endif
	if( status >= 0xf0 ) return 1; // non midi event
	return 0;  // else it's midi
}

nonMidiEvent *Track::nextnonMidi() 
{
	int statuslen, datalen, varQL;
	unsigned status;
	unsigned char statusbytes[4], *bigbuffer;
	// create event, transfer from chunk in 
	nonMidiEvent *evptr;
	VarQuantity VQ;
	// the first thing is delta time represented as a midi variable quatity
	// determine length of varquantity, then transfer
	varQL = varQtyLen( bigchunk->currentdata() );
	bigchunk->unload( VQ.getaddr(), varQL );
	VQ.length( varQL );
	evptr = new nonMidiEvent( VQ.getvalue() );
	// can have up to three status bytes here, must look at the first
	status = (unsigned) *(bigchunk->currentdata()) ;
	status = 0xff & status;
	// now we case on this
	switch( status )
	{
	case 0xf0:  // sysex message
		statuslen = 2; break;
	case 0xff:  // this form has three status bytes	
		statuslen = 3; break;
	default:
		fprintf(stderr,"unknown status encountered: %x\n", status );
		exit(3); break;
	}		
	bigchunk->unload( statusbytes, statuslen );
	evptr->eventstat( statusbytes, statuslen );
	datalen = 0;
	if( status == 0xf0 )
	{
		// sysex message length found at second byte
		datalen = (int)statusbytes[1];
		if( datalen )
		{
			// alloocate buffer and transfer
			bigbuffer = new unsigned char[datalen];	
			bigchunk->unload( bigbuffer, datalen );
			evptr->eventdata( bigbuffer, datalen );
			delete bigbuffer;
		}
	}	
	if( status == 0xff ) {
		// other message length found at third byte
		datalen = (int)statusbytes[2];
		if(	datalen )
		{
			// alloocate buffer and transfer
			bigbuffer = new unsigned char[datalen];	
			bigchunk->unload( bigbuffer, datalen );
			evptr->eventdata( bigbuffer, datalen );
			delete bigbuffer;
		}
	}
	evptr->eventlen( statuslen + datalen );			
	// completed non-midi event
	return(evptr);
}	

MidiEvent *Track::nextMidi()
{
	int varQL;
	unsigned char status, databytes[4];
	// create event, transfer from chunk in 
	MidiEvent *evptr;
	VarQuantity VQ;
	// the first thing is delta time represented as a midi variable quatity
	// determine length of varquantity, then transfer
	varQL = varQtyLen( bigchunk->currentdata() );
	bigchunk->unload( VQ.getaddr(), varQL );
	VQ.length( varQL );
	evptr = new MidiEvent( VQ.getvalue() );
	status = 0;
	if( (unsigned)*( bigchunk->currentdata() ) >= 0x80 ) 
		bigchunk->unload( &status, 1 );
	// assumming here always two data bytes!
	bigchunk->unload( databytes, 2 );
	// fill out event
	evptr->event( status, databytes, 2 ); 
	// completed midi event
	//fprintf(stderr, "MidiEvent: status=%x b1=%x b2=%x\n", status,
	//	databytes[0], databytes[1] );
	return(evptr);
}	

Chunk *Track::nextChunk() {

	if( bigchunk->length() )
	{
		delete bigchunk;
		bigchunk = new Chunk(ct_track, 0 );  
	}
	if( mfile ) 
	{
		//if( mfile->getistream()->eof() ) return NULL;
		bigchunk->read( mfile->getistream() );
	}
	return bigchunk;
}

/******************************************************/
/******************* SORTED NOTES CLASS ***************/
/******************************************************/

SortedNotes::SortedNotes() 
{
	head = nextstart = (struct NoteEventList *) 0;
}
	
SortedNotes::~SortedNotes() 
{
	struct NoteEventList *nexthead;
	while( head ) {
		nexthead = head->next;
		delete head;
		head = nexthead;
		}
}

int SortedNotes::tracknotes( Track *t )
{	
	struct NoteEventList *next = head;
	// track all the notes in the sequence, begin from T=0
	unsigned char buff[2];
	unsigned long deltatime, runningtime = 0;
	int totalN = 0;
	while( next ) 
	{
		if( next->eventtime < runningtime ) 
		{
			fprintf(stderr, "tracknotes problem: eventtime less than running\n");
			return 0;
		}
		deltatime = next->eventtime - runningtime;
		buff[0] = next->pitch;
		buff[1] = next->velocity;
		t->recordMevent(deltatime, next->status, buff, 2 );
#if DEBUG
		fprintf(stderr, "MidiEvent: dT %x status %x Note %d Veloc %d\n",
			(unsigned)deltatime, next->status, next->pitch, next->velocity );
#endif
		runningtime = next->eventtime;
		if( next->status >> 4 == NOTEOFF ) totalN++;
		next = next->next;
	}
	return totalN;
}

int SortedNotes::verifynotes( )
{	
	struct NoteEventList *next = head;
	// verify the sequence for time order
	unsigned long runningtime = 0;
	while( next )
	{
		if( next->eventtime < runningtime ) 
		{
			fprintf(stderr, "verifynotes: eventtime less than running\n");
			return 0;
		}
		runningtime = next->eventtime;
		next = next->next;
	}
	return 1;
}

struct NoteEventList *SortedNotes::getfirst()
{
	return( head );
}

struct NoteEventList *srchnext(struct NoteEventList *test, 
			unsigned long time ) 
{ 
	struct NoteEventList *lasttest = test;

	if( test->eventtime > time )
		// the test slot time is already past us
		 return(NULL);

	if( test->eventtime == time )
		// the test slot time is the same as us
		 return(test);

	// go until we are before
	while( test && test->eventtime < time )
	{
			lasttest = test;
			test = test->next;
		}
	if( !test || test->eventtime > time ) // too far
		return( lasttest );
	// else right here
	return( test );
}

int SortedNotes::addtolist( unsigned char status, struct Note n, int poff,
		 unsigned long time )
{
	struct NoteEventList *p, *temp, *appendpoint;

	if( !this->verifynotes() ) return -1;
	// create our data item
	if( !(p = new struct NoteEventList)) return 1;
	p->status = status;
	p->pitch = n.pitch + poff;
	p->velocity = n.velocity;
	p->eventtime = time;
	p->next = (struct NoteEventList *)0;
		
	// make this the head of the list if no head
	if( !head ) 
	{
		head = nextstart = p;
		appendpoint = 0;
	}
	// look for insert point from nextstart
	else if( (appendpoint = srchnext( nextstart, time )) )
	{
		// use it
		temp = appendpoint->next;
		appendpoint->next = p;
		p->next = temp;
	}
	// look for insert point from head
	else if( (appendpoint = srchnext( head, time )) )
	{
		// use it
		temp = appendpoint->next;
		appendpoint->next = p;
		p->next = temp;
	}
	else 
	{
		fprintf(stderr, "*opps- can't find ordered place for note *");
		return -1;
	}
#if DEBUG
	fprintf(stderr, "note(s=%x p=%d v=%x) pof=%d tm=%x ADD at %x\n",
		0xff&status, n.pitch, n.velocity, poff,
		(unsigned)time, (long unsigned)appendpoint) ;
#endif
	return( 0 );
}


