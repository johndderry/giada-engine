//
// C++ Interface: control
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


#ifndef __CONTROL_H__
#define __CONTROL_H__


/*
 *	control::
 *
 *	used to be giadia's glue_* global functions
 */

namespace control {

extern bool quit;
extern bool restart;

/* start, stop, rewind sequencer
 * if gui == true the signal comes from an internal interaction on the
 * GUI, otherwise it's a MIDI/Jack/external signal. */

void startStopSeq(bool gui=false);
void startSeq    (bool gui=false);
void stopSeq     (bool gui=false);
void playAll     ();
void rewindSeq   ();


/* start/stopActionRec
 * handle the action recording. */

void startStopActionRec();
void startActionRec();
void stopActionRec();

/* start/stopInputRec
 * handle the input recording (take). If gui == true the signal comes
 * from an internal interaction on the GUI, otherwise it's a
 * MIDI/Jack/external signal. Alert displays or not the popup message
 * if there are no available channels. */

void startStopInputRec(bool gui=true, bool alert=true);
int  startInputRec    (bool gui=true);
int  stopInputRec     (bool gui=true);

/* start/stopReadingRecs
 * handle the 'R' button. If gui == true the signal comes from an
 * internal interaction on the GUI, otherwise it's a MIDI/Jack/external
 * signal. */

void startStopReadingRecs(class SampleChannel *ch, bool gui=true);
void startReadingRecs    (class SampleChannel *ch, bool gui=true);
void stopReadingRecs     (class SampleChannel *ch, bool gui=true);

void quantize(int val);

void setChanVol(class Channel *ch, float v, bool gui=true);
void setOutVol (float v, bool gui=true);
void setInVol  (float v, bool gui=true);

void startStopMetronome(bool gui=true);

/* beatsDivide/Multiply
 * shrinks or enlarges the number of beats by 2. */

void beatsMultiply();
void beatsDivide();
void breaksMultiply();
void breaksDivide();

/* keyPress / keyRelease
 * handle the key pressure, either via mouse/keyboard or MIDI. If gui
 * is true it means that the event comes from the main window (mouse,
 * keyb or MIDI), otherwise the event comes from the action recorder. */

void keyPress  (class Channel       *ch, bool ctrl=0, bool shift=0);
void keyPress  (class SampleChannel *ch, bool ctrl=0, bool shift=0);
void keyPress  (class MidiChannel   *ch, bool ctrl=0, bool shift=0);
void keyRelease(class Channel       *ch, bool ctrl=0, bool shift=0);
void keyRelease(class SampleChannel *ch, bool ctrl=0, bool shift=0);

/* set Bpm/Beats/Breaks */

//void setBpm(const char *v1, const char *v2);
void setBeats(int beats, int bars, bool expand);
void setBreaks(int breaks);

/* mute
 * set mute on or off. If gui == true the signal comes from an internal
 * interaction on the GUI, otherwise it's a MIDI/Jack/external signal. */

void setMute(class Channel *ch, bool gui=true);

/* solo on/off
 * set solo on and off. If gui == true the signal comes from an internal
 * interaction on the GUI, otherwise it's a MIDI/Jack/external signal. */

void setSoloOn (class Channel *ch, bool gui=true);
void setSoloOff(class Channel *ch, bool gui=true);

void setPitch(class SampleChannel *ch, float val, bool numeric);

/* addChannel
 * add an empty new channel to the stack. Returns the new channel. */

class Channel *addChannel(int column, int type);
class Channel *addChannel(int column, int type, void *);

/* clear... , reset...
 */

void clearAllSamples();
void clearAllRecs();
void resetToInitState();


/* loadChannel
 * fill an existing channel with a wave. */

int loadChannel(class SampleChannel *ch, const char *fname);

void deleteChannel(class Channel *ch);

void freeChannel(class Channel *ch);

/** FIXME - nobody will call these via MIDI/keyb/mouse! */
int loadPatch(const char *fname, const char *fpath, bool isProject);
int savePatch(const char *fullpath, const char *name, bool isProject);
int saveProject(const char *folderPath, const char *projName);

}	// end namespace

#endif
