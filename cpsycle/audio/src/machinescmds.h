/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINESCMDS_H
#define psy_audio_MACHINESCMDS_H

/* local */
#include "machines.h"
/* container */
#include <undoredo.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_Property;
struct psy_audio_Machine;

/* InsertMachineCommand */

typedef struct InsertMachineCommand {
	psy_Command command;
	psy_audio_Machines* machines;
	struct psy_audio_Machine* machine;
	uintptr_t slot;
	bool restoreconnection;
	psy_audio_Connections connections;
} InsertMachineCommand;

void insertmachinecommand_dispose(InsertMachineCommand*);

InsertMachineCommand* insertmachinecommand_allocinit(psy_audio_Machines*,
	uintptr_t slot, struct psy_audio_Machine*);

void insertmachinecommand_execute(InsertMachineCommand*, uintptr_t param);
void insertmachinecommand_revert(InsertMachineCommand*);

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	struct psy_audio_Machine* machine;
	uintptr_t slot;
	psy_audio_Connections connections;
} DeleteMachineCommand;

void deletemachinecommand_dispose(DeleteMachineCommand*);

DeleteMachineCommand* deletemachinecommand_allocinit(psy_audio_Machines*,
	uintptr_t slot);

void deletemachinecommand_execute(DeleteMachineCommand*, uintptr_t param);
void deletemachinecommand_revert(DeleteMachineCommand*);


/* ConnectMachineCommand */

typedef struct ConnectMachineCommand {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Wire wire;
	double volume;
	psy_audio_PinMapping pins;
	bool restore;
} ConnectMachineCommand;

void connectmachinecommand_dispose(ConnectMachineCommand*);

ConnectMachineCommand* connectmachinecommand_alloc_init(psy_audio_Machines*,
	psy_audio_Wire);


/* DisconnectMachineCommand */

typedef struct DisconnectMachineCommand {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Wire wire;
	double volume;
	psy_audio_PinMapping pins;
} DisconnectMachineCommand;

void disconnectmachinecommand_dispose(DisconnectMachineCommand*);

DisconnectMachineCommand* disconnectmachinecommand_alloc_init(psy_audio_Machines*,
	psy_audio_Wire);

void disconnectmachinecommand_execute(DisconnectMachineCommand*,
	uintptr_t param);
void disconnectmachinecommand_revert(DisconnectMachineCommand*);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINESCMDS_H */
