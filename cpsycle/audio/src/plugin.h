// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PLUGIN_H
#define psy_audio_PLUGIN_H

#include "custommachine.h"
#include "library.h"
#include "logicalchannels.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_Preset;
struct psy_audio_Presets;

typedef struct psy_audio_Plugin {
	psy_audio_CustomMachine custommachine;	
	struct CMachineInterface* mi;
	psy_Library library;	
	psy_audio_MachineInfo* plugininfo;
	unsigned int preventnewline;
	psy_Table parameters;
	struct psy_audio_Presets* presets;
	uintptr_t currprog;
	psy_audio_LogicalChannels logicalchannels;
} psy_audio_Plugin;

int psy_audio_plugin_init(psy_audio_Plugin*, psy_audio_MachineCallback*,
	const char* path, const char* root);
bool psy_audio_plugin_psycle_test(const char* path, const char* root,
	psy_audio_MachineInfo*);

INLINE psy_audio_Machine* psy_audio_plugin_base(psy_audio_Plugin* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLUGIN_H */
