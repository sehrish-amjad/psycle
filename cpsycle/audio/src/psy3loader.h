/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PSY3LOADER_H
#define psy_audio_PSY3LOADER_H

#include "instruments.h"
#include "wire.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;

/*!
** @struct psy_audio_PSY3Loader
** @brief Load filter for psy3 songs and instruments.
**
** @details This is the new current main format.
*/
typedef struct psy_audio_PSY3Loader {
	struct psy_audio_SongFile* songfile;
	psy_audio_LegacyWires legacywires;	
	struct PsyFile* fp;
	struct psy_audio_Song* song;
	int progress;
	int32_t seqbus;
} psy_audio_PSY3Loader;

void psy_audio_psy3loader_init(psy_audio_PSY3Loader*,
	struct psy_audio_SongFile*);
void psy_audio_psy3loader_dispose(psy_audio_PSY3Loader*);

int psy_audio_psy3loader_check_machines(psy_audio_PSY3Loader*);

int psy_audio_psy3loader_load(psy_audio_PSY3Loader*);
int psy_audio_psy3loader_loadpsins(psy_audio_PSY3Loader*,
	psy_audio_InstrumentIndex);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY3LOADER_H */
