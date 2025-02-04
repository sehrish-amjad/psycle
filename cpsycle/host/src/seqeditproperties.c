/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditproperties.h"

#ifdef PSYCLE_USE_SEQEDITOR

/* host */
#include "resources/resource.h"
#include "sequencetrackbox.h"
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* psy_audio_Loop */
typedef struct psy_audio_Loop {
	psy_audio_PatternNode* start;
	psy_audio_PatternNode* end;
} psy_audio_Loop;

/* SeqEditPropertiesEntry */

/* implementation */
void seqeditpropertiesentry_init(SeqEditPropertiesEntry* self,
	psy_ui_Component* parent, const char* title, SeqEditState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_title(&self->component, title);
	self->state = state;
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.5, 0.0));
}

/* SeqEditPropertiesSeqEntry */

/* prototypes */
static void seqeditentryproperties_update(SeqEditPropertiesSeqEntry*);
static void seqeditpropertiesseqentry_select(SeqEditPropertiesSeqEntry*,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2);

/* implementation */
void seqeditpropertiesseqentry_init(SeqEditPropertiesSeqEntry* self,
	psy_ui_Component* parent, const char* title, SeqEditState* state)
{
	seqeditpropertiesentry_init(&self->entry, parent, title, state);	
	psy_signal_connect(&self->entry.component.signal_select_section, self,
		seqeditpropertiesseqentry_select);
	self->orderindex = psy_audio_orderindex_make_invalid();	
	/* position */
	labelpair_init(&self->offset, seqeditpropertiesseqentry_base(self),
		"seqedit.position", state->propertiesview_desc_column_width);
	psy_ui_component_set_align(psy_ui_label_base(&self->offset.second),
		psy_ui_ALIGN_CLIENT);	
	/* length */
	labelpair_init(&self->length, seqeditpropertiesseqentry_base(self),
		"seqedit.length", state->propertiesview_desc_column_width);
	psy_ui_component_set_align(psy_ui_label_base(&self->length.second),
		psy_ui_ALIGN_CLIENT);	
	/* end */
	labelpair_init(&self->end, seqeditpropertiesseqentry_base(self),
		"seqedit.end", state->propertiesview_desc_column_width);
	psy_ui_component_set_align(psy_ui_label_base(&self->end.second),
		psy_ui_ALIGN_CLIENT);	
}

void seqeditpropertiesseqentry_select(SeqEditPropertiesSeqEntry* self,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2)
{
	seqeditpropertiesseqentry_set_order_index(self,
		psy_audio_orderindex_make(param1, param2));
}

void seqeditpropertiesseqentry_set_order_index(SeqEditPropertiesSeqEntry* self,
	psy_audio_OrderIndex orderindex)
{
	self->orderindex = orderindex;
	seqeditentryproperties_update(self);
}

void seqeditentryproperties_update(SeqEditPropertiesSeqEntry* self)
{
	psy_audio_Sequence* sequence;

	sequence = seqeditstate_sequence(self->entry.state);
	if (sequence) {
		char text[64];
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(sequence, self->orderindex);
		*text = '\0';
		if (seqentry) {
			psy_snprintf(text, 64, "%7.3f",
				psy_dsp_beatpos_real(psy_audio_sequenceentry_offset(
					seqentry)));
		}
		psy_ui_label_set_text(&self->offset.second, text);
		*text = '\0';
		if (seqentry) {
			psy_snprintf(text, 64, "%7.3f",
				psy_dsp_beatpos_real(psy_audio_sequenceentry_length(
					seqentry)));
		}
		psy_ui_label_set_text(&self->length.second, text);
		*text = '\0';
		if (seqentry) {
			psy_snprintf(text, 64, "%7.3f",
				(psy_dsp_beatpos_real(psy_dsp_beatpos_add(
					psy_audio_sequenceentry_offset(seqentry),
					psy_audio_sequenceentry_length(seqentry)))));
		}
		psy_ui_label_set_text(&self->end.second, text);
	}
}

/* SeqEditEmptyProperties */

/* implementation */
void seqeditemptyproperties_init(SeqEditEmptyProperties* self,
	psy_ui_Component* parent, SeqEditState* state)
{	
	seqeditpropertiesentry_init(&self->entry, parent, "seqedit.noitem", state);
	psy_ui_component_set_id(&self->entry.component, SEQEDITITEM_NONE);
}

/* SeqEditPatternProperties */

/* implementation */
void seqeditpatternproperties_init(SeqEditPatternProperties* self,
	psy_ui_Component* parent, SeqEditState* state)
{		
	seqeditpropertiesseqentry_init(&self->entry, parent,
		"seqedit.patternitem", state);
	psy_ui_component_set_id(&self->entry.entry.component, SEQEDITITEM_PATTERN);		
}

/* SeqEditSampleProperties */

/* implementation */
void seqeditsampleproperties_init(SeqEditSampleProperties* self,
	psy_ui_Component* parent, SeqEditState* state)
{	
	seqeditpropertiesseqentry_init(&self->entry, parent,
		"seqedit.sampleitem", state);
	psy_ui_component_set_id(&self->entry.entry.component, SEQEDITITEM_SAMPLE);
}

/* SeqEditMarkerProperties */

/* prototypes */
static void seqeditmarkerproperties_on_edit_changed(SeqEditMarkerProperties*,
	psy_ui_Text* sender);
static void seqeditmarkerproperties_select(SeqEditMarkerProperties*,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2);

/* implementation */
void seqeditmarkerproperties_init(SeqEditMarkerProperties* self,
	psy_ui_Component* parent, SeqEditState* state)
{	
	seqeditpropertiesseqentry_init(&self->entry, parent,
		"seqedit.markeritem", state);	
	psy_signal_connect(&self->entry.entry.component.signal_select_section, self,
		seqeditmarkerproperties_select);
	psy_ui_component_set_id(&self->entry.entry.component, SEQEDITITEM_MARKER);
	labeledit_init(&self->name, &self->entry.entry.component, "seqedit.name");	
	psy_signal_connect(&self->name.edit.signal_change,
		self, seqeditmarkerproperties_on_edit_changed);	
}

void seqeditmarkerproperties_select(SeqEditMarkerProperties* self,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2)
{
	seqeditmarkerproperties_set_order_index(self,
		psy_audio_orderindex_make(param1, param2));	
}

void seqeditmarkerproperties_set_order_index(SeqEditMarkerProperties* self,
	psy_audio_OrderIndex orderindex)
{
	psy_audio_SequenceEntry* entry;	
		
	seqeditpropertiesseqentry_set_order_index(&self->entry, orderindex);
	entry = psy_audio_sequence_entry(seqeditstate_sequence(
		self->entry.entry.state), orderindex);
	if (entry) {
		psy_ui_text_set_text(&self->name.edit, psy_audio_sequenceentry_label(
			entry));
	}
}

void seqeditmarkerproperties_on_edit_changed(SeqEditMarkerProperties* self,
	psy_ui_Text* sender)
{
	psy_audio_Sequence* sequence;
	
	sequence = seqeditstate_sequence(self->entry.entry.state);
	if (sequence) {
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(sequence, self->entry.orderindex);
		if (seqentry && seqentry->type == psy_audio_SEQUENCEENTRY_MARKER) {
			psy_audio_SequenceMarkerEntry* marker;

			marker = (psy_audio_SequenceMarkerEntry*)seqentry;
			psy_audio_sequencemarkerentry_set_text(marker, psy_ui_text_text(sender));			
			psy_ui_component_invalidate(self->entry.entry.state->view);
			psy_ui_component_set_focus(self->entry.entry.state->view);
		}
	}
}

void seqeditmarkerproperties_on_edit_reject(SeqEditMarkerProperties* self,
	psy_ui_Text* sender)
{
	psy_ui_component_set_focus(self->entry.entry.state->view);
}

/* SeqEditTimesigProperties */

/* prototypes */
static void seqedittimesigproperties_on_numerator(SeqEditTimesigProperties*,
	psy_ui_NumberEdit* sender);
static void seqedittimesigproperties_on_denominator(SeqEditTimesigProperties*,
	psy_ui_NumberEdit* sender);
static void seqedittimesigproperties_select(SeqEditTimesigProperties*,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2);
static psy_audio_PatternNode* seqedittimesigproperties_timesig(
	SeqEditTimesigProperties*, uintptr_t index);

/* implementation */
void seqedittimesigproperties_init(SeqEditTimesigProperties* self,
	psy_ui_Component* parent, SeqEditState* state)
{	
	seqeditpropertiesentry_init(&self->entry, parent, "seqedit.timesigitem",
		state);	
	psy_signal_connect(&self->entry.component.signal_select_section, self,
		seqedittimesigproperties_select);
	psy_ui_component_set_id(&self->entry.component, SEQEDITITEM_TIMESIG);
	self->timesigindex = psy_INDEX_INVALID;	
	/* numerator */
	psy_ui_numberedit_init(&self->numerator, seqedittimesigproperties_base(self),
		"seqedit.numerator", 1, 1, 0xF);	
	psy_ui_label_set_char_number(&self->numerator.desc, 14.0);	
	/* denominator */
	psy_ui_numberedit_init(&self->denominator, seqedittimesigproperties_base(self),
		"seqedit.denominator", 1, 1, 0xF);	
	psy_ui_label_set_char_number(&self->denominator.desc, 14.0);
	/* position */	
	labelpair_init(&self->offset, seqedittimesigproperties_base(self),
		"seqedit.position", state->propertiesview_desc_column_width);		
	/* connect signals */
	psy_signal_connect(&self->numerator.signal_changed, self,
		seqedittimesigproperties_on_numerator);
	psy_signal_connect(&self->denominator.signal_changed, self,
		seqedittimesigproperties_on_denominator);	
}

void seqedittimesigproperties_select(SeqEditTimesigProperties* self,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2)
{
	seqedittimesigproperties_set_timesig_index(self, param1);
}

void seqedittimesigproperties_set_timesig_index(SeqEditTimesigProperties* self,
	uintptr_t timesigindex)
{
	psy_audio_Pattern* pattern;

	self->timesigindex = timesigindex;	
	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);	
	if (pattern) {
		psy_audio_PatternNode* node;
					
		node = seqedittimesigproperties_timesig(self, self->timesigindex);
		if (node) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent e;
			char text[64];

			entry = (psy_audio_PatternEntry*)(node->entry); 
			e = *psy_audio_patternentry_front(entry);
			psy_ui_numberedit_set_value(&self->numerator, (e.cmd));
			psy_ui_numberedit_set_value(&self->denominator, (e.parameter));				
			psy_snprintf(text, 64, "%7.3f", (float)psy_dsp_beatpos_real(
				psy_audio_patternentry_offset(entry)));
			psy_ui_label_set_text(&self->offset.second, text);
		}
	}
}

void seqedittimesigproperties_on_numerator(SeqEditTimesigProperties* self,
	psy_ui_NumberEdit* sender)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		psy_audio_PatternNode* node;		
						
		node = seqedittimesigproperties_timesig(self, self->timesigindex);
		if (node) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* e;

			entry = (psy_audio_PatternEntry*)(node->entry); 
			e = psy_audio_patternentry_front(entry);
			e->cmd = (uint8_t)psy_ui_numberedit_value(sender);
			psy_signal_emit(&self->entry.state->signal_timesigchanged,
				self->entry.state, 0);
		}		
	}
}

void seqedittimesigproperties_on_denominator(SeqEditTimesigProperties* self,
	psy_ui_NumberEdit* sender)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry* entry;		
						
		node = seqedittimesigproperties_timesig(self, self->timesigindex);
		if (node) {
			psy_audio_PatternEvent* e;

			entry = (psy_audio_PatternEntry*)(node->entry); 
			e = psy_audio_patternentry_front(entry);
			e->parameter = (uint8_t)psy_ui_numberedit_value(sender);
			psy_signal_emit(&self->entry.state->signal_timesigchanged,
				self->entry.state, 0);
		}		
	}
}

psy_audio_PatternNode* seqedittimesigproperties_timesig(
	SeqEditTimesigProperties* self, uintptr_t index)
{
	psy_audio_Pattern* pattern;

	assert(self);
	
	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);		
	if (pattern) {		
		psy_audio_PatternNode* rv;
		psy_audio_PatternNode* curr;	
		uintptr_t c;

		assert(self);
	
		curr = psy_audio_pattern_begin(pattern);
		c = 0;
		rv = NULL;
		while (curr != NULL) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent e;

			entry = psy_audio_patternnode_entry(curr);
			if (psy_audio_patternentry_track(entry) ==
					psy_audio_GLOBALPATTERN_TIMESIGTRACK) {
				e = *psy_audio_patternentry_front(entry);
				if (e.note == psy_audio_NOTECOMMANDS_TIMESIG) {
					if (c == self->timesigindex) {
						rv = curr;									
						break;
					}
					++c;
				}
			}
			psy_audio_patternnode_next(&curr);		
		}	
		return rv;		
	}
	return NULL;
}

/* SeqEditLoopProperties */

/* prototypes */
static void seqeditloopproperties_on_num_loops(SeqEditLoopProperties*,
	psy_ui_NumberEdit* sender);
static void seqeditloopproperties_select(SeqEditLoopProperties*,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2);
static psy_audio_Loop seqeditloopproperties_loop_at(SeqEditLoopProperties*,
	uintptr_t index);

/*  implementation */
void seqeditloopproperties_init(SeqEditLoopProperties* self,
	psy_ui_Component* parent, SeqEditState* state)
{	
	seqeditpropertiesentry_init(&self->entry, parent, "seqedit.loopitem",
		state);
	psy_ui_component_set_id(&self->entry.component, SEQEDITITEM_LOOP);
	psy_signal_connect(&self->entry.component.signal_select_section, self,
		seqeditloopproperties_select);
	psy_ui_numberedit_init(&self->numloops, seqeditloopproperties_base(self),
		"seqedit.repetitions", 1, 1, 0xF);	
	psy_ui_label_set_char_number(&self->numloops.desc,
		state->propertiesview_desc_column_width);	
	/* position */
	labelpair_init(&self->offset, seqeditloopproperties_base(self),
		"seqedit.position", state->propertiesview_desc_column_width);
	psy_ui_component_set_align(psy_ui_label_base(&self->offset.second),
		psy_ui_ALIGN_CLIENT);	
	/* length */
	labelpair_init(&self->length, seqeditloopproperties_base(self),
		"seqedit.length", state->propertiesview_desc_column_width);
	psy_ui_component_set_align(psy_ui_label_base(&self->length.second),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(labelpair_base(&self->length), psy_ui_ALIGN_TOP);
	/* end */
	labelpair_init(&self->end, seqeditloopproperties_base(self),
		"seqedit.end", state->propertiesview_desc_column_width);
	psy_ui_component_set_align(psy_ui_label_base(&self->end.second),
		psy_ui_ALIGN_CLIENT);
	seqeditloopproperties_setloopindex(self, psy_INDEX_INVALID);	
	psy_signal_connect(&self->numloops.signal_changed, self,
		seqeditloopproperties_on_num_loops);	
}

void seqeditloopproperties_select(SeqEditLoopProperties* self,
	psy_ui_Component* sender, uintptr_t param1, uintptr_t param2)
{
	seqeditloopproperties_setloopindex(self, param1);
}

void seqeditloopproperties_on_num_loops(SeqEditLoopProperties* self,
	psy_ui_NumberEdit* sender)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		psy_audio_Loop loop;		
						
		loop = seqeditloopproperties_loop_at(self, self->loopindex);			
		if (loop.end) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* e;

			entry = (psy_audio_PatternEntry*)(loop.end->entry); 
			e = psy_audio_patternentry_front(entry);
			e->parameter = 0xB0 | (uint8_t)psy_ui_numberedit_value(sender);
			psy_signal_emit(&self->entry.state->signal_loopchanged,
				self->entry.state, 0);
		}		
	}
}

void seqeditloopproperties_setloopindex(SeqEditLoopProperties* self,
	uintptr_t loopindex)
{
	psy_audio_Pattern* pattern;

	self->loopindex = loopindex;	
	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {
		char text[64];
		psy_audio_Loop loop;
		psy_audio_PatternEntry* start;
		psy_audio_PatternEntry* end;
		psy_audio_PatternEvent e;
					
		loop = loop = seqeditloopproperties_loop_at(self, loopindex);
		if (loop.start) {
			start = (psy_audio_PatternEntry*)(loop.start->entry);
		} else {
			start = NULL;
		}
		if (loop.end) {
			end = (psy_audio_PatternEntry*)(loop.end->entry);
		} else {
			end = NULL;
		}
		if (end) {		
			e = *psy_audio_patternentry_front(end);
			psy_ui_numberedit_set_value(&self->numloops, (e.parameter & 0x0F));
		}
		if (start) {			
			psy_snprintf(text, 64, "%7.3f", (float)psy_dsp_beatpos_real(
				psy_audio_patternentry_offset(start)));
			psy_ui_label_set_text(&self->offset.second, text);
		} else {
			psy_ui_label_set_text(&self->offset.second, "");
		}
		if (start && end) {		
			psy_snprintf(text, 64, "%7.3f", (float)
				(psy_dsp_beatpos_real(psy_audio_patternentry_offset(end)) -
				psy_dsp_beatpos_real(psy_audio_patternentry_offset(start))));
			psy_ui_label_set_text(&self->length.second, text);
		} else {
			psy_ui_label_set_text(&self->length.second, "");
		}
		if (end) {
			psy_snprintf(text, 64, "%7.3f", (float)psy_dsp_beatpos_real(
				psy_audio_patternentry_offset(end)));
			psy_ui_label_set_text(&self->end.second, text);
		} else {
			psy_ui_label_set_text(&self->end.second, "");
		}
	}
}

psy_audio_Loop seqeditloopproperties_loop_at(SeqEditLoopProperties* self,
	uintptr_t index)
{
	psy_audio_Pattern* pattern;
	psy_audio_Loop rv;

	pattern = psy_audio_patterns_at(
		&self->entry.state->cmds->workspace->song->patterns_,
		psy_audio_GLOBALPATTERN);				
	if (pattern) {		
		psy_audio_PatternNode* curr;	
		uintptr_t c;		
		
		curr = psy_audio_pattern_begin(pattern);
		c = 0;
		rv.start = rv.end = NULL;
		while (curr != NULL) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent e;

			entry = psy_audio_patternnode_entry(curr);
			if (psy_audio_patternentry_track(entry) ==
					psy_audio_GLOBALPATTERN_TIMESIGTRACK) {
				e = *psy_audio_patternentry_front(entry);
				if (e.cmd == 0xFE && ((e.parameter & 0xFF) == 0xB0)) {
					if (c == index) {
						rv.start = curr;				
						rv.end = psy_audio_patternnode_next_track(curr,
							psy_audio_GLOBALPATTERN_TIMESIGTRACK);					
						break;
					}
					++c;
				}
			}
			psy_audio_patternnode_next(&curr);		
		}		
	}
	rv.end = rv.start = NULL;
	return rv;
}


/* SeqEditProperties */

/* prototypes */
static void seqeditproperties_on_item_selected(SeqEditProperties*,
	SeqEditState* sender, uintptr_t itemtype, uintptr_t param1,
	uintptr_t param2);
static void seqeditproperties_on_cursor_changed(SeqEditProperties*,
	psy_audio_Sequence*);
static void seqeditproperties_on_track_reposition(SeqEditProperties*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
static void seqeditproperties_on_song_changed(SeqEditProperties*,
	psy_audio_Player* sender);
static void seqeditproperties_connect_song(SeqEditProperties*);
static void seqeditproperties_set_caption(SeqEditProperties*, const char* text);

/*  implementation */
void seqeditproperties_init(SeqEditProperties* self, psy_ui_Component* parent,
	SeqEditState* state)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_id(&self->component, SEQEDIT_PROPERTIESVIEW);
	psy_ui_component_set_style_type(&self->component, STYLE_SEQEDT_PROPERTIES);
	self->state = state;
	self->itemtype = SEQEDITITEM_NONE;
	self->param1 = 0;
	self->param2 = 0;		
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(30.0, 20.0));	
	/* caption */
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_set_align(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->caption, &self->top, "seqedit.noitem");
	psy_ui_component_set_align(psy_ui_label_base(&self->caption),
		psy_ui_ALIGN_CLIENT);	
	/* notebook */
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_set_margin(&self->notebook.component,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 2.0));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	seqeditemptyproperties_init(&self->empty, psy_ui_notebook_base(
		&self->notebook),
		self->state);
	seqeditpatternproperties_init(&self->pattern, psy_ui_notebook_base(
		&self->notebook), self->state);	
	seqeditsampleproperties_init(&self->sample, psy_ui_notebook_base(
		&self->notebook), self->state);	
	seqeditmarkerproperties_init(&self->marker, psy_ui_notebook_base(
		&self->notebook), self->state);
	seqedittimesigproperties_init(&self->timesig, psy_ui_notebook_base(
		&self->notebook), self->state);
	seqeditloopproperties_init(&self->loop, psy_ui_notebook_base(
		&self->notebook), self->state);
	seqeditproperties_select(self, SEQEDITITEM_NONE, 0, 0);
	psy_signal_connect(&self->state->signal_itemselected, self,
		seqeditproperties_on_item_selected);
	psy_signal_connect(
		&self->state->cmds->workspace->player_.signal_song_changed,
		self, seqeditproperties_on_song_changed);
	seqeditproperties_connect_song(self);	
}

void seqeditproperties_select(SeqEditProperties* self, SeqEditItemType type,
	uintptr_t param1, uintptr_t param2)
{	
	self->itemtype = type;
	self->param1 = param1;
	self->param2 = param2;
	psy_ui_notebook_select_by_component_id(&self->notebook, type);
	if (psy_ui_notebook_active_page(&self->notebook)) {
		psy_ui_component_select_section(psy_ui_notebook_active_page(
			&self->notebook), param1, param2);		
		seqeditproperties_set_caption(self, psy_ui_component_title(
			psy_ui_notebook_active_page(&self->notebook)));
	} else {
		seqeditproperties_set_caption(self, NULL);
	}	
}

void seqeditproperties_set_caption(SeqEditProperties* self, const char* text)
{	
	psy_ui_label_set_text(&self->caption, (text)
		? text
		: "seqedit.noitem");
}

void seqeditproperties_on_cursor_changed(SeqEditProperties* self,
	psy_audio_Sequence* sequence)
{
	if (!psy_audio_orderindex_equal(&sequence->cursor.order_index,
			sequence->lastcursor.order_index) ||
			self->itemtype > SEQEDITITEM_MARKER) {
		psy_audio_SequenceEntry* entry;		

		entry = psy_audio_sequence_entry(sequence,
			sequence->cursor.order_index);
		if (entry) {
			SeqEditItemType item_type;
			uintptr_t param1;
			uintptr_t param2;

			param1 = sequence->cursor.order_index.track;
			param2 = sequence->cursor.order_index.order;
			switch (entry->type) {
			case psy_audio_SEQUENCEENTRY_PATTERN:
				item_type = SEQEDITITEM_PATTERN;				
				break;
			case psy_audio_SEQUENCEENTRY_SAMPLE:
				item_type = SEQEDITITEM_SAMPLE;				
				break;
			case psy_audio_SEQUENCEENTRY_MARKER:
				item_type = SEQEDITITEM_MARKER;				
				break;
			default:
				item_type = SEQEDITITEM_NONE;				
				break;
			}			
			seqeditproperties_select(self, item_type, param1, param2);
		}
	}
}

void seqeditproperties_on_item_selected(SeqEditProperties* self,
	SeqEditState* sender, uintptr_t itemtype, uintptr_t param1,
	uintptr_t param2)
{
	seqeditproperties_select(self, (SeqEditItemType)itemtype, param1, param2);
}

void seqeditproperties_on_song_changed(SeqEditProperties* self,
	psy_audio_Player* sender)
{
	seqeditproperties_connect_song(self);		
}

void seqeditproperties_connect_song(SeqEditProperties* self)
{
	psy_audio_Song* song;

	song = workspace_song(self->state->cmds->workspace);
	if (song) {		
		psy_signal_connect(&song->sequence_.signal_track_reposition,
			self, seqeditproperties_on_track_reposition);
		psy_signal_connect(&song->sequence_.signal_cursor_changed,
			self, seqeditproperties_on_cursor_changed);
		seqeditproperties_select(self, SEQEDITITEM_PATTERN, 0, 0);
	} else {
		seqeditproperties_select(self, SEQEDITITEM_NONE, 0, 0);
	}
}

void seqeditproperties_on_track_reposition(SeqEditProperties* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{	
	seqeditproperties_select(self, self->itemtype, self->param1, self->param2);
}

#endif /* PSYCLE_USE_SEQEDITOR */
