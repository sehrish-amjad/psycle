/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_LOGICALCHANNELS_H
#define psy_audio_LOGICALCHANNELS_H

#include <hashtbl.h>

#include "constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct psy_audio_LogicalChannels
** @brief Maps an unlimited index space to MAX_TRACKS
**
** @details
** Some Plugins can only handle MAX_TRACKS (64) Channels
** psy_audio_LogicalChannels maps an unlimited index space to this limit. This
** is needed for Multisequence tracks, that add the current SequenceTrack Index
** multiplied with MAX_TRACKS (64) to the patternevent channel nuumber
*/

typedef struct psy_audio_LogicalChannels
{
	bool physical_active[MAX_TRACKS];
	psy_Table logicalmap;
} psy_audio_LogicalChannels;

void psy_audio_logicalchannels_init(psy_audio_LogicalChannels*);
void psy_audio_logicalchannels_dispose(psy_audio_LogicalChannels*);

uintptr_t psy_audio_logicalchannels_physical(psy_audio_LogicalChannels*,
	uintptr_t logical);
void psy_audio_logicalchannels_reset(psy_audio_LogicalChannels*);
	
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LOGICALCHANNELS_H */
