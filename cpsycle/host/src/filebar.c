/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "filebar.h"
/* host */
#include "resources/resource.h"


/* prototypes */
static void filebar_init_buttons(FileBar*);
static void filebar_connect_configure(FileBar*);
static void filebar_on_new_song(FileBar*, psy_ui_Component* sender);
static void filebar_on_disk_op(FileBar*, psy_ui_Component* sender);
static void filebar_on_load_song(FileBar*, psy_ui_Component* sender);
static void filebar_on_save_song_as(FileBar*, psy_ui_Component* sender);
static void filebar_on_ft2_explorer(FileBar*, psy_Property* sender);
static void filebar_on_render(FileBar*, psy_ui_Component* sender);

/* implementation */
void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(filebar_base(self), parent, NULL);	
	self->workspace_ = workspace;
	psy_ui_component_set_default_align(filebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_label_init_text(&self->desc_, filebar_base(self), "file.song");
	filebar_init_buttons(self);
	filebar_connect_configure(self);
}

void filebar_init_buttons(FileBar* self)
{
	assert(self);
	
	psy_ui_button_init_text_connect(&self->song_new_, filebar_base(self),
		"file.new", self, filebar_on_new_song);	
	psy_ui_button_load_resource(&self->song_new_, IDB_NEW_DARK,
		IDB_NEW_DARK, psy_ui_colour_white());	
	psy_ui_button_init_text_connect(&self->song_disk_op_, filebar_base(self),
		"file.disk_op", self, filebar_on_disk_op);	
	psy_ui_button_init_text_connect(&self->song_load_, filebar_base(self),
		"file.load", self, filebar_on_load_song);	
	psy_ui_button_load_resource(&self->song_load_, IDB_OPEN_LIGHT,
		IDB_OPEN_DARK, psy_ui_colour_white());	
	psy_ui_button_init_text_connect(&self->song_save_, filebar_base(self),
		"file.save", self, filebar_on_save_song_as);
	psy_ui_button_load_resource(&self->song_save_, IDB_SAVE_LIGHT,
		IDB_SAVE_DARK, psy_ui_colour_white());	
	psy_ui_button_init_text_connect(&self->song_render_, filebar_base(self),
		"file.render", self, filebar_on_render);	
	psy_ui_button_load_resource(&self->song_render_, IDB_PULSE_LIGHT,
		IDB_PULSE_DARK, psy_ui_colour_white());	
}

void filebar_connect_configure(FileBar* self)
{
	assert(self);
	
	psy_configuration_connect(psycleconfig_misc(workspace_cfg(
		self->workspace_)), "ft2fileexplorer", self, filebar_on_ft2_explorer);
	psy_configuration_configure(psycleconfig_misc(workspace_cfg(
		self->workspace_)), "ft2fileexplorer");
}

void filebar_on_new_song(FileBar* self, psy_ui_Component* sender)
{
	assert(self);
	
	workspace_new_song(self->workspace_);
}

void filebar_on_disk_op(FileBar* self, psy_ui_Component* sender)
{
	assert(self);
	
	workspace_disk_op_song(self->workspace_);
}

void filebar_on_load_song(FileBar* self, psy_ui_Component* sender)
{	
	assert(self);
			
	workspace_load_song(self->workspace_);
}

void filebar_on_save_song_as(FileBar* self, psy_ui_Component* sender)
{	
	assert(self);
				
	workspace_save_song_as(self->workspace_);
}

void filebar_on_ft2_explorer(FileBar* self, psy_Property* sender)
{
	assert(self);
	
	if (psy_property_item_bool(sender)) {				
		psy_ui_component_show(psy_ui_button_base(&self->song_disk_op_));
		psy_ui_component_hide(psy_ui_button_base(&self->song_load_));
		psy_ui_component_hide(psy_ui_button_base(&self->song_save_));
	} else {
		psy_ui_component_hide(psy_ui_button_base(&self->song_disk_op_));
		psy_ui_component_show(psy_ui_button_base(&self->song_load_));
		psy_ui_component_show(psy_ui_button_base(&self->song_save_));
	}
	if (psy_ui_component_draw_visible(psy_ui_app_main(psy_ui_app()))) {
		psy_ui_component_align_invalidate(psy_ui_app_main(psy_ui_app()));		
	}
}

void filebar_on_render(FileBar* self, psy_ui_Component* sender)
{
	assert(self);
	
	workspace_select_view(self->workspace_,
		viewindex_make(VIEW_ID_RENDERVIEW));
}
