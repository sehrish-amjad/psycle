/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCEENTRY_H
#define psy_audio_SEQUENCEENTRY_H

/* audio */
#include "samples.h"
/* container */
#include <list.h>
#include <signal.h>
#include <command.h>
/* detail */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_SequenceEntry
**
** item of the playorder list of a song (multisequence)
*/

typedef enum psy_audio_SequenceEntryType {
	psy_audio_SEQUENCEENTRY_PATTERN = 1,
	psy_audio_SEQUENCEENTRY_SAMPLE,
	psy_audio_SEQUENCEENTRY_MARKER
} psy_audio_SequenceEntryType;

/*
** psy_audio_SequenceEntry
**
** Base class entry inside a track of a sequence
*/

struct psy_audio_SequenceEntry;

typedef	void (*psy_audio_fp_sequenceentry_dispose)
	(struct psy_audio_SequenceEntry*);
typedef struct psy_audio_SequenceEntry* (*psy_audio_fp_sequenceentry_clone)
	(const struct psy_audio_SequenceEntry*);
typedef	psy_dsp_beatpos_t (*psy_audio_fp_sequenceentry_length)
	(const struct psy_audio_SequenceEntry*);
typedef	void (*psy_audio_fp_sequenceentry_setlength)
	(struct psy_audio_SequenceEntry*, psy_dsp_beatpos_t);
typedef	const char* (*psy_audio_fp_sequenceentry_label)
	(const struct psy_audio_SequenceEntry*);

typedef struct psy_audio_SequenceEntryVtable {
	psy_audio_fp_sequenceentry_dispose dispose;
	psy_audio_fp_sequenceentry_clone clone;
	psy_audio_fp_sequenceentry_length length;
	psy_audio_fp_sequenceentry_setlength setlength;
	psy_audio_fp_sequenceentry_label label;
} psy_audio_SequenceEntryVtable;

typedef struct psy_audio_SequenceEntry {
	psy_audio_SequenceEntryVtable* vtable;
	psy_audio_SequenceEntryType type;
	/* absolute start position in the song */
	psy_dsp_beatpos_t offset;	
	/* offset to the playlist index position allowing free positioning */
	psy_dsp_beatpos_t repositionoffset;
	uintptr_t row;
	bool selplay;
} psy_audio_SequenceEntry;

void psy_audio_sequenceentry_init_all(psy_audio_SequenceEntry*,
	psy_audio_SequenceEntryType type, psy_dsp_beatpos_t offset);

INLINE psy_audio_SequenceEntry* psy_audio_sequenceentry_clone(
	const psy_audio_SequenceEntry* self)
{
	return self->vtable->clone(self);
}

INLINE psy_dsp_beatpos_t psy_audio_sequenceentry_length(
	const psy_audio_SequenceEntry* self)
{
	return self->vtable->length(self);
}

INLINE void psy_audio_sequenceentry_dispose(psy_audio_SequenceEntry* self)
{
	self->vtable->dispose(self);
}

INLINE void psy_audio_sequenceentry_setlength(
	psy_audio_SequenceEntry* self, psy_dsp_beatpos_t length)
{
	self->vtable->setlength(self, length);
}

INLINE const char* psy_audio_sequenceentry_label(
	const psy_audio_SequenceEntry* self)
{
	return self->vtable->label(self);
}

INLINE psy_dsp_beatpos_t psy_audio_sequenceentry_offset(
	const psy_audio_SequenceEntry* self)
{
	return self->offset;
}

INLINE psy_dsp_beatpos_t psy_audio_sequenceentry_reposition_offset(
	const psy_audio_SequenceEntry* self)
{
	return self->repositionoffset;
}

INLINE psy_dsp_beatpos_t psy_audio_sequenceentry_right_offset(
	const psy_audio_SequenceEntry* self)
{
	return psy_dsp_beatpos_add(self->offset,
		psy_audio_sequenceentry_length(self));
}

struct psy_audio_Patterns;
struct psy_audio_Pattern;

/*!
** @struct psy_audio_SequencePatternEntry
** @brief Entry inside a track of a sequence with a pattern index
*/
typedef struct psy_audio_SequencePatternEntry {
	/*! @extends  */
	psy_audio_SequenceEntry entry;	
	/*! @internal */
	/* playorder value (the pattern to be played) */
	uintptr_t patternslot;
	struct psy_audio_Patterns* patterns;			
} psy_audio_SequencePatternEntry;

void psy_audio_sequencepatternentry_init(psy_audio_SequencePatternEntry*,
	uintptr_t patternslot, psy_dsp_beatpos_t offset);

psy_audio_SequencePatternEntry* psy_audio_sequencepatternentry_alloc(void);
psy_audio_SequencePatternEntry* psy_audio_sequencepatternentry_allocinit(
	uintptr_t patternslot, psy_dsp_beatpos_t offset);

INLINE void psy_audio_sequencepatternentry_setpatternslot(
	psy_audio_SequencePatternEntry* self, uintptr_t slot)
{
	assert(self);

	self->patternslot = slot;
}

INLINE uintptr_t psy_audio_sequencepatternentry_patternslot(const
	psy_audio_SequencePatternEntry* self)
{
	assert(self);

	return self->patternslot;
}

struct psy_audio_Pattern* psy_audio_sequencepatternentry_pattern(const
	psy_audio_SequencePatternEntry* self, struct psy_audio_Patterns* patterns);

/*
** psy_audio_SequenceSampleEntry
**
** Entry inside a track of a sequence with a pattern index
*/
typedef struct psy_audio_SequenceSampleEntry {
	/* inherits*/
	psy_audio_SequenceEntry entry;		
	/*! @internal */
	/* playorder value (the pattern to be played) */	
	psy_audio_Samples* samples;	
	psy_audio_SampleIndex sampleindex;	
	/* psy_INDEX_INVALID = song sampler index */
	uintptr_t samplerindex;
} psy_audio_SequenceSampleEntry;

void psy_audio_sequencesampleentry_init(psy_audio_SequenceSampleEntry*,
	psy_dsp_beatpos_t offset, psy_audio_SampleIndex);

psy_audio_SequenceSampleEntry* psy_audio_sequencesampleentry_alloc(void);
psy_audio_SequenceSampleEntry* psy_audio_sequencesampleentry_allocinit(
	psy_dsp_beatpos_t offset, psy_audio_SampleIndex);

INLINE void psy_audio_sequencesampleentry_setsampleslot(
	psy_audio_SequenceSampleEntry* self, psy_audio_SampleIndex index)
{
	assert(self);

	self->sampleindex = index;
}

INLINE psy_audio_SampleIndex psy_audio_sequencesampleentry_samplesindex(const
	psy_audio_SequenceSampleEntry* self)
{
	assert(self);

	return self->sampleindex;
}

INLINE uintptr_t psy_audio_sequencesampleentry_samplerindex(const
	psy_audio_SequenceSampleEntry* self)
{
	assert(self);

	return self->samplerindex;
}

INLINE void psy_audio_sequencesampleentry_setsamplerindex(
	psy_audio_SequenceSampleEntry* self, uintptr_t index)
{
	assert(self);

	self->samplerindex = index;
}

INLINE psy_audio_Sample* psy_audio_sequencesampleentry_sample(const
	psy_audio_SequenceSampleEntry* self, psy_audio_Samples* samples)
{
	assert(self);

	if (samples) {
		return (psy_audio_Sample*)psy_audio_samples_at(samples,
			self->sampleindex);
	}
	return NULL;
}

/*
** psy_audio_SequenceMarkerEntry
**
** Entry inside a track of a sequence with a label
*/
typedef struct psy_audio_SequenceMarkerEntry {
	/*! @extends  */
	psy_audio_SequenceEntry entry;
	/*! @internal */
	char* text;
	psy_dsp_beatpos_t length;
} psy_audio_SequenceMarkerEntry;

void psy_audio_sequencemarkerentry_init(psy_audio_SequenceMarkerEntry*,
	psy_dsp_beatpos_t offset, const char* text);

psy_audio_SequenceMarkerEntry* psy_audio_sequencemarkerentry_alloc(void);
psy_audio_SequenceMarkerEntry* psy_audio_sequencemarkerentry_allocinit(
	psy_dsp_beatpos_t offset, const char* text);

void psy_audio_sequencemarkerentry_set_text(psy_audio_SequenceMarkerEntry*,
	const char* text);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCEENTRY_H */
