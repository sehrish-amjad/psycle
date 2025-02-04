/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "plugineditor.h"

/* host */
#include "styles.h"
/* file */
#include <dir.h>
/* audio */
#include <luaplugin.h>
#include <exclusivelock.h>
#include <songio.h>
/* platform */
#include "../../detail/portable.h"

#define BLOCKSIZE 128 * 1024

static const char* plugin_source =
"function psycle.info()""\n"
"  local machinemodes = require('psycle.machinemodes')""\n"
"  return  {""\n"
"	 vendor = 'psycle',""\n"
"	 name = 'newplugin',""\n"
"	 generator = 1, --machine.GENERATOR,""\n"
"	 mode = machinemodes.GENERATOR,""\n"
"	 version = 0,""\n"
"	 api = 0 --noteon = 1""\n"
"  }""\n"
"end""\n"
"""\n"
"function psycle.start()""\n"
"  psycle.setmachine(require('newplugin.machine'))""\n"
"end""\n"
"""\n";

static const char* machine_source =
"machine = require('psycle.machine'):new()""\n"
"""\n"
"function machine:init(samplerate)""\n"
"end""\n"
"""\n"
"function machine:work(num)""\n"
"end""\n"
"""\n"
"return machine""\n";

void plugineditorcreatebar_init(PluginEditorCreateBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->desc, &self->component,
		"Plugin Name");
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_text_init(&self->name, &self->component);	
	psy_ui_component_set_align(&self->name.component, psy_ui_ALIGN_CLIENT);
	psy_ui_text_set_text(&self->name, "newplugin");
	psy_ui_button_init_text(&self->create, &self->component, "Create");
	psy_ui_component_set_align(&self->create.component, psy_ui_ALIGN_RIGHT);	
}

/* PluginEditor */

/* prototypes */
static void plugineditor_on_destroyed(PluginEditor*);
static void plugineditor_init_title_bar(PluginEditor*);
static void plugineditor_on_machines_change_slot(PluginEditor*,
	psy_audio_Machines*, uintptr_t slot);
static void plugineditor_on_song_changed(PluginEditor*,
	psy_audio_Player* sender);
static void plugineditor_connect_machine_signals(PluginEditor*, Workspace*);
static void plugineditor_on_new_plugin(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_on_reload(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_on_save(PluginEditor*, psy_ui_Component* sender);
static void plugineditor_build_plugin_list(PluginEditor*, Workspace*);
static void plugineditor_build_file_list(PluginEditor*);
static void plugineditor_on_plugin_selected(PluginEditor*, psy_ui_Component* sender, int slot);
static void plugineditor_on_file_selected(PluginEditor*, psy_ui_Component* sender, int slot);
static int plugineditor_on_enum_dir(PluginEditor*, const char* path, int flag);
static void plugineditor_on_create_new_plugin(PluginEditor*, psy_ui_Component* sender);
static void writetext(const char* path, const char* text);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PluginEditor* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			plugineditor_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void plugineditor_init(PluginEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->workspace = workspace;
	self->basepath = 0;	
	self->instanceidx = psy_INDEX_INVALID;
	self->zoom = 1.0;
	plugineditor_init_title_bar(self);	
	psy_ui_component_init(&self->bar, &self->component, NULL);
	psy_ui_component_set_align(&self->bar, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->reload, &self->bar);
	psy_ui_component_set_align(&self->reload.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->newplugin, &self->bar);
	psy_ui_button_set_text(&self->newplugin, "New Plugin");
	psy_ui_component_set_align(&self->newplugin.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->newplugin.signal_clicked, self,
		&plugineditor_on_new_plugin);
	psy_ui_button_set_text(&self->reload, "Reload");
	psy_signal_connect(&self->reload.signal_clicked, self,
		&plugineditor_on_reload);
	psy_ui_button_init(&self->save, &self->bar);
	psy_ui_component_set_align(&self->save.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_set_text(&self->save, "Save");
	psy_signal_connect(&self->save.signal_clicked, self,
		&plugineditor_on_save);	
	/* Plugin select */
	psy_ui_component_init(&self->row0, &self->component, NULL);
	psy_ui_component_set_align(&self->row0, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->plugindesc, &self->row0,
		"Plugin");
	psy_ui_component_set_align(&self->plugindesc.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->pluginselector, &self->row0);
	/* psy_ui_combobox_set_char_number(&self->pluginselector, 32); */
	psy_ui_component_set_align(&self->pluginselector.component, psy_ui_ALIGN_CLIENT);
	/* File Select */
	psy_ui_component_init(&self->row1, &self->component, NULL);
	psy_ui_component_set_align(&self->row1, psy_ui_ALIGN_TOP);
	psy_ui_label_init_text(&self->filedesc, &self->row1, "File");
	psy_ui_component_set_align(&self->filedesc.component, psy_ui_ALIGN_LEFT);
	psy_ui_combobox_init(&self->fileselector, &self->row1);
	/* psy_ui_combobox_set_char_number(&self->fileselector, 32); */
	psy_ui_component_set_align(&self->fileselector.component, psy_ui_ALIGN_CLIENT);
	plugineditorcreatebar_init(&self->createbar, &self->component, workspace);
	psy_signal_connect(&self->createbar.create.signal_clicked, self,
		plugineditor_on_create_new_plugin);
	psy_ui_component_set_align(&self->createbar.component, psy_ui_ALIGN_TOP);
	psy_ui_component_hide(&self->createbar.component);
	/* editor */
	psy_ui_editor_init(&self->editor, &self->component);	
	psy_ui_component_set_align(&self->editor.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&workspace->player_.signal_song_changed, self,
		plugineditor_on_song_changed);
	psy_signal_connect(&self->pluginselector.signal_selchanged, self,
		plugineditor_on_plugin_selected);
	psy_signal_connect(&self->fileselector.signal_selchanged, self,
		plugineditor_on_file_selected);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(80.0, 0.0));		
	plugineditorbar_init(&self->status, &self->component);	
	plugineditorbar_set_editor(&self->status, self);
	psy_ui_component_set_align(&self->status.component, psy_ui_ALIGN_BOTTOM);
	psy_table_init(&self->pluginmappping);
	plugineditor_build_plugin_list(self, workspace);
	plugineditor_connect_machine_signals(self, workspace);
}

void plugineditor_on_destroyed(PluginEditor* self)
{
	psy_table_dispose(&self->pluginmappping);
}

void plugineditor_init_title_bar(PluginEditor* self)
{
	titlebar_init(&self->titlebar, &self->component, "machinebar.editor");
	closebar_set_property(&self->titlebar.close_bar_,
		psy_configuration_at(psycleconfig_general(workspace_cfg(self->workspace)),
		"bench.showplugineditor"));	
}

void plugineditor_on_machines_change_slot(PluginEditor* self,
	psy_audio_Machines* machines, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(machines, slot);
	if (machine && psy_audio_machine_info(machine) &&
			psy_audio_machine_info(machine)->type == psy_audio_LUA) {
		const char* path;
		char catchername[256];
		
		psy_snprintf(catchername, 256, "%s:0",
			psy_audio_machine_info(machine)->name);		
		path = psy_audio_plugincatcher_search_path(
			&self->workspace->player_.plugincatcher, catchername, psy_audio_LUA);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
			}
			self->instanceidx = slot;
		}		
	}
}

void plugineditor_on_song_changed(PluginEditor* self, psy_audio_Player* sender)
{
	plugineditor_connect_machine_signals(self, self->workspace);
}

void plugineditor_connect_machine_signals(PluginEditor* self,
	Workspace* workspace)
{
	if (workspace->song) {
		psy_signal_connect(&workspace->song->machines_.signal_slotchange, self,
			plugineditor_on_machines_change_slot);
	}
}

void plugineditor_on_new_plugin(PluginEditor* self, psy_ui_Component* sender)
{
	psy_ui_component_show(&self->createbar.component);
	psy_ui_component_align_invalidate(&self->component);
}

void plugineditor_on_reload(PluginEditor* self, psy_ui_Component* sender)
{
	if (workspace_song(self->workspace) && self->instanceidx) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(
			psy_audio_song_machines(workspace_song(self->workspace)),
			self->instanceidx);
		if (machine) {			
			psy_audio_exclusivelock_enter();
			psy_audio_machine_reload(machine);
			psy_audio_exclusivelock_leave();
		}
	}
}

void plugineditor_on_save(PluginEditor* self, psy_ui_Component* sender)
{
	char path[4096];

	psy_ui_combobox_text(&self->fileselector, path);
	if (path[0] != '\0') {
		psy_ui_editor_save(&self->editor, path);
	}
}

void plugineditor_build_plugin_list(PluginEditor* self, Workspace* workspace)
{	
	/*psy_Property* plugins;

	plugins = psy_audio_plugincatcher_plugins(&workspace->player.plugincatcher);
	psy_table_clear(&self->pluginmappping);
	psy_ui_combobox_clear(&self->pluginselector);
	if (plugins) {
		psy_List* p;
		int c;

		for (p = psy_property_begin(plugins), c = 0; p != NULL; p = p->next) {
			psy_Property* property;

			property = (psy_Property*)psy_list_entry(p);
			if (psy_property_at_int(property, "type", -1) == psy_audio_LUA) {
				psy_ui_combobox_add_text(&self->pluginselector,
					psy_property_key(property));
				psy_table_insert(&self->pluginmappping, (uintptr_t)c, property);
				++c;
			}
		}
	}	*/
}

void plugineditor_on_plugin_selected(PluginEditor* self, psy_ui_Component*
	sender, int slot)
{
	psy_Property* p;

	p = (psy_Property*)psy_table_at(&self->pluginmappping, slot);
	if (p) {
		const char* path;

		path = psy_property_at_str(p, "path", 0);
		if (path) {
			if (self->basepath == 0 || (strcmp(path, self->basepath) != 0)) {
				psy_ui_editor_clear(&self->editor);
				psy_ui_editor_load(&self->editor, path);
				self->basepath = path;
				self->instanceidx = psy_INDEX_INVALID;
				plugineditor_build_file_list(self);
				psy_ui_combobox_select(&self->fileselector, 1);
				plugineditor_on_file_selected(self, &self->fileselector.component, 1);
			}
		}
	}	
}

void plugineditor_on_file_selected(PluginEditor* self, psy_ui_Component* sender, int slot)
{
	char path[4096];

	psy_ui_combobox_text(&self->fileselector, path);
	psy_ui_editor_clear(&self->editor);
	psy_ui_editor_load(&self->editor, path);	
}

void plugineditor_build_file_list(PluginEditor* self)
{
	if (self->basepath) {
		char prefix[4096];
		char name[4096];
		char ext[4096];
		char path[4096];

		psy_ui_combobox_clear(&self->fileselector);
		psy_ui_combobox_add_text(&self->fileselector, self->basepath);
		psy_dir_extract_path(self->basepath, prefix, name, ext);		
		psy_snprintf(path, 4096, "%s\\%s", prefix, name);		
		psy_dir_enumerate_recursive(self, path, "*.lua",
			psy_audio_PLUGIN, (psy_fp_findfile)plugineditor_on_enum_dir);		
	}
}

int plugineditor_on_enum_dir(PluginEditor* self, const char* path, int type)
{
	psy_ui_combobox_add_text(&self->fileselector, path);
	return 1;
}

void plugineditor_on_create_new_plugin(PluginEditor* self,
	psy_ui_Component* sender)
{
	const char* dir;
	psy_Path path;	
	psy_Path plugindir;
	
	dir = psy_configuration_value_str(
		psycleconfig_directories(workspace_cfg(self->workspace)),		
		"plugins.luascripts", PSYCLE_LUASCRIPTS_DEFAULT_DIR);
	psy_path_init(&path, "");
	psy_path_set_prefix(&path, dir);
	psy_path_set_name(&path, "newplugin.lua");
	writetext(psy_path_full(&path), plugin_source);
	psy_path_dispose(&path);
	psy_path_init(&plugindir, "");
	psy_path_set_prefix(&plugindir, dir);
	psy_path_set_name(&plugindir, "newplugin");
	if (!psy_direxists(psy_path_full(&plugindir))) {
		psy_mkdir(psy_path_full(&plugindir));
	}
	psy_path_init(&path, "");
	psy_path_set_prefix(&path, psy_path_full(&plugindir));
	psy_path_dispose(&plugindir);	
	psy_path_set_name(&path, "machine.lua");
	writetext(psy_path_full(&path), machine_source);
	psy_path_dispose(&path);
}

void writetext(const char* path, const char* text)
{
	FILE* fp;

	fp = fopen(path, "wb");
	if (fp) {		
		uintptr_t lengthdoc;
		uintptr_t i;

		lengthdoc = psy_strlen(text);
		for (i = 0; i < lengthdoc; i += BLOCKSIZE) {
			uintptr_t grabsize;

			grabsize = lengthdoc - i;
			if (grabsize > BLOCKSIZE) {
				grabsize = BLOCKSIZE;
			}
			fwrite(&text[i], grabsize, 1, fp);
		}
		fclose(fp);
	}
}

void plugineditor_on_focus(PluginEditor* self, psy_ui_Component* sender)
{
	
}

void plugineditor_update_font(PluginEditor* self)
{	
	psy_ui_FontInfo fontinfo;
	psy_ui_Font font;
	double zoomrate;

	assert(self);

	zoomrate = psy_ui_app_zoom_rate(psy_ui_app()) * self->zoom;	
	psy_ui_fontinfo_init(&fontinfo, "Consolas", 16);
	fontinfo.lfHeight = (int32_t)((double)fontinfo.lfHeight * zoomrate);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_set_font(&self->editor.text_area_.pane.component, &font);
	psy_ui_font_dispose(&font);	
	if (psy_ui_component_visible(&self->component)) {
		psy_ui_component_align_full(&self->component);
		psy_ui_component_invalidate(&self->component);
	}
}
