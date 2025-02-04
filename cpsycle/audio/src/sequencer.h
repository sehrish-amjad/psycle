/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCER_H
#define psy_audio_SEQUENCER_H

/* audio */
#include "activechannels.h"
#include "constants.h"
#include "hostsequencertime.h"
#include "sequence.h"
#include "sequencertime.h"
#include "machines.h"
/* container */
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define METRONOME_TRACK 65535

typedef enum {
	psy_audio_SEQUENCERPLAYMODE_PLAYALL,
	psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS,
	psy_audio_SEQUENCERPLAYMODE_PLAYSEL
} psy_audio_SequencerPlayMode;

typedef struct psy_audio_SequencerTrackState {
	psy_dsp_beatpos_t retrigger_offset;
	psy_dsp_beatpos_t retrigger_step;
} psy_audio_SequencerTrackState;

typedef struct psy_audio_SequencerTrack {
	psy_audio_SequenceTrack* track;
	psy_audio_SequencerTrackState state;
	psy_audio_SequenceTrackIterator* iterator;
	uintptr_t channeloffset;
	bool global;
} psy_audio_SequencerTrack;

INLINE psy_audio_SequenceEntry* psy_audio_sequencertrack_entry(
	psy_audio_SequencerTrack* self)
{
	if (self->iterator) {
		return psy_audio_sequencetrackiterator_entry(self->iterator);
	}
	return NULL;
}

typedef struct {
	bool active;
	int count;	
	psy_dsp_beatpos_t offset;
} psy_audio_SequencerLoop;

typedef struct {
	bool active;	
	bool dostop;
	psy_dsp_beatpos_t offset;	
} psy_audio_SequencerJump;

typedef struct {
	bool active;
	double rowspeed; /* line delay */
} psy_audio_SequencerRowDelay;

typedef struct {
	/* metronome */
	bool active;
	bool precounting;
	psy_dsp_beatpos_t precount;
	psy_dsp_beatpos_t currprecount;
} psy_audio_SequencerMetronome;

void psy_audio_sequencermetronome_init(psy_audio_SequencerMetronome*);


/*! 
** \struct psy_audio_Sequencer
** \brief collects the sequence events for the player processing of machines
*/

typedef struct psy_audio_Sequencer {
	/* signals */	
	psy_Signal signal_play_line_changed;
	psy_Signal signal_play_status_changed;
	/*! @internal */
	psy_audio_SequencerTime seqtime;
	psy_audio_HostSequencerTime hostseqtime;
	double beatspersample;
	uintptr_t lpb; /* global */
	double lpbspeed; /* pattern */
	uintptr_t extraticks;
	uintptr_t tpb;	
	double window;	
	psy_List* currtracks;
	psy_audio_PatternNode* events;
	psy_audio_PatternNode* globalevents;
	psy_audio_PatternNode* delayedevents;
	psy_audio_PatternNode* inputevents;
	psy_audio_SequencerPlayMode mode;
	bool looping;
	double linetickcount;
	psy_audio_SequencerJump jump;
	psy_audio_SequencerRowDelay rowdelay;
	psy_audio_SequencerLoop loop;
	psy_Table lastmachine;
	psy_dsp_beatpos_t playbeatloopstart;
	psy_dsp_beatpos_t playbeatloopend;
	psy_dsp_beatpos_t numplaybeats;
	bool calcduration;
	uintptr_t playtrack;	
	psy_audio_PatternNode** qsortarray;
	uintptr_t qsortarraysize;	
	bool auto_note_off;
	/* metronome */
	psy_audio_SequencerMetronome metronome;
	psy_audio_PatternEvent metronome_event;
	psy_audio_PatternEvent sample_event;
	/* references */
	psy_audio_Sequence* sequence;
	psy_audio_Machines* machines;
} psy_audio_Sequencer;

void psy_audio_sequencer_init(psy_audio_Sequencer*, psy_audio_Sequence*,
	psy_audio_Machines*);
void psy_audio_sequencer_dispose(psy_audio_Sequencer*);

void psy_audio_sequencer_reset(psy_audio_Sequencer*, psy_audio_Sequence*,
	psy_audio_Machines*, double samplerate);
uintptr_t psy_audio_sequencer_frame_tick(psy_audio_Sequencer*,
	uintptr_t numsamples);
void psy_audio_sequencer_tick(psy_audio_Sequencer*, double offset);
uintptr_t psy_audio_sequencer_updatelinetickcount(psy_audio_Sequencer*,
	uintptr_t amount);
void psy_audio_sequencer_update_host_seq_time(psy_audio_Sequencer* self,
	bool follow_song);
/* clock */
void psy_audio_sequencer_clock_start(psy_audio_Sequencer*);
void psy_audio_sequencer_clock(psy_audio_Sequencer*);
void psy_audio_sequencer_clock_continue(psy_audio_Sequencer*);
void psy_audio_sequencer_clock_stop(psy_audio_Sequencer*);

void psy_audio_sequencer_on_newline(psy_audio_Sequencer*);
void psy_audio_sequencer_set_position(psy_audio_Sequencer*,
	double position);

INLINE const psy_audio_SequencerTime* psy_audio_sequencer_time(const
	psy_audio_Sequencer* self)
{
	return &self->seqtime;
}

INLINE double psy_audio_sequencer_position(const
	psy_audio_Sequencer* self)
{
	return self->seqtime.position;
}

void psy_audio_sequencer_start(psy_audio_Sequencer*);

INLINE void psy_audio_sequencer_stop(psy_audio_Sequencer* self)
{
	self->seqtime.playstopping = TRUE;
	self->seqtime.playing = FALSE;
	self->seqtime.playstarting = FALSE;	
	self->metronome.precounting = FALSE;
}

void psy_audio_sequencer_set_num_play_beats(psy_audio_Sequencer*,
	psy_dsp_beatpos_t);

INLINE psy_List* psy_audio_sequencer_tick_events(psy_audio_Sequencer* self)
{
	return self->events;
}

psy_List* psy_audio_sequencer_machine_tick_events(psy_audio_Sequencer*,
	uintptr_t slot);
psy_List* psy_audio_sequencer_timed_events(psy_audio_Sequencer*,
	uintptr_t slot, uintptr_t amount);
void psy_audio_sequencer_append(psy_audio_Sequencer*, psy_List* events);
void psy_audio_sequencer_setsamplerate(psy_audio_Sequencer*,
	double samplerate);

INLINE double psy_audio_sequencer_sample_rate(
	const psy_audio_Sequencer* self)
{
	return self->seqtime.samplerate;
}
void psy_audio_sequencer_set_bpm(psy_audio_Sequencer*, double bpm);

INLINE double psy_audio_sequencer_bpm(
	const psy_audio_Sequencer* self)
{
	return self->seqtime.bpm;
}

void psy_audio_sequencer_set_lpb(psy_audio_Sequencer*, uintptr_t lpb);

INLINE uintptr_t psy_audio_sequencer_lpb(const psy_audio_Sequencer* self)
{
	return self->lpb;
}

void psy_audio_sequencer_set_ticks_per_beat(psy_audio_Sequencer*,
	uintptr_t ticks);
void psy_audio_sequencer_set_extra_ticks_per_beat(psy_audio_Sequencer*,
	uintptr_t ticks);

INLINE uintptr_t psy_audio_sequencer_frames(const psy_audio_Sequencer* self,
	double offset)
{
	return (uintptr_t)(offset / self->beatspersample);
}

INLINE double psy_audio_sequencer_frame_to_offset(
	psy_audio_Sequencer* self, uintptr_t numsamples)
{	
	return numsamples * self->beatspersample;
}

INLINE int psy_audio_sequencer_playing(const psy_audio_Sequencer* self)
{
	return self->seqtime.playing;
}

void psy_audio_sequencer_add_input_event(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, uintptr_t track);
void psy_audio_sequencer_record_input_event(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, uintptr_t track, psy_dsp_beatpos_t
	playposition);

INLINE void psy_audio_sequencer_set_play_mode(psy_audio_Sequencer* self,
	psy_audio_SequencerPlayMode mode)
{
	self->mode = mode;
}

INLINE void psy_audio_sequencer_loop(psy_audio_Sequencer* self)
{
	self->looping = TRUE;
}

INLINE void psy_audio_sequencer_stop_loop(psy_audio_Sequencer* self)
{
	self->looping = FALSE;
}

INLINE bool psy_audio_sequencer_looping(const psy_audio_Sequencer* self)
{
	return (self->looping != FALSE);
}

INLINE psy_audio_SequencerPlayMode psy_audio_sequencer_play_mode(
	const psy_audio_Sequencer* self)
{
	return self->mode;
}

INLINE double psy_audio_sequencer_speed(
	const psy_audio_Sequencer* self)
{
	double rv;

	if (self->extraticks != 0) {
		rv = self->lpbspeed * (self->tpb /
			(double)(self->extraticks * self->lpb + self->tpb));
	} else {
		rv = self->lpbspeed;
	}	
	if (self->rowdelay.active) {
		rv *= self->rowdelay.rowspeed;
	}
	if (self->calcduration && rv < 0.1) {
		rv = 1.0;
	}
	return rv;
}

INLINE double psy_audio_sequencer_beats_per_sample(
	const psy_audio_Sequencer* self)
{
	return self->beatspersample;
}

INLINE double psy_audio_sequencer_curr_beats_per_line(
	const psy_audio_Sequencer* self)
{
	return 1.0 / (self->lpb * psy_audio_sequencer_speed(self));		
}

INLINE psy_dsp_seconds_t psy_audio_sequencer_curr_play_time(
	const psy_audio_Sequencer* self)
{
	return psy_audio_sequencertime_play_time(&self->seqtime);	
}

psy_dsp_percent_t psy_audio_sequencer_row_progress(const
	psy_audio_Sequencer* self, uintptr_t track);

psy_audio_SequenceCursor psy_audio_sequencer_play_cursor(
	const psy_audio_Sequencer*);

void psy_audio_sequencer_notify_newline(psy_audio_Sequencer* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCER_H */
