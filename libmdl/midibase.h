// midibase.h  *** MIDIMAKER - Basic Classes ***
// c++ classes to create a standard midi file
// (c) 2015 John Derry

#define COPYBYTES( d, s, l )		while( l-- ) *d++ = *s++;

/* 
	Chunk type enumeration
*/
enum chunktype { ct_undef, ct_header, ct_track  };

/********************************************************/
/***************** CHUNK CLASS	*************************/
/********************************************************/
/** 
  Represents a chunk of data in the midifile, either the 
  header chunk or a track chunk.
*/
class Chunk {
private:
	struct {
		unsigned char ID[4];
		unsigned char length[4];
		} header;					// build our header here
	unsigned char *start, *current;	// read/write pointers
	unsigned long clength;			// keep track of length

	// low level copy in/out
	void copybytes( unsigned char *, unsigned char *, unsigned );
	void copybytes( unsigned char *, const unsigned char *, unsigned );
	void headerlen( unsigned char *, unsigned ) ;
public:
	Chunk(enum chunktype type, unsigned len);
	~Chunk();
	unsigned load(unsigned char *p, unsigned int l);
	unsigned unload(unsigned char *p, unsigned int l);
	unsigned length();
	int length( unsigned );
	int write(FILE *s);
	int read(FILE *s);
	int write(SeqContext *c, snd_seq_addr_t *s);
	int read(SeqContext *c, snd_seq_addr_t *s);
	unsigned char *currentdata();
	enum chunktype gettype();
	bool complete();
	};

/******************************************************/
/***************** VARQUANTITY CLASS ******************/
/******************************************************/
/**
  Represent a midi variable-length quantity
*/
class VarQuantity {
	int varquantitylen;				// actual length of varquantity in bytes
	unsigned char varquantity[4];	// four bytes max required to hold it

public:
	VarQuantity(unsigned long);
	VarQuantity();
	~VarQuantity();
	unsigned char *getaddr();
	int length();
	void length(int);
	unsigned long getvalue();
	void showtime();
};

/******************************************************/
/******************* MIDIEVENT CLASS ******************/
/******************************************************/
/**
  Represent a midi event in the track
*/
class MidiEvent {
	int databyteslen;				// actual length of databytes 
	unsigned char databytes[2];		// the actual databytes
	unsigned char status;			// status 
	VarQuantity *deltime;			// delta time 

public:
	MidiEvent(unsigned long dt);
	~MidiEvent();
	void event( unsigned char , unsigned char *, int );
	unsigned char eventstatus( );
	unsigned char byte0();
	unsigned char byte1();
	int  eventdata( unsigned char * );
	void print();
	int chunkevent( Chunk * );  
  long getdtime();
	int getdtlen();
};

/******************************************************/
/**************** NON-MIDIEVENT CLASS *****************/
/******************************************************/
/**
  Used to create non-midi (sysex) event data in a track
*/
class nonMidiEvent {
	int statusbyteslen;			// actual length of statusbytes 
	int databyteslen;			// actual length of databytes 
	unsigned char *databytes;	// pointer to data bytes
	unsigned char status[4];	// at most three status bytes required
	VarQuantity *deltime;		// pointer to deltatime
	VarQuantity *textlen;		// pointer to textlen if we create
public:
	nonMidiEvent(unsigned long dt);
	~nonMidiEvent();
	void eventstat( unsigned char *, int );
	int eventstat( unsigned char * );
	void eventlen( unsigned long  );
	unsigned long eventlen( );
	void eventdata( unsigned char *, int );
	int eventdata( unsigned char * );
	void print();
	int chunkevent( Chunk * );  
	int getdtlen();
	int getlenlen();
};

/* these foward references are for here for Track */
class MidiFile;

/******************************************************/
/******************* TRACK CLASS **********************/
/******************************************************/
/**
  Used to create and maintain midi track data
*/
class Track {
	unsigned long abstime;		// keep track of absolute time
	int trackfinished;			// completion flag
	unsigned long runninglen;	// track running length
	MidiFile *mfile;			// MidiFile pointer for assoc file
	Chunk *bigchunk;			// pointer to our accumulation or load chunk
	int varQtyLen(unsigned char *);		// determine byte count in varQ
public:
	Track( MidiFile *, unsigned long );
	~Track();
	int recordMevent( unsigned long, unsigned char, unsigned char *, int ) ;
	int recordNONMevent( unsigned long, unsigned char *, int, unsigned long, unsigned char *, int ) ;
	int recordAbsMevent( unsigned long, unsigned char, unsigned char, unsigned char ) ;
	int nextEvent() ;
	nonMidiEvent *nextnonMidi();	
	MidiEvent *nextMidi();
	Chunk *nextChunk();
	void finish();
	void finish(unsigned long, SeqContext *);
};

// Add some support classes for libmdl data handling
// including SortedNotes, PlayerPart

#include <string.h>

#define NOTEON 	0x90
#define NOTEOFF	0x80
#define NoteSeqLabelLen 16

#define MAXINSTNAME 128
#define MAXPLAYNAME 128
#define MAXPLAYINST 32

/**
	Basic description of a midi note event
*/
struct Note {
	unsigned char pitch;
	unsigned char velocity;
	unsigned int duration;
	} ;
	
/**
	Used to describe an element in a linked list of midi note events
*/
struct NoteEventList {
	unsigned char status;
	unsigned char pitch;
	unsigned char velocity;
	unsigned long eventtime;
	struct NoteEventList *next;
	} ;

/******************************************************/
/******************* SORTED NOTES CLASS ***************/
/******************************************************/
/**
	Holds notes in a time-sorted sequence for conversion
	to type 1 midi format:
	all notes for all midi channels together in one sequence.
*/
class SortedNotes {
	struct NoteEventList *head, *nextstart;
public:
	SortedNotes();
	~SortedNotes();
	struct NoteEventList *getfirst();
	int tracknotes(Track *);
	int verifynotes();
	int addtolist( unsigned char status, struct Note n, int poff, unsigned long t );	
};

