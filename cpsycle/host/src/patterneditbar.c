/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patterneditbar.h"
/* host */
#include "styles.h"


/* prototypes */
static void patterneditbar_on_set_duration(PatternEditBar*,
	psy_ui_Button* sender);
static void patterneditbar_on_set_select(PatternEditBar*,
	psy_ui_Button* sender);
static psy_dsp_beatpos_t patterneditbar_duration(const PatternEditBar*,
	psy_ui_ButtonIcon);
static psy_ui_ButtonIcon patterneditbar_icon(const PatternEditBar*,
	psy_dsp_beatpos_t duration);

/* implementation */
void patterneditbar_init(PatternEditBar* self, psy_ui_Component* parent,
	PianoGridState* gridstate, PatternViewState* state, Workspace* workspace)
{
	assert(self);
	assert(state);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	self->gridstate = gridstate;
	self->state = state;	
	self->workspace_ = workspace;	
	psy_ui_component_set_default_align(patterneditbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero()); //make_em(0.0, 0.5, 0.0, 0.0));	
	psy_ui_component_set_align_expand(patterneditbar_base(self), psy_ui_HEXPAND);
	psy_ui_button_init_icon_connect(&self->endless, patterneditbar_base(self),
		psy_ui_ICON_ENDLESS, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->semibreve, patterneditbar_base(self),
		psy_ui_ICON_SEMIBREVE, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->minim, patterneditbar_base(self),
		psy_ui_ICON_MINIM, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->minim_dot, patterneditbar_base(self),
		psy_ui_ICON_MINIM_DOT, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->crotchet, patterneditbar_base(self),
		psy_ui_ICON_CROTCHET, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->crotchet_dot, patterneditbar_base(self),
		psy_ui_ICON_CROTCHET_DOT, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->icon_quaver, patterneditbar_base(self),
		psy_ui_ICON_QUAVER, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->quaver_dot, patterneditbar_base(self),
		psy_ui_ICON_QUAVER_DOT, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->semiquaver, patterneditbar_base(self),
		psy_ui_ICON_SEMIQUAVER, self, patterneditbar_on_set_duration);
	psy_ui_button_init_icon_connect(&self->select, patterneditbar_base(self),
		psy_ui_ICON_SELECT, self, patterneditbar_on_set_select);
	patterneditbar_update_selection(self);
}

void patterneditbar_on_set_duration(PatternEditBar* self,
	psy_ui_Button* sender)
{
	assert(self);
					
	self->state->insert_duration = patterneditbar_duration(self, sender->icon);
	self->gridstate->select_mode = FALSE;
	patterneditbar_update_selection(self);
}

void patterneditbar_on_set_select(PatternEditBar* self,
	psy_ui_Button* sender)
{
	assert(self);
	
	self->gridstate->select_mode = TRUE;	
	patterneditbar_update_selection(self);
	psy_ui_button_highlight(sender);
}

psy_dsp_beatpos_t patterneditbar_duration(const PatternEditBar* self,
	psy_ui_ButtonIcon icon)
{
	assert(self);
	
	switch (icon) {
	case psy_ui_ICON_ENDLESS:
		return psy_dsp_beatpos_zero();		
	case psy_ui_ICON_SEMIBREVE:
		return psy_dsp_beatpos_make_real(4.0, psy_dsp_DEFAULT_PPQ);		
	case psy_ui_ICON_MINIM:
		return psy_dsp_beatpos_make_real(2.0, psy_dsp_DEFAULT_PPQ);		
	case psy_ui_ICON_MINIM_DOT:
		return psy_dsp_beatpos_make_real(3.0, psy_dsp_DEFAULT_PPQ);		
	case psy_ui_ICON_CROTCHET:
		return psy_dsp_beatpos_make_real(1.0, psy_dsp_DEFAULT_PPQ);		
	case psy_ui_ICON_CROTCHET_DOT:
		return psy_dsp_beatpos_make_real(1.5, psy_dsp_DEFAULT_PPQ);		
	case psy_ui_ICON_QUAVER:
		return psy_dsp_beatpos_make_real(0.5, psy_dsp_DEFAULT_PPQ);		
	case psy_ui_ICON_QUAVER_DOT:
		return psy_dsp_beatpos_make_real(0.75, psy_dsp_DEFAULT_PPQ);
	case psy_ui_ICON_SEMIQUAVER:
		return psy_dsp_beatpos_make_real(0.25, psy_dsp_DEFAULT_PPQ);		
	default:
		return psy_dsp_beatpos_zero();		
	}
}

psy_ui_ButtonIcon patterneditbar_icon(const PatternEditBar* self,
	psy_dsp_beatpos_t duration)
{
	assert(self);
		
	if (psy_dsp_beatpos_equal(duration, psy_dsp_beatpos_zero())) {
		return psy_ui_ICON_ENDLESS;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(4.0, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_SEMIBREVE;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(2.0, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_MINIM;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(3.0, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_MINIM_DOT;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(1.0, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_CROTCHET;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(1.5, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_CROTCHET_DOT;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(0.5, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_QUAVER;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(0.75, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_QUAVER_DOT;
	} else if (psy_dsp_beatpos_equal(duration, 
			psy_dsp_beatpos_make_real(0.25, psy_dsp_DEFAULT_PPQ))) {
		return psy_ui_ICON_SEMIQUAVER;
	}
	return psy_ui_ICON_ENDLESS;		
}

void patterneditbar_update_selection(PatternEditBar* self)	
{
	psy_ui_ButtonIcon curr_icon;
	psy_ui_Button* buttons[] = {
		&self->endless,
		&self->semibreve,
		&self->minim,
		&self->minim_dot,
		&self->crotchet,
		&self->crotchet_dot,
		&self->icon_quaver,
		&self->quaver_dot,
		&self->semiquaver,
		&self->select,
		NULL
	};
	uintptr_t i;
	
	assert(self);
		
	curr_icon = patterneditbar_icon(self, self->state->insert_duration);
	for (i = 0; buttons[i] != NULL; ++i) {
		psy_ui_Button* button;
		
		button = buttons[i];
		if (curr_icon == button->icon) {
			psy_ui_button_highlight(button);
		} else {
			psy_ui_button_disable_highlight(button);
		}
	}
}
