//
// C++ Interface: engine.h
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

#include "ge_window.h"
#include "ge_mixed.h"
#include "ge_channel.h"
#include "ge_actionWidget.h"
#include "ge_actionChannel.h"
#include "ge_muteChannel.h"
#include "ge_envelopeChannel.h"
#include "ge_pianoRoll.h"
#include "ge_browser.h"
#include "ge_waveform.h"
#include "gd_browser.h"
#include "gd_keyGrabber.h"
#include "gd_midiGrabber.h"
#include "gd_midiOutputSetup.h"
#include "gd_actionEditor.h"
#include "gd_editor.h"
#include "gd_beatsInput.h"
#include "gd_bpmInput.h"
#include "gd_warnings.h"
#include "gd_config.h"
#include "gd_devInfo.h"
#include "gg_waveTools.h"
#include "gg_keyboard.h"
#include "gg_launcher.h"
#include "gd_mainWindow.h"
