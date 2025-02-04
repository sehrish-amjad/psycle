/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "machinecombobox.h"
#include <songio.h>
#include <containerconvert.h>
#include <plugin_interface.h>
// 
#include "../../detail/portable.h"

static void machinecombobox_on_destroyed(MachineComboBox*);
static void machinecombobox_onmachineboxselchange(MachineComboBox*,
	psy_ui_Component* sender, int sel);
static void machinecombobox_buildmachinebox(MachineComboBox*);
static void machinecombobox_onmachinesinsert(MachineComboBox*, psy_audio_Machines* sender, uintptr_t slot);
static int machinecombobox_insertmachine(MachineComboBox*, size_t slot, psy_audio_Machine*);
static void machinecombobox_onmachinesremoved(MachineComboBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinecombobox_onmachineselect(MachineComboBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinecombobox_on_song_changed(MachineComboBox*, psy_audio_Player* sender);
static void machinecombobox_connectsongsignals(MachineComboBox*);
static void machinecombobox_connectinstrumentsignals(MachineComboBox*);
static void machinecombobox_clearmachinebox(MachineComboBox* self);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineComboBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->machinebox.component.vtable);
		super_vtable = *(self->machinebox.component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			machinecombobox_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->machinebox.component, &vtable);
}

void machinecombobox_init(MachineComboBox* self, psy_ui_Component* parent,
	bool showmaster, Workspace* workspace)
{		
	psy_ui_Margin margin;

	self->workspace = workspace;
	self->sync_machines_select = TRUE;
	self->showmaster = showmaster;
	self->column = psy_INDEX_INVALID;
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);	
	self->machines = &workspace->song->machines_;
	self->instruments = &workspace->song->instruments_;
	psy_table_init(&self->comboboxslots);
	psy_table_init(&self->slotscombobox);	
	/* Machine ComboBox */
	psy_ui_combobox_init(&self->machinebox, parent);
	psy_ui_combobox_set_char_number(&self->machinebox, 10);	
	psy_signal_connect(&self->machinebox.signal_selchanged, self,
		machinecombobox_onmachineboxselchange);
	machinecombobox_buildmachinebox(self);	
	//psy_signal_connect(&workspace->signal_songchanged, self,
		//machinecombobox_onsongchanged);
	//machinecombobox_connectsongsignals(self);		
	psy_signal_init(&self->signal_selected);
}

void machinecombobox_on_destroyed(MachineComboBox* self)
{
	psy_table_dispose(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
	if (self->machines) {
		psy_signal_disconnect(&self->workspace->player_.signal_song_changed,
			self, machinecombobox_on_song_changed);
		psy_signal_disconnect(&self->machines->signal_insert, self,
			machinecombobox_onmachinesinsert);
		psy_signal_disconnect(&self->machines->signal_removed, self,
			machinecombobox_onmachinesremoved);
		psy_signal_disconnect(&self->machines->signal_slotchange, self,
			machinecombobox_onmachineselect);
	}
	psy_signal_dispose(&self->signal_selected);
	super_vtable.on_destroyed(&self->machinebox.component);
}

void machinecombobox_clearmachinebox(MachineComboBox* self)
{
	psy_ui_combobox_clear(&self->machinebox);
	psy_table_dispose(&self->comboboxslots);
	psy_table_init(&self->comboboxslots);
	psy_table_dispose(&self->slotscombobox);
	psy_table_init(&self->slotscombobox);
}

void machinecombobox_onmachinesinsert(MachineComboBox* self, psy_audio_Machines* sender,
	uintptr_t slot)
{	
	//machinecombobox_buildmachinebox(self);
	//psy_ui_combobox_select(&self->machinebox,
		//psy_audio_machines_selected(sender));
}

void machinecombobox_onmachinesremoved(MachineComboBox* self, psy_audio_Machines* sender,
	uintptr_t slot)
{
	//machinecombobox_buildmachinebox(self);
	//psy_ui_combobox_select(&self->machinebox,
		//psy_audio_machines_selected(sender));
}

void machinecombobox_buildmachinebox(MachineComboBox* self)
{
	machinecombobox_clearmachinebox(self);
	if (psy_audio_machines_size(self->machines) == 1) {
		psy_ui_combobox_add_text(&self->machinebox, psy_ui_translate(
			"mv.no-machines-loaded"));
		psy_ui_combobox_select(&self->machinebox, 0);
	} else if (self->machines &&
			(psy_audio_machines_size(self->machines) > 0)) {
		uintptr_t i;

		for (i = 0; i <= psy_audio_machines_maxindex(self->machines); ++i) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, i);
			if (machine) {
				machinecombobox_insertmachine(self, i, machine);
			}
		}
		i = psy_ui_combobox_add_text(&self->machinebox,
			"------------------------------");
		psy_table_insert(&self->comboboxslots, i, (void*)psy_INDEX_INVALID);
		psy_table_insert(&self->slotscombobox, psy_INDEX_INVALID, (void*)i);
	}
}

int machinecombobox_insertmachine(MachineComboBox* self, size_t slot, psy_audio_Machine* machine)
{				
	if ((slot != psy_audio_MASTER_INDEX || self->showmaster) && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->shortname) {
		intptr_t comboboxindex;
		char buffer[128];

		psy_snprintf(buffer, 128, "%02X: %s", slot, 
			machine->vtable->info(machine)->shortname); 
		comboboxindex = psy_ui_combobox_add_text(&self->machinebox, buffer);
		psy_table_insert(&self->comboboxslots, comboboxindex, (void*)slot);
		psy_table_insert(&self->slotscombobox, slot, (void*) comboboxindex);
	}
	return 1;
}

void machinecombobox_onmachineboxselchange(MachineComboBox* self,
	psy_ui_Component* sender, int sel)
{	
	if (self->sync_machines_select) {
		size_t slot;

		slot = (size_t)psy_table_at(&self->comboboxslots, sel);
		psy_audio_machines_select(self->machines, slot);
	}
	psy_signal_emit(&self->signal_selected, self, 0);
}

void machinecombobox_onmachineselect(MachineComboBox* self, psy_audio_Machines* machines,
	uintptr_t slot)
{	
	if (self->sync_machines_select) {
		machinecombobox_select(self, slot);		
	}
}


void machinecombobox_on_song_changed(MachineComboBox* self,
	psy_audio_Player* sender)
{	
	psy_audio_Song* song;
	
	song = psy_audio_player_song(sender);
	if (song) {
		self->machines = &song->machines_;
		self->instruments = &song->instruments_;
	} else {
		self->machines = NULL;
		self->instruments = NULL;
	}
	machinecombobox_connectsongsignals(self);
	machinecombobox_buildmachinebox(self);
}

void machinecombobox_connectsongsignals(MachineComboBox* self)
{
	psy_signal_connect(&self->machines->signal_insert, self,
		machinecombobox_onmachinesinsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		machinecombobox_onmachinesremoved);
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinecombobox_onmachineselect);
}

void machinecombobox_select(MachineComboBox* self, uintptr_t slot)
{
	intptr_t comboboxindex;

	if (psy_table_exists(&self->slotscombobox, slot)) {
		comboboxindex = (intptr_t)psy_table_at(&self->slotscombobox, slot);
	} else {
		comboboxindex = psy_ui_combobox_count(&self->machinebox) - 1;
	}	
	psy_ui_combobox_select(&self->machinebox, comboboxindex);	
}
