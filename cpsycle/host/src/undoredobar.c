/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "undoredobar.h"
/* host */
#include "resources/resource.h"
#include "styles.h"


#define UNDOREDOBAR_UPDATERATE 100


/* prototypes */
static void undoredobar_on_undo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_on_redo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_on_timer(UndoRedoBar*, uintptr_t timer_id);

/* vtable */
static psy_ui_ComponentVtable undoredobar_vtable;
static bool undoredobar_vtable_initialized = FALSE;

static void undoredobar_vtable_init(UndoRedoBar* self)
{
	if (!undoredobar_vtable_initialized) {
		undoredobar_vtable = *(self->component.vtable);
		undoredobar_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			undoredobar_on_timer;
		undoredobar_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(undoredobar_base(self),
		&undoredobar_vtable);
}

/* implementation */
void undoredobar_init(UndoRedoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);

	psy_ui_component_init(undoredobar_base(self), parent, NULL);
	undoredobar_vtable_init(self);
	self->workspace_ = workspace;
	psy_ui_component_set_default_align(undoredobar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_connect(&self->undo_, undoredobar_base(self),
		self, undoredobar_on_undo);
	psy_ui_button_set_text(&self->undo_, "undo.undo");
	psy_ui_button_load_resource(&self->undo_, IDB_UNDO_LIGHT,
		IDB_UNDO_DARK, psy_ui_colour_white());
	psy_ui_button_init_connect(&self->redo_, undoredobar_base(self),
		self, undoredobar_on_redo);
	psy_ui_button_set_text(&self->redo_,"undo.redo");
	psy_ui_button_load_resource(&self->redo_, IDB_REDO_LIGHT,
		IDB_REDO_DARK, psy_ui_colour_white());	
	psy_ui_component_start_timer(undoredobar_base(self), 0,
		UNDOREDOBAR_UPDATERATE);
}

void undoredobar_on_undo(UndoRedoBar* self, psy_ui_Component* sender)
{
	assert(self);

	workspace_undo(self->workspace_);	
}

void undoredobar_on_redo(UndoRedoBar* self, psy_ui_Component* sender)
{
	assert(self);

	workspace_redo(self->workspace_);
}

void undoredobar_on_timer(UndoRedoBar* self, uintptr_t timer_id)
{
	assert(self);

#ifndef PSYCLE_DEBUG_PREVENT_TIMER_DRAW
	if (workspace_currview_has_undo(self->workspace_)) {
		psy_ui_component_enable_input(psy_ui_button_base(&self->undo_),
			psy_ui_NONE_RECURSIVE);
	} else {
		psy_ui_component_prevent_input(psy_ui_button_base(&self->undo_),
			psy_ui_NONE_RECURSIVE);
	}
	if (workspace_currview_has_redo(self->workspace_)) {
		psy_ui_component_enable_input(psy_ui_button_base(&self->redo_),
			psy_ui_NONE_RECURSIVE);
	} else {
		psy_ui_component_prevent_input(psy_ui_button_base(&self->redo_),
			psy_ui_NONE_RECURSIVE);
	}
#endif
}
