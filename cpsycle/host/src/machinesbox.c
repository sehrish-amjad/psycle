/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machinesbox.h"
/* host */
#include "paramviews.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void machinesbox_on_destroyed(MachinesBox*);
static void machinesbox_clearmachinebox(MachinesBox*);
static void machinesbox_build(MachinesBox*);
static void machinesbox_insertslot(MachinesBox*, uintptr_t slot, psy_audio_Machine*);
static void machinesbox_insertmachine(MachinesBox*, uintptr_t slot, psy_audio_Machine*);
static int machinesbox_checkmachinemode(MachinesBox*, psy_audio_Machine*);
static void machinesbox_onmachineselected(MachinesBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinesbox_onmachinesinsert(MachinesBox*, psy_audio_Machines* sender, uintptr_t slot);
static void machinesbox_onmachinesremoved(MachinesBox*, psy_audio_Machines*, uintptr_t slot);
static void machinesbox_onlistboxselected(MachinesBox*, psy_ui_Component* sender,
	intptr_t slot);
static void machinesbox_on_key_down(MachinesBox*, psy_ui_KeyboardEvent*);
static void machinesbox_on_key_up(MachinesBox*, psy_ui_KeyboardEvent*);
static bool machinesbox_testmachinemode(const MachinesBox*, uintptr_t index);
static psy_audio_Machine* machinesbox_machine_at(MachinesBox*,
	uintptr_t listbox_index);
static uintptr_t machinesbox_machine_id_at(MachinesBox*, uintptr_t listbox_index);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachinesBox* self)
{
	if (!vtable_initialized) {
		vtable = *(machinesbox_base(self)->vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			machinesbox_on_destroyed;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			machinesbox_on_key_down;
		vtable.onkeyup =
			(psy_ui_fp_component_on_key_event)
			machinesbox_on_key_up;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(machinesbox_base(self), &vtable);
}

/* implementation */
void machinesbox_init(MachinesBox* self, psy_ui_Component* parent,
	psy_audio_Machines* machines, MachineBoxMode mode, Workspace* workspace)
{	
	assert(self);
	
	psy_ui_listbox_init_multi_select(&self->listbox, parent);
	vtable_init(self);	
	self->workspace = workspace;	
	self->mode = mode;
	self->showslots = 1;
	psy_table_init(&self->listboxslots);
	psy_table_init(&self->slotslistbox);	
	psy_ui_component_set_style_type(machinesbox_base(self), STYLE_BOX);		
	machinesbox_set_machines(self, machines);
	psy_signal_connect(&self->listbox.signal_selchanged, self,
		machinesbox_onlistboxselected);	
	psy_signal_connect(&self->listbox.component.signal_key_down, self,
		machinesbox_on_key_down);
	psy_signal_connect(&self->listbox.component.signal_key_up, self,
		machinesbox_on_key_up);
}

void machinesbox_on_destroyed(MachinesBox* self)
{
	psy_table_dispose(&self->listboxslots);
	psy_table_dispose(&self->slotslistbox);
}

void machinesbox_build(MachinesBox* self)
{
	machinesbox_clearmachinebox(self);
	if (self->showslots) {
		int slot;
		int start;
		int end;

		start = self->mode == MACHINEBOX_FX ? 0x40 : 0;
		end = self->mode == MACHINEBOX_ALL ? 0xFF : start + 0x3F;

		for (slot = start; slot <= end; ++slot) {
			machinesbox_insertslot(self, slot, psy_audio_machines_at(self->machines, slot));
		}
	} else {
		psy_TableIterator it;
	
		for (it = psy_audio_machines_begin(self->machines); 
				!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			machinesbox_insertmachine(self, psy_tableiterator_key(&it), machine);
		}
	}
}

void machinesbox_insertslot(MachinesBox* self, uintptr_t slot, psy_audio_Machine* machine)
{
		uintptr_t listboxindex;

		char buffer[128];
		psy_snprintf(buffer, 128, "%02X:", slot);
		if (machine && machine->vtable->info(machine)) {
			strcat(buffer, machine->vtable->info(machine)->shortname); 
		} else {
			strcat(buffer, ""); 
		}
		listboxindex = psy_ui_listbox_add_text(&self->listbox, buffer);
		psy_table_insert(&self->listboxslots, listboxindex, (void*)slot);
		psy_table_insert(&self->slotslistbox, slot, (void*) listboxindex);
}

void machinesbox_insertmachine(MachinesBox* self, uintptr_t slot, psy_audio_Machine* machine)
{	
	if (machinesbox_checkmachinemode(self, machine) && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->shortname) {
		machinesbox_insertslot(self, slot, machine);
	}	
}

int machinesbox_checkmachinemode(MachinesBox* self, psy_audio_Machine* machine)
{
	if (!machine) {
		return 0;
	}
	if (self->mode == MACHINEBOX_FX && 
		psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) {
		return 0;
	}
	if (self->mode == MACHINEBOX_GENERATOR &&
		psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX) {
		return 0;
	}
	if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_MASTER) {
		return 0;
	}
	return 1;
}

void machinesbox_clearmachinebox(MachinesBox* self)
{
	psy_ui_listbox_clear(&self->listbox);
	psy_table_clear(&self->listboxslots);
	psy_table_clear(&self->slotslistbox);	
}

void machinesbox_onlistboxselected(MachinesBox* self, psy_ui_Component* sender,
	intptr_t sel)
{	
	if (psy_table_exists(&self->listboxslots, sel)) {
		uintptr_t slot;

		slot = (uintptr_t)psy_table_at(&self->listboxslots, sel);
		if (self->machines) {
			// prevent self notify to keep multi selection
			psy_signal_disconnect(&self->machines->signal_slotchange, self,
				machinesbox_onmachineselected);
			psy_audio_machines_select(self->machines, slot);
			psy_signal_connect(&self->machines->signal_slotchange, self,
				machinesbox_onmachineselected);
		}
	}
}

void machinesbox_onmachinesinsert(MachinesBox* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	if (machinesbox_checkmachinemode(self,
			psy_audio_machines_at(self->machines, slot))) {
		uintptr_t boxindex;

		machinesbox_build(self);
		boxindex = (uintptr_t)psy_table_at(&self->slotslistbox, slot);
		psy_ui_listbox_set_cur_sel(&self->listbox, boxindex);
	}
}

void machinesbox_onmachineselected(MachinesBox* self,
	psy_audio_Machines* sender, uintptr_t slot)
{			
	if (psy_table_exists(&self->slotslistbox, slot)) {
		uintptr_t boxindex;

		boxindex = (uintptr_t)psy_table_at(&self->slotslistbox, slot);
		psy_ui_listbox_set_cur_sel(&self->listbox, boxindex);
	} else {
		psy_ui_listbox_set_cur_sel(&self->listbox, -1);
	}
}

bool machinesbox_testmachinemode(const MachinesBox* self, uintptr_t index)
{
	if (self->mode == MACHINEBOX_ALL) {
		return TRUE;
	}
	if (self->mode == MACHINEBOX_FX && index >= 0x40 && index <= 0x80) {
		return TRUE;
	}
	if (self->mode == MACHINEBOX_GENERATOR && index <= 0x40) {
		return TRUE;
	}
	return FALSE;
}

void machinesbox_onmachinesremoved(MachinesBox* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	if (psy_table_exists(&self->slotslistbox, slot)) {
		machinesbox_build(self);
		psy_ui_listbox_set_cur_sel(&self->listbox,
			psy_audio_machines_selected(sender));
	}
}

void machinesbox_clone(MachinesBox* self)
{
	intptr_t selcount;
	
	selcount = psy_ui_listbox_sel_count(&self->listbox);
	if (selcount) {
		intptr_t selection[256];	
		intptr_t i;
		psy_audio_Machine* srcmachine = 0;

		psy_ui_listbox_sel_items(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {				
			if (psy_table_exists(&self->listboxslots, selection[i])) {
				uintptr_t slot;
				psy_audio_Machine* machine;
				
				slot = (uintptr_t)psy_table_at(&self->listboxslots,
					selection[i]);
				machine = psy_audio_machines_at(self->machines, slot);
				if (machine && srcmachine == 0) {
					srcmachine = machine;
					break;
				}
			}
		}
		if (srcmachine) {
			for (i = 0; i < selcount; ++i) {				
				if (psy_table_exists(&self->listboxslots,
						selection[i])) {
					uintptr_t slot;
					psy_audio_Machine* machine;
					
					slot = (uintptr_t)psy_table_at(&self->listboxslots,
						selection[i]);
					machine = psy_audio_machines_at(self->machines, slot);
					if (machine != srcmachine) {
						psy_audio_Machine* clone;

						clone = psy_audio_machine_clone(srcmachine);
						if (clone) {
							psy_audio_machine_set_position(clone,
								psy_audio_MACH_AUTO_POSITION,
								psy_audio_MACH_AUTO_POSITION);
							psy_audio_machines_insert(self->machines, slot,
								clone);
						}
					}
				}
			}
		}
	}
}

void machinesbox_remove(MachinesBox* self)
{	
	int selcount;	
	
	selcount = (int)psy_ui_listbox_sel_count(&self->listbox);
	if (selcount > 0) {
		intptr_t* selection;		

		selection = (intptr_t*)malloc(selcount * sizeof(intptr_t));
		if (selection) {
			intptr_t i;

			psy_ui_listbox_sel_items(&self->listbox, selection, selcount);
			for (i = 0; i < selcount; ++i) {				
				psy_audio_machines_remove(self->machines,
					machinesbox_machine_id_at(self, selection[i]), TRUE);
			}
			free(selection);
		}
	}
}

void machinesbox_exchange(MachinesBox* self)
{
	int selcount;	

	selcount = (int)psy_ui_listbox_sel_count(&self->listbox);
	if (selcount > 0) {
		intptr_t selection[256];	
		intptr_t i;
		uintptr_t srcslot = UINTPTR_MAX;

		psy_ui_listbox_sel_items(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {
			uintptr_t mac_id;

			mac_id = machinesbox_machine_id_at(self, selection[i]);
			if (mac_id != psy_INDEX_INVALID) {
				srcslot = mac_id;			
				break;			
			}
		}
		if (srcslot != UINTPTR_MAX) {
			for (i = 0; i < selcount; ++i) {
				uintptr_t mac_id;

				mac_id = machinesbox_machine_id_at(self, selection[i]);
				if (mac_id != psy_INDEX_INVALID && mac_id != srcslot) {		
					psy_audio_machines_exchange(self->machines, srcslot,
						mac_id);
					break;					
				}
			}
		}
	}
}

void machinesbox_muteunmute(MachinesBox* self)
{
	int selcount;

	selcount = (int)psy_ui_listbox_sel_count(&self->listbox);
	if (selcount > 0) {
		intptr_t selection[256];
		intptr_t i;		

		psy_ui_listbox_sel_items(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {
			psy_audio_Machine* machine;
			
			machine = machinesbox_machine_at(self, selection[i]);
			if (machine) {
				if (psy_audio_machine_muted(machine)) {
					psy_audio_machine_unmute(machine);
				} else {
					psy_audio_machine_mute(machine);
				}			
			}
		}
		++self->machines->opcount;
	}
}

void machinesbox_show_parameters(MachinesBox* self)
{	
	int selcount;	
	
	selcount = (int)psy_ui_listbox_sel_count(&self->listbox);
	if (selcount > 0) {
		intptr_t selection[256];
		intptr_t i;

		psy_ui_listbox_sel_items(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {			
			paramviews_show(workspace_param_views(self->workspace),
				machinesbox_machine_id_at(self, selection[i]));
		}
	}
}

void machinesbox_connect_to_master(MachinesBox* self)
{
	int selcount;

	selcount = (int)psy_ui_listbox_sel_count(&self->listbox);
	if (selcount > 0) {
		intptr_t selection[256];
		intptr_t i;

		psy_ui_listbox_sel_items(&self->listbox, selection, selcount);
		for (i = 0; i < selcount; ++i) {
			uintptr_t mac_id;

			mac_id = machinesbox_machine_id_at(self, selection[i]);
			if (mac_id) {
				psy_audio_machines_connect(self->machines,
					psy_audio_wire_make(mac_id, psy_audio_MASTER_INDEX));
			}			
		}
	}
}

psy_audio_Machine* machinesbox_machine_at(MachinesBox* self, uintptr_t listbox_index)
{
	assert(self);
	
	return psy_audio_machines_at(self->machines, machinesbox_machine_id_at(
		self, listbox_index));		
}

uintptr_t machinesbox_machine_id_at(MachinesBox* self, uintptr_t listbox_index)
{
	assert(self);

	if (psy_table_exists(&self->listboxslots, listbox_index)) {	
		return (uintptr_t)psy_table_at(&self->listboxslots,
			listbox_index);		
	}
	return psy_INDEX_INVALID;
}

void machinesbox_set_machines(MachinesBox* self, psy_audio_Machines* machines)
{
	self->machines = machines;
	machinesbox_build(self);
	psy_signal_connect(&self->machines->signal_insert, self,
		machinesbox_onmachinesinsert);
	psy_signal_connect(&self->machines->signal_removed, self,
		machinesbox_onmachinesremoved);
	psy_signal_connect(&self->machines->signal_slotchange, self,
		machinesbox_onmachineselected);
}

void machinesbox_on_key_down(MachinesBox* self, psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);	
}

void machinesbox_on_key_up(MachinesBox* self, psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
}

void machinesbox_add_sel(MachinesBox* self, uintptr_t slot)
{
	/* if (psy_table_exists(&self->slotslistbox, slot)) {
		uintptr_t boxindex;

		boxindex = (uintptr_t)psy_table_at(&self->slotslistbox, slot);
		psy_ui_listbox_addsel(&self->listbox, boxindex);
	} */
}

void machinesbox_deselect_all(MachinesBox* self)
{
	psy_ui_listbox_set_cur_sel(&self->listbox, -1);
}
