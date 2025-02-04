/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqedittimesig.h"

#ifdef PSYCLE_USE_SEQEDITOR

/* host */
#include "resources/resource.h"
#include "sequencetrackbox.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <patterns.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 10.0

/* SeqEditLoopState */
void seqedittimesigstate_init(SeqEditTimeSigState* self)
{
	seqedittimesigstate_reset(self);
}

void seqedittimesigstate_startdrag(SeqEditTimeSigState* self,
	psy_audio_PatternNode* node)
{
	seqedittimesigstate_reset(self);
	if (node) {
		self->start = node;
		self->drag = TRUE;
	}
}

void seqedittimesigstate_remove(SeqEditTimeSigState* self,
	psy_audio_PatternNode* node)
{
	seqedittimesigstate_startdrag(self, node);
	self->drag = FALSE;
	self->remove = TRUE;
}

void seqedittimesigstate_reset(SeqEditTimeSigState* self)
{
	self->drag = FALSE;
	self->remove = FALSE;
	self->start = NULL;
}

/* SeqEditTimeSig*/
/* prototypes */
static void seqedittimesig_updatepattern(SeqEditTimeSig*);
static void seqedittimesig_ondraw(SeqEditTimeSig*, psy_ui_Graphics*);
static void seqedittimesig_on_mouse_down(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_onmousemove(SeqEditTimeSig*, psy_ui_MouseEvent*);
static void seqedittimesig_on_mouse_up(SeqEditTimeSig*, psy_ui_MouseEvent*);
static uintptr_t seqedittimesig_timesig_index(SeqEditTimeSig*);
/* vtable */
static psy_ui_ComponentVtable seqedittimesig_vtable;
static psy_ui_ComponentVtable seqedittimesig_supervtable;
static bool seqedittimesig_vtable_initialized = FALSE;

static void seqedittimesig_vtable_init(SeqEditTimeSig* self)
{
	if (!seqedittimesig_vtable_initialized) {
		seqedittimesig_vtable = *(self->component.vtable);
		seqedittimesig_supervtable = seqedittimesig_vtable;
		seqedittimesig_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqedittimesig_ondraw;
		seqedittimesig_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			seqedittimesig_on_mouse_down;
		seqedittimesig_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			seqedittimesig_onmousemove;
		seqedittimesig_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			seqedittimesig_on_mouse_up;
		seqedittimesig_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittimesig_vtable;
}

/* implementation */
void seqedittimesig_init(SeqEditTimeSig* self, psy_ui_Component* parent,
	SeqEditTimeSigState* timesigstate, SeqEditState* state,
	psy_audio_PatternNode* node)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, NULL);
	seqedittimesig_vtable_init(self);
	psy_ui_component_set_style_type(&self->component,
		STYLE_SEQEDT_LOOP);
	psy_ui_component_set_style_type_hover(&self->component,
		STYLE_SEQEDT_LOOP_HOVER);
	psy_ui_component_set_style_type_active(&self->component,
		STYLE_SEQEDT_LOOP_ACTIVE);
	self->state = state;
	self->timesigstate = timesigstate;
	self->node = node;
	seqedittimesig_updatepattern(self);
}

SeqEditTimeSig* seqedittimesig_alloc(void)
{
	return (SeqEditTimeSig*)malloc(sizeof(SeqEditTimeSig));
}

SeqEditTimeSig* seqedittimesig_allocinit(psy_ui_Component* parent,
	SeqEditTimeSigState* timesigstate, SeqEditState* state,
	psy_audio_PatternNode* node)
{
	SeqEditTimeSig* rv;

	rv = seqedittimesig_alloc();
	if (rv) {
		seqedittimesig_init(rv, parent, timesigstate, state, node);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void seqedittimesig_updatepattern(SeqEditTimeSig* self)
{
	psy_audio_Sequence* sequence;

	self->pattern = NULL;
	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		self->pattern = psy_audio_patterns_at(sequence->patterns,
			psy_audio_GLOBALPATTERN);
	}
	else {
		self->pattern = NULL;
	}
}

void seqedittimesig_updateposition(SeqEditTimeSig* self)
{
	psy_ui_component_set_position(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make_px(
				seqeditstate_beat_to_px(self->state,
					seqedittimesig_offset(self)), 0.0),
			psy_ui_size_make(
				psy_ui_value_make_px(10.0),
				psy_ui_value_make_eh(2.0))));
}

void seqedittimesig_ondraw(SeqEditTimeSig* self, psy_ui_Graphics* g)
{
	if (self->node) {
		psy_audio_PatternEntry* patternentry;
		psy_audio_PatternEvent* e;

		patternentry = (psy_audio_PatternEntry*)self->node->entry;
		e = psy_audio_patternentry_front(patternentry);
		if (e->note == psy_audio_NOTECOMMANDS_TIMESIG) {
			int numerator;
			int denominator;
			double cpx;
			const psy_ui_TextMetric* tm;
			char text[64];

			numerator = e->cmd;
			denominator = e->parameter;
			cpx = 0.0;
			tm = psy_ui_component_textmetric(&self->component);
			psy_snprintf(text, 64, "%d", (int)(numerator));
			psy_ui_graphics_textout(g, psy_ui_realpoint_make(cpx, 0.0), text, psy_strlen(text));
			psy_snprintf(text, 64, "%d", (int)(denominator));
			psy_ui_graphics_textout(g, psy_ui_realpoint_make(cpx, tm->tmHeight), text,
				psy_strlen(text));
		}
	}
}

void seqedittimesig_on_mouse_down(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	if (!self->pattern && !self->node) {
		psy_ui_mouseevent_stop_propagation(ev);
		return;
	}
	seqedittimesigstate_startdrag(self->timesigstate, self->node);
	if (psy_ui_mouseevent_button(ev) == 1) {
		seqedittimesig_select(self);
		psy_ui_component_capture(&self->component);
	}
	else if (psy_ui_mouseevent_button(ev) == 2) {
		seqedittimesigstate_remove(self->timesigstate, self->node);
	}
}

void seqedittimesig_onmousemove(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	psy_dsp_beatpos_t offset;
	psy_ui_RealRectangle position;

	if (!self->timesigstate->drag || !self->pattern ||
		!seqedittimesig_entry_const(self)) {
		return;
	}
	position = psy_ui_component_position(&self->component);
	offset = seqeditstate_quantize(self->state,
		seqeditstate_pxtobeat(self->state,
			psy_ui_mouseevent_pt(ev).x + position.left));
	offset = psy_dsp_beatpos_max(psy_dsp_beatpos_zero(), offset);
	if (psy_dsp_beatpos_not_equal(seqedittimesig_offset(self), offset)) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* e;

		psy_audio_exclusivelock_enter();
		e = psy_audio_patternentry_clone(seqedittimesig_entry(self));
		psy_audio_pattern_remove(self->pattern, self->node);
		psy_audio_pattern_find_node(self->pattern,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK, offset,
			psy_dsp_beatpos_make_real(1.0, psy_dsp_DEFAULT_PPQ),
			&prev);
		self->node = psy_audio_pattern_insert(self->pattern, prev,
			psy_audio_GLOBALPATTERN_TIMESIGTRACK, offset, e);
		psy_audio_patternentry_dispose(e);
		free(e);
		psy_audio_exclusivelock_leave();
		seqedittimesig_updateposition(self);
		psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
		seqeditstate_set_cursor(self->state, offset);
		seqedittimesig_select(self);
	}
}

void seqedittimesig_on_mouse_up(SeqEditTimeSig* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_release_capture(&self->component);
	seqedittimesigstate_reset(self->timesigstate);
	psy_ui_component_remove_style_state(&self->component,
		psy_ui_STYLESTATE_ACTIVE);
}

void seqedittimesig_select(SeqEditTimeSig* self)
{
	psy_ui_component_add_style_state(&self->component,
		psy_ui_STYLESTATE_ACTIVE);
	psy_signal_emit(&self->state->signal_itemselected, self->state, 3,
		SEQEDITITEM_TIMESIG, seqedittimesig_timesig_index(self),
		psy_INDEX_INVALID);
}

uintptr_t seqedittimesig_timesig_index(SeqEditTimeSig* self)
{
	uintptr_t rv;
	psy_audio_PatternNode* curr;
	uintptr_t c;

	assert(self);

	curr = psy_audio_pattern_begin(self->pattern);
	c = 0;
	rv = psy_INDEX_INVALID;
	while (curr != NULL) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent e;

		entry = psy_audio_patternnode_entry(curr);
		if (psy_audio_patternentry_track(entry) ==
			psy_audio_GLOBALPATTERN_TIMESIGTRACK) {
			e = *psy_audio_patternentry_front(entry);
			if (e.note == psy_audio_NOTECOMMANDS_TIMESIG) {
				if (curr == self->node) {
					rv = c;
					break;
				}
				++c;
			}
		}
		psy_audio_patternnode_next(&curr);
	}
	return rv;
}


/* SeqEditTimeSig*/
/* prototypes */
static void seqedittimesigs_on_destroyed(SeqEditTimeSigs*);
static void seqedittimesigs_on_mouse_down(SeqEditTimeSigs*, psy_ui_MouseEvent*);
static void seqedittimesigs_onmousedoubleclick(SeqEditTimeSigs*, psy_ui_MouseEvent*);
static void seqedittimesigs_onpreferredsize(SeqEditTimeSigs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqedittimesigs_editnode(SeqEditTimeSigs*, psy_audio_PatternNode*,
	psy_ui_RealPoint);
static void seqedittimesigs_onalign(SeqEditTimeSigs*);
static void seqedittimesigs_on_song_changed(SeqEditTimeSigs*,
	psy_audio_Player* sender);
static void seqedittimesigs_build(SeqEditTimeSigs*);
static void seqedittimesigs_ontimesigchanged(SeqEditTimeSigs*, SeqEditState* sender);
static void seqedittimesigs_remove(SeqEditTimeSigs*, psy_audio_PatternNode*);
static SeqEditTimeSig* seqedittimesigs_timesigcomponent(SeqEditTimeSigs*,
	psy_audio_PatternNode*);
/* vtable */
static psy_ui_ComponentVtable seqedittimesigs_vtable;
static bool seqedittimesigs_vtable_initialized = FALSE;

static void seqedittimesigs_vtable_init(SeqEditTimeSigs* self)
{
	if (!seqedittimesigs_vtable_initialized) {
		seqedittimesigs_vtable = *(self->component.vtable);
		seqedittimesigs_vtable.on_destroyed =
			(psy_ui_fp_component)
			seqedittimesigs_on_destroyed;
		seqedittimesigs_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			seqedittimesigs_onpreferredsize;
		seqedittimesigs_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			seqedittimesigs_on_mouse_down;
		seqedittimesigs_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			seqedittimesigs_onmousedoubleclick;
		seqedittimesigs_vtable.onalign =
			(psy_ui_fp_component)
			seqedittimesigs_onalign;
		seqedittimesigs_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittimesigs_vtable;
}

void seqedittimesigs_init(SeqEditTimeSigs* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	assert(self);
	assert(state);

	psy_ui_component_init(&self->component, parent, NULL);
	seqedittimesigs_vtable_init(self);
	self->state = state;
	self->entries = NULL;
	seqedittimesigstate_init(&self->timesigstate);
	psy_signal_connect(
		&self->state->cmds->workspace->player_.signal_song_changed, self,
		seqedittimesigs_on_song_changed);
	seqedittimesigs_build(self);
	psy_signal_connect(&self->state->signal_timesigchanged, self,
		seqedittimesigs_ontimesigchanged);
}

void seqedittimesigs_on_destroyed(SeqEditTimeSigs* self)
{
	psy_list_free(self->entries);
	self->entries = NULL;
}

void seqedittimesigs_on_mouse_down(SeqEditTimeSigs* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 2 && self->timesigstate.remove) {
		psy_signal_emit(&self->state->signal_itemselected, self->state, 3,
			SEQEDITITEM_TIMESIG, psy_INDEX_INVALID, psy_INDEX_INVALID);
		seqedittimesigs_remove(self, self->timesigstate.start);
		seqedittimesigstate_reset(&self->timesigstate);
		seqedittimesigs_onalign(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void seqedittimesigs_remove(SeqEditTimeSigs* self, psy_audio_PatternNode* node)
{
	SeqEditTimeSig* timesig;

	timesig = seqedittimesigs_timesigcomponent(self, node);
	if (timesig) {
		psy_List* p;

		p = psy_list_find_entry(self->entries, timesig);
		if (p) {
			psy_list_remove(&self->entries, p);
		}
		psy_audio_pattern_remove(timesig->pattern, timesig->node);
		psy_ui_component_remove(&self->component, &timesig->component);
	}
}

SeqEditTimeSig* seqedittimesigs_timesigcomponent(SeqEditTimeSigs* self,
	psy_audio_PatternNode* node)
{
	SeqEditTimeSig* rv;
	psy_List* p;

	rv = NULL;
	for (p = self->entries; p != NULL; p = p->next) {
		SeqEditTimeSig* loop;

		loop = (SeqEditTimeSig*)(p->entry);
		if (loop->node == node) {
			rv = loop;
			break;
		}
	}
	return rv;
}

void seqedittimesigs_onmousedoubleclick(SeqEditTimeSigs* self,
	psy_ui_MouseEvent* ev)
{
	SeqEditTimeSig* seqedittimesig;
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;
	psy_audio_PatternEvent e;
	psy_audio_PatternEntry insert;

	node = seqeditstate_node(self->state, psy_ui_mouseevent_pt(ev),
		psy_dsp_beatpos_zero(), psy_dsp_beatpos_one(), &prev);
	psy_audio_patternevent_init_all(&e, psy_audio_NOTECOMMANDS_TIMESIG, 0, 0,
		0, 4, 4);
	psy_audio_patternentry_init(&insert);
	psy_audio_patternentry_set_event(&insert, e, 0);
	node = psy_audio_pattern_insert(seqeditstate_globalpattern(
		self->state), prev, 0, seqeditstate_quantize(self->state,
			seqeditstate_pxtobeat(self->state, psy_ui_mouseevent_pt(ev).x)),
		&insert);
	psy_audio_patternentry_dispose(&insert);
	seqedittimesigs_build(self);
	seqedittimesig = seqedittimesigs_timesigcomponent(self, node);
	if (seqedittimesig) {
		seqedittimesig_select(seqedittimesig);
	}
	psy_ui_component_invalidate(&self->component);
}

void seqedittimesigs_onpreferredsize(SeqEditTimeSigs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (seqeditstate_sequence(self->state)) {
		rv->width = psy_ui_value_make_px(seqeditstate_beat_to_px(self->state,
			psy_dsp_beatpos_add(
				psy_audio_sequence_duration(seqeditstate_sequence(self->state)),
				psy_dsp_beatpos_make_real(400.0, psy_dsp_DEFAULT_PPQ))));
	}
	else {
		rv->width = psy_ui_value_make_px(0.0);
	}
	rv->height = psy_ui_value_make_eh(2.0);
}

void seqedittimesigs_build(SeqEditTimeSigs* self)
{
	psy_audio_Sequence* sequence;

	psy_list_free(self->entries);
	self->entries = NULL;
	psy_ui_component_clear(&self->component);
	sequence = seqeditstate_sequence(self->state);
	if (sequence && sequence->patterns) {
		psy_List* p;

		for (p = sequence->globaltrack.nodes; p != NULL; p = p->next) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)p->entry;
			if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* seqpatternentry;
				psy_audio_Pattern* pattern;

				seqpatternentry = (psy_audio_SequencePatternEntry*)seqentry;
				pattern = psy_audio_sequencepatternentry_pattern(seqpatternentry,
					sequence->patterns);
				if (pattern) {
					psy_audio_PatternNode* node;

					for (node = psy_audio_pattern_begin(pattern); node != NULL; node = node->next) {
						psy_audio_PatternEntry* patternentry;
						psy_audio_PatternEvent* e;

						patternentry = (psy_audio_PatternEntry*)node->entry;
						e = psy_audio_patternentry_front(patternentry);
						if (e->note == psy_audio_NOTECOMMANDS_TIMESIG) {
							SeqEditTimeSig* timesig;

							timesig = seqedittimesig_allocinit(&self->component,
								&self->timesigstate, self->state, node);
							psy_list_append(&self->entries, timesig);
						}
					}
				}
				seqedittimesigs_onalign(self);
			}
		}
	}
}

void seqedittimesigs_onalign(SeqEditTimeSigs* self)
{
	psy_List* p;

	for (p = self->entries; p != NULL; p = p->next) {
		seqedittimesig_updateposition((SeqEditTimeSig*)(p->entry));
	}
}

void seqedittimesigs_on_song_changed(SeqEditTimeSigs* self,
	psy_audio_Player* sender)
{
	seqedittimesigs_build(self);
	psy_ui_component_invalidate(&self->component);
}

void seqedittimesigs_ontimesigchanged(SeqEditTimeSigs* self,
	SeqEditState* sender)
{
	psy_ui_component_invalidate(&self->component);
}

#endif /* PSYCLE_USE_SEQEDITOR */

