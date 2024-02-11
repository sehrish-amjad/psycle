/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_EVENTDRIVERS_H
#define psy_audio_EVENTDRIVERS_H

/* driver */
#include "../../driver/eventdriver.h"
/* local */
#include "eventdriverplugin.h"
#include "library.h"
/* file */
#include <propertiesio.h>
/* container */
#include <configuration.h>
#include <list.h>
#include <hashtbl.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_EventDrivers
**
** container for all active inputdriver
*/

typedef void (*fp_eventdriver_input)(void* context, psy_EventDriver* sender);

typedef struct psy_audio_EventDrivers {
	psy_List* drivers;
	psy_EventDriver* kbddriver;	
	void* context;
	void* systemhandle;	
	psy_Signal signal_input;
	psy_Property* cmds;
	/* references */
	psy_Configuration* config_;
} psy_audio_EventDrivers;

void psy_audio_eventdrivers_init(psy_audio_EventDrivers*, void* systemhandle,
	psy_Configuration* config, psy_Property* cmddef);
void psy_audio_eventdrivers_dispose(psy_audio_EventDrivers*);

psy_EventDriver* psy_audio_eventdrivers_driver(psy_audio_EventDrivers*,
	uintptr_t index);
uintptr_t psy_audio_eventdrivers_size(psy_audio_EventDrivers*);
void psy_audio_eventdrivers_idle(psy_audio_EventDrivers*);
void psy_audio_eventdrivers_connect(psy_audio_EventDrivers*, void* context,
	fp_eventdriver_input);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_EVENTDRIVERS_H */
