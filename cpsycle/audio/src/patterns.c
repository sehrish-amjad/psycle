/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patterns.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void psy_audio_patterns_init_signals(psy_audio_Patterns*);
static void psy_audio_patterns_init_global(psy_audio_Patterns*);
static void psy_audio_patterns_dispose_signals(psy_audio_Patterns*);
static uintptr_t psy_audio_patterns_firstfreeslot(psy_audio_Patterns*);
static void psy_audio_patterns_disposeslots(psy_audio_Patterns*);
static void psy_audio_patterns_onpatternnamechanged(psy_audio_Patterns*,
	psy_audio_Pattern* sender);
static void psy_audio_patterns_onpatternlengthchanged(psy_audio_Patterns*,
	psy_audio_Pattern* sender);
/* implementation */
void psy_audio_patterns_init(psy_audio_Patterns* self)
{
	assert(self);

	psy_table_init(&self->slots);
	self->songtracks = 16;	
	self->sharetracknames = TRUE;
	psy_table_init(&self->shared_track_names);
	psy_audio_trackstate_init(&self->trackstate);
	psy_audio_patterns_init_signals(self);	
	psy_audio_patterns_init_global(self);
}

void psy_audio_patterns_init_signals(psy_audio_Patterns* self)
{
	assert(self);

	psy_signal_init(&self->signal_numsongtrackschanged);
	psy_signal_init(&self->signal_name_changed);
	psy_signal_init(&self->signal_tracknamechanged);
	psy_signal_init(&self->signal_length_changed);
}

void psy_audio_patterns_init_global(psy_audio_Patterns* self)
{
	psy_audio_Pattern* pattern;
	psy_audio_PatternEntry entry;
	psy_audio_PatternEvent e;
	
	pattern = psy_audio_patterns_insert(self, psy_audio_GLOBALPATTERN,
		psy_audio_pattern_alloc_init());
	psy_audio_patternentry_init(&entry);
	psy_audio_patternevent_init_all(&e, psy_audio_NOTECOMMANDS_TIMESIG,
		0, 0, 0, 4, 4);
	psy_audio_patternentry_set_event(&entry, e, 0);
	psy_audio_pattern_insert(pattern, NULL, 1, psy_dsp_beatpos_zero(), &entry);
	psy_audio_patternentry_dispose(&entry);
}

void psy_audio_patterns_dispose(psy_audio_Patterns* self)
{	
	assert(self);

	psy_audio_patterns_disposeslots(self);
	psy_audio_trackstate_dispose(&self->trackstate);
	psy_table_dispose_all(&self->shared_track_names, (psy_fp_disposefunc)NULL);
	psy_audio_patterns_dispose_signals(self);
}

void psy_audio_patterns_dispose_signals(psy_audio_Patterns* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_numsongtrackschanged);
	psy_signal_dispose(&self->signal_name_changed);
	psy_signal_dispose(&self->signal_tracknamechanged);
	psy_signal_dispose(&self->signal_length_changed);
}

void psy_audio_patterns_disposeslots(psy_audio_Patterns* self)
{	
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->slots);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Pattern* pattern;
		
		pattern = (psy_audio_Pattern*)psy_tableiterator_value(&it);
		psy_audio_pattern_dispose(pattern);
		free(pattern);
	}
	psy_table_dispose(&self->slots);
	psy_signal_disconnect_all(&self->signal_name_changed);
}

void psy_audio_patterns_clear(psy_audio_Patterns* self)
{
	assert(self);

	psy_audio_patterns_disposeslots(self);
	psy_table_init(&self->slots);
	psy_table_dispose_all(&self->shared_track_names, (psy_fp_disposefunc)NULL);
	psy_table_init(&self->shared_track_names);
	psy_audio_patterns_init_global(self);	
}

psy_audio_Pattern* psy_audio_patterns_insert(psy_audio_Patterns* self,
	uintptr_t slot, psy_audio_Pattern* pattern)
{
	assert(self);
	
	psy_audio_patterns_remove(self, slot);
	psy_table_insert(&self->slots, slot, pattern);
	psy_signal_connect(&pattern->signal_name_changed, self,
		psy_audio_patterns_onpatternnamechanged);
	psy_signal_connect(&pattern->signal_length_changed, self,
		psy_audio_patterns_onpatternlengthchanged);
	return pattern;
}

uintptr_t psy_audio_patterns_append(psy_audio_Patterns* self, psy_audio_Pattern* pattern)
{
	uintptr_t slot;
	
	assert(self);

	slot = psy_audio_patterns_firstfreeslot(self);
	psy_audio_patterns_insert(self, slot, pattern);
	return slot;
}

uintptr_t psy_audio_patterns_firstfreeslot(psy_audio_Patterns* self)
{
	uintptr_t rv = 0;

	assert(self);

	/* find first free slot */
	while (psy_table_exists(&self->slots, rv)) {
		++rv;
	}
	return rv;
}

psy_audio_Pattern* psy_audio_patterns_at(psy_audio_Patterns* self,
	uintptr_t slot)
{
	return psy_table_at(&self->slots, slot);
}

const psy_audio_Pattern* psy_audio_patterns_at_const(const psy_audio_Patterns* self,
	uintptr_t slot)
{
	return psy_table_at_const(&self->slots, slot);
}

uintptr_t psy_audio_patterns_slot(psy_audio_Patterns* self,
	psy_audio_Pattern* pattern)
{
	uintptr_t rv;
	psy_TableIterator it;

	assert(self);

	rv = psy_INDEX_INVALID;
	for (it = psy_table_begin(&self->slots);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		if (pattern == psy_tableiterator_value(&it)) {
			rv = psy_tableiterator_key(&it);
			break;
		}
	}
	return rv;
}

void psy_audio_patterns_erase(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	assert(self);

	pattern = psy_audio_patterns_at(self, slot);
	psy_table_remove(&self->slots, slot);
	if (pattern) {
		psy_signal_disconnect(&pattern->signal_name_changed, self,
			psy_audio_patterns_onpatternnamechanged);
		psy_signal_disconnect(&pattern->signal_length_changed, self,
			psy_audio_patterns_onpatternlengthchanged);
	}
}

void psy_audio_patterns_remove(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	assert(self);

	pattern = psy_audio_patterns_at(self, slot);
	psy_table_remove(&self->slots, slot);
	if (pattern) {
		psy_audio_pattern_dispose(pattern);
		free(pattern);
	}
}

uintptr_t psy_audio_patterns_size(psy_audio_Patterns* self)
{
	assert(self);

	return psy_table_size(&self->slots);
}

void psy_audio_patterns_activate_solo_track(psy_audio_Patterns* self,
	uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_activatesolotrack(&self->trackstate, track);
}

void psy_audio_patterns_set_solo_track(psy_audio_Patterns* self,
	uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_setsolotrack(&self->trackstate, track);
}

void psy_audio_patterns_deactivate_solo_track(psy_audio_Patterns* self)
{
	assert(self);

	psy_audio_trackstate_deactivatesolotrack(&self->trackstate);
}

void psy_audio_patterns_mute_track(psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_mutetrack(&self->trackstate, track);
}

void psy_audio_patterns_unmute_track(psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_unmutetrack(&self->trackstate, track);
}

bool psy_audio_patterns_is_track_muted(const psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istrackmuted(&self->trackstate, track);
}

bool psy_audio_patterns_is_track_soloed(const psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istracksoloed(&self->trackstate, track);
}

uintptr_t psy_audio_patterns_track_soloed(const psy_audio_Patterns* self)
{
	assert(self);

	return psy_audio_trackstate_tracksoloed(&self->trackstate);
}

void psy_audio_patterns_arm_track(psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_armtrack(&self->trackstate, track);	
}

void psy_audio_patterns_unarm_track(psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_unarmtrack(&self->trackstate, track);	
}

bool psy_audio_patterns_is_track_armed(const psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istrackarmed(&self->trackstate, track);
}

uintptr_t psy_audio_psy_audio_patterns_track_armed_count(const
	psy_audio_Patterns* self)
{
	return psy_audio_trackstate_trackarmedcount(&self->trackstate);
}

void psy_audio_patterns_set_num_tracks(psy_audio_Patterns* self,
	uintptr_t num_tracks)
{
	assert(self);

	self->songtracks = num_tracks;
	psy_signal_emit(&self->signal_numsongtrackschanged, self, 0);
}

uintptr_t psy_audio_patterns_num_tracks(const psy_audio_Patterns* self)
{
	assert(self);

	return self->songtracks;
}

void psy_audio_patterns_onpatternnamechanged(psy_audio_Patterns* self,
	psy_audio_Pattern* sender)
{
	uintptr_t slot;
	
	assert(self);

	slot = psy_audio_patterns_slot(self, sender);
	if (slot != psy_INDEX_INVALID) {
		psy_signal_emit(&self->signal_name_changed, self, 1, slot);
	}	
}

void psy_audio_patterns_onpatternlengthchanged(psy_audio_Patterns* self,
	psy_audio_Pattern* sender)
{
	uintptr_t slot;

	assert(self);

	slot = psy_audio_patterns_slot(self, sender);
	if (slot != psy_INDEX_INVALID) {
		psy_signal_emit(&self->signal_length_changed, self, 1, slot);
	}
}

void psy_audio_patterns_set_shared_track_name(psy_audio_Patterns* self,
	uintptr_t track_index, const char* name)
{
	char* curr;
	
	assert(self);
	
	curr = (char*)psy_table_at(&self->shared_track_names, track_index);
	if ((curr == NULL) && (name == NULL)) {
		return;
	}
	if ((curr != NULL) && (name != NULL) && (strcmp(name, curr) == 0)) {
		return;
	}
	free(curr);
	curr = NULL;
	if (name) {
		psy_table_insert(&self->shared_track_names, track_index,
			psy_strdup(name));
	}
	psy_signal_emit(&self->signal_tracknamechanged, self, 1, track_index);
}

const char* psy_audio_patterns_shared_track_name(const psy_audio_Patterns* self,
	uintptr_t track_index)
{	
	assert(self);
	
	return (char*)psy_table_at_const(&self->shared_track_names, track_index);	
}

const char* psy_audio_patterns_track_name(const psy_audio_Patterns* self,
	uintptr_t slot, uintptr_t track)
{
	const psy_audio_Pattern* pattern;
	
	assert(self);
	
	if ((self->sharetracknames) || (slot == psy_INDEX_INVALID)) {
		return psy_audio_patterns_shared_track_name(self, track);
	}
	pattern = psy_audio_patterns_at_const(self, slot);
	if (pattern) {
		return psy_audio_pattern_track_name(pattern, track);
	}
	return NULL;
}

void psy_audio_patterns_share_names(psy_audio_Patterns* self)
{
	assert(self);
	
	self->sharetracknames = TRUE;
	psy_signal_emit(&self->signal_tracknamechanged, self, 1, psy_INDEX_INVALID);
}

void psy_audio_patterns_use_individual_names(psy_audio_Patterns* self)
{
	assert(self);
	
	self->sharetracknames = FALSE;
	psy_signal_emit(&self->signal_tracknamechanged, self, 1, psy_INDEX_INVALID);
}
