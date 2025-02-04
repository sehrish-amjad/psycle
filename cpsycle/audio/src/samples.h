/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SAMPLES_H
#define psy_audio_SAMPLES_H

#include "sample.h"

#include <hashtbl.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct psy_audio_SampleIndex
** @brief Addresses a sample inside samples.
**
** @details
** Pair of two indexes (slot; subslot)
** 1. slot:    selects a group
** 2. subslot: selects a slot inside a group pointing
**	           to a sample
**
** Structure:
** psy_audio_Samples <@>----- psy_audio_SamplesGroup <@>----- psy_audio_Sample
**                          *
** Example
**
** Samples
** Groups	Group Samples
** 0 Drums  0 Hi Hat
**          1 Bass
** 1 Piano  0 lower keys
**          1 middle keys
**          2 upper keys
** 2 Guitar 0 Guitar
*/

typedef struct psy_audio_SampleIndex {
	uintptr_t slot;
	uintptr_t subslot;
} psy_audio_SampleIndex;

psy_audio_SampleIndex psy_audio_sampleindex_make(uintptr_t slot, uintptr_t subslot);

INLINE uintptr_t psy_audio_sampleindex_slot(const psy_audio_SampleIndex* self)
{
	assert(self);

	return self->slot;
}

INLINE uintptr_t psy_audio_sampleindex_subslot(
	const psy_audio_SampleIndex* self)
{
	assert(self);

	return self->subslot;
}

INLINE bool psy_audio_sampleindex_invalid(const psy_audio_SampleIndex* self)
{
	assert(self);

	return (self->slot == psy_INDEX_INVALID) ||
		   (self->subslot == psy_INDEX_INVALID);
}

INLINE bool psy_audio_sampleindex_valid(const psy_audio_SampleIndex* self)
{
	return !psy_audio_sampleindex_invalid(self);
}

INLINE psy_audio_SampleIndex psy_audio_sampleindex_zero(void)
{
	return psy_audio_sampleindex_make(psy_INDEX_INVALID,
		psy_INDEX_INVALID);
}

/*!
** @struct psy_audio_SamplesGroup
** @brief Group of samples
*/

typedef struct psy_audio_SamplesGroup {	
	psy_Table container;
} psy_audio_SamplesGroup;

/*!
** @memberof psy_audio_SamplesGroup
** @return begin of the group
*/
psy_TableIterator psy_audio_samplesgroup_begin(psy_audio_SamplesGroup*);

/*!
** @struct psy_audio_Samples
** @brief Container of samples with pregrouping for sampler, sampulse and ft2.
*/

typedef struct psy_audio_Samples {
	psy_Table groups;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_audio_SampleIndex selected;
} psy_audio_Samples;


void psy_audio_samples_init(psy_audio_Samples*);
void psy_audio_samples_dispose(psy_audio_Samples*);

/*!
** @memberof psy_audio_Samples
** inserts the new sample
*/
void psy_audio_samples_insert(psy_audio_Samples*, psy_audio_Sample*,
	psy_audio_SampleIndex);
/*!
** @memberof psy_audio_Samples
** Removes and deallocates the sample at the index
*/
void psy_audio_samples_remove(psy_audio_Samples*, psy_audio_SampleIndex);
/*!
** @memberof psy_audio_Samples
** Removes the sample (not deallocated) at the index
*/
void psy_audio_samples_erase(psy_audio_Samples*, psy_audio_SampleIndex);
/*!
** @memberof psy_audio_Samples
** Return sample at the index or NULL
*/
psy_audio_Sample* psy_audio_samples_at(psy_audio_Samples*,
	psy_audio_SampleIndex);
/*!
** @memberof psy_audio_Samples
** Return const sample at the index or NULL
*/
const psy_audio_Sample* psy_audio_samples_at_const(const psy_audio_Samples*,
	psy_audio_SampleIndex);
/*!
** @memberof psy_audio_Samples
** @return Number of samples in the group of the slot.
*/
uintptr_t psy_audio_samples_size(const psy_audio_Samples*, uintptr_t slot);
/*!
** @memberof psy_audio_Samples
** @return number of all samples
*/
uintptr_t psy_audio_samples_count(const psy_audio_Samples*);
/*!
** @memberof psy_audio_Samples
** @return number of groups
*/
uintptr_t psy_audio_samples_groupsize(const psy_audio_Samples*);
/*!
** @memberof psy_audio_Samples
** @return Iterator of the groups pointing to the start
*/
psy_TableIterator psy_audio_samples_begin(psy_audio_Samples*);
/*!
** @memberof psy_audio_Samples
** @return Iterator of the samples pointing to the start of the group of the slot
*/
psy_TableIterator psy_audio_samples_groupbegin(psy_audio_Samples*,
	uintptr_t slot);
/*!
** @memberof psy_audio_Samples
** @return first free main group slot starting at startslot limiting maxslots
*/
uintptr_t psy_audio_samples_freeslot(psy_audio_Samples*, uintptr_t startslot,
	uintptr_t maxslots);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLES_H */
