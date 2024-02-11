/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "messageview.h"
/* host */
#include "cmdsgeneral.h"


/* prototypes */
static void messageview_init_close(MessageView*, InputHandler*);
static void messageview_init_tabbar(MessageView*);
static void messageview_init_notebook(MessageView*);
static void messageview_init_status(MessageView*);

/* implementation */
void messageview_init(MessageView* self, psy_ui_Component* parent,
	InputHandler* input_handler)
{	
	assert(self);	
	
	psy_ui_component_init(&self->component, parent, NULL);	
	messageview_init_close(self, input_handler);
	messageview_init_tabbar(self);
	messageview_init_notebook(self);	
	messageview_init_status(self);	
	psy_ui_tabbar_select(&self->tabbar_, 0);
}

void messageview_init_close(MessageView* self, InputHandler* input_handler)
{
	assert(self);

	closebar_init_cmd(&self->close_, messageview_base(self), input_handler,
		"general", psy_eventdrivercmd_make_cmd(CMD_IMM_TERMINAL));
}

void messageview_init_tabbar(MessageView* self)
{
	assert(self);

	psy_ui_tabbar_init(&self->tabbar_, messageview_base(self));
	psy_ui_tabbar_set_tab_align(&self->tabbar_, psy_ui_ALIGN_TOP);
	psy_ui_tabbar_append(&self->tabbar_, "Status",
		psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID, psy_ui_colour_white());
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar_),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(psy_ui_tabbar_base(&self->tabbar_),
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));
}

void messageview_init_notebook(MessageView* self)
{
	assert(self);

	psy_ui_notebook_init(&self->notebook_, messageview_base(self));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook_),
		psy_ui_ALIGN_CLIENT);
	psy_ui_notebook_connect_controller(&self->notebook_,
		&self->tabbar_.signal_change);
}

void messageview_init_status(MessageView* self)
{
	assert(self);
	
	psy_ui_terminal_init(&self->terminal_, psy_ui_notebook_base(
		&self->notebook_));
	psy_ui_component_set_align(psy_ui_terminal_base(&self->terminal_),
		psy_ui_ALIGN_CLIENT);	
}
