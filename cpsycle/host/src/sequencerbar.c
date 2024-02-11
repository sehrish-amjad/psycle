/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sequencerbar.h"
/* host */
#include "styles.h"


/* prototypes */
static void sequencerbar_connect_buttons(SequencerBar*, psy_Configuration*);
static void sequencerbar_on_piano_kbd(SequencerBar*, psy_Property* sender);
static void sequencerbar_on_seq_editor(SequencerBar*, psy_Property* sender);
static void sequencerbar_on_step_sequencer(SequencerBar*, psy_Property* sender);

/* implementation */
void sequencerbar_init(SequencerBar* self, psy_ui_Component* parent,
	psy_Configuration* misc, psy_Configuration* general)
{
	assert(self);
	assert(misc);
	assert(general);
	
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_prevent_app_focus_out(&self->component);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.25, 0.0));
	psy_ui_checkbox_init_exchange(&self->follow_song_, &self->component,				
		psy_configuration_at(misc, "followsong"));
	psy_ui_checkbox_init_exchange(&self->show_names_, &self->component,
		psy_configuration_at(general, "showpatternnames"));
	psy_ui_checkbox_init_exchange(&self->record_note_off_, &self->component,
		psy_configuration_at(misc, "recordnoteoff"));
	psy_ui_checkbox_init_exchange(&self->record_tweak_, &self->component,
		psy_configuration_at(misc, "record-tweak"));
	psy_ui_checkbox_init_exchange(&self->multi_channel_audition_, &self->component,
		psy_configuration_at(misc, "multikey"));
	psy_ui_checkbox_init_exchange(&self->allow_notes_to_effect_, &self->component,
		psy_configuration_at(misc, "notestoeffects"));		
	/* view buttons */
	psy_ui_component_init(&self->view_buttons_, &self->component, NULL);
	psy_ui_component_prevent_app_focus_out(&self->view_buttons_);
	psy_ui_component_set_align(&self->view_buttons_, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->view_buttons_,
		psy_ui_margin_make_em(1.0, 0.0, 0.0, 0.0));
	psy_ui_component_set_default_align(&self->view_buttons_, psy_ui_ALIGN_TOP,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 0.0));
	/* seq edit */
	psy_ui_button_init(&self->toggle_seq_edit_, &self->view_buttons_);
	psy_ui_button_set_text(&self->toggle_seq_edit_, "seqview.showseqeditor");
	psy_ui_button_exchange(&self->toggle_seq_edit_,
		psy_configuration_at(general, "bench.showsequenceedit"));	
	psy_ui_button_set_text_alignment(&self->toggle_seq_edit_, (psy_ui_Alignment)
		(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_button_set_icon(&self->toggle_seq_edit_, psy_ui_ICON_MORE);
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->toggle_seq_edit_));
	/* step seq */
	psy_ui_button_init(&self->toggle_step_seq_, &self->view_buttons_);
	psy_ui_button_set_text(&self->toggle_step_seq_, "seqview.showstepsequencer");
	psy_ui_button_exchange(&self->toggle_step_seq_,
		psy_configuration_at(general, "bench.showstepsequencer"));	
	psy_ui_button_set_icon(&self->toggle_step_seq_, psy_ui_ICON_MORE);
	psy_ui_button_set_text_alignment(&self->toggle_step_seq_,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->toggle_step_seq_));
	/* kbd */
	psy_ui_button_init(&self->toggle_kbd_, &self->view_buttons_);
	psy_ui_button_set_text(&self->toggle_kbd_, "seqview.showpianokbd");
	psy_ui_button_exchange(&self->toggle_kbd_,
		psy_configuration_at(general, "bench.showpianokbd"));	
	psy_ui_button_set_icon(&self->toggle_kbd_, psy_ui_ICON_MORE);
	psy_ui_button_set_text_alignment(&self->toggle_kbd_,
		(psy_ui_Alignment)(psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL));
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->toggle_kbd_));
	/* prevent focus */
	psy_ui_component_prevent_app_focus_out_recursive(&self->component);
	/* connect */
	sequencerbar_connect_buttons(self, general);	
}

void sequencerbar_connect_buttons(SequencerBar* self, psy_Configuration* cfg)
{	
	assert(self);

	psy_configuration_connect(cfg,
		"bench.showstepsequencer", self, sequencerbar_on_step_sequencer);
	psy_configuration_connect(cfg,
		"bench.showsequenceedit", self, sequencerbar_on_seq_editor);
	psy_configuration_connect(cfg,
		"bench.showpianokbd", self, sequencerbar_on_piano_kbd);			
}

void sequencerbar_on_piano_kbd(SequencerBar* self, psy_Property* sender)
{	
	assert(self);

	if (psy_property_item_bool(sender)) {
		psy_ui_button_highlight(&self->toggle_kbd_);
		psy_ui_button_set_icon(&self->toggle_kbd_, psy_ui_ICON_LESS);
	} else {		
		psy_ui_button_disable_highlight(&self->toggle_kbd_);
		psy_ui_button_set_icon(&self->toggle_kbd_, psy_ui_ICON_MORE);
	}
}

void sequencerbar_on_seq_editor(SequencerBar* self, psy_Property* sender)
{
	assert(self);

	if (psy_property_item_bool(sender)) {		
		psy_ui_button_set_icon(&self->toggle_seq_edit_, psy_ui_ICON_LESS);
	} else {				
		psy_ui_button_set_icon(&self->toggle_seq_edit_, psy_ui_ICON_MORE);
	}
}

void sequencerbar_on_step_sequencer(SequencerBar* self, psy_Property* sender)
{
	assert(self);

	if (psy_property_item_bool(sender)) {		
		psy_ui_button_set_icon(&self->toggle_step_seq_, psy_ui_ICON_LESS);
	} else {		
		psy_ui_button_set_icon(&self->toggle_step_seq_, psy_ui_ICON_MORE);
	}
}
