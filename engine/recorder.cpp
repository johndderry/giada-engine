//
// C++ Implementation: recorder
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


#include <math.h>
#include <alsa/asoundlib.h>
#include "engine.h"


namespace recorder 
{
gVector<int>		frames;
gVector<action*>	actions;
gVector< gVector<action*> > global;

bool active = false;
bool sortedActions = false;

composite cmp;


/* ------------------------------------------------------------------ */


void init() 
{
	sortedActions = false;
	active = false;
	clearAll();
}


/* ------------------------------------------------------------------ */


bool canRec(Channel *ch) 
{
	/* NO recording if:
	 * recorder is inactive
	 * mixer is not running
	 * mixer is recording a take in this channel ch
	 * channel is empty */

        /* first check for midiIn or tunnelIn are false */

	if( !ch->midiIn && !ch->tunnelIn )
		return 0;
	
	if (!active || !G_Mixer.running || G_Mixer.chanInput == ch || (ch->type == CHANNEL_SAMPLE && ((SampleChannel*)ch)->wave == NULL))
		return 0;
	return 1;
}


/* ------------------------------------------------------------------ */


void rec(int index, int type, int frame, uint32_t iValue, float fValue)
{
	/* make sure frame is even */
	
	if (frame % 2 != 0)
		frame++;
		
	/* allocating the action */

	action *a = new action;
	a->chan   = index;
	a->type   = type;
	a->frame  = frame;
	a->iValue = iValue;
	a->fValue = fValue;

	/* check if the frame exists in the stack. If it exists, we don't extend
	 * the stack, but we add (or push) a new action to it. */

	int frameToExpand = frames.size;
	for (int i=0; i<frameToExpand; i++)
		if (frames.at(i) == frame) {
			frameToExpand = i;
			break;
		}

	/* espansione dello stack frames nel caso l'azione ricada in frame
	 * non precedentemente memorizzati (frameToExpand == frames.size).
	 * Espandere frames è facile, basta aggiungere un frame in coda.
	 * Espandere global è più complesso: bisogna prima allocare una
	 * cella in global (per renderlo parallelo a frames) e poi
	 * inizializzare il suo sub-stack (di action). */

	if (frameToExpand == (int) frames.size) {
		frames.add(frame);
		global.add(actions);							// array of actions added
		global.at(global.size-1).add(a);	// action added
	}
	else {

		/* no duplicates, please */

		for (unsigned t=0; t<global.at(frameToExpand).size; t++) {
			action *ac = global.at(frameToExpand).at(t);
			if (ac->chan   == index  &&
			    ac->type   == type   &&
			    ac->frame  == frame  &&
			    ac->iValue == iValue &&
			    ac->fValue == fValue)
				return;
		}

		global.at(frameToExpand).add(a);		// expand array
	}

	/* if WITH_VST create a new VST event and attach it to our action.
	 * Nota bene: the VST event occurs on localFrame=0: this is a
	 * user-generated event after all! */

#ifdef WITH_VST
	if (type == ACTION_MIDI)
		a->event = G_PluginHost.createVstMidiEvent(a->iValue);
#endif

	/* don't activate the channel (readActions == false), it's up to
	 * the other layers */

	Channel *ch = G_Mixer.getChannelByIndex(index);
	ch->hasActions = true;

	sortedActions = false;

	gLog("[REC] action type=%d recorded on frame=%d, chan=%d, iValue=%d (%X), fValue=%f\n",
		type, frame, index, iValue, iValue, fValue);
	//print();
}


/* ------------------------------------------------------------------ */


void clearChan(int index) 
{
	gLog("[REC] clearing chan %d...\n", index);

	for (unsigned i=0; i<global.size; i++) {	// for each frame i
		unsigned j=0;
		while (true) {
			if (j == global.at(i).size) break; 	  // for each action j of frame i
			action *a = global.at(i).at(j);
			if (a->chan == index)	{
#ifdef WITH_VST
				if (a->type == ACTION_MIDI)
					delete a->event;
#endif
				delete a;
				global.at(i).del(j);
			}
			else
				j++;
		}
	}

	Channel *ch = G_Mixer.getChannelByIndex(index);
	ch->hasActions = false;
	optimize();
	//print();
}


/* ------------------------------------------------------------------ */


void clearAction(int index, char act) 
{
	gLog("[REC] clearing action %d from chan %d...\n", act, index);
	for (unsigned i=0; i<global.size; i++) {						// for each frame i
		unsigned j=0;
		while (true) {                                   // for each action j of frame i
			if (j == global.at(i).size)
				break;
			action *a = global.at(i).at(j);
			if (a->chan == index && (act & a->type) == a->type)	{ // bitmask
				delete a;
				global.at(i).del(j);
			}
			else
				j++;
		}
	}
	Channel *ch = G_Mixer.getChannelByIndex(index);
	ch->hasActions = false;   /// FIXME - why this? Isn't it useless if we call chanHasActions?
	optimize();
	chanHasActions(index);    /// FIXME
	//print();
}


/* ------------------------------------------------------------------ */


void deleteAction(int chan, int frame, char type, bool checkValues, uint32_t iValue, float fValue) 
{
	/* find the frame 'frame' */

	bool found = false;
	for (unsigned i=0; i<frames.size && !found; i++) {
		if (frames.at(i) == frame) {

			/* find the action in frame i */

			for (unsigned j=0; j<global.at(i).size; j++) {
				action *a = global.at(i).at(j);

				/* action comparison logic */

				bool doit = (a->chan == chan && a->type == (type & a->type));
				if (checkValues)
					doit &= (a->iValue == iValue && a->fValue == fValue);

				if (doit) {
					int lockStatus = 0;
					while (lockStatus == 0) {
						lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_recs);
						if (lockStatus == 0) {
#ifdef WITH_VST
							if (type == ACTION_MIDI)
								delete a->event;
#endif
							delete a;
							global.at(i).del(j);
							pthread_mutex_unlock(&G_Mixer.mutex_recs);
							found = true;
							break;
						}
						else
							gLog("[REC] delete action: waiting for mutex...\n");
					}
				}
			}
		}
	}
	if (found) {
		optimize();
		chanHasActions(chan);
		gLog("[REC] action deleted, type=%d frame=%d chan=%d iValue=%d (%X) fValue=%f\n",
			type, frame, chan, iValue, iValue, fValue);
	}
	else
		gLog("[REC] unable to find action! type=%d frame=%d chan=%d iValue=%d (%X) fValue=%f\n",
			type, frame, chan, iValue, iValue, fValue);
}


/* ------------------------------------------------------------------ */


void deleteActions(int chan, int frame_a, int frame_b, char type) 
{
	sortActions();
	gVector<int> dels;

	for (unsigned i=0; i<frames.size; i++)
		if (frames.at(i) > frame_a && frames.at(i) < frame_b)
			dels.add(frames.at(i));

	for (unsigned i=0; i<dels.size; i++)
		deleteAction(chan, dels.at(i), type, false); // false == don't check values
}


/* ------------------------------------------------------------------ */


void clearAll() 
{
	while (global.size > 0) {
		for (unsigned i=0; i<global.size; i++) {
			for (unsigned k=0; k<global.at(i).size; k++) {
#ifdef WITH_VST
				if (global.at(i).at(k)->type == ACTION_MIDI)
					delete global.at(i).at(k)->event;
#endif
				delete global.at(i).at(k);
			}
			global.at(i).clear();
			global.del(i);
		}
	}

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		G_Mixer.channels.at(i)->hasActions  = false;
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE)
			((SampleChannel*)G_Mixer.channels.at(i))->readActions = false;
	}

	global.clear();
	frames.clear();
}


/* ------------------------------------------------------------------ */


void optimize() 
{
	/* do something until the i frame is empty. */

	unsigned i = 0;
	while (true) {
		if (i == global.size) return;
		if (global.at(i).size == 0) {
			global.del(i);
			frames.del(i);
		}
		else
			i++;
	}

	sortActions();
}


/* ------------------------------------------------------------------ */


void sortActions() 
{
	if (sortedActions)
		return;
	for (unsigned i=0; i<frames.size; i++)
		for (unsigned j=0; j<frames.size; j++)
			if (frames.at(j) > frames.at(i)) {
				frames.swap(j, i);
				global.swap(j, i);
			}
	sortedActions = true;
	//print();
}


/* ------------------------------------------------------------------ */


void updateBpm(float oldval, float newval, int oldquanto) 
{
	for (unsigned i=0; i<frames.size; i++) {

		float frame  = ((float) frames.at(i)/newval) * oldval;
		frames.at(i) = (int) frame;

		/* the division up here cannot be precise. A new frame can be 44099
		 * and the quantizer set to 44100. That would mean two recs completely
		 * useless. So we compute a reject value ('scarto'): if it's lower
		 * than 6 frames the new frame is collapsed with a quantized frame. */
		/** CHECKME - maybe 6 frames are too low */

		if (frames.at(i) != 0) {
			int scarto = oldquanto % frames.at(i);
			if (scarto > 0 && scarto <= 6)
				frames.at(i) = frames.at(i) + scarto;
		}

		/* never ever have odd frames. */

		if (frames.at(i) % 2 != 0)
			frames.at(i)++;
	}

	/* update structs */

	for (unsigned i=0; i<frames.size; i++) {
		for (unsigned j=0; j<global.at(i).size; j++) {
			action *a = global.at(i).at(j);
			a->frame = frames.at(i);
		}
	}

	//print();
}


/* ------------------------------------------------------------------ */


void updateSamplerate(int systemRate, int patchRate) 
{
	/* diff ratio: systemRate / patchRate
	 * e.g.  44100 / 96000 = 0.4... */

	if (systemRate == patchRate)
		return;

	gLog("[REC] systemRate (%d) != patchRate (%d), converting...\n", systemRate, patchRate);

	float ratio = systemRate / (float) patchRate;
	for (unsigned i=0; i<frames.size; i++) {

		gLog("[REC]    oldFrame = %d", frames.at(i));

		float newFrame = frames.at(i);
		newFrame = floorf(newFrame * ratio);

		frames.at(i) = (int) newFrame;

		if (frames.at(i) % 2 != 0)
			frames.at(i)++;

		gLog(", newFrame = %d\n", frames.at(i));
	}

	/* update structs */

	for (unsigned i=0; i<frames.size; i++) {
		for (unsigned j=0; j<global.at(i).size; j++) {
			action *a = global.at(i).at(j);
			a->frame = frames.at(i);
		}
	}
}


/* ------------------------------------------------------------------ */


void expand(int old_fpb, int new_fpb) 
{
	/* this algorithm requires multiple passages if we expand from e.g. 2
	 * to 16 beats, precisely 16 / 2 - 1 = 7 times (-1 is the first group,
	 * which exists yet). If we expand by a non-multiple, the result is zero,
	 * due to float->int implicit cast */

	unsigned pass = (int) (new_fpb / old_fpb) - 1;
	if (pass == 0) pass = 1;

	unsigned init_fs = frames.size;

	for (unsigned z=1; z<=pass; z++) {
		for (unsigned i=0; i<init_fs; i++) {
			unsigned newframe = frames.at(i) + (old_fpb*z);
			frames.add(newframe);
			global.add(actions);
			for (unsigned k=0; k<global.at(i).size; k++) {
				action *a = global.at(i).at(k);
				rec(a->chan, a->type, newframe, a->iValue, a->fValue);
			}
		}
	}
	gLog("[REC] expanded recs\n");
	//print();
}


/* ------------------------------------------------------------------ */


void shrink(int new_fpb) 
{
	/* easier than expand(): here we delete eveything beyond old_framesPerBars. */

	unsigned i=0;
	while (true) {
		if (i == frames.size) break;

		if (frames.at(i) >= new_fpb) {
			for (unsigned k=0; k<global.at(i).size; k++)
				delete global.at(i).at(k);
			global.at(i).clear();
			global.del(i);											// shrink global
			frames.del(i);											// shrink frames
		}
		else
			i++;
	}
	optimize();
	gLog("[REC] shrinked recs\n");
	//print();
}


/* ------------------------------------------------------------------ */


void chanHasActions(int index) 
{
	Channel *ch = G_Mixer.getChannelByIndex(index);
	if (global.size == 0) {
		ch->hasActions = false;
		return;
	}
	for (unsigned i=0; i<global.size && !ch->hasActions; i++) {
		for (unsigned j=0; j<global.at(i).size && !ch->hasActions; j++) {
			if (global.at(i).at(j)->chan == index)
				ch->hasActions = true;
		}
	}
}


/* ------------------------------------------------------------------ */


int getNextAction(int chan, char type, int frame, action **out, uint32_t iValue, uint32_t iValue2) 
{
	sortActions();  // mandatory

	unsigned i=0;
	while (i < frames.size && frames.at(i) <= frame) i++;

	if (i == frames.size)   // no further actions past 'frame'
		return -1;

	for (; i<global.size; i++)
		for (unsigned j=0; j<global.at(i).size; j++) {
			action *a = global.at(i).at(j);
			if (a->chan == chan && (type & a->type) == a->type) {
				if (iValue == 0 || a->iValue == iValue || (iValue2 != 0 && a->iValue == iValue2)) {
					*out = global.at(i).at(j);
					return 1;
				}
			}
		}

	return -2;   // no 'type' actions found
}


/* ------------------------------------------------------------------ */


int getAction(int chan, char action, int frame, struct action **out) 
{
	for (unsigned i=0; i<global.size; i++)
		for (unsigned j=0; j<global.at(i).size; j++)
			if (frame  == global.at(i).at(j)->frame &&
					action == global.at(i).at(j)->type &&
					chan   == global.at(i).at(j)->chan)
			{
				*out = global.at(i).at(j);
				return 1;
			}
	return 0;
}


/* ------------------------------------------------------------------ */


void startOverdub(int index, char actionMask, int frame) 
{
	/* prepare the composite struct */

	if (actionMask == ACTION_KEYS) {
		cmp.a1.type = ACTION_KEYPRESS;
		cmp.a2.type = ACTION_KEYREL;
	}
	else {
		cmp.a1.type = ACTION_MUTEON;
		cmp.a2.type = ACTION_MUTEOFF;
	}
	cmp.a1.chan  = index;
	cmp.a2.chan  = index;
	cmp.a1.frame = frame;
	// cmp.a2.frame doesn't exist yet

	/* avoid underlying action truncation: if action2.type == nextAction:
	 * you are in the middle of a composite action, truncation needed */

	rec(index, cmp.a1.type, frame);

	action *act = NULL;
	int res = getNextAction(index, cmp.a1.type | cmp.a2.type, cmp.a1.frame, &act);
	if (res == 1) {
		if (act->type == cmp.a2.type) {
			int truncFrame = cmp.a1.frame-kernelAudio::realBufsize;
			if (truncFrame < 0)
				truncFrame = 0;
			gLog("[REC] add truncation at frame %d, type=%d\n", truncFrame, cmp.a2.type);
			rec(index, cmp.a2.type, truncFrame);
		}
	}

	SampleChannel *ch = (SampleChannel*) G_Mixer.getChannelByIndex(index);
	ch->readActions = false;   // don't use disableRead()
}


/* ------------------------------------------------------------------ */


void stopOverdub(int frame) 
{
	cmp.a2.frame  = frame;
	bool ringLoop = false;
	bool nullLoop = false;

	/* ring loop verification, i.e. a composite action with key_press at
	 * frame N and key_release at frame M, with M <= N */

	if (cmp.a2.frame < cmp.a1.frame) {
		ringLoop = true;
		gLog("[REC] ring loop! frame1=%d < frame2=%d\n", cmp.a1.frame, cmp.a2.frame);
		rec(cmp.a2.chan, cmp.a2.type, G_Mixer.totalFrames); 	// record at the end of the sequencer
	}
	else
	if (cmp.a2.frame == cmp.a1.frame) {
		nullLoop = true;
		gLog("[REC]  null loop! frame1=%d == frame2=%d\n", cmp.a1.frame, cmp.a2.frame);
		deleteAction(cmp.a1.chan, cmp.a1.frame, cmp.a1.type, false); // false == don't check values
	}

	SampleChannel *ch = (SampleChannel*) G_Mixer.getChannelByIndex(cmp.a2.chan);
	ch->readActions = false;      // don't use disableRead()

	/* remove any nested action between keypress----keyrel, then record */

	if (!nullLoop)
		deleteActions(cmp.a2.chan, cmp.a1.frame, cmp.a2.frame, cmp.a1.type);
		deleteActions(cmp.a2.chan, cmp.a1.frame, cmp.a2.frame, cmp.a2.type);

	if (!ringLoop && !nullLoop) {
		rec(cmp.a2.chan, cmp.a2.type, cmp.a2.frame);

		/* avoid underlying action truncation, if keyrel happens inside a
		* composite action */

		action *act = NULL;
		int res = getNextAction(cmp.a2.chan, cmp.a1.type | cmp.a2.type, cmp.a2.frame, &act);
		if (res == 1) {
			if (act->type == cmp.a2.type) {
				gLog("[REC] add truncation at frame %d, type=%d\n", act->frame, act->type);
				deleteAction(act->chan, act->frame, act->type, false); // false == don't check values
			}
		}
	}
}


/* ------------------------------------------------------------------ */


void print() 
{
	gLog("[REC] ** print debug **\n");
	for (unsigned i=0; i<global.size; i++) {
		gLog("  frame %d\n", frames.at(i));
		for (unsigned j=0; j<global.at(i).size; j++) {
			gLog("    action %d | chan %d | frame %d\n", global.at(i).at(j)->type, global.at(i).at(j)->chan, global.at(i).at(j)->frame);
		}
	}
}


#if 0
int toMidi( int index, MidiFile *mf )
{
	action		*act;
	MidiEvent	event;
	Channel 	*ch = G_Mixer.getChannelByIndex(index);
	uint32_t 	ivalue;  // used only for MIDI events
	
	for (unsigned i=0; i<global.size; i++)
		for (unsigned j=0; j<global.at(i).size; j++) {
			act = global.at(i).at(j);
			if( act->chan == index && act->type & ACTION_MIDI ) {
				// convert this event
				event.tick = act->frame;
				event.track = ((MidiChannel *)ch)->midiOutChan;
				ivalue = act->iValue;
				gLog("frame %d | chan %d | action %d | ivalue %x | ", act->frame, act->chan, act->type, ivalue );
				if( (ivalue&0xff000000) == (unsigned)MIDI_NOTE_ON ) {
					event.makeNoteOn( (ivalue>>16) & 0xff, (ivalue>>8) & 0xff );
					gLog("noteON\n");
				}
				else if( (ivalue&0xff000000) == (unsigned)MIDI_NOTE_OFF ) { 
					event.makeNoteOff( (ivalue>>16) & 0xff, (ivalue>>8) & 0xff );
					gLog("noteOFF\n");
				}
				else 
					gLog("[RECORDER] toMidi: unknown message type %x\n", ivalue );
				
				mf->addEvent( event );
			}
		}
		
	return 1;
}


int fromMidi( int index, MidiFile *mf )
{
	MidiEvent	*event;
	//Channel 	*ch = G_Mixer.getChannelByIndex(index);
	uint32_t 	ivalue;  // used only for MIDI events
	
	for (int i=0; i < (*mf)[0].size(); i++) {
		
		event = &(*mf)[0][i];
		ivalue = 0;
		
		if( event->isNoteOn() ) 
			ivalue = MIDI_NOTE_ON | event->getKeyNumber() << 16 | event->getVelocity() << 8 ;
		if(	event->isNoteOff() )
			ivalue = MIDI_NOTE_OFF | event->getKeyNumber() << 16 | event->getVelocity() << 8 ;

		// any midi channel is being ignored	

		if( ivalue ) 
			recorder::rec(index, ACTION_MIDI, event->tick, ivalue );  
	}

	return 1;
}
#else
int toMidi( int index, Track *mf )
{
	action		*act;
	//MidiEvent	event;
	Channel 	*ch = G_Mixer.getChannelByIndex(index);
	uint32_t 	ivalue;  // used only for MIDI events
	
	for (unsigned i=0; i<global.size; i++)
		for (unsigned j=0; j<global.at(i).size; j++) {
			act = global.at(i).at(j);
			if( act->chan == index && act->type & ACTION_MIDI ) {
				// convert this event
				//event.tick = act->frame;
				//event.track = ((MidiChannel *)ch)->midiOutChan;
				ivalue = act->iValue;
				gLog("frame %d | chan %d | action %d | ivalue %x | ", act->frame, act->chan, act->type, ivalue );

				if( (ivalue&0xff000000) == (unsigned)MIDI_NOTE_ON ) {
				//	event.makeNoteOn( (ivalue>>16) & 0xff, (ivalue>>8) & 0xff );
					mf->recordAbsMevent(act->frame, 
						0x90 | ((MidiChannel *)ch)->midiOutChan, 
						(ivalue>>16) & 0xff, (ivalue>>8) & 0xff );
					gLog("noteON\n");
				}
				else if( (ivalue&0xff000000) == (unsigned)MIDI_NOTE_OFF ) { 
				//	event.makeNoteOff( (ivalue>>16) & 0xff, (ivalue>>8) & 0xff );
					mf->recordAbsMevent(act->frame, 
						0x80 | ((MidiChannel *)ch)->midiOutChan, 
						(ivalue>>16) & 0xff, (ivalue>>8) & 0xff );
					gLog("noteOFF\n");
				}
				else 
					gLog("[RECORDER] toMidi: unknown message type %x\n", ivalue );
				
				//mf->addEvent( event );
			}
		}
		
	return 1;
}


int fromMidi( int index, Track *mf )
{
	MidiEvent	*event;
	//nonMidiEvent	*nm_event;
	//Channel 	*ch = G_Mixer.getChannelByIndex(index);
	uint32_t 	ivalue = 0;  // used only for MIDI events
	long unsigned tick = 0;
	int etype;

	while( (etype = mf->nextEvent()) != -1 )
		if( etype ) {
			// non -midi
			/* nm_event = */ mf->nextnonMidi();
		} else {
			event = mf->nextMidi();
			if( (event->eventstatus() & 0xf0) == 0x90 ) 
				ivalue = MIDI_NOTE_ON | event->byte0() << 16 | event->byte1() << 8 ;
			else if( (event->eventstatus() & 0xf0) == 0x80 )
				ivalue = MIDI_NOTE_OFF | event->byte0() << 16 | event->byte1() << 8 ;

			tick = tick + event->getdtime();
			if( ivalue ) 
				recorder::rec(index, ACTION_MIDI, tick, ivalue );  
		}

	return 1;
}
#endif


} // namespace
