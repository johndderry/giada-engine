//
// C++ Implementation: midichannel
//
// Description: 
//
//	GiadaEngine is a derivative of Giada
//
// Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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


MidiChannel::MidiChannel(int bufferSize)
	: Channel    (CHANNEL_MIDI, STATUS_OFF, bufferSize),
	  midiOut    (true),
	  midiOutChan(0),
	  midiOutProg      (false),
	  midiOutBank      (false),
	  midiProgChg      (0),
	  midiBankChg      (0)
      //midifile(0)
{

	if( G_Conf.incrMidiChan ) {
		midiOut = true;
		midiOutChan = G_Mixer.nextMidiChan++;
		if( G_Mixer.nextMidiChan > 15 ) G_Mixer.nextMidiChan = 0;
	}

#ifdef WITH_VST // init VstEvents stack
	freeVstMidiEvents(true);
#endif
}


/* ------------------------------------------------------------------ */


MidiChannel::~MidiChannel() {}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::freeVstMidiEvents(bool init) {
	if (events.numEvents == 0 && !init)
		return;
	memset(events.events, 0, sizeof(VstEvent*) * MAX_VST_EVENTS);
	events.numEvents = 0;
	events.reserved  = 0;
}

#endif


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(uint32_t msg) {
	addVstMidiEvent(G_PluginHost.createVstMidiEvent(msg));
}

#endif


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(VstMidiEvent *e) {
	if (events.numEvents < MAX_VST_EVENTS) {	
		events.events[events.numEvents] = (VstEvent*) e;
		events.numEvents++;
		/*
		gLog("[MidiChannel] VstMidiEvent added - numEvents=%d offset=%d note=%d number=%d velo=%d\n", 
			events.numEvents, 
			e->deltaFrames,
			e->midiData[0],
			e->midiData[1],
			e->midiData[2]
		);*/
	}
	else
		gLog("[MidiChannel] channel %d VstEvents = %d > MAX_VST_EVENTS, nothing to do\n", index, events.numEvents);
}

#endif


/* ------------------------------------------------------------------ */


void MidiChannel::onBar(int frame) {}


/* ------------------------------------------------------------------ */


void MidiChannel::stop() {

	switch (status) {
		case STATUS_PLAY:
			status = STATUS_ENDING;
			break;
		case STATUS_ENDING:
			status = STATUS_OFF;
			break;
		case STATUS_WAIT:
			status = STATUS_OFF;
			break;
		case STATUS_OFF:
			status = STATUS_WAIT;
			break;
	}
}



/* ------------------------------------------------------------------ */


void MidiChannel::empty() {}


/* ------------------------------------------------------------------ */


void MidiChannel::quantize(int index, int localFrame, int globalFrame) {}


/* ------------------------------------------------------------------ */

#ifdef WITH_VST

VstEvents *MidiChannel::getVstEvents() {
	return (VstEvents *) &events;
}

#endif


/* ------------------------------------------------------------------ */


//void MidiChannel::parseAction(recorder::action *a, int localFrame, int globalFrame) {
//	if (a->type == ACTION_MIDI)
//		sendMidi(a, localFrame/2);
//}

void MidiChannel::parseAction(void *a, int localFrame, int globalFrame) {
	if ( ((recorder::action *) a)->type == ACTION_MIDI)
		sendMidi( (recorder::action *) a, localFrame/2);
}


/* ------------------------------------------------------------------ */


void MidiChannel::onZero(int frame) {
	if (status == STATUS_ENDING) {
    for(int i=0;i<128;i++)
      if( openNotes[i] ) {
        gLog("[onZero] Note silenced: %d\n",i);
        kernelMidi::send(0x80004000 | MIDI_CHANS[midiOutChan] | i<<16 );
      }
		status = STATUS_OFF;
    }
	else
	if (status == STATUS_WAIT)
		status = STATUS_PLAY;
}


/* ------------------------------------------------------------------ */


void MidiChannel::setMute(bool internal) {
	mute = true;  	// internal mute does not exist for midi (for now)
	if (midiOut)
		kernelMidi::send((uint32_t) MIDI_CHANS[midiOutChan] | MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent((uint32_t) MIDI_CHANS[midiOutChan] | MIDI_ALL_NOTES_OFF);
#endif
}


/* ------------------------------------------------------------------ */


void MidiChannel::unsetMute(bool internal) {
	mute = false;  	// internal mute does not exist for midi (for now)
}


/* ------------------------------------------------------------------ */


void MidiChannel::process(float *buffer) {

//
//  midi channels do nothing here
//
#if 0

#ifdef WITH_VST
	G_PluginHost.processStack(vChan, PluginHost::CHANNEL, this);
	freeVstMidiEvents();
#endif

	for (int j=0; j<bufferSize; j+=2) {
		buffer[j]   += vChan[j]   * volume; // * panLeft;   future?
		buffer[j+1] += vChan[j+1] * volume; // * panRight;  future?
	}
}
#endif

}

/* ------------------------------------------------------------------ */

void MidiChannel::start(int frame, bool doQuantize) {
	
	// send these anytime we hit the play button and midiOut is enabled
	if( midiOut ) {
		if( midiOutBank )  
			kernelMidi::send((uint32_t) (MIDI_CHANS[midiOutChan] | MIDI_CONTROLLER | midiBankChg) );
		if( midiOutProg ) 
			kernelMidi::send( (MIDI_CHANS[midiOutChan] | MIDI_PROGRAM) >>24, midiProgChg, -1  );
	}
	
	for(int i=0;i<128;i++) openNotes[i] = false;

	switch (status) {
		case STATUS_PLAY:
			status = STATUS_ENDING;
			break;
		case STATUS_ENDING:
		case STATUS_WAIT:
			status = STATUS_OFF;
			break;
		case STATUS_OFF:
			status = STATUS_WAIT;
			break;
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::stopBySeq() {
	kill(0);
}


/* ------------------------------------------------------------------ */


void MidiChannel::kill(int frame) {
	if (status & (STATUS_PLAY | STATUS_ENDING)) {
		if (midiOut)
			kernelMidi::send((uint32_t) MIDI_CHANS[midiOutChan] | MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent((uint32_t) MIDI_CHANS[midiOutChan] | MIDI_ALL_NOTES_OFF);
#endif
	}
	status = STATUS_OFF;
}


/* ------------------------------------------------------------------ */


int MidiChannel::loadByPatch(const char *f, int i) {
	volume      = G_Patch.getVol(i);
	index       = G_Patch.getIndex(i);
	mute        = G_Patch.getMute(i);
	mute_s      = G_Patch.getMute_s(i);
	solo        = G_Patch.getSolo(i);
	panLeft     = G_Patch.getPanLeft(i);
	panRight    = G_Patch.getPanRight(i);

	midiOut     = G_Patch.getMidiValue(i, "Out");
	midiOutChan = G_Patch.getMidiValue(i, "OutChan");
	midiOutProg = G_Patch.getMidiValue(i, "OutProg");
	midiOutBank = G_Patch.getMidiValue(i, "OutBank");
	midiProgChg = G_Patch.getMidiValue(i, "ProgChg");
	midiBankChg = G_Patch.getMidiValue(i, "BankChg");

	readPatchMidiIn(i);

	return SAMPLE_LOADED_OK;  /// TODO - change name, it's meaningless here
}


/* ------------------------------------------------------------------ */

void MidiChannel::regOpenNote( uint32_t data ) {
		// Register Open Note - keep track of all notes with key down state
    //gLog("[regOpenNote] data = %x\n", data );
    if( (data & 0xf0000000 ) == 0x90000000 ) openNotes[ (data & 0xff0000)>>16 ] = true;
		else
      if( (data & 0xf0000000 ) == 0x80000000 ) openNotes[ (data & 0xff0000)>>16 ] = false;
   
}

void MidiChannel::sendMidi(void *a, int localFrame) 
{
	recorder::action *act = (recorder::action *) a;

	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut) {
      regOpenNote( act->iValue );
			kernelMidi::send(act->iValue | MIDI_CHANS[midiOutChan]);
    }
#ifdef WITH_VST
		act->event->deltaFrames = localFrame;
		addVstMidiEvent(act->event);
#endif
	}
}


void MidiChannel::sendMidi(uint32_t data) 
{
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut) {
      regOpenNote( data );
			kernelMidi::send(data | MIDI_CHANS[midiOutChan]);
    }
#ifdef WITH_VST
		addVstMidiEvent(data);
#endif
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::recvMidi(uint32_t data) 
{
	int c = data & 0x0f000000;
	// ignore any channel info
	data = data & 0xf0ffffff;
	if( c == 9 )		
		// hack for Alesis drum pads
		recorder::rec(this->index, ACTION_MIDI, G_Mixer.actualFrame+1, data);
	else 
		recorder::rec(this->index, ACTION_MIDI, G_Mixer.actualFrame, data);
	
}


/* ------------------------------------------------------------------ */

void MidiChannel::rewind() {
	if (midiOut)
		kernelMidi::send((uint32_t) MIDI_CHANS[midiOutChan] | MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent((uint32_t) MIDI_CHANS[midiOutChan] | MIDI_ALL_NOTES_OFF);
#endif
}


/* ------------------------------------------------------------------ */


void MidiChannel::writePatch(FILE *fp, int i, bool isProject)
{
	Channel::writePatch(fp, i, isProject);

	const char *path;
	path = pathfile.c_str();
	if (isProject)
		path = gBasename(path).c_str();  // make it portable
		
	fprintf(fp, "samplepath%d=%s\n",         i, path);
	fprintf(fp, "chanMidiOut%d=%u\n",        i, midiOut);
	fprintf(fp, "chanMidiOutChan%d=%u\n",    i, midiOutChan);
	fprintf(fp, "chanMidiOutProg%d=%u\n",    i, midiOutProg);
	fprintf(fp, "chanMidiOutBank%d=%u\n",    i, midiOutBank);
	fprintf(fp, "chanMidiProgChg%d=%u\n",    i, midiProgChg);
	fprintf(fp, "chanMidiBankChg%d=%u\n",    i, midiBankChg);
}


/* ------------------------------------------------------------------ */


int MidiChannel::save(const char *path)
{
	gLog("[MidiChannel] save to %s ...", path ); 
	
	MidiFile *midifile = new MidiFile( path, "w+" );
	Track	 *miditrack = new Track( midifile, 1024 );
	//midifile->absoluteTicks();
	midifile->timing( -1 );
	
	if( recorder::toMidi( index, miditrack ) ) {
		miditrack->finish();	
		//if( midifile->write( path ) ) {
			gLog("success\n");
			delete miditrack;
			delete midifile;			
			pathfile = path;
			name     = gStripExt(gBasename(path).c_str());
			return 1;
		//}
	}
		
	gLog("failure\n");
	delete midifile;
	return 0;
	}


/* ------------------------------------------------------------------ */


int MidiChannel::load(const char *path)
{
	gLog("[MidiChannel] load from %s ...", path ); 

	MidiFile *midifile = new MidiFile( path, "r" );
	Track	 *miditrack = new Track( midifile, 1024 );
	midifile->timing( -1 );
	
	if( miditrack->nextChunk() ) {
		
		if( recorder::fromMidi( index, miditrack ) ) {
			gLog("success\n");
			delete miditrack;
			delete midifile;
			pathfile = path;
			name     = gStripExt(gBasename(path).c_str());
			return 1;
		}
	}

	gLog("failure\n");
	delete midifile;
	return 0;
}


