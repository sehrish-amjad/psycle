/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternproperties.h"

#ifdef PSYCLE_USE_PATTERN_VIEW

/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void patternproperties_on_name_changed(PatternProperties*,
	psy_audio_Patterns* sender, uintptr_t pat_index);
static void patternproperties_on_length_changed(PatternProperties*,
	psy_audio_Patterns* sender, uintptr_t pat_index);
static void patternproperties_on_key_down(PatternProperties*,
	psy_ui_KeyboardEvent*);
static void patternproperties_on_key_up(PatternProperties*,
	psy_ui_KeyboardEvent*);
static void patternproperties_on_focus(PatternProperties*);
static void patternproperties_on_timesig_numerator(PatternProperties*,
	psy_ui_NumberEdit* sender);
static void patternproperties_on_timesig_denominator(PatternProperties*,
	psy_ui_NumberEdit* sender);
static void patternproperties_on_length_edit_changed(PatternProperties*,
	psy_ui_NumberEdit* sender);
static void patternproperties_on_name_edit_accept(PatternProperties*,
	psy_ui_Text* sender);
static void patternproperties_connect_song(PatternProperties*,
	psy_audio_Song* song);
static void patternproperties_on_cursor_changed(PatternProperties*,
	psy_audio_Sequence* sender);	
static void patternproperties_on_song_changed(PatternProperties*,
	psy_audio_Player* sender);
static void patternproperties_select(PatternProperties*,
	uintptr_t pattern_index);

/* vtable */
static psy_ui_ComponentVtable patternproperties_vtable;
static bool patternproperties_vtable_initialized = FALSE;

static void patternproperties_vtable_init(PatternProperties* self)
{
	assert(self);
	
	if (!patternproperties_vtable_initialized) {
		patternproperties_vtable = *(self->component.vtable);
		patternproperties_vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			patternproperties_on_key_down;
		patternproperties_vtable.onkeyup =
			(psy_ui_fp_component_on_key_event)
			patternproperties_on_key_up;
		patternproperties_vtable.on_focus =
			(psy_ui_fp_component)
			patternproperties_on_focus;
		patternproperties_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &patternproperties_vtable);		
}

/* implementation */
void patternproperties_init(PatternProperties* self, psy_ui_Component* parent,
	PatternViewState* state, Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);		
	patternproperties_vtable_init(self);
	self->pattern_index = psy_INDEX_INVALID;
	self->workspace = workspace;
	self->state = state;
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->component);
	closebar_init(&self->close_bar, patternproperties_base(self), NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));	
	/* name */
	psy_ui_label_init_text(&self->name_label, &self->component,
		"pv.patname");	
	psy_ui_text_init(&self->name_edit, &self->component);	
	psy_ui_text_set_char_number(&self->name_edit, 40);
	psy_ui_text_enable_input_field(&self->name_edit);
	psy_signal_connect(&self->name_edit.signal_accept, self,
		patternproperties_on_name_edit_accept);
	/* length */
	psy_ui_label_init_text(&self->length_label, &self->component,
		"pv.length");	
	psy_ui_numberedit_init(&self->length_edit, &self->component,
		"", 0, 0, 128);		
	psy_ui_numberedit_set_type(&self->length_edit, psy_ui_NUMBEREDIT_TYPE_REAL);
	psy_ui_numberedit_set_edit_char_number(&self->length_edit, 8);
	psy_signal_connect(&self->length_edit.signal_changed, self,
		patternproperties_on_length_edit_changed);	
	/* timesig numerator */
	psy_ui_numberedit_init(&self->timesig_numerator, &self->component,
		"Timesignature", 0, 0, 128);	
	psy_signal_connect(&self->timesig_numerator.signal_changed, self,
		patternproperties_on_timesig_numerator);
	/* timesig denomiator */
	psy_ui_numberedit_init(&self->timesig_denominator, &self->component,
		"", 0, 0, 128);
	psy_signal_connect(&self->timesig_denominator.signal_changed, self,
		patternproperties_on_timesig_denominator);		
	/* connect */
	psy_signal_connect(&workspace->player_.signal_song_changed, self,
		patternproperties_on_song_changed);
	patternproperties_connect_song(self, workspace_song(workspace));
	if (workspace_song(workspace)) {
		patternproperties_select(self, psy_audio_sequence_patternindex(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			self->state->cursor.order_index));
	}
}

void patternproperties_on_length_edit_changed(PatternProperties* self,
	psy_ui_NumberEdit* sender)
{
	psy_audio_Pattern* pattern;		
	
	assert(self);
		
	pattern = patternviewstate_pattern(self->state);
	if (pattern) {		
		psy_audio_pattern_set_length(pattern, 
			psy_dsp_beatpos_make_real(
				psy_ui_numberedit_value(&self->length_edit),
				psy_dsp_DEFAULT_PPQ));	
	}
}

void patternproperties_on_name_edit_accept(PatternProperties* self,
	psy_ui_Text* sender)
{
	psy_audio_Pattern* pattern;		
	
	assert(self);
		
	pattern = patternviewstate_pattern(self->state);
	if (pattern) {
		psy_audio_pattern_set_name(pattern, psy_ui_text_text(&self->name_edit));		
	}
}

void patternproperties_on_key_down(PatternProperties* self,
	psy_ui_KeyboardEvent* ev)
{	
	psy_ui_keyboardevent_stop_propagation(ev);
}

void patternproperties_on_key_up(PatternProperties* self,
	psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
}

void patternproperties_on_focus(PatternProperties* self)
{
	psy_ui_component_set_focus(psy_ui_numberedit_base(&self->length_edit));
}

void patternproperties_on_name_changed(PatternProperties* self,
	psy_audio_Patterns* sender, uintptr_t pat_index)
{		
	psy_audio_Pattern* pattern;
	
	assert(self);
	
	pattern = psy_audio_patterns_at(sender, pat_index);
	if (pattern) {
		psy_ui_text_set_text(&self->name_edit, psy_audio_pattern_name(pattern));	
	}
}

void patternproperties_on_length_changed(PatternProperties* self,
	psy_audio_Patterns* sender, uintptr_t pat_index)
{	
	psy_audio_Pattern* pattern;
	
	assert(self);
	
	pattern = psy_audio_patterns_at(sender, pat_index);
	if (pattern) {	
		psy_ui_numberedit_set_value(&self->length_edit,
			psy_dsp_beatpos_real(psy_audio_pattern_length(pattern)));
	}
}

void patternproperties_on_timesig_numerator(PatternProperties* self,
	psy_ui_NumberEdit* sender)
{
	psy_audio_Pattern* pattern;		
	
	assert(self);
		
	pattern = patternviewstate_pattern(self->state);
	if (pattern) {
		pattern->time_sig_.cmd = (uint8_t)psy_ui_numberedit_value(sender);
	}
}

void patternproperties_on_timesig_denominator(PatternProperties* self,
	psy_ui_NumberEdit* sender)
{
	psy_audio_Pattern* pattern;		
	
	assert(self);
		
	pattern = patternviewstate_pattern(self->state);
	if (pattern) {
		pattern->time_sig_.parameter = (uint8_t)psy_ui_numberedit_value(sender);
	}
}

void patternproperties_select(PatternProperties* self, uintptr_t pattern_index)
{
	psy_audio_Pattern* pattern;		
	
	assert(self);
		
	pattern = patternviewstate_pattern(self->state);
	if (pattern && self->pattern_index != pattern_index) {
		self->pattern_index = pattern_index;
				
		if (pattern) {
			psy_ui_text_set_text(&self->name_edit,
				psy_audio_pattern_name(pattern));			
			psy_ui_numberedit_set_value(&self->length_edit, 
				psy_dsp_beatpos_real(psy_audio_pattern_length(pattern)));
			psy_ui_numberedit_set_value(&self->timesig_numerator,
				(double)pattern->time_sig_.cmd);
			psy_ui_numberedit_set_value(&self->timesig_denominator,
				(double)pattern->time_sig_.parameter);
		} else {
			psy_ui_text_set_text(&self->name_edit, "");
			psy_ui_numberedit_set_value(&self->length_edit, 0.0);
		}		
	}
}

void patternproperties_on_song_changed(PatternProperties* self,
	psy_audio_Player* sender)
{
	assert(self);
	
	self->pattern_index = psy_INDEX_INVALID;
	patternproperties_connect_song(self, psy_audio_player_song(sender));	
}
	
void patternproperties_on_cursor_changed(PatternProperties* self,
	psy_audio_Sequence* sender)
{
	assert(self);	
			
	if (!workspace_song(self->workspace)) {
		patternproperties_select(self, psy_INDEX_INVALID);		
	} else if (!psy_audio_orderindex_equal(&sender->cursor.order_index,
			sender->lastcursor.order_index)) {		
		patternproperties_select(self, psy_audio_sequence_patternindex(
			psy_audio_song_sequence(workspace_song(self->workspace)),
			sender->cursor.order_index));
	}
}

void patternproperties_connect_song(PatternProperties* self,
	psy_audio_Song* song)
{
	assert(self);
	
	if (song) {
		psy_audio_Patterns* patterns;
		psy_audio_Sequence* sequence;
		
		patterns = psy_audio_song_patterns(song);
		psy_signal_connect(&patterns->signal_name_changed,
			self, patternproperties_on_name_changed);
		psy_signal_connect(&patterns->signal_length_changed,
			self, patternproperties_on_length_changed);
		sequence = psy_audio_song_sequence(song);
		psy_signal_connect(&sequence->signal_cursor_changed,
			self, patternproperties_on_cursor_changed);
	}
}

#endif /* PSYCLE_USE_PATTERN_VIEW */
