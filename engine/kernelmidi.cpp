//
// C++ Implementation: kernelmidi
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


namespace kernelMidi 
{

int			api         = 0;      // one api for both in & out
RtMidiOut	*midiOut    = NULL;
RtMidiIn	*midiIn     = NULL;
unsigned	numOutPorts = 0;
unsigned	numInPorts  = 0;
bool		midiStatus  = false;

cb_midiLearn *cb_learn = NULL;
void         *cb_data  = NULL;


/* ------------------------------------------------------------------ */


void startMidiLearn(cb_midiLearn *cb, void *data) 
{
	cb_learn = cb;
	cb_data  = data;
}


/* ------------------------------------------------------------------ */


void stopMidiLearn() 
{
	cb_learn = NULL;
	cb_data  = NULL;
}


/* ------------------------------------------------------------------ */


void setApi(int _api) 
{
	api = api;
	gLog("[KM] using system 0x%x\n", api);
}


/* ------------------------------------------------------------------ */


int openOutDevice(int port) 
{
	try {
		midiOut = new RtMidiOut((RtMidi::Api) api, "Giada MIDI Output");
		kernelMidi::midiStatus = true;
  }
  catch (RtMidiError &error) {
    gLog("[KM] MIDI out device error: %s\n", error.getMessage().c_str());
    kernelMidi::midiStatus = false;
    return 0;
  }

	/* print output ports */

	numOutPorts = midiOut->getPortCount();
  gLog("[KM] %d output MIDI ports found\n", numOutPorts);
  for (unsigned i=0; i<numOutPorts; i++)
		gLog("  %d) %s\n", i, getOutPortName(i));

	/* try to open a port, if enabled */

	if (port != -1 && numOutPorts > 0) {
		try {
			midiOut->openPort(port, getOutPortName(port));
			gLog("[KM] MIDI out port %d open\n", port);
			return 1;
		}
		catch (RtMidiError &error) {
			gLog("[KM] unable to open MIDI out port %d: %s\n", port, error.getMessage().c_str());
			kernelMidi::midiStatus = false;
			return 0;
		}
	}
	else
		return 2;
}


/* ------------------------------------------------------------------ */


int openInDevice(int port)
{
	try {
		midiIn = new RtMidiIn((RtMidi::Api) api, "Giada MIDI input");
		kernelMidi::midiStatus = true;
  }
  catch (RtMidiError &error) {
    gLog("[KM] MIDI in device error: %s\n", error.getMessage().c_str());
    kernelMidi::midiStatus = false;
    return 0;
  }

	/* print input ports */

	numInPorts = midiIn->getPortCount();
  gLog("[KM] %d input MIDI ports found\n", numInPorts);
  for (unsigned i=0; i<numInPorts; i++)
		gLog("  %d) %s\n", i, getInPortName(i));

	/* try to open a port, if enabled */

	if (port != -1 && numInPorts > 0) {
		try {
			midiIn->openPort(port, getInPortName(port));
			midiIn->ignoreTypes(true, false, true); // ignore all system/time msgs, for now
			gLog("[KM] MIDI in port %d open\n", port);
			midiIn->setCallback(&callback);
			return 1;
		}
		catch (RtMidiError &error) {
			gLog("[KM] unable to open MIDI in port %d: %s\n", port, error.getMessage().c_str());
			kernelMidi::midiStatus = false;
			return 0;
		}
	}
	else
		return 2;
}


/* ------------------------------------------------------------------ */


bool hasAPI(int API) 
{
	std::vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* ------------------------------------------------------------------ */


const char *getOutPortName(unsigned p) 
{
	try { return midiOut->getPortName(p).c_str(); }
	catch (RtMidiError &error) { return NULL; }
}

const char *getInPortName(unsigned p) 
{
	try { return midiIn->getPortName(p).c_str(); }
	catch (RtMidiError &error) { return NULL; }
}


/* ------------------------------------------------------------------ */


void send(uint32_t data) 
{
	if (!kernelMidi::midiStatus)
		return;

	//std::vector<unsigned char> msg(1, 0x00);
	//msg[0] = getB1(data);
	//msg[1] = getB2(data);
	//msg[2] = getB3(data);

  std::vector<unsigned char> msg(1, getB1(data));
  msg.push_back(getB2(data));
  msg.push_back(getB3(data));

	midiOut->sendMessage(&msg);
	//gLog("[KM] send msg=0x%X (%X %X %X)\n", data, msg[0], msg[1], msg[2]);
}


/* ------------------------------------------------------------------ */


void send(int b1, int b2, int b3) 
{
	if (!kernelMidi::midiStatus)
		return;

	std::vector<unsigned char> msg(1, b1);

	if (b2 != -1)
		msg.push_back(b2);
	if (b3 != -1)
		msg.push_back(b3);

	midiOut->sendMessage(&msg);
	//gLog("[KM] send msg=(%X %X %X)\n", b1, b2, b3);
}


/* ------------------------------------------------------------------ */


void callback(double t, std::vector<unsigned char> *msg, void *data) 
{
	/* 0.8.0 - for now we handle other midi signals (common and real-time
	 * messages) as unknown, for debugging purposes */

	if (msg->size() < 3) {
#if 0
		gLog("[KM] MIDI received - unkown signal - size=%d, value=0x", (int) msg->size());
		for (unsigned i=0; i<msg->size(); i++)
			gLog("%X", (int) msg->at(i));
		gLog("\n");
#endif
		return;
	}

	/* in this place we want to catch two things: a) note on/note off
	 * from a keyboard and b) knob/wheel/slider movements from a
	 * controller */

	uint32_t input = getIValue(msg->at(0), msg->at(1), msg->at(2));
	uint32_t chan  = input & 0x0F000000;
	uint32_t value = input & 0x0000FF00;
	uint32_t pure  = input & 0xFFFF0000;   // input without 'value' byte

	gLog("[KM] MIDI received - 0x%X (chan %d)", input, chan >> 24);

	/* start dispatcher. If midi learn is on don't parse channels, just
	 * learn incoming midi signal. Otherwise process master events first,
	 * then each channel in the stack. This way incoming signals don't
	 * get processed by control::* when midi learning is on. */

	if (cb_learn)	{
		gLog("\n");
		cb_learn(pure, cb_data);
	}
	else {

		/* process master events */

		if      (pure == G_Conf.midiInRewind) {
			gLog(" >>> rewind (global) (pure=0x%X)", pure);
			control::rewindSeq();
		}
		else if (pure == G_Conf.midiInStartStop) {
			gLog(" >>> startStop (global) (pure=0x%X)", pure);
			control::startStopSeq();
		}
		else if (pure == G_Conf.midiInActionRec) {
			gLog(" >>> actionRec (global) (pure=0x%X)", pure);
			control::startStopActionRec();
		}
		else if (pure == G_Conf.midiInInputRec) {
			gLog(" >>> inputRec (global) (pure=0x%X)", pure);
			control::startStopInputRec(false, false);   // update gui, no popup messages
		}
		else if (pure == G_Conf.midiInMetronome) {
			gLog(" >>> metronome (global) (pure=0x%X)", pure);
			control::startStopMetronome(false);
		}
		else if (pure == G_Conf.midiInVolumeIn) {
			float vf = (value >> 8)/127.0f;
			gLog(" >>> input volume (global) (pure=0x%X, value=%d, float=%f)", pure, value >> 8, vf);
			control::setInVol(vf, false);
		}
		else if (pure == G_Conf.midiInVolumeOut) {
			float vf = (value >> 8)/127.0f;
			gLog(" >>> output volume (global) (pure=0x%X, value=%d, float=%f)", pure, value >> 8, vf);
			control::setOutVol(vf, false);
		}
		else if (pure == G_Conf.midiInBeatDouble) {
			gLog(" >>> sequencer x2 (global) (pure=0x%X)", pure);
			control::beatsMultiply();
		}
		else if (pure == G_Conf.midiInBeatHalf) {
			gLog(" >>> sequencer /2 (global) (pure=0x%X)", pure);
			control::beatsDivide();
		}

		/* process channels */

		for (unsigned i=0; i<G_Mixer.channels.size; i++) {

			Channel *ch = (Channel*) G_Mixer.channels.at(i);

			if (ch->midiIn) {

				if (pure == ch->midiInKeyPress) {
					gLog(" >>> keyPress, ch=%d (pure=0x%X)", ch->index, pure);
					control::keyPress(ch, false, false);
				}
				else if (pure == ch->midiInKeyRel) {
					gLog(" >>> keyRel ch=%d (pure=0x%X)", ch->index, pure);
					control::keyRelease(ch, false, false);
				}
				else if (pure == ch->midiInMute) {
					gLog(" >>> mute ch=%d (pure=0x%X)", ch->index, pure);
					control::setMute(ch, false);
				}
				else if (pure == ch->midiInSolo) {
					gLog(" >>> solo ch=%d (pure=0x%X)", ch->index, pure);
					ch->solo ? control::setSoloOn(ch, false) : control::setSoloOff(ch, false);
				}
				else if (pure == ch->midiInVolume) {
					float vf = (value >> 8)/127.0f;
					gLog(" >>> volume ch=%d (pure=0x%X, value=%d, float=%f)", ch->index, pure, value >> 8, vf);
					control::setChanVol(ch, vf, false);
				}
				else if (pure == ((SampleChannel*)ch)->midiInPitch) {
					float vf = (value >> 8)/(127/4.0f); // [0-127] ~> [0.0 4.0]
					gLog(" >>> pitch ch=%d (pure=0x%X, value=%d, float=%f)", ch->index, pure, value >> 8, vf);
					control::setPitch((SampleChannel*)ch, vf, false);
				}
				else if (pure == ((SampleChannel*)ch)->midiInReadActions) {
					gLog(" >>> start/stop read actions ch=%d (pure=0x%X)", ch->index, pure);
					control::startStopReadingRecs((SampleChannel*)ch, false);
				}
			}
			if( ch->tunnelIn ) {
				// convert any noteon at 0 velocity to noteoff
				if( (input & MIDI_NOTE_ON) && (((input>>8)&0xff) == 0) )
					input = (input & ~0x10000000) | 0x00004000;
				if( recorder::active ) {
        				gLog(" >>> tunnel ch=%d (input=0x%X)", ch->index, input);
        				((MidiChannel*)ch)->recvMidi(input);
				} 
				else 
				if( ((MidiChannel*)ch)->midiOut ) {
        			gLog(" >>> echo ch=%d (input=0x%X)", ch->index, input);
					send((uint32_t)(input & MIDI_CHANNEL_MASK) | MIDI_CHANS[((MidiChannel*)ch)->midiOutChan] );
				}
			}	
		}
		gLog("\n");
	}
}


/* ------------------------------------------------------------------ */


std::string getRtMidiVersion()
{
	return midiOut->getVersion();
}


/* ------------------------------------------------------------------ */


std::string getMidiFileVersion()
{
	return "x.xx";
}

}  // namespace
