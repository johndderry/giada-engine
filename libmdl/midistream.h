// midistream.h  *** MIDIMAKER - Basic Classes ***
// c++ classes for midi file and stream support
// RATSTER version began June2011

/********************************************************/
/****************** MIDIFILE CLASS ********************/
/******************************************************/
/**
 Provide a connection to file-based midi data
*/
class MidiFile {
	unsigned char hdrbytes[6];	/* build/decode our dynamic header here */
	FILE *midiout, *midiin;
	int accessmode;
public:
	MidiFile(const char *, const char *);
	~MidiFile();
	void type( unsigned char );
	void tracks( unsigned char );
	void timing( unsigned char );
	unsigned char type( );
	unsigned char tracks( );
	unsigned char timing( );
	int access();
	void writeheader( );
	void readheader( );
	FILE  *getostream();
	FILE  *getistream();
};
