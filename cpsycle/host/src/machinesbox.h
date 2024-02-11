/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/


#if !defined(MACHINESBOX_H)
#define MACHINESBOX_H

/* host */
#include "machines.h"
#include "workspace.h"
/* ui */
#include <uilistbox.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct
** @brief Displays the machines of the current song
** 
** @details
** And machine slot used by the pattern and machine view inside a listbox.
** The view is synchronized with 'psy_audio_Machines', which stores the
** machines and the selected machine slot of the song.
*/

typedef enum {
	MACHINEBOX_ALL,
	MACHINEBOX_FX,
	MACHINEBOX_GENERATOR
} MachineBoxMode;

typedef struct MachinesBox {
	psy_ui_ListBox listbox;	
	psy_audio_Machines* machines;
	psy_Table listboxslots;
	psy_Table slotslistbox;
	MachineBoxMode mode;
	int showslots;
	Workspace* workspace;
} MachinesBox;

void machinesbox_init(MachinesBox*, psy_ui_Component* parent,
	psy_audio_Machines*, MachineBoxMode, Workspace*);
void machinesbox_clone(MachinesBox*);
void machinesbox_remove(MachinesBox*);
void machinesbox_exchange(MachinesBox*);
void machinesbox_show_parameters(MachinesBox*);
void machinesbox_muteunmute(MachinesBox*);
void machinesbox_connect_to_master(MachinesBox*);
void machinesbox_set_machines(MachinesBox*, psy_audio_Machines*);
void machinesbox_add_sel(MachinesBox*, uintptr_t slot);
void machinesbox_deselect_all(MachinesBox*);

INLINE psy_ui_Component* machinesbox_base(MachinesBox* self)
{
	return psy_ui_listbox_base(&self->listbox);
}

#ifdef __cplusplus
}
#endif

#endif
