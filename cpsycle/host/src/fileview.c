/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "fileview.h"
/* host */
#include "inputhandler.h"
#include "styles.h"
#include "viewindex.h"
/* ui */
#include <uiapp.h>
/* file */
#include <dir.h>
/* container */
#include <qsort.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"


/* FileViewFilter */

/* prototypes */
static void fileviewfilter_on_destroyed(FileViewFilter*);
static void fileviewfilter_build(FileViewFilter*, const psy_List* types);
static void fileviewfilter_on_item(FileViewFilter*, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable fileviewfilter_vtable;
static bool fileviewfilter_vtable_initialized = FALSE;

static void fileviewfilter_vtable_init(FileViewFilter* self)
{
	if (!fileviewfilter_vtable_initialized) {
		fileviewfilter_vtable = *(self->component.vtable);
		fileviewfilter_vtable.on_destroyed =
			(psy_ui_fp_component)
			fileviewfilter_on_destroyed;
		fileviewfilter_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &fileviewfilter_vtable);	
}

/* implementation */
void fileviewfilter_init(FileViewFilter* self, psy_ui_Component* parent,
	const char* title)
{	
	assert(self);
	
	psy_ui_component_init(fileviewfilter_base(self), parent, NULL);
	fileviewfilter_vtable_init(self);	
	psy_ui_component_set_default_align(fileviewfilter_base(self),
		psy_ui_ALIGN_TOP, psy_ui_defaults_vmargin(psy_ui_app_defaults()));
	psy_ui_label_init_text(&self->desc, fileviewfilter_base(self), title);
	psy_ui_component_init(&self->items, fileviewfilter_base(self), NULL);
	psy_ui_component_set_align(&self->items, psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->items, psy_ui_margin_make_em(
		0.0, 0.0, 0.0, 2.0));
	psy_ui_component_set_default_align(&self->items, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_app_defaults()));
	psy_property_init(&self->filter);	
	self->show_all_ = FALSE;
	psy_signal_init(&self->signal_changed);	
}

void fileviewfilter_on_destroyed(FileViewFilter* self)
{
	assert(self);

	psy_property_dispose(&self->filter);	
	psy_signal_dispose(&self->signal_changed);
}

void fileviewfilter_set_filter(FileViewFilter* self, const psy_List* types)
{		
	assert(self);
	
	fileviewfilter_build(self, types);
}

void fileviewfilter_connect(FileViewFilter* self, void* context, void* fp)
{
	assert(self);

	psy_signal_connect(&self->signal_changed, context, fp);		
}

void fileviewfilter_show_all(FileViewFilter* self)
{
	assert(self);

	self->show_all_ = TRUE;
}

void fileviewfilter_show_filter(FileViewFilter* self)
{
	assert(self);

	self->show_all_ = FALSE;
}

void fileviewfilter_build(FileViewFilter* self, const psy_List* types)
{
	const psy_List* p;
	psy_Property* choice;
	uintptr_t i;
	
	assert(self);
	
	psy_ui_component_clear(&self->items);
	psy_property_clear(&self->filter);
	choice = psy_property_append_choice(&self->filter, "types", 0);
	for (p = types, i = 0; p != NULL; p = p->next, ++i) {
		psy_Property* curr;
		const psy_FileSelectFilterItem* item;
		char key[64];		
		psy_ui_CheckBox* check;	
		
		item = (const psy_FileSelectFilterItem*)p->entry;
		psy_snprintf(key, 64, "filter-%d", (int)i);
		curr = psy_property_set_text(psy_property_set_id(
			psy_property_append_str(choice, key, item->wildcard),
			FILEVIEWFILTER_PSY), item->label);
		check = psy_ui_checkbox_allocinit(&self->items);
		psy_ui_checkbox_exchange(check, curr);
	}
	psy_property_connect(choice, self, fileviewfilter_on_item);
}

const char* fileviewfilter_type(const FileViewFilter* self)
{
	static const char* all = "*.*";	
	psy_Property* item;
	const psy_Property* types;
	
	assert(self);

	if (self->show_all_) {
		return all;
	}	
	types = psy_property_at_const(&self->filter, "types",
		PSY_PROPERTY_TYPE_NONE);
	if (types) {
		item = psy_property_at_choice((psy_Property*)types);
		if (item) {
			return psy_property_item_str(item);
		}
	}
	return all;
}

void fileviewfilter_on_item(FileViewFilter* self, psy_Property* sender)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 0);
}


/* FileViewFolderLinks */

/* prototypes */
static void fileviewlinks_on_destroyed(FileViewLinks*);
static void fileviewlinks_on_button(FileViewLinks*, psy_ui_Button* sender);

/* vtable */
static psy_ui_ComponentVtable fileviewlinks_vtable;
static bool fileviewlinks_vtable_initialized = FALSE;

static void fileviewlinks_vtable_init(FileViewLinks* self)
{
	assert(self);
	
	if (!fileviewlinks_vtable_initialized) {
		fileviewlinks_vtable = *(self->component.vtable);
		fileviewlinks_vtable.on_destroyed =
			(psy_ui_fp_component)
			fileviewlinks_on_destroyed;
		fileviewlinks_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &fileviewlinks_vtable);
}

/* implementation */
void fileviewlinks_init(FileViewLinks* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(fileviewlinks_base(self), parent, NULL);
	fileviewlinks_vtable_init(self);
	psy_signal_init(&self->signal_selected);
	psy_table_init(&self->locations_);
	psy_ui_component_set_style_type(fileviewlinks_base(self),
		STYLE_FILEVIEW_LINKS);
	psy_ui_component_set_default_align(fileviewlinks_base(self),
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
}

void fileviewlinks_on_destroyed(FileViewLinks* self)
{
	assert(self);
	
	psy_signal_dispose(&self->signal_selected);
	psy_table_dispose_all(&self->locations_, NULL);	
}

void fileview_add_drives(FileViewLinks* self)
{
	psy_List* p;
	psy_List* q;

	assert(self);

	fileviewlinks_add(self, ".."psy_SLASHSTR, "..");
	for (q = p = psy_drives(); p != NULL; psy_list_next(&p)) {
		fileviewlinks_add(self, (char*)psy_list_entry(p),
			(char*)psy_list_entry(p));
	}
	psy_list_deallocate(&q, NULL);
}

void fileviewlinks_add(FileViewLinks* self, const char* label,
	const char* path)
{
	psy_ui_Button* button;

	assert(self);
	
	button = psy_ui_button_allocinit(fileviewlinks_base(self));
	if (!button) {
		return;
	}
	psy_signal_connect(&button->signal_clicked, self, 
		fileviewlinks_on_button);
	psy_ui_button_prevent_translation(button);
	psy_ui_button_set_text(button, label);
	psy_table_insert(&self->locations_, psy_strhash(label),
		psy_strdup(path));
}

const char* fileviewlinks_path(FileViewLinks* self, uintptr_t index)
{
	assert(self);

	return (const char*)psy_table_at(&self->locations_, index);
}

void fileviewlinks_on_button(FileViewLinks* self, psy_ui_Button* sender)
{	
	assert(self);
	
	psy_signal_emit(&self->signal_selected, self, 1,
		psy_strhash(psy_ui_button_text(sender)));
}

void fileviewlinks_change_path(FileViewLinks* self, uintptr_t index,
	const char* path)
{
	char* old;
	char* insert;
	
	assert(self);
	
	insert = psy_strdup(path);
	old = psy_table_at(&self->locations_, index);
	free(old);
	old = NULL;
	psy_table_insert(&self->locations_, index, insert);
}


/* FileViewCommands */

/* implementation */
void fileviewcommands_init(FileViewCommands* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(fileviewcommands_base(self), parent, NULL);
	psy_ui_component_set_default_align(fileviewcommands_base(self),
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());		
	psy_ui_button_init_text(&self->save_select, fileviewcommands_base(self),
		"file.save");
	psy_ui_button_init_text(&self->mkdir, fileviewcommands_base(self),
		"file.mkdir");
	psy_ui_text_init(&self->dir, fileviewcommands_base(self));
	psy_ui_text_enable_input_field(&self->dir);
	psy_ui_text_set_char_number(&self->dir, 10.0);
	psy_ui_component_hide(psy_ui_text_base(&self->dir));
	psy_ui_button_init_text(&self->refresh, fileviewcommands_base(self),
		"file.refresh");	
	psy_ui_button_init_text(&self->showall, fileviewcommands_base(self),
		"file.showall");	
	psy_ui_button_init_text(&self->exit, fileviewcommands_base(self),
		"file.exit");
}

void fileviewcommands_set_select(FileViewCommands* self)
{
	assert(self);

	psy_ui_button_set_text(&self->save_select, "Select");
}

void fileviewcommands_set_save(FileViewCommands* self)
{
	assert(self);

	psy_ui_button_set_text(&self->save_select, "file.save");
}


/* FileView */

/* prototypes */
static void fileview_on_destroyed(FileView*);
static void fileview_init_base(FileView*);
static void fileview_init_title(FileView*, psy_ui_Component* viewbar_parent);
static void fileview_init_filename_edit(FileView*);
static void fileview_init_options(FileView*);
static void fileview_init_links(FileView*);
static void fileview_init_filename_edit(FileView*);
static void fileview_init_files(FileView*, InputHandler*);
static void fileview_add_links(FileView*);
static void fileview_init_filters(FileView*);
static void fileview_init_commands(FileView*);
static void fileview_on_file_selected(FileView*, FileBox* sender);
static void fileview_on_dir_changed(FileView*, FileBox* sender);
static void fileview_on_preview_selected(FileView*, FileBox* sender);
static void fileview_on_link(FileView*, FileViewLinks* sender, intptr_t index);
static void fileview_update_dir_label(FileView*);
static void fileview_on_load_filter(FileView*, psy_ui_Component* sender);
static void fileview_on_save_filter(FileView*, psy_ui_Component* sender);
static void fileview_on_mkdir(FileView*, psy_ui_Component* sender);
static void fileview_on_exit(FileView*, psy_ui_Component* sender);
static void fileview_on_key_down(FileView*, psy_ui_KeyboardEvent*);
static void fileview_do_exit(FileView*);
static void fileview_on_save_or_select_button(FileView*,
	psy_ui_Component* sender);
static void fileview_update_save_view(FileView*, bool has_save);
static void fileview_on_focus(FileView*);
static void fileview_on_dir_edit_accept(FileView*, psy_ui_Text* sender);
static void fileview_on_dir_edit_reject(FileView*, psy_ui_Text* sender);
static void fileview_update_file_name(FileView*, const psy_FileSelect*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(FileView* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			fileview_on_destroyed;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			fileview_on_key_down;
		vtable.on_focus =
			(psy_ui_fp_component)
			fileview_on_focus;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void fileview_init(FileView* self, psy_ui_Component* parent,
	psy_ui_Component* viewbar_parent, InputHandler* input_handler,
	const char* recent_path)
{	
	assert(self);
		
	psy_ui_component_init(fileview_base(self), parent, NULL);
	vtable_init(self);	
	self->recent_files_ = psy_strdup(recent_path);
	psy_fileselect_init(&self->select_load_);
	psy_fileselect_init(&self->select_save_);
	psy_fileselect_init(&self->select_dir_);
	fileview_init_base(self);
	fileview_init_title(self, viewbar_parent);
	fileview_init_filename_edit(self);	
	fileview_init_options(self);
	fileview_init_files(self, input_handler);			
	fileview_update_dir_label(self);
}

void fileview_on_destroyed(FileView* self)
{	
	assert(self);
	
	psy_fileselect_dispose(&self->select_load_);
	psy_fileselect_dispose(&self->select_save_);
	psy_fileselect_dispose(&self->select_dir_);
	free(self->recent_files_);
	self->recent_files_ = NULL;
}

FileView* fileview_alloc(void)
{
	return (FileView*)malloc(sizeof(FileView));
}

FileView* fileview_alloc_init(psy_ui_Component* parent,
	psy_ui_Component* viewbar_parent, InputHandler* input,
	const char* recent_files)
{
	FileView* rv;

	rv = fileview_alloc();
	if (rv) {
		fileview_init(rv, parent, viewbar_parent, input, recent_files);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void fileview_init_base(FileView* self)
{
	assert(self);

	psy_ui_component_set_id(fileview_base(self), VIEW_ID_FILEVIEW);
	psy_ui_component_set_tab_index(fileview_base(self), 0);
	psy_ui_component_set_preferred_size(fileview_base(self),
		psy_ui_size_make_em(80.0, 25.0));
	psy_ui_component_set_padding(fileview_base(self),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
}

void fileview_init_title(FileView* self, psy_ui_Component* viewbar_parent)
{
	assert(self);

	psy_ui_label_init_text(&self->view_bar_, viewbar_parent, "FileView");
	psy_ui_component_set_title(fileview_base(self), "FileView");
	psy_ui_component_set_style_type(psy_ui_label_base(&self->view_bar_),
		psy_ui_STYLE_TAB_SELECT);
	psy_ui_component_set_id(psy_ui_label_base(&self->view_bar_),
		VIEW_ID_FILEVIEW);
	psy_ui_component_set_align(psy_ui_label_base(&self->view_bar_),
		psy_ui_ALIGN_LEFT);	
}

void fileview_init_links(FileView* self)
{
	assert(self);

	fileviewlinks_init(&self->links_, &self->options_);
	psy_ui_component_set_align(fileviewlinks_base(&self->links_),
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->links_.signal_selected, self, fileview_on_link);
	fileview_add_links(self);
}

void fileview_init_filename_edit(FileView* self)
{
	assert(self);

	psy_ui_component_init(&self->bottom_, fileview_base(self), NULL);
	psy_ui_component_set_align(&self->bottom_, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->bottom_, psy_ui_margin_make_em(
		0.5, 0.0, 0.0, 0.0));
	psy_ui_label_init(&self->dir_, &self->bottom_);
	psy_ui_component_set_style_type(psy_ui_label_base(&self->dir_),
		STYLE_FILEVIEW_DIRBAR);
	psy_ui_component_set_align(psy_ui_label_base(&self->dir_),
		psy_ui_ALIGN_TOP);
	psy_ui_component_init_align(&self->file_bar_, &self->bottom_, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->file_bar_,
		psy_ui_margin_make_em(0.5, 0.0, 0.0, 0.0));
	psy_ui_label_init_text(&self->file_desc_, &self->file_bar_, "file.file");
	psy_ui_component_set_align(psy_ui_label_base(&self->file_desc_),
		psy_ui_ALIGN_LEFT);
	psy_ui_text_init(&self->file_name_, &self->file_bar_);
	psy_ui_text_set_text(&self->file_name_, psy_fileselect_value(
		&self->select_load_));
	psy_ui_component_set_align(psy_ui_text_base(&self->file_name_),
		psy_ui_ALIGN_CLIENT);
}

void fileview_init_options(FileView* self)
{
	assert(self);

	psy_ui_component_init_align(&self->left_, fileview_base(self), NULL,
		psy_ui_ALIGN_LEFT);	
	psy_ui_component_set_style_type(&self->left_, STYLE_SIDE_VIEW);
	psy_ui_component_init_align(&self->options_, &self->left_, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->options_,
		psy_ui_margin_make_em(1.25, 1.0, 0.0, 1.0));
	fileview_init_filters(self);
	fileview_init_commands(self);
	fileview_init_links(self);
}

void fileview_add_links(FileView* self)
{
	assert(self);

	fileview_add_drives(&self->links_);
	fileviewlinks_add(&self->links_, "Recent", "//recent");
	fileviewlinks_add(&self->links_, "Home", psy_dir_home());
}

void fileview_init_filters(FileView* self)
{
	assert(self);

	psy_ui_component_init(&self->filters_, &self->options_, NULL);
	psy_ui_component_set_align(&self->filters_, psy_ui_ALIGN_LEFT);
	psy_ui_component_set_margin(&self->filters_,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_component_set_default_align(&self->filters_,
		psy_ui_ALIGN_TOP, psy_ui_margin_make_em(0.25, 0.0, 1.0, 0.0));
	fileviewfilter_init(&self->load_filter_, &self->filters_, "Item:");
	fileviewfilter_connect(&self->load_filter_, self, fileview_on_load_filter);
	fileviewfilter_init(&self->save_filter_, &self->filters_, "Save as:");
	fileviewfilter_connect(&self->save_filter_, self, fileview_on_save_filter);
}

void fileview_init_commands(FileView* self)
{
	assert(self);

	fileviewcommands_init(&self->buttons_, &self->options_);
	psy_ui_component_set_margin(&self->buttons_.component,
		psy_ui_margin_make_em(0.0, 4.0, 2.0, 4.0));
	psy_ui_component_set_align(&self->buttons_.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_button_connect(&self->buttons_.save_select,
		self, fileview_on_save_or_select_button);
	psy_ui_button_connect(&self->buttons_.mkdir,
		self, fileview_on_mkdir);
	psy_signal_connect(&self->buttons_.dir.signal_accept, self,
		fileview_on_dir_edit_accept);
	psy_signal_connect(&self->buttons_.dir.signal_reject, self,
		fileview_on_dir_edit_reject);
	psy_ui_button_connect(&self->buttons_.showall,
		self, fileview_on_load_filter);
	psy_ui_button_connect(&self->buttons_.refresh,
		self, fileview_on_load_filter);
	psy_ui_button_connect(&self->buttons_.exit,
		self, fileview_on_exit);
}

void fileview_init_files(FileView* self, InputHandler* input_handler)
{
	assert(self);
	
	psy_ui_component_init_align(&self->client_, fileview_base(self), NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_style_type(&self->client_, STYLE_CLIENT_VIEW);
	filebox_init(&self->file_box_, &self->client_, input_handler);
	psy_ui_component_set_align(filebox_base(&self->file_box_),
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_margin(filebox_base(&self->file_box_),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_component_set_align(psy_ui_text_base(&self->file_name_),
		psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->file_box_.signal_selected, self,
		fileview_on_file_selected);
	psy_signal_connect(&self->file_box_.signal_dir_changed, self,
		fileview_on_dir_changed);
	psy_signal_connect(&self->file_box_.signal_preview, self,
		fileview_on_preview_selected);
}

void fileview_on_file_selected(FileView* self, FileBox* sender)
{		
	char path[4096];
	
	assert(self);
	
	psy_ui_text_set_text(&self->file_name_, filebox_file_name(
		&self->file_box_));
	filebox_full_name(&self->file_box_, path, 4096);
	psy_fileselect_set_value(&self->select_load_, path);
	psy_fileselect_set_directory(&self->select_load_,
		psy_path_full(&self->file_box_.curr_dir));
	psy_fileselect_notify(&self->select_load_);
	psy_ui_app_continue(psy_ui_app());
}

void fileview_on_preview_selected(FileView* self, FileBox* sender)
{
	char path[4096];

	assert(self);

	psy_ui_text_set_text(&self->file_name_,
		filebox_preview_name(&self->file_box_));
	filebox_full_preview_name(&self->file_box_, path, 4096);
	psy_fileselect_set_value(&self->select_load_, path);
	psy_fileselect_notify_file_name_change(&self->select_load_);
}

void fileview_on_dir_changed(FileView* self, FileBox* sender)
{
	assert(self);
	
	psy_ui_label_set_text(&self->dir_, psy_path_full(
		&self->file_box_.curr_dir));
	psy_fileselect_set_directory(&self->select_load_,
		psy_path_full(&self->file_box_.curr_dir));
	psy_fileselect_notify_change_dir(&self->select_load_);
}

void fileview_update_dir_label(FileView* self)
{	
	assert(self);
	
	psy_ui_label_set_text(&self->dir_, psy_path_full(
		&self->file_box_.curr_dir));
}

void fileview_filename(const FileView* self, char* file_name_,
	uintptr_t maxsize)
{
	assert(self);
	
	file_name_[0] = '\0';
	if (psy_strlen(psy_ui_text_text(&self->file_name_)) > 0) {
		psy_snprintf(file_name_, maxsize, "%s%s%s",
			psy_path_prefix(&self->file_box_.curr_dir),
			psy_SLASHSTR, psy_ui_text_text(&self->file_name_));
		printf("%s\n", file_name_);
	}	
}

void fileview_on_link(FileView* self, FileViewLinks* sender, intptr_t index)
{
	const char* path;
	
	assert(self);
	
	path = fileviewlinks_path(sender, index);
	if (strcmp(path, "//recent") == 0) {		
		self->file_box_.read_from_file_list = TRUE;
		fileview_set_directory(self, self->recent_files_);
	} else if (path) {
		self->file_box_.read_from_file_list = FALSE;
		fileview_set_directory(self, path);
	}	
}

void fileview_set_directory(FileView* self, const char* path)
{	
	assert(self);
	
	filebox_set_directory(&self->file_box_, path);
	fileview_update_dir_label(self);
}

void fileview_on_load_filter(FileView* self, psy_ui_Component* sender)
{	
	assert(self);
	
	if (sender == psy_ui_button_base(&self->buttons_.showall)) {
		fileviewfilter_show_all(&self->load_filter_);
	} else {
		fileviewfilter_show_filter(&self->load_filter_);
	}
	filebox_set_wildcard(&self->file_box_, fileviewfilter_type(
		&self->load_filter_));
	psy_ui_component_align_full(filebox_base(&self->file_box_));
}

void fileview_on_save_filter(FileView* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_strlen(psy_ui_text_text(&self->file_name_)) > 0) {
		psy_Path path;
		psy_Path ext;
		
		psy_path_init(&path, psy_ui_text_text(&self->file_name_));
		psy_path_init(&ext, fileviewfilter_type(&self->save_filter_));
		psy_path_set_ext(&path, psy_path_ext(&ext));
		psy_ui_text_set_text(&self->file_name_, psy_path_full(&path));
		psy_path_dispose(&ext);
		psy_path_dispose(&path);
	}
}

void fileview_on_exit(FileView* self, psy_ui_Component* sender)
{
	assert(self);

	fileview_do_exit(self);
}

void fileview_do_exit(FileView* self)
{
	assert(self);

	if (psy_fileselect_cancel_connected(&self->select_save_)) {
		psy_fileselect_cancel(&self->select_save_);
	} else if (psy_fileselect_cancel_connected(&self->select_load_)) {
		psy_fileselect_cancel(&self->select_load_);
	} else if (psy_fileselect_cancel_connected(&self->select_dir_)) {
		psy_fileselect_cancel(&self->select_dir_);
	}
	psy_ui_app_continue(psy_ui_app());
}

void fileview_on_save_or_select_button(FileView* self,
	psy_ui_Component* sender)
{	
	assert(self);

	if (psy_fileselect_connected(&self->select_dir_)) {
		psy_fileselect_set_value(&self->select_dir_,
		psy_path_full(&self->file_box_.curr_dir));
		psy_fileselect_notify(&self->select_dir_);
	} else {
		char path[4096];		
		
		fileview_filename(self, path, 4096);
		psy_fileselect_set_value(&self->select_save_, path);
		psy_fileselect_notify(&self->select_save_);
		filebox_refresh(&self->file_box_);
	}
}

void fileview_set_load_filter(FileView* self, const psy_List* types)
{
	assert(self);
	
	fileviewfilter_set_filter(&self->load_filter_, types);
	filebox_set_wildcard(&self->file_box_, fileviewfilter_type(&self->load_filter_));
	psy_ui_component_align_full(filebox_base(&self->file_box_));
}

void fileview_set_save_filter(FileView* self, const psy_List* types)
{
	assert(self);

	fileviewfilter_set_filter(&self->save_filter_, types);
	if (!psy_fileselect_connected(&self->select_load_)) {
		filebox_set_wildcard(&self->file_box_, fileviewfilter_type(
			&self->save_filter_));
		psy_ui_component_align_full(filebox_base(&self->file_box_));
	}
}

void fileview_update_save_view(FileView* self, bool has_save)
{
	assert(self);
			
	if (has_save) {
		psy_ui_component_show(fileviewfilter_base(&self->save_filter_));
		psy_ui_component_show(psy_ui_button_base(&self->buttons_.save_select));
	} else {
		psy_ui_component_hide(&self->save_filter_.component);
		psy_ui_component_hide(psy_ui_button_base(&self->buttons_.save_select));
	}	
	psy_ui_component_align_invalidate(fileview_base(self));	
}

void fileview_set_load_select(FileView* self, const psy_FileSelect* select)
{	
	assert(self);
	
	psy_fileselect_copy(&self->select_load_, select);
	
	if (select) {				
		filebox_show_files_and_directories(&self->file_box_);
		if (psy_strlen(select->title) > 0) {
			psy_ui_label_set_text(&self->view_bar_, select->title);
		}
		if (select->has_preview) {
			filebox_enable_preview(&self->file_box_);
		} else {
			filebox_disable_preview(&self->file_box_);
		}		
		fileviewcommands_set_save(&self->buttons_);		
		if (!self->file_box_.read_from_file_list &&
				psy_strlen(psy_fileselect_directory(select)) != 0) {
			fileview_set_directory(self, psy_fileselect_directory(select));
		}
		fileview_set_load_filter(self, select->filter_items);
	} else {
		filebox_disable_preview(&self->file_box_);
	}	
	fileview_update_file_name(self, &self->select_load_);	
}

void fileview_update_file_name(FileView* self, const psy_FileSelect* select)
{
	psy_Path path;

	assert(self);
	
	if (!select) {
		return;
	}
	psy_path_init(&path, psy_fileselect_value(select));		
	psy_ui_text_set_text(&self->file_name_, psy_path_filename(&path));
	psy_path_dispose(&path);
}

void fileview_set_save_select(FileView* self, const psy_FileSelect* select)
{
	assert(self);
		
	psy_fileselect_copy(&self->select_save_, select);
	if (select) {		
		if (!psy_fileselect_connected(&self->select_load_)) {			
			if (psy_strlen(select->title) > 0) {				
				psy_ui_label_set_text(&self->view_bar_, select->title);
			}			
		} else {
			char text[512];

			psy_snprintf(text, 512, "%s / %s", psy_ui_label_text(&self->view_bar_),
				select->title);
			psy_ui_label_set_text(&self->view_bar_, text);
		}
		fileviewcommands_set_save(&self->buttons_);		
		filebox_show_files_and_directories(&self->file_box_);
		if (!self->file_box_.read_from_file_list &&
				psy_strlen(psy_fileselect_directory(select)) != 0) {
			fileview_set_directory(self, psy_fileselect_directory(select));
		}
		fileview_set_save_filter(self, select->filter_items);
		fileview_update_dir_label(self);
	}
}

void fileview_set_dir_select(FileView* self, const psy_FileSelect* select)
{
	assert(self);
			
	psy_fileselect_copy(&self->select_dir_, select);
	if (select) {
		if (psy_strlen(select->title) > 0) {
			psy_ui_label_set_text(&self->view_bar_, select->title);
		}
		else {
			psy_ui_label_set_text(&self->view_bar_, "DiskOp");
		}
		fileviewcommands_set_select(&self->buttons_);		
		filebox_show_only_directories(&self->file_box_);
		if (psy_strlen(psy_fileselect_directory(select)) != 0) {
			fileview_set_directory(self, psy_fileselect_directory(select));
		} else {
			filebox_refresh(&self->file_box_);
		}
		fileview_update_dir_label(self);
	}
}

void fileview_on_focus(FileView* self)
{
	assert(self);
	
	psy_ui_component_set_focus(filebox_base(&self->file_box_));
}

void fileview_on_mkdir(FileView* self, psy_ui_Component* sender)
{
	assert(self);
	
	psy_ui_component_show(psy_ui_text_base(&self->buttons_.dir));
	psy_ui_component_align_invalidate(&self->options_);	
	psy_ui_component_set_focus(psy_ui_textpane_base(&self->buttons_.dir.pane));
}

void fileview_on_dir_edit_accept(FileView* self, psy_ui_Text* sender)
{	
	assert(self);
		
	if (psy_strlen(psy_ui_text_text(sender)) > 0) {
		char path[4096];
		
		path[0] = '\0';
		psy_snprintf(path, 4096, "%s%s%s",
			psy_path_prefix(&self->file_box_.curr_dir),
			psy_SLASHSTR, psy_ui_text_text(sender));
		printf("path %s\n", path);
		psy_mkdir(path);
		filebox_set_wildcard(&self->file_box_, fileviewfilter_type(
			&self->load_filter_));
		psy_ui_component_hide(psy_ui_text_base(&self->buttons_.dir));
		psy_ui_component_align_invalidate(&self->options_);
		psy_ui_component_align_full(filebox_base(&self->file_box_));
	}		
}

void fileview_on_dir_edit_reject(FileView* self, psy_ui_Text* sender)
{
	assert(self);
	
	psy_ui_component_hide(psy_ui_text_base(&self->buttons_.dir));
	psy_ui_component_align_invalidate(&self->options_);	
}

void fileview_add_link(FileView* self, const char* label, const char* path)
{
	assert(self);
	
	fileviewlinks_add(&self->links_, label, path);	
}

void fileview_on_key_down(FileView* self, psy_ui_KeyboardEvent* ev)
{
	assert(self);

	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		fileview_do_exit(self);		
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}
