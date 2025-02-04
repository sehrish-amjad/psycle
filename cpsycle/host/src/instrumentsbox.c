/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instrumentsbox.h"
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

#define FT2_INSTRUMENTNAME_LEN 22

static void instrumentsbox_buildlist(InstrumentsBox*);
static void instrumentsbox_buildgroup(InstrumentsBox*);
static void instrumentsbox_add_string(InstrumentsBox*, const char* text);
static void instrumentsbox_oninstrumentslotchanged(InstrumentsBox*,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentsbox_on_instrument_insert(InstrumentsBox*,
	psy_ui_Component* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentsbox_on_instrument_removed(InstrumentsBox*,
	psy_ui_Component* sender, const psy_audio_InstrumentIndex* slot);
static void instrumentsbox_on_group_list_changed(InstrumentsBox*,
	psy_ui_Component* sender, int slot);
static void instrumentsbox_onlistchanged(InstrumentsBox*,
	psy_ui_Component* sender, int slot);
static const char* instrumentsbox_groupname(InstrumentsBox*,
	uintptr_t groupslot);

void instrumentsbox_init(InstrumentsBox* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments)
{		
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	/* Groups */
	psy_ui_component_init(&self->groupheader, &self->component, NULL);
	psy_ui_component_set_align(&self->groupheader, psy_ui_ALIGN_TOP);	
	psy_ui_label_init_text(&self->header, &self->component,
		"instrumentsbox.instrument-groups");
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);	
	psy_ui_listbox_init(&self->grouplist, &self->component);
	psy_ui_component_set_maximum_size(&self->grouplist.component,
		psy_ui_size_make_em(0.0, 10.0));
	psy_ui_component_set_minimum_size(&self->grouplist.component,
		psy_ui_size_make_em(0.0, 10.0));
	psy_ui_component_set_align(&self->grouplist.component, psy_ui_ALIGN_TOP);
	/* Instruments */
	psy_ui_component_init(&self->instheader, &self->component, NULL);
	psy_ui_component_set_align(&self->instheader, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->group, &self->component,
		"instrumentsbox.group-instruments");
	psy_ui_component_set_align(&self->group.component, psy_ui_ALIGN_TOP);
	psy_ui_listbox_init(&self->instrumentlist, &self->component);	
	psy_ui_component_set_align(&self->instrumentlist.component,
		psy_ui_ALIGN_CLIENT);
	instrumentsbox_set_instruments(self, instruments);
	psy_signal_connect(&self->grouplist.signal_selchanged, self,
		instrumentsbox_on_group_list_changed);
	psy_signal_connect(&self->instrumentlist.signal_selchanged, self,
		instrumentsbox_onlistchanged);
}

void instrumentsbox_buildgroup(InstrumentsBox* self)
{
	assert(self);
	
	if (self->instruments) {
		uintptr_t slot = 0;
		char text[40];

		psy_ui_listbox_clear(&self->grouplist);		
		for (; slot < 256; ++slot) {
			if (psy_audio_instruments_group_at(self->instruments, slot)) {
				psy_snprintf(text, 20, "%02X*:%s", slot,
					instrumentsbox_groupname(self, slot));
			} else {
				psy_snprintf(text, 20, "%02X:%s", slot,
					instrumentsbox_groupname(self, slot));
			}
			psy_ui_listbox_add_text(&self->grouplist, text);
		}
	}
}

void instrumentsbox_buildlist(InstrumentsBox* self)
{	
	uintptr_t groupslot;
	uintptr_t slot = 0;
	char buffer[FT2_INSTRUMENTNAME_LEN + 4];

	assert(self);
	
	psy_ui_listbox_clear(&self->instrumentlist);
	groupslot = psy_ui_listbox_cur_sel(&self->grouplist);
	if (groupslot == psy_INDEX_INVALID) {
		groupslot = 0;
	}
	for ( ; slot < 256; ++slot) {
		psy_audio_Instrument* instrument;
		
		if ((instrument = psy_audio_instruments_at(self->instruments,
			psy_audio_instrumentindex_make(groupslot, slot)))) {
			psy_snprintf(buffer, sizeof(buffer), "%02X*:%s", slot,
				psy_audio_instrument_name(instrument));
		} else {
			psy_snprintf(buffer, sizeof(buffer), "%02X:%s", slot, "");
		}
		instrumentsbox_add_string(self, buffer);
	}
}

void instrumentsbox_add_string(InstrumentsBox* self, const char* text)
{
	psy_ui_listbox_add_text(&self->instrumentlist, text);
}

void instrumentsbox_on_group_list_changed(InstrumentsBox* self,
	psy_ui_Component* sender, int slot)
{
	psy_audio_InstrumentIndex index;

	assert(self);
	
	index = instrumentsbox_selected(self);
	instrumentsbox_buildlist(self);
	psy_ui_listbox_set_cur_sel(&self->grouplist, index.group);
	psy_ui_listbox_set_cur_sel(&self->instrumentlist, index.subslot);
	psy_audio_instruments_select(self->instruments, index);
}

void instrumentsbox_onlistchanged(InstrumentsBox* self, psy_ui_Component*
	sender, int slot)
{
	uintptr_t groupslot;
	
	assert(self);

	psy_signal_disconnect(&self->instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
	groupslot = psy_ui_listbox_cur_sel(&self->grouplist);
	if (groupslot == psy_INDEX_INVALID) {
		groupslot = 0;
	}
	psy_audio_instruments_select(self->instruments,
		psy_audio_instrumentindex_make(groupslot, slot));
	psy_signal_connect(&self->instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
}

void instrumentsbox_on_instrument_insert(InstrumentsBox* self, psy_ui_Component*
	sender, const psy_audio_InstrumentIndex* slot)
{	
	psy_audio_Instrument* instrument;
	char buffer[FT2_INSTRUMENTNAME_LEN + 4];

	assert(self);
	assert(slot);
	assert(!psy_audio_instrumentindex_invalid(slot));
	
	if ((instrument = psy_audio_instruments_at(self->instruments, *slot))) {
		psy_snprintf(buffer, sizeof(buffer), "%02X*:%s", (int)slot->subslot,
			psy_audio_instrument_name(instrument));
	} else {
		psy_snprintf(buffer, sizeof(buffer), "%02X:%s", (int)slot->subslot, "");
	}
	psy_ui_listbox_set_text(&self->instrumentlist, buffer, slot->subslot);

	if (psy_audio_instruments_group_at(self->instruments, slot->group)) {
		psy_snprintf(buffer, 20, "%02X*:%s", slot->group,
			instrumentsbox_groupname(self, slot->group));
	} else {
		psy_snprintf(buffer, 20, "%02X:%s", slot->group,
			instrumentsbox_groupname(self, slot->group));
	}
	psy_ui_listbox_set_text(&self->grouplist, buffer, slot->group);
}

void instrumentsbox_on_instrument_removed(InstrumentsBox* self,
	psy_ui_Component* sender, const psy_audio_InstrumentIndex* slot)
{	
	char buffer[FT2_INSTRUMENTNAME_LEN + 4];	

	assert(self);
	assert(slot);
	assert(!psy_audio_instrumentindex_invalid(slot));	
	
	psy_snprintf(buffer, sizeof(buffer), "%02X:%s", (int)slot->subslot, "");	
	psy_ui_listbox_set_text(&self->instrumentlist, buffer, slot->subslot);

	if (psy_audio_instruments_group_at(self->instruments, slot->group)) {		
		psy_snprintf(buffer, 20, "%02X*:%s", slot->group,
			instrumentsbox_groupname(self, slot->group));
	} else {
		psy_snprintf(buffer, 20, "%02X:%s", slot->group,
			instrumentsbox_groupname(self, slot->group));
	}
	psy_ui_listbox_set_text(&self->grouplist, buffer, slot->group);
}

const char* instrumentsbox_groupname(InstrumentsBox* self, uintptr_t groupslot)
{
	assert(self);
	
	if (groupslot == 0) {
		return "Sampler";
	} else if (groupslot == 1) {
		return "Sampulse";
	}
	return "";	
}

void instrumentsbox_oninstrumentslotchanged(InstrumentsBox* self,
	psy_audio_Instrument* sender, const psy_audio_InstrumentIndex* slot)
{	
	psy_audio_InstrumentIndex curr;
	
	assert(self);
	assert(slot);
	assert(!psy_audio_instrumentindex_invalid(slot));

	curr = instrumentsbox_selected(self);
	if (slot->group != curr.group && slot->subslot != curr.subslot) {
		instrumentsbox_buildlist(self);
		psy_ui_listbox_set_cur_sel(&self->grouplist, slot->group);
		psy_ui_listbox_set_cur_sel(&self->instrumentlist, slot->subslot);
	}
}

void instrumentsbox_set_instruments(InstrumentsBox* self, psy_audio_Instruments*
	instruments)
{
	assert(self);
	
	self->instruments = instruments;
	instrumentsbox_buildgroup(self);
	instrumentsbox_buildlist(self);
	psy_ui_listbox_set_cur_sel(&self->grouplist, 0);
	psy_ui_listbox_set_cur_sel(&self->instrumentlist, 0);	
	psy_signal_connect(&instruments->signal_insert, self,
		instrumentsbox_on_instrument_insert);
	psy_signal_connect(&instruments->signal_removed, self,
		instrumentsbox_on_instrument_removed);
	psy_signal_connect(&instruments->signal_slotchange, self,
		instrumentsbox_oninstrumentslotchanged);
}

psy_audio_InstrumentIndex instrumentsbox_selected(InstrumentsBox* self)
{
	return psy_audio_instrumentindex_make(
		psy_ui_listbox_cur_sel(&self->grouplist),
		psy_ui_listbox_cur_sel(&self->instrumentlist));
}

void instrumentsbox_rebuild(InstrumentsBox* self)
{
	assert(self);
	
	instrumentsbox_buildgroup(self);
	instrumentsbox_buildlist(self);
	if (self->instruments) {
		psy_ui_listbox_set_cur_sel(&self->grouplist,
			psy_audio_instruments_selected(self->instruments).group);
		psy_ui_listbox_set_cur_sel(&self->instrumentlist,
			psy_audio_instruments_selected(self->instruments).subslot);
	}
}
