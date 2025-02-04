/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instruments.h"
/* std */
#include <stdlib.h>


/* InstrumentIndex */

/* implementation */
psy_audio_InstrumentIndex psy_audio_instrumentindex_make(uintptr_t group,
	uintptr_t subslot)
{
	psy_audio_InstrumentIndex rv;

	rv.group = group;
	rv.subslot = subslot;
	return rv;
}


/* InstrumentsGroup */

/* implementation */
void psy_audio_instrumentsgroup_init(psy_audio_InstrumentsGroup* self)
{
	assert(self);
	
	psy_table_init(&self->container);
}

void psy_audio_instrumentsgroup_dispose(psy_audio_InstrumentsGroup* self)
{
	assert(self);
	
	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)
		psy_audio_instrument_dispose);	
}

psy_audio_InstrumentsGroup* psy_audio_instrumentsgroup_alloc(void)
{
	return (psy_audio_InstrumentsGroup*)malloc(sizeof(
		psy_audio_InstrumentsGroup));
}

psy_audio_InstrumentsGroup* psy_audio_instrumentsgroup_allocinit(void)
{
	psy_audio_InstrumentsGroup* rv;

	rv = psy_audio_instrumentsgroup_alloc();
	if (rv) {
		psy_audio_instrumentsgroup_init(rv);
	}
	return rv;
}

void psy_audio_instrumentsgroup_insert(psy_audio_InstrumentsGroup* self,
	psy_audio_Instrument* instrument, uintptr_t slot)
{
	assert(self);
	
	if (instrument) {
		psy_audio_Instrument* oldinstrument;

		oldinstrument = psy_table_at(&self->container, slot);
		if (oldinstrument) {
			psy_audio_instrument_deallocate(oldinstrument);
		}
		psy_table_insert(&self->container, slot, instrument);
	}
}

void psy_audio_instrumentsgroup_remove(psy_audio_InstrumentsGroup* self,
	uintptr_t slot)
{
	psy_audio_Instrument* instrument;

	assert(self);
	
	instrument = psy_table_at(&self->container, slot);
	if (instrument) {		
		psy_table_remove(&self->container, slot);
		psy_audio_instrument_dispose(instrument);
		free(instrument);
	}
}

psy_audio_Instrument* psy_audio_instrumentsgroup_at(
	psy_audio_InstrumentsGroup* self, uintptr_t slot)
{
	assert(self);
	
	return psy_table_at(&self->container, slot);
}

uintptr_t psy_audio_instrumentsgroup_size(const psy_audio_InstrumentsGroup*
	self)
{
	assert(self);
	
	return psy_table_size(&self->container);
}


/* psy_audio_Instruments */

/* implementation */
void psy_audio_instruments_init(psy_audio_Instruments* self)
{
	assert(self);
	
	psy_table_init(&self->groups_);
	self->selected_ = psy_audio_instrumentindex_make(0, 0);
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);	
}

void psy_audio_instruments_dispose(psy_audio_Instruments* self)
{	
	assert(self);
	
	psy_table_dispose_all(&self->groups_, (psy_fp_disposefunc)
		psy_audio_instrumentsgroup_dispose);
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);
}

void psy_audio_instruments_insert(psy_audio_Instruments* self,
	psy_audio_Instrument* instrument, psy_audio_InstrumentIndex index)
{
	psy_audio_InstrumentsGroup* group;

	assert(self);
	
	group = psy_table_at(&self->groups_, index.group);
	if (!group) {
		group = psy_audio_instrumentsgroup_allocinit();
		if (group) {
			psy_table_insert(&self->groups_, index.group, group);
		}
	}
	if (group) {
		psy_audio_instrumentsgroup_insert(group, instrument, index.subslot);
		psy_signal_emit(&self->signal_insert, self, 1, &index);
	}
}

void psy_audio_instruments_remove(psy_audio_Instruments* self,
	psy_audio_InstrumentIndex index)
{	
	psy_audio_InstrumentsGroup* group;

	assert(self);
	
	group = psy_table_at(&self->groups_, index.group);
	if (group) {
		psy_audio_instrumentsgroup_remove(group, index.subslot);
		if (psy_audio_instrumentsgroup_size(group) == 0) {
			psy_table_remove(&self->groups_, index.group);
			psy_audio_instrumentsgroup_dispose(group);
			free(group);
		}
		psy_signal_emit(&self->signal_removed, self, 1, &index);
	}
}

void psy_audio_instruments_select(psy_audio_Instruments* self,
	psy_audio_InstrumentIndex index)
{
	assert(self);
	
	self->selected_ = index;	
	psy_signal_emit(&self->signal_slotchange, self, 1, &index);
}

void psy_audio_instruments_dec(psy_audio_Instruments* self)
{
	psy_audio_InstrumentIndex index;

	assert(self);
		
	index = psy_audio_instruments_selected(self);
	if (index.subslot > 0) {
		--index.subslot;
		psy_audio_instruments_select(self, index);		
	}
}

void psy_audio_instruments_inc(psy_audio_Instruments* self)
{
	psy_audio_InstrumentIndex index;

	assert(self);

	index = psy_audio_instruments_selected(self);
	++index.subslot;
	psy_audio_instruments_select(self, index);
}

psy_audio_Instrument* psy_audio_instruments_at(psy_audio_Instruments* self,
	psy_audio_InstrumentIndex index)
{
	psy_audio_InstrumentsGroup* group;

	assert(self);
	
	group = psy_table_at(&self->groups_, index.group);
	if (group) {
		return psy_audio_instrumentsgroup_at(group, index.subslot);
	}
	return NULL;
}

uintptr_t psy_audio_instruments_size(psy_audio_Instruments* self,
	uintptr_t group_index)
{
	psy_audio_InstrumentsGroup* group;

	assert(self);
	
	group = psy_table_at(&self->groups_, group_index);
	if (group) {
		return psy_table_size(&group->container);
	}
	return 0;
}

uintptr_t psy_audio_instruments_groupsize(psy_audio_Instruments* self)
{
	assert(self);
	
	return psy_table_size(&self->groups_);
}

psy_TableIterator psy_audio_instruments_begin(psy_audio_Instruments* self)
{
	assert(self);
	
	return psy_table_begin(&self->groups_);
}

psy_TableIterator psy_audio_instruments_groupbegin(psy_audio_Instruments* self,
	uintptr_t slot)
{
	psy_audio_InstrumentsGroup* group;

	assert(self);
	
	group = psy_table_at(&self->groups_, slot);
	if (group) {
		return psy_table_begin(&group->container);
	}
	return tableend;
}

void psy_audio_instruments_insertgroup(psy_audio_Instruments* self,
	psy_audio_InstrumentsGroup* group, uintptr_t group_index)
{
	psy_audio_InstrumentsGroup* oldgroup;

	assert(self);
	
	oldgroup = psy_table_at(&self->groups_, group_index);
	if (oldgroup) {
		psy_table_remove(&self->groups_, group_index);
		psy_audio_instrumentsgroup_dispose(oldgroup);
		free(oldgroup);
	}
	psy_table_insert(&self->groups_, group_index, group);
}

void psy_audio_instruments_removegroup(psy_audio_Instruments* self,
	uintptr_t group_index)
{
	psy_audio_InstrumentsGroup* group;

	assert(self);
	
	group = psy_table_at(&self->groups_, group_index);
	if (group) {
		psy_audio_InstrumentIndex index;

		psy_table_remove(&self->groups_, group_index);
		psy_audio_instrumentsgroup_dispose(group);
		free(group);
		group = NULL;
		index = psy_audio_instrumentindex_make(group_index, 0);
		psy_signal_emit(&self->signal_removed, self, 1, &index);
	}	
}

psy_audio_InstrumentsGroup* psy_audio_instruments_group_at(
	psy_audio_Instruments* self, uintptr_t group_index)
{
	assert(self);
	
	return (psy_audio_InstrumentsGroup*)psy_table_at(&self->groups_,
		group_index);
}
