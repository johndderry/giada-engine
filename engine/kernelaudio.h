//
// C++ Interface: kernelaudio
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


#ifndef KERNELAUDIO_H
#define KERNELAUDIO_H


//#include <RtAudio.h>
#include "../rtaudio/RtAudio.h"

#if defined(__linux__)
	#include <jack/jack.h>
	#include <jack/intclient.h>
	#include <jack/transport.h>
#endif


/* -- kernel audio -------------------------------------------------- */
#define SYS_API_JACK	0x01  // 0000 0001
#define SYS_API_ALSA	0x02  // 0000 0010
#define SYS_API_DS	0x04  // 0000 0100
#define SYS_API_ASIO	0x08  // 0000 1000
#define SYS_API_CORE	0x10  // 0001 0000
#define SYS_API_PULSE	0x20  // 0010 0000
#define SYS_API_ANY     0x3F  // 0011 1111

#define KERNEL_OK               0
#define KERNEL_UNDERRUN         -1
#define KERNEL_CRITICAL         -2

#define DEFAULT_SOUNDDEV_OUT    0     /// FIXME - please override with rtAudio::getDefaultDevice (or similar)
#define DEFAULT_SOUNDDEV_IN     -1	// no recording by default: input disabled


namespace kernelAudio {

	/* namespace variables declared as extern in header file */

	extern RtAudio		*system;
	extern unsigned		numDevs;
	extern bool 		inputEnabled;
	extern unsigned		realBufsize; 	// real bufsize from the soundcard
	extern int		api;
	extern bool		audioStatus;

	int openDevice(
			int api,
			int outDev,
			int inDev,
			int outChan,
			int inChan,
			int samplerate,
			int buffersize);
	int closeDevice();

	int startStream();
	int stopStream();

	bool		isProbed       (unsigned dev);
	bool		isDefaultIn    (unsigned dev);
	bool		isDefaultOut   (unsigned dev);
	const char	*getDeviceName  (unsigned dev);
	unsigned	getMaxInChans  (int dev);
	unsigned	getMaxOutChans (unsigned dev);
	unsigned	getDuplexChans (unsigned dev);
	int		getTotalFreqs  (unsigned dev);
	int		getFreq        (unsigned dev, int i);
	int		getDeviceByName(const char *name);
	int		getDefaultOut  ();
	int		getDefaultIn   ();
	bool		hasAPI         (int API);

	std::string getRtAudioVersion();

#ifdef __linux__
	jack_client_t *jackGetHandle();
	void jackStart();
	void jackStop();
	void jackSetSyncCb();
	int  jackSyncCb(jack_transport_state_t state, jack_position_t *pos, void *arg);
#endif

}

#endif
