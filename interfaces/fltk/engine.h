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

#include "../../engine/config.h"
#include "../../engine/utility.h"
#include "../../engine/patch.h"
#include "../../engine/wave.h"
#include "../../engine/channel.h"
#include "../../engine/midichannel.h"
#include "../../engine/samplechannel.h"
#include "../../engine/interface.h"
#include "../../engine/control.h"
#include "../../engine/recorder.h"
#include "../../engine/mixer.h"
#include "../../engine/kernelmidi.h"
#include "../../engine/kernelaudio.h"
#include "../../engine/init.h"

#ifdef WITH_VST
#include "../../engine/pluginHost.h"
#endif

