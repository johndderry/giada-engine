//
// C++ Implementation: control
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

#include <FL/Fl.H>
#include <FL/x.H>

#include "engine.h"


namespace control {


bool quit = false;
bool restart = false;


void startStopSeq(bool gui)
{
	G_Mixer.running ? stopSeq(gui) : startSeq(gui);
}


/* ------------------------------------------------------------------ */


void startSeq(bool gui)
{
	G_Mixer.running = true;

	if (gui) {
#ifdef __linux__
		kernelAudio::jackStart();
#endif
	}

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M) {
		kernelMidi::send(MIDI_START, -1, -1);
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
	}

	if( G_Interface )
		G_Interface->updatePlay(1, G_Interface->controller);
		
}


/* ------------------------------------------------------------------ */


void stopSeq(bool gui) {

	mh_stopSequencer();

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_STOP, -1, -1);

#ifdef __linux__
	if (gui)
		kernelAudio::jackStop();
#endif

	/* what to do if we stop the sequencer and some action recs are active?
	* Deactivate the button and delete any 'rec on' status */

	if (recorder::active) {
		recorder::active = false;
		if( G_Interface )		
			G_Interface->updateRecAction(0, G_Interface->controller);
			
	}

	/* if input recs are active (who knows why) we must deactivate them.
	* One might stop the sequencer while an input rec is running. */

	if (G_Mixer.chanInput != NULL) {
		mh_stopInputRec();
		if( G_Interface ) 	
			G_Interface->updateRecInput(0, G_Interface->controller);
			
	}

	if( G_Interface ) 	
		G_Interface->updatePlay(0, G_Interface->controller);
		
}


/* ------------------------------------------------------------------ */


void rewindSeq() {
	mh_rewindSequencer();
	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}


/* ------------------------------------------------------------------ */


void playAll() {

	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE) {
			if( ((SampleChannel*)G_Mixer.channels.at(i))->mode & LOOP_ANY ) 
				G_Mixer.channels.at(i)->start(0,false);
		} else {
			G_Mixer.channels.at(i)->start(0,false);		
		}
}


/* ------------------------------------------------------------------ */


void startStopActionRec() {
	recorder::active ? stopActionRec() : startActionRec();
}


/* ------------------------------------------------------------------ */


void startActionRec() {
	if (kernelAudio::audioStatus == false)
		return;
	if (!G_Mixer.running)
		startSeq();	        // start the sequencer for convenience
	recorder::active = true;

	if( G_Interface ) 
		G_Interface->updateRecAction(1, G_Interface->controller);
		
}


/* ------------------------------------------------------------------ */


void stopActionRec() {

	/* stop the recorder and sort new actions */

	recorder::active = false;
	recorder::sortActions();

	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE) {
			SampleChannel *ch = (SampleChannel*) G_Mixer.channels.at(i);
			if (ch->hasActions)
				ch->readActions = true;
			else
				ch->readActions = false;
			if( G_Interface ) 
				G_Interface->setChannelWithActions(ch->guiChannel, 
					G_Interface->keyboard);
		}

	if( G_Interface ) 
		G_Interface->updateRecAction(0, G_Interface->controller);
		

	/* in case acton editor is on, refresh it */

	//gu_refreshActionEditor();
}


/* ------------------------------------------------------------------ */


void startStopReadingRecs(SampleChannel *ch, bool gui) {
	if (ch->readActions)
		stopReadingRecs(ch, gui);
	else
		startReadingRecs(ch, gui);
}


/* ------------------------------------------------------------------ */


void startReadingRecs(SampleChannel *ch, bool gui) {
	if (G_Conf.treatRecsAsLoops)
		ch->recStatus = REC_WAITING;
	else
		ch->setReadActions(true);

	if (!gui && G_Interface ) 
		G_Interface->updateActionButton(1, ch->guiChannel);

}


/* ------------------------------------------------------------------ */


void stopReadingRecs(SampleChannel *ch, bool gui) {

	/* if "treatRecsAsLoop" wait until the sequencer reaches beat 0, so put
	* the channel in REC_ENDING status */

	if (G_Conf.treatRecsAsLoops)
		ch->recStatus = REC_ENDING;
	else
		ch->setReadActions(false);

	if (!gui && G_Interface ) 
		G_Interface->updateActionButton(0, ch->guiChannel);

}


/* ------------------------------------------------------------------ */


void quantize(int val) {
	G_Mixer.quantize = val;
	G_Mixer.updateQuanto();
}


/* ------------------------------------------------------------------ */


void setChanVol(Channel *ch, float v, bool gui) {

	ch->volume = v;

	/* also update wave editor if it's shown */

#if 0
	gdEditor *editor = (gdEditor*) gu_getSubwindow(mainWin, WID_SAMPLE_EDITOR);
	if (editor) {
		setVolEditor(editor, (SampleChannel*) ch, v, false);
		Fl::lock();
		editor->volume->value(v);
		Fl::unlock();
	}
#endif

	if (!gui && G_Interface ) {
		//ch->guiChannel->vol->value(v);
		G_Interface->setChanVol( ch->guiChannel, v, NULL );
	}
}


/* ------------------------------------------------------------------ */


void setOutVol(float v, bool gui) {
	G_Mixer.outVol = v;
	if (!gui && G_Interface ) 	
		G_Interface->setOutVol(v, G_Interface->inout);
		
}


/* ------------------------------------------------------------------ */


void setInVol(float v, bool gui)
{
	G_Mixer.inVol = v;
	if (!gui && G_Interface ) 	
		G_Interface->setInVol(v, G_Interface->inout);
		
}


/* ------------------------------------------------------------------ */


void clearAllSamples()
{
	G_Mixer.running = false;
	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		G_Mixer.channels.at(i)->empty();
		if( G_Interface ) 
			G_Interface->reset(G_Mixer.channels.at(i)->guiChannel, NULL);
	}
	recorder::init();
	return;
}


/* ------------------------------------------------------------------ */


void clearAllRecs()
{
	recorder::init();
	//gu_update_controls();
}


/* ------------------------------------------------------------------ */


void resetToInitState()
{
	G_Mixer.ready = false;

	mh_clear();
	if( G_Interface ) 
		G_Interface->clear( G_Interface->controller );

	recorder::init();

	G_Patch.setDefault();
	G_Mixer.init();
#ifdef WITH_VST
	G_PluginHost.freeAllStacks();
#endif
//	if (resetGui)
//		gu_update_controls();

	G_Mixer.ready = true;
}


/* ------------------------------------------------------------------ */


void startStopMetronome(bool gui)
{
	G_Mixer.metronome = !G_Mixer.metronome;
	if (!gui && G_Interface ) 		
		G_Interface->updateMetronome(G_Mixer.metronome, G_Interface->controller);
		
}


/* ------------------------------------------------------------------ */


void setMute(Channel *ch, bool gui)
{
	if (recorder::active && recorder::canRec(ch)) {
		if (!ch->mute)
			recorder::startOverdub(ch->index, ACTION_MUTES, G_Mixer.actualFrame);
		else
		recorder::stopOverdub(G_Mixer.actualFrame);
	}

	ch->mute ? ch->unsetMute(false) : ch->setMute(false);

	if (!gui && G_Interface) 	
		G_Interface->setChanMute( ch->guiChannel, ch->mute, G_Interface->keyboard );	
	
}


/* ------------------------------------------------------------------ */

static bool __soloSession__ = false;

void setSoloOn(Channel *ch, bool gui)
{
	/* if there's no solo session, store mute configuration of all chans
	* and start the session */

	if (!__soloSession__) {
		for (unsigned i=0; i<G_Mixer.channels.size; i++) {
			Channel *och = G_Mixer.channels.at(i);
			och->mute_s  = och->mute;
		}
		__soloSession__ = true;
	}

	ch->solo = !ch->solo;

	/* mute all other channels and unmute this (if muted) */

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {
		Channel *och = G_Mixer.channels.at(i);
		if (!och->solo && !och->mute) {
			och->setMute(false);
			if( G_Interface ) 			
				G_Interface->setChanMute( och->guiChannel, true, G_Interface->keyboard );			
			
		}
	}

	if (ch->mute) {
		ch->unsetMute(false);
		if( G_Interface ) 			
			G_Interface->setChanMute( ch->guiChannel, false, G_Interface->keyboard );		
	}

	if (!gui && G_Interface ) 	
		G_Interface->setChanSolo( ch->guiChannel, true, G_Interface->keyboard );
		
}


/* ------------------------------------------------------------------ */


void setSoloOff(Channel *ch, bool gui)
{
	/* if this is uniqueSolo, stop solo session and restore mute status,
	* else mute this */

	if (mh_uniqueSolo(ch)) {
		__soloSession__ = false;
		for (unsigned i=0; i<G_Mixer.channels.size; i++) {
			Channel *och = G_Mixer.channels.at(i);
			if (och->mute_s) {
				och->setMute(false);
				if( G_Interface ) 				
					G_Interface->setChanMute( och->guiChannel, true, G_Interface->keyboard );					
				
			}
			else {
				och->unsetMute(false);
				if( G_Interface ) {
					
					G_Interface->setChanMute( och->guiChannel, true, G_Interface->keyboard );
					
				}
			}
			och->mute_s = false;
		}
	}
	else {
		ch->setMute(false);
		if( G_Interface ) 			
			G_Interface->setChanMute( ch->guiChannel, true, G_Interface->keyboard );
	}

	ch->solo = !ch->solo;

	if (!gui && G_Interface )
		G_Interface->setChanSolo( ch->guiChannel, false, G_Interface->keyboard );
		
}


/* ------------------------------------------------------------------ */


void setPanning(class gdEditor *win, SampleChannel *ch, float val)
{
	if (val < 1.0f) {
		ch->panLeft = 1.0f;
		ch->panRight= 0.0f + val;

		char buf[8];
		sprintf(buf, "%d L", abs((ch->panRight * 100.0f) - 100));
//		win->panNum->value(buf);
	}
	else if (val == 1.0f) {
		ch->panLeft = 1.0f;
		ch->panRight= 1.0f;
//	  win->panNum->value("C");
	}
	else {
		ch->panLeft = 2.0f - val;
		ch->panRight= 1.0f;

		char buf[8];
		sprintf(buf, "%d R", abs((ch->panLeft * 100.0f) - 100));
//		win->panNum->value(buf);
	}
//	win->panNum->redraw();
}


/* ------------------------------------------------------------------ */


void startStopInputRec(bool gui, bool alert)
{
	if (G_Mixer.chanInput == NULL) {
		if (!startInputRec(gui)) {
			if (alert && G_Interface ) 
				G_Interface->alert("No channels available for recording.", 
							G_Interface->controller );
			gLog("[CONTROL] no channels available for recording\n");
		}
	}
	else
		stopInputRec(gui);
}



/* ------------------------------------------------------------------ */


int startInputRec(bool gui)
{
	if (kernelAudio::audioStatus == false)
		return -1;

	SampleChannel *ch = mh_startInputRec();
	if (ch == NULL && G_Interface )	{          // no chans available		
		G_Interface->updateRecInput(0, G_Interface->controller);		
		return 0;
	}

	if (!G_Mixer.running) {
		startSeq();
		if( G_Interface )
			G_Interface->updatePlay(1, G_Interface->controller);
			
	}

	setChanVol(ch, 1.0f, false); // false = not from gui click

	if( G_Interface ) 
		G_Interface->labelWave( ch->guiChannel, ch->name.c_str(), 
				G_Interface->controller );

	if (!gui && G_Interface ) 		
		G_Interface->updateRecInput(1, G_Interface->controller);


	return 1;

}


/* ------------------------------------------------------------------ */


int stopInputRec(bool gui)
{
	SampleChannel *ch = mh_stopInputRec();

	if (ch->mode & (LOOP_BASIC | LOOP_ONCE | LOOP_REPEAT))
		ch->start(0, true);  // on frame 0: user-generated event

	if (!gui && G_Interface) 		
		G_Interface->updateRecInput(0, G_Interface->controller);
		
	return 1;
}


/* ------------------------------------------------------------------ */


int savePatch(const char *fullpath, const char *name, bool isProject)
{
	if (G_Patch.write(fullpath, name, isProject) == 1) {
		strcpy(G_Patch.name, name);
		G_Patch.name[strlen(name)] = '\0';
		//gu_update_win_label(name);
		gLog("[CONTROL] patch saved as %s\n", fullpath);
		return 1;
	}
	else
		return 0;
}


/* ------------------------------------------------------------------ */


int saveProject(const char *folderPath, const char *projName)
{
	if (gIsProject(folderPath)) {
		gLog("[CONTROL] the project folder already exists\n");
		// don't exit
	}
	else if (!gMkdir(folderPath)) {
		gLog("[CONTROL] unable to make project directory!\n");
		return 0;
	}

	/* copy all samples and midi channels to files inside the project folder */

	for (unsigned i=0; i<G_Mixer.channels.size; i++) {

		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE) {

			/* Takes and logical ones are saved	* via saveSample() */
			SampleChannel *ch = (SampleChannel*) G_Mixer.channels.at(i);

			if (ch->wave == NULL)
				continue;

			/* update the new samplePath: everything now comes from the
			* project folder (folderPath) */

			char samplePath[PATH_MAX];
			sprintf(samplePath, "%s%s%s.%s", folderPath, gGetSlash().c_str(), ch->basename().c_str(), ch->extension().c_str());

			gLog("[control] save project sample file %s\n", samplePath );

			/* remove any existing file */

			if (gFileExists(samplePath))
				remove(samplePath);
			if (ch->save(samplePath))
				ch->pathfile = samplePath;
		}
		else if (G_Mixer.channels.at(i)->type == CHANNEL_MIDI) {

			MidiChannel *ch = (MidiChannel*) G_Mixer.channels.at(i);

			/* update the new samplePath: everything now comes from the
			* project folder (folderPath) */

			char samplePath[PATH_MAX];
			sprintf(samplePath, "%s%s%s.%s", folderPath, gGetSlash().c_str(), ch->basename().c_str(), ch->extension().c_str());

			gLog("[control] save project midi file %s\n", samplePath );

			/* remove any existing file */

			if (gFileExists(samplePath))
				remove(samplePath);
			if (ch->save(samplePath))
				ch->pathfile = samplePath;
		}
	}

	char gptcPath[PATH_MAX];
	sprintf(gptcPath, "%s%s%s.gptc", folderPath, gGetSlash().c_str(), gStripExt(projName).c_str());
	savePatch(gptcPath, projName, true); // true == it's a project

	return 1;
}


/* ------------------------------------------------------------------ */


void keyPress(Channel *ch, bool ctrl, bool shift)
{
	if (ch->type == CHANNEL_SAMPLE)
		keyPress((SampleChannel*)ch, ctrl, shift);
	else
		keyPress((MidiChannel*)ch, ctrl, shift);
}


/* ------------------------------------------------------------------ */


void keyRelease(Channel *ch, bool ctrl, bool shift)
{
	if (ch->type == CHANNEL_SAMPLE)
		keyRelease((SampleChannel*)ch, ctrl, shift);
}


/* ------------------------------------------------------------------ */


void keyPress(MidiChannel *ch, bool ctrl, bool shift)
{
	if (ctrl)
		setMute(ch);
	else
	if (shift)
		ch->kill(0);        // on frame 0: user-generated event
	else
		ch->start(0, true); // on frame 0: user-generated event
}


/* ------------------------------------------------------------------ */


void keyPress(SampleChannel *ch, bool ctrl, bool shift)
{
	/* case CTRL */

	if (ctrl)
		setMute(ch);

	/* case SHIFT
	*
	* action recording on:
	* 		if seq is playing, rec a killchan
	* action recording off:
	* 		if chan has recorded events:
	* 		|	 if seq is playing OR channel 'c' is stopped, de/activate recs
	* 		|	 else kill chan
	*		else kill chan */

	else
	if (shift) {
		if (recorder::active) {
			if (G_Mixer.running) {
				ch->kill(0); // on frame 0: user-generated event
				if (recorder::canRec(ch) && !(ch->mode & LOOP_ANY))   // don't record killChan actions for LOOP channels
					recorder::rec(ch->index, ACTION_KILLCHAN, G_Mixer.actualFrame);
			}
		}
		else {
			if (ch->hasActions) {
				if (G_Mixer.running || ch->status == STATUS_OFF)
					ch->readActions ? stopReadingRecs(ch) : startReadingRecs(ch);
				else
					ch->kill(0);  // on frame 0: user-generated event
			}
			else
				ch->kill(0);    // on frame 0: user-generated event
		}
	}

	/* case no modifier */

	else {

		/* record now if the quantizer is off, otherwise let mixer to handle it
		* when a quantoWait has passed. Moreover, KEYPRESS and KEYREL are
		* meaningless for loop modes */

		if (G_Mixer.quantize == 0 && recorder::canRec(ch) && !(ch->mode & LOOP_ANY))
		{
			if (ch->mode == SINGLE_PRESS)
				recorder::startOverdub(ch->index, ACTION_KEYS, G_Mixer.actualFrame);
			else
				recorder::rec(ch->index, ACTION_KEYPRESS, G_Mixer.actualFrame);
		}

		ch->start(0, true); // on frame 0: user-generated event
	}

	/* the GUI update is done by gui_refresh() */
}


/* ------------------------------------------------------------------ */


void keyRelease(SampleChannel *ch, bool ctrl, bool shift)
{
	if (!ctrl && !shift) {
		ch->stop();

		/* record a key release only if channel is single_press. For any
		* other mode the KEY REL is meaningless. */

		if (ch->mode == SINGLE_PRESS && recorder::canRec(ch))
			recorder::stopOverdub(G_Mixer.actualFrame);
	}

	/* the GUI update is done by gui_refresh() */

}


/* ------------------------------------------------------------------ */


void setPitch( SampleChannel *ch, float val, bool numeric)
{
	if (numeric) {
		if (val <= 0.0f)
			val = 0.1000f;
		if (val > 4.0f)
			val = 4.0000f;
	}

	ch->setPitch(val);
}


/* ------------------------------------------------------------------ */


Channel *addChannel(int column, int type)
{
	Channel *ch    = G_Mixer.addChannel(type);

	if( G_Interface ) 
		ch->guiChannel = G_Interface->addChannel(column, (void *)ch, G_Interface->keyboard);
	else 
		ch->guiChannel = (void * )0;
	setChanVol(ch, 1.0, false); // false = not from gui click
	return ch;
}



/* ------------------------------------------------------------------ */


Channel *addChannel(int column, int type, void *p)
{
	Channel *ch    = G_Mixer.addChannel(type);

	if( G_Interface ) 
		ch->guiChannel = G_Interface->addChannel(column, (void *)ch, p);
	else 
		ch->guiChannel = (void * )0;
	setChanVol(ch, 1.0, false); // false = not from gui click
	return ch;
}



/* ------------------------------------------------------------------ */



int loadPatch(const char *fname, const char *fpath, bool isProject)
{
	/* is it a valid patch? */

	int res = G_Patch.open(fname);
	if (res != PATCH_OPEN_OK)
		return res;


	/* reset the system. False = don't update the gui right now */

	resetToInitState();

	/* mixerHandler will update the samples inside Mixer */

	mh_loadPatch(isProject, fname);

	/* take the patch name and update the main window's title */

	G_Patch.getName();

	G_Patch.readRecs();

#ifdef WITH_VST
	int resPlugins = G_Patch.readPlugins();
#endif

	/* this one is vital: let recorder recompute the actions' positions if
	* the current samplerate != patch samplerate */

	recorder::updateSamplerate(G_Conf.samplerate, G_Patch.samplerate);

	/* save patchPath by taking the last dir of the broswer, in order to
	* reuse it the next time */

	G_Conf.setPath(G_Conf.patchPath, fpath);

	gLog("[CONTROL] patch %s loaded\n", fname);

#ifdef WITH_VST
	if (resPlugins != 1)
		gdAlert("Some VST files were not loaded successfully.");
#endif

	return res;
}



/* ------------------------------------------------------------------ */



void setBeats(int beats, int bars, bool expand)
{
	/* temp vars to store old data (they are necessary) */

	int      oldvalue = G_Mixer.beats;
	unsigned oldfpb		= G_Mixer.totalFrames;

	if (beats > MAX_BEATS)
		G_Mixer.beats = MAX_BEATS;
	else if (beats < 1)
		G_Mixer.beats = 1;
	else
		G_Mixer.beats = beats;

	/* update bars - bars cannot be greate than beats and must be a sub
	* multiple of beats. If not, approximation to the nearest (and greater)
	* value available. */

	if (bars > G_Mixer.beats)
		G_Mixer.bars = G_Mixer.beats;
	else if (bars <= 0)
		G_Mixer.bars = 1;
	else if (beats % bars != 0) {
		G_Mixer.bars = bars + (beats % bars);
		if (beats % G_Mixer.bars != 0) // it could be an odd value, let's check it (and avoid it)
			G_Mixer.bars = G_Mixer.bars - (beats % G_Mixer.bars);
	}
	else
		G_Mixer.bars = bars;

	G_Mixer.updateFrameBars();

	/* update recorded actions */

	if (expand) {
		if (G_Mixer.beats > oldvalue)
			recorder::expand(oldfpb, G_Mixer.totalFrames);
		//else if (G_Mixer.beats < oldvalue)
		//	recorder::shrink(G_Mixer.totalFrames);
	}

	if( G_Interface ) 
		G_Interface->setBeatMeter(G_Mixer.beats, G_Mixer.bars, G_Interface->timing);

	//gu_refreshActionEditor();  // in case the action editor is open
}


/* ------------------------------------------------------------------ */


void setBreaks(int breaks)
{

	if (breaks > MAX_BREAKS)
		G_Mixer.breaks = MAX_BREAKS;
	else if (breaks < 1)
		G_Mixer.breaks = 1;
	else
		G_Mixer.breaks = breaks;

	if( G_Interface ) 
 		G_Interface->setBreakMeter(G_Mixer.breaks, G_Interface->timing);

}


/* ------------------------------------------------------------------ */


/* never expand or shrink recordings (last param of setBeats = false):
* this is live manipulation */

void beatsMultiply()
{
	setBeats(G_Mixer.beats*2, G_Mixer.bars, false);
}

void beatsDivide()
{
	setBeats(G_Mixer.beats/2, G_Mixer.bars, false);
}


/* ------------------------------------------------------------------ */


void breaksMultiply()
{
	setBreaks(G_Mixer.breaks*2);
}

void breaksDivide()
{
	setBreaks(G_Mixer.breaks/2);
}

}	// end namespace
