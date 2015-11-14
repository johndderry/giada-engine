//
// C++ Implementation: mixer
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


#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif

class Mixer G_Mixer;

/* ---------------------------------------------------------------------- */


Mixer::Mixer() 	{}
Mixer::~Mixer() {}


#define TICKSIZE 38


float Mixer::tock[TICKSIZE] = {
	 0.059033,  0.117240,  0.173807,  0.227943,  0.278890,  0.325936,
	 0.368423,  0.405755,  0.437413,  0.462951,  0.482013,  0.494333,
	 0.499738,  0.498153,  0.489598,  0.474195,  0.452159,  0.423798,
	 0.389509,  0.349771,  0.289883,  0.230617,  0.173194,  0.118739,
	 0.068260,  0.022631, -0.017423, -0.051339,	-0.078721, -0.099345,
	-0.113163, -0.120295, -0.121028, -0.115804, -0.105209, -0.089954,
	-0.070862, -0.048844
};


float Mixer::tick[TICKSIZE] = {
	  0.175860,  0.341914,  0.488904,  0.608633,  0.694426,  0.741500,
	  0.747229,  0.711293,	0.635697,  0.524656,  0.384362,  0.222636,
	  0.048496, -0.128348, -0.298035, -0.451105, -0.579021, -0.674653,
	 -0.732667, -0.749830, -0.688924, -0.594091, -0.474481, -0.340160,
	 -0.201360, -0.067752,  0.052194,  0.151746,  0.226280,  0.273493,
	  0.293425,  0.288307,  0.262252,  0.220811,  0.170435,  0.117887,
	  0.069639,  0.031320
};


/* ------------------------------------------------------------------ */


void Mixer::init() {
	quanto      = 1;
	docross     = false;
	rewindWait  = false;
	recordWait	= false;
	running     = false;
	ready       = true;
	waitRec     = 0;
	actualFrame = 0;
	bpm 		= DEFAULT_BPM;
	bars		= DEFAULT_BARS;
	beats		= DEFAULT_BEATS;
	breaks		= DEFAULT_BREAKS;
	quantize    = DEFAULT_QUANTIZE;
	metronome   = false;

	tickTracker = 0;
	tockTracker = 0;
	tackTracker = 0;
	tickPlay    = false;
	tockPlay    = false;
	tackPlay    = false;

	outVol       = DEFAULT_OUT_VOL;
	inVol        = DEFAULT_IN_VOL;
	peakOut      = 0.0f;
	peakIn	     = 0.0f;
	chanInput    = NULL;
	inputTracker = 0;

	actualBeat    = 0;
	actualBreak   = 0;
	for( int i = 0; i < MAX_BREAKS; i++ ) breakId[i] = -1;
	suspend = false;
	nextMidiChan  = 0;

	midiTCstep    = 0;
	midiTCrate    = (G_Conf.samplerate / G_Conf.midiTCfps) * 2;  // dealing with stereo vals
	midiTCframes  = 0;
	midiTCseconds = 0;
	midiTCminutes = 0;
	midiTChours   = 0;

	/* alloc virtual input channels. vChanInput new is done in
	 * updateFrameBars, because of its variable size */
	/** TODO - set kernelAudio::realBufsize * 2 as private member */

	vChanInput   = NULL;
	//vChanInToOut = (float *) malloc(kernelAudio::realBufsize * 2 * sizeof(float));
	vChanInToOut = new float[ kernelAudio::realBufsize * 2 ];

	pthread_mutex_init(&mutex_recs, NULL);
	pthread_mutex_init(&mutex_chans, NULL);
	pthread_mutex_init(&mutex_plugins, NULL);

	updateFrameBars();
	rewind();
}


/* ------------------------------------------------------------------ */


Channel *Mixer::addChannel(int type) {

	Channel *ch;
	int bufferSize = kernelAudio::realBufsize*2;

	if (type == CHANNEL_SAMPLE)
		ch = new SampleChannel(bufferSize);
	else {
		Channel *tch;
		ch = new MidiChannel(bufferSize);
		// when adding channel turn off all other channel tunnelling 
		for (unsigned i=0; i<G_Mixer.channels.size; i++) 
			if ( (tch = G_Mixer.channels.at(i)) )  tch->tunnelIn = false;
	}

	while (true) {
		int lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.add(ch);
			pthread_mutex_unlock(&mutex_chans);
			break;
		}
	}

	ch->index = getNewIndex();
	gLog("[mixer] channel @ %d added type=%d; total channels=%d\n", 
		ch->index, ch->type, channels.size);
	return ch;
}


/* ------------------------------------------------------------------ */


void Mixer::swapMidiIn(int n) {

	Channel *tch;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) 
		if ( (tch = G_Mixer.channels.at(i)) )  tch->tunnelIn = false;
	if( n >= 0 ) G_Mixer.channels.at(n)->tunnelIn = true;
}


/* ------------------------------------------------------------------ */


int Mixer::getNewIndex() {

	/* always skip last channel: it's the last one just added */

	if (channels.size == 1)
		return 0;

	int index = 0;
	for (unsigned i=0; i<channels.size-1; i++) {
		if (channels.at(i)->index > index)
			index = channels.at(i)->index;
		}
	index += 1;
	return index;
}


/* ------------------------------------------------------------------ */


int Mixer::deleteChannel(Channel *ch) {
	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&mutex_chans);
		if (lockStatus == 0) {
			channels.del(ch);
			delete ch;
			pthread_mutex_unlock(&mutex_chans);
			return 1;
		}
		//else
		//	gLog("[mixer::deleteChannel] waiting for mutex...\n");
	}
}


/* ------------------------------------------------------------------ */


Channel *Mixer::getChannelByIndex(int index) {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->index == index)
			return channels.at(i);
	gLog("[mixer::getChannelByIndex] channel at index %d not found!\n", index);
	return NULL;
}


/* ------------------------------------------------------------------ */


void Mixer::sendMIDIsync() {

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M) {
		if (actualFrame % (framesPerBeat/24) == 0)
			kernelMidi::send(MIDI_CLOCK, -1, -1);
	}
	else
	if (G_Conf.midiSync == MIDI_SYNC_MTC_M) {

		/* check if a new timecode frame has passed. If so, send MIDI TC
		 * quarter frames. 8 quarter frames, divided in two branches:
		 * 1-4 and 5-8. We check timecode frame's parity: if even, send
		 * range 1-4, if odd send 5-8. */

		if (actualFrame % midiTCrate == 0) {

			/* frame low nibble
			 * frame high nibble
			 * seconds low nibble
			 * seconds high nibble */

			if (midiTCframes % 2 == 0) {
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes & 0x0F)  | 0x00, -1);
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTCframes >> 4)    | 0x10, -1);
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds & 0x0F) | 0x20, -1);
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTCseconds >> 4)   | 0x30, -1);
			}

			/* minutes low nibble
			 * minutes high nibble
			 * hours low nibble
			 * hours high nibble SMPTE frame rate */

			else {
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes & 0x0F) | 0x40, -1);
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTCminutes >> 4)   | 0x50, -1);
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours & 0x0F)   | 0x60, -1);
				kernelMidi::send(MIDI_MTC_QUARTER, (midiTChours >> 4)     | 0x70, -1);
			}

			midiTCframes++;

			/* check if total timecode frames are greater than timecode fps:
			 * if so, a second has passed */

			if (midiTCframes > G_Conf.midiTCfps) {
				midiTCframes = 0;
				midiTCseconds++;
				if (midiTCseconds >= 60) {
					midiTCminutes++;
					midiTCseconds = 0;
					if (midiTCminutes >= 60) {
						midiTChours++;
						midiTCminutes = 0;
					}
				}
				//gLog("%d:%d:%d:%d\n", midiTChours, midiTCminutes, midiTCseconds, midiTCframes);
			}
		}
	}
}


/* ------------------------------------------------------------------ */


void Mixer::sendMIDIrewind() {

	midiTCframes  = 0;
	midiTCseconds = 0;
	midiTCminutes = 0;
	midiTChours   = 0;

	/* For cueing the slave to a particular start point, Quarter Frame
	 * messages are not used. Instead, an MTC Full Frame message should
	 * be sent. The Full Frame is a SysEx message that encodes the entire
	 * SMPTE time in one message */

	if (G_Conf.midiSync == MIDI_SYNC_MTC_M) {
		kernelMidi::send(MIDI_SYSEX, 0x7F, 0x00);  // send msg on channel 0
		kernelMidi::send(0x01, 0x01, 0x00);        // hours 0
		kernelMidi::send(0x00, 0x00, 0x00);        // mins, secs, frames 0
		kernelMidi::send(MIDI_EOX, -1, -1);        // end of sysex
	}
}

/* ------------------------------------------------------------------ */


int Mixer::masterPlay(
	void *out_buf, void *in_buf, unsigned n_frames,
	double streamTime, RtAudioStreamStatus status, void *userData) {
	return G_Mixer.__masterPlay(out_buf, in_buf, n_frames);
}


/* ------------------------------------------------------------------ */


int Mixer::__masterPlay(void *out_buf, void *in_buf, unsigned bufferFrames) {

	if (!ready)
		return 0;

	float *outBuf = ((float *) out_buf);
	float *inBuf  = ((float *) in_buf);
	bufferFrames *= 2;     // stereo
	peakOut       = 0.0f;  // reset peak calculator
	peakIn        = 0.0f;  // reset peak calculator

	/* always clean each buffer */

	memset(outBuf, 0, sizeof(float) * bufferFrames);         // out
	memset(vChanInToOut, 0, sizeof(float) * bufferFrames);   // inToOut vChan

	pthread_mutex_lock(&mutex_chans);
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			((SampleChannel*)channels.at(i))->clear();
	pthread_mutex_unlock(&mutex_chans);

	for (unsigned j=0; j<bufferFrames; j+=2) {

		if (kernelAudio::inputEnabled) {

			/* input peak calculation (left chan only so far). */

			if (inBuf[j] * inVol > peakIn)
				peakIn = inBuf[j] * inVol;

			/* "hear what you're playing" - process, copy and paste the input buffer
			 * onto the output buffer */

			if (inToOut) {
				vChanInToOut[j]   = inBuf[j]   * inVol;
				vChanInToOut[j+1] = inBuf[j+1] * inVol;
			}
		}

		/* operations to do if the sequencer is running:
		 * - compute quantizer
		 * - time check for LOOP_REPEAT
		 * - reset loops at beat 0
		 * - read recorded actions
		 * - reset actualFrame */

		if (running) {

			/* line in recording */

			if (chanInput != NULL && kernelAudio::inputEnabled) {

				/* delay comp: wait until waitRec reaches delayComp. WaitRec
				 * returns to 0 in mixerHandler, as soon as the recording ends */

				if (waitRec < G_Conf.delayComp)
					waitRec += 2;
				else {
					vChanInput[inputTracker]   += inBuf[j]   * inVol;
					vChanInput[inputTracker+1] += inBuf[j+1] * inVol;
					inputTracker += 2;
					if (inputTracker >= totalFrames)
						inputTracker = 0;
				}
			}

			/* quantizer computations: quantize rewind and all channels. */

			if (quantize > 0 && quanto > 0) {
				if (actualFrame % (quanto) == 0) {   // is quanto!
					if (rewindWait) {
						rewindWait = false;
						rewind();
					}
					pthread_mutex_lock(&mutex_chans);
					for (unsigned k=0; k<channels.size; k++)
						channels.at(k)->quantize(k, j, actualFrame);  // j == localFrame
					pthread_mutex_unlock(&mutex_chans);
				}
			}

			if( G_Conf.useAltClick & metronome ) {

				if (actualFrame % framesPerBar == 0 && actualFrame != 0) 
					tackPlay = true;

				if (actualFrame % framesPerBar == ((beats-1)*framesPerBar)/beats) 
					tickPlay = true;
				
			}

			/* reset LOOP_REPEAT, if a bar has passed */

			if (actualFrame % framesPerBar == 0 && actualFrame != 0) {
				if (metronome && !G_Conf.useAltClick )
					tickPlay = true;

				pthread_mutex_lock(&mutex_chans);
				for (unsigned k=0; k<channels.size; k++)
					channels.at(k)->onBar(j);
				pthread_mutex_unlock(&mutex_chans);
			}

			/* reset loops on beat 0 */

			if (actualFrame == 0) {
				pthread_mutex_lock(&mutex_chans);
				for (unsigned k=0; k<channels.size; k++)
					channels.at(k)->onZero(j);
				pthread_mutex_unlock(&mutex_chans);
			}

			/* reading all actions recorded */

			pthread_mutex_lock(&mutex_recs);
			for (unsigned y=0; y<recorder::frames.size; y++) {
				if (recorder::frames.at(y) == actualFrame) {
					for (unsigned z=0; z<recorder::global.at(y).size; z++) {
						int index   = recorder::global.at(y).at(z)->chan;
						Channel *ch = getChannelByIndex(index);
						ch->parseAction(recorder::global.at(y).at(z), j, actualFrame);
					}
					break;
				}
			}
			pthread_mutex_unlock(&mutex_recs);

			/* increase actualFrame */

			actualFrame += 2;

			/* if actualFrame > totalFrames the sequencer returns to frame 0,
			 * beat 0. This must be the last operation. */

			if (actualFrame > totalFrames) {
				actualFrame = 0;
				actualBeat  = 0;
				if( G_Conf.sampleEndMode == 1 && chanInput != NULL ) {
					mh_stopInputRec();
					// clear the recording button from being still ON
					G_Interface->updateRecAction(0, G_Interface->controller);
				}

				if( recordWait ) {
					recordWait = false;
					chanInput = waitChan;
					inputTracker = actualFrame;
				}
				        
				if ( !suspend ) advance();
			}
			else
			if (actualFrame % framesPerBeat == 0 && actualFrame > 0) {
				actualBeat++;

				/* avoid tick and tock to overlap when a new bar has passed (which
				 * is also a beat) */

				if (metronome && !tickPlay)
					tockPlay = true;
			}

			sendMIDIsync();

		} // if (running)

		/* sum channels, CHANNEL_SAMPLE only */

		pthread_mutex_lock(&mutex_chans);
		for (unsigned k=0; k<channels.size; k++) {
			if (channels.at(k)->type == CHANNEL_SAMPLE)
				((SampleChannel*)channels.at(k))->sum(j, running);
		}
		pthread_mutex_unlock(&mutex_chans);

		/* metronome play */
		/** FIXME - move this one after the peak meter calculation */

		if( G_Conf.useMidiClick ) {
			if( tockPlay ) {
				if( tockTracker++ == 0) 
					kernelMidi::send((uint32_t) MIDI_CHANS[9] | MIDI_NOTE_ON | G_Conf.midiTockMesg << 8 );
				if( tockTracker >= TICKSIZE-1) {
					kernelMidi::send((uint32_t) MIDI_CHANS[9] | MIDI_NOTE_OFF | G_Conf.midiTockMesg << 8 );
					tockPlay = false;
					tockTracker = 0;
				}
			}
			if( tickPlay ) {
				if( tickTracker++ == 0) 
					kernelMidi::send((uint32_t) MIDI_CHANS[9] | MIDI_NOTE_ON | G_Conf.midiTickMesg << 8 );
				if( tickTracker >= TICKSIZE-1) {
					kernelMidi::send((uint32_t) MIDI_CHANS[9] | MIDI_NOTE_OFF | G_Conf.midiTickMesg << 8 );
					tickPlay = false;
					tickTracker = 0;
				}
			}
			if( tackPlay ) {
				if( tackTracker++ == 0) 
					kernelMidi::send((uint32_t) MIDI_CHANS[9] | MIDI_NOTE_ON | G_Conf.midiTackMesg << 8 );
				if( tackTracker >= TICKSIZE-1) {
					kernelMidi::send((uint32_t) MIDI_CHANS[9] | MIDI_NOTE_OFF | G_Conf.midiTackMesg << 8 );
					tackPlay = false;
					tackTracker = 0;
				}
			}
		}
		else {			
			if (tockPlay) {
				outBuf[j]   += tock[tockTracker];
				outBuf[j+1] += tock[tockTracker];
				tockTracker++;
				if (tockTracker >= TICKSIZE-1) {
					tockPlay    = false;
					tockTracker = 0;
				}
			}			
			if (tickPlay) {
				outBuf[j]   += tick[tickTracker];
				outBuf[j+1] += tick[tickTracker];
				tickTracker++;
				if (tickTracker >= TICKSIZE-1) {
					tickPlay    = false;
					tickTracker = 0;
				}
			}
		}
	} // end loop J


	/* final loop: sum virtual channels and process plugins. */

	pthread_mutex_lock(&mutex_chans);
	for (unsigned k=0; k<channels.size; k++)
		channels.at(k)->process(outBuf);
	pthread_mutex_unlock(&mutex_chans);

	/* processing fxs master in & out, if any. */

#ifdef WITH_VST
	pthread_mutex_lock(&mutex_plugins);
	G_PluginHost.processStack(outBuf, PluginHost::MASTER_OUT);
	G_PluginHost.processStack(vChanInToOut, PluginHost::MASTER_IN);
	pthread_mutex_unlock(&mutex_plugins);
#endif

	/* post processing master fx + peak calculation. */

	for (unsigned j=0; j<bufferFrames; j+=2) {

		/* merging vChanInToOut, if enabled */

		if (inToOut) {
			outBuf[j]   += vChanInToOut[j];
			outBuf[j+1] += vChanInToOut[j+1];
		}

		outBuf[j]   *= outVol;
		outBuf[j+1] *= outVol;

		/* computes the peak for the left channel (so far). */

		if (outBuf[j] > peakOut)
			peakOut = outBuf[j];

		if (G_Conf.limitOutput) {
			if (outBuf[j] > 1.0f)
				outBuf[j] = 1.0f;
			else if (outBuf[j] < -1.0f)
				outBuf[j] = -1.0f;

			if (outBuf[j+1] > 1.0f)
				outBuf[j+1] = 1.0f;
			else if (outBuf[j+1] < -1.0f)
				outBuf[j+1] = -1.0f;
		}
	}

	return 0;
}


/* ------------------------------------------------------------------ */


void Mixer::updateFrameBars() {

	/* seconds ....... total time of play (in seconds) of the whole
	 *                 sequencer. 60 / bpm == how many seconds lasts one bpm
	 * totalFrames ... number of frames in the whole sequencer, x2 because
	 * 								 it's stereo
	 * framesPerBar .. n. of frames within a bar
	 * framesPerBeat . n. of frames within a beat */

	float seconds     = (60.0f / bpm) * beats;
	totalFrames       = G_Conf.samplerate * seconds * 2;
	framesPerBar      = totalFrames / bars;
	framesPerBeat     = totalFrames / beats;
	framesInSequencer = framesPerBeat * MAX_BEATS;

	/* big troubles if frames are odd. */

	if (totalFrames % 2 != 0)
		totalFrames--;
	if (framesPerBar % 2 != 0)
		framesPerBar--;
	if (framesPerBeat % 2 != 0)
		framesPerBeat--;

	updateQuanto();

	/* realloc input virtual channel, if not NULL. TotalFrames is changed! */

	if (vChanInput != NULL)
		delete[] vChanInput;
	vChanInput = new float[ totalFrames ];
	if (!vChanInput)
		gLog("[Mixer] vChanInput realloc error!\n");
}


/* ------------------------------------------------------------------ */


int Mixer::close() {
	running = false;
	while (channels.size > 0)
		deleteChannel(channels.at(0));
	delete[] vChanInput;
	delete[] vChanInToOut;
	return 1;
}


/* ------------------------------------------------------------------ */


bool Mixer::isSilent() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->status == STATUS_PLAY)
			return false;
	return true;
}


/* ------------------------------------------------------------------ */


void Mixer::rewind() {

	actualFrame = 0;
	actualBeat  = 0;
	actualBreak = 0;

	if (running)
		for (unsigned i=0; i<channels.size; i++)
			channels.at(i)->rewind();

	sendMIDIrewind();

	// Enter the initial mbreak if necessary
	int mbreak = breakId[actualBreak];

	if( mbreak >= 0 ) 
		enterMBreak( mbreak );

}


/* ------------------------------------------------------------------ */


void Mixer::updateQuanto() {

	/* big troubles if frames are odd. */

	if (quantize != 0)
		quanto = framesPerBeat / quantize;
	if (quanto % 2 != 0)
		quanto++;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasLogicalSamples() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)channels.at(i))->wave)
				if (((SampleChannel*)channels.at(i))->wave->isLogical)
					return true;
	return false;
}


/* ------------------------------------------------------------------ */


bool Mixer::hasEditedSamples() {
	for (unsigned i=0; i<channels.size; i++)
		if (channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)channels.at(i))->wave)
				if (((SampleChannel*)channels.at(i))->wave->isEdited)
					return true;
	return false;
}


/* ------------------------------------------------------------------ */


bool Mixer::mergeVirtualInput() {
	if (vChanInput == NULL) {
		gLog("[Mixer] virtual input channel not alloc'd\n");
		return false;
	}
	else {
#ifdef WITH_VST
		G_PluginHost.processStackOffline(vChanInput, PluginHost::MASTER_IN, 0, totalFrames);
#endif
		int numFrames = totalFrames*sizeof(float);
		memcpy(chanInput->wave->data, vChanInput, numFrames);
		memset(vChanInput, 0, numFrames); // clear vchan
		return true;
	}
}


/* ------------------------------------------------------------------ */


void Mixer::enterMBreak(int n) {

	gLog("[Mixer] entering Mbreak: %d", n);

	int mbreak, started=0, stopped=0;
	Channel *ch;

	if (n < 0) {
		// not a numbered break ( == -1 )
		//   stop all channels with mbreak >= 0 

		for (int i = 0; i < (int)channels.size; i++ ) {
			
			ch = channels.at(i);
			if( ch->mbreak >= 0 && ch->status == STATUS_PLAY &&
			    (ch->type == CHANNEL_MIDI || ((SampleChannel *)ch)->mode & LOOP_ANY) )
				{ stopped++; ch->stop(); }
			
		}
		gLog(" Stopped %d channels\n", stopped );

		return;
	}


	// numbered break - enable next break
	for (int i = 0; i < (int)channels.size; i++ ) {
		//   deactivate unused channels and activate used channels
		ch = channels.at(i);
		mbreak = ch->mbreak;
		gLog("channel[%d] mbreak = %d, ", i, mbreak );
		if (mbreak == n) {
			if( ch->status != STATUS_PLAY &&
			    (ch->type == CHANNEL_MIDI || ((SampleChannel *)ch)->mode & LOOP_ANY ) )
				{ started++; channels.at(i)->start(0, false); }
		} 
		else 
		if( mbreak >= 0  ) {
			if( ch->status == STATUS_PLAY )
				{ stopped++; channels.at(i)->stop(); }
		}
	}
	gLog(" Stopped %d started %d channels\n", stopped, started );
}


/* ------------------------------------------------------------------ */


void Mixer::advance() {

	// advance to the next song break (section)
	int lastBreakId = breakId[actualBreak++];

	/* check limits and call break entry code if necessary */
	if( actualBreak >= breaks ) actualBreak = 0;

	if( lastBreakId != breakId[actualBreak] )
		enterMBreak( breakId[actualBreak] );
}


/* ---------------------------------------------------------------------- */


/*
 * Mixer handler stuff is here 
 */

/* ---------------------------------------------------------------------- */

void mh_stopSequencer()
{
	G_Mixer.running = false;
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		G_Mixer.channels.at(i)->stopBySeq();
}


/* ------------------------------------------------------------------ */


void mh_clear()
{
	G_Mixer.running = false;
	while (G_Mixer.channels.size > 0)
		G_Mixer.channels.del(0U);  // unsigned	
}


/* ------------------------------------------------------------------ */


bool mh_uniqueSolo(Channel *ch)
{
	int solos = 0;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		Channel *ch = G_Mixer.channels.at(i);
		if (ch->solo) solos++;
		if (solos > 1) return false;
	}
	return true;
}


/* ------------------------------------------------------------------ */


/** TODO - revision needed: mh should not call control::addChannel */

void mh_loadPatch(bool isProject, const char *projPath)
{
	G_Mixer.init();
	G_Mixer.ready = false;   // put it in wait mode

	int numChans = G_Patch.getNumChans();
	for (int i=0; i<numChans; i++) {

		Channel *ch = control::addChannel(G_Patch.getColumn(i), G_Patch.getType(i));

		char smpPath[PATH_MAX];

		/* projects < 0.6.3 version are not portable. Just use the regular
		 * samplePath */

		if (isProject && G_Patch.version >= 0.63f)
			sprintf(smpPath, "%s%s%s", gDirname(projPath).c_str(), gGetSlash().c_str(), G_Patch.getSamplePath(i).c_str());
		else
			sprintf(smpPath, "%s", G_Patch.getSamplePath(i).c_str());

		ch->loadByPatch(smpPath, i);
	}

	G_Mixer.outVol     = G_Patch.getOutVol();
	G_Mixer.inVol      = G_Patch.getInVol();
	G_Mixer.bpm        = G_Patch.getBpm();
	G_Mixer.bars       = G_Patch.getBars();
	G_Mixer.beats      = G_Patch.getBeats();
	G_Mixer.breaks     = G_Patch.getBreaks();
	G_Mixer.quantize   = G_Patch.getQuantize();
	G_Mixer.metronome  = G_Patch.getMetronome();
	G_Patch.lastTakeId = G_Patch.getLastTakeId();
	G_Patch.samplerate = G_Patch.getSamplerate();

	/* rewind and update frames in Mixer (it's vital) */

	G_Mixer.rewind();
	G_Mixer.updateFrameBars();
	G_Mixer.ready = true;
}


/* ------------------------------------------------------------------ */


void mh_rewindSequencer()
{
	if (G_Mixer.quantize > 0 && G_Mixer.running)   // quantize rewind
		G_Mixer.rewindWait = true;
	else
		G_Mixer.rewind();
}


/* ------------------------------------------------------------------ */


SampleChannel *mh_startInputRec()
{
	/* search for the next available channel */

	SampleChannel *chan = NULL;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*) G_Mixer.channels.at(i))->canInputRec()) {
			chan = (SampleChannel*) G_Mixer.channels.at(i);
			break;
		}
	}

	/* no chans available? Return */

	if (chan == NULL)
		return NULL;

	if( G_Conf.sampleEndMode == 1 ) {	
		// for end mode = ending, don't start recording until frame 0
		G_Mixer.recordWait = true;
		G_Mixer.waitChan = chan;
	}

	//Wave *w = new Wave();
	//if (!w->allocEmpty(G_Mixer.totalFrames))
	//	return NULL;

	/* increase lastTakeId until the sample name TAKE-[n] is unique */

	char name[32];
	sprintf(name, "TAKE-%d", G_Patch.lastTakeId);
	while (!mh_uniqueSamplename(chan, name)) {
		G_Patch.lastTakeId++;
		sprintf(name, "TAKE-%d", G_Patch.lastTakeId);
	}

	chan->allocEmpty(G_Mixer.totalFrames, G_Patch.lastTakeId);
	if( !G_Mixer.recordWait ) {

		G_Mixer.chanInput = chan;

		/* start to write from the actualFrame, not the beginning */
		/** FIXME: move this before wave allocation*/

		G_Mixer.inputTracker = G_Mixer.actualFrame;
	}

	gLog(
		"[mh] start input recs using chan %d with size %d, frame=%d\n",
		chan->index, G_Mixer.totalFrames, G_Mixer.inputTracker
	);

	return chan;
}


/* ------------------------------------------------------------------ */


SampleChannel *mh_stopInputRec()
{
	gLog("[mh] stop input recs\n");
	G_Mixer.mergeVirtualInput();
	SampleChannel *ch = G_Mixer.chanInput;
	G_Mixer.chanInput = NULL;
	G_Mixer.waitRec   = 0;					// if delay compensation is in use
	return ch;
}


/* ------------------------------------------------------------------ */


bool mh_uniqueSamplename(SampleChannel *ch, const char *name)
{
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		if (ch != G_Mixer.channels.at(i)) {
			if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE) {
				SampleChannel *other = (SampleChannel*) G_Mixer.channels.at(i);
				if (other->wave != NULL)
					if (!strcmp(name, other->name.c_str()))
						return false;
			}
		}
	}
	return true;
}
