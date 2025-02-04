/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "kbdhelp.h"
#include "cmdsgeneral.h"

/* prototypes */
static void kbdhelp_buildtabbar(KbdHelp*);
static void kbdhelp_ontabbarchange(KbdHelp*, psy_ui_Component* sender,
	int tabindex);
static void kbdhelp_setcmdsection(KbdHelp* self, uintptr_t index);
static const psy_Property* kbdhelp_cmds(const KbdHelp*);

/* implementation */
void kbdhelp_init(KbdHelp* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(kbdhelp_base(self), parent, NULL);
	self->workspace = workspace;
	closebar_init(&self->close_bar, kbdhelp_base(self), NULL);
	closebar_set_cmd(&self->close_bar, workspace_input_handler(self->workspace),
		"general", psy_eventdrivercmd_make_cmd(CMD_IMM_HELPSHORTCUT));	
	kbdbox_init(&self->kbdbox, kbdhelp_base(self), workspace);
	psy_ui_component_set_align(kbdbox_base(&self->kbdbox),
		psy_ui_ALIGN_CENTER);	
	psy_ui_tabbar_init(&self->tabbar, &self->component);
	psy_ui_tabbar_set_tab_align(&self->tabbar, psy_ui_ALIGN_TOP);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_RIGHT);
	psy_ui_tabbar_prevent_translation(&self->tabbar);
	psy_signal_connect(&self->tabbar.signal_change, self,
		kbdhelp_ontabbarchange);	
	kbdhelp_buildtabbar(self);
}

void kbdhelp_buildtabbar(KbdHelp* self)
{		
	const psy_Property* cmds;
	const psy_List* p;

	cmds = kbdhelp_cmds(self);
	if (!cmds) {
		return;
	}
	for (p = psy_property_begin_const(cmds); p != NULL; p = p->next) {
		const psy_Property* property;

		property = (const psy_Property*)psy_list_entry_const(p);
		if (psy_property_is_section(property)) {
			psy_ui_tabbar_append(&self->tabbar, psy_property_text(property),
				psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID,
				psy_ui_colour_white());
		}
	}		
	psy_ui_tabbar_select(&self->tabbar, 0);
}

void kbdhelp_ontabbarchange(KbdHelp* self, psy_ui_Component* sender,
	int tabindex)
{	
	kbdhelp_setcmdsection(self, tabindex);
}

void kbdhelp_setcmdsection(KbdHelp* self, uintptr_t index)
{
	const psy_Property* cmds;
	const psy_Property* section;

	kbdbox_clear_descriptions(&self->kbdbox);
	cmds = kbdhelp_cmds(self);
	if (!cmds) {
		return;
	}
	section = psy_property_at_index_const(cmds, index);
	if (section) {
		const psy_List* p;

		for (p = psy_property_begin_const(section); p != NULL;
			psy_list_next_const(&p)) {
			const psy_Property* property;

			property = (const psy_Property*)psy_list_entry_const(p);
			kbdbox_set_description(&self->kbdbox,
				(uint32_t)psy_property_item_int(property),
				psy_property_short_text(property));
		}
	}
}

const psy_Property* kbdhelp_cmds(const KbdHelp* self)
{
	psy_EventDriver* kbd;

	kbd = workspace_kbd_driver(self->workspace);
	if (kbd && psy_eventdriver_configuration(kbd)) {
		return psy_property_at_section_const(
			psy_eventdriver_configuration(kbd), "cmds");
	}
	return NULL;
}
