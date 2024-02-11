/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "filebox.h"
/* host */
#include "inputhandler.h"
#include "styles.h"
#include "trackercmds.h"
/* container */
#include <qsort.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"


/* FileLine */

/* prototypes */
static void fileline_on_destroyed(FileLine*);
static void fileline_init_preview(FileLine*);
static void fileline_init_name(FileLine*);
static void fileline_init_size(FileLine*);
static void fileline_size_str(const FileLine*, char* rv);

/* vtable */
static psy_ui_ComponentVtable fileline_vtable;
static bool fileline_vtable_initialized = FALSE;

static void fileline_vtable_init(FileLine* self)
{
	assert(self);

	if (!fileline_vtable_initialized) {
		fileline_vtable = *(self->component.vtable);
		fileline_vtable.on_destroyed =
			(psy_ui_fp_component)
			fileline_on_destroyed;
		fileline_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &fileline_vtable);
}

/* implementation */
void fileline_init(FileLine* self, psy_ui_Component* parent, const char* path,
	bool is_dir, bool has_preview)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	fileline_vtable_init(self);
	self->path = psy_strdup(path);
	psy_ui_component_set_preferred_height(&self->component,
		psy_ui_value_make_eh(1.5));
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_TOP);
	if (has_preview) {
		fileline_init_preview(self);
	}
	fileline_init_name(self);
	if (!is_dir) {
		fileline_init_size(self);
	}
}

void fileline_on_destroyed(FileLine* self)
{
	assert(self);

	free(self->path);
	self->path = NULL;
}

FileLine* fileline_alloc(void)
{
	return (FileLine*)malloc(sizeof(FileLine));
}

FileLine* fileline_alloc_init(psy_ui_Component* parent, const char* path,
	bool is_dir, bool has_preview)
{
	FileLine* rv;

	rv = fileline_alloc();
	if (rv) {
		fileline_init(rv, parent, path, is_dir, has_preview);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void fileline_init_preview(FileLine* self)
{
	assert(self);

	psy_ui_button_init(&self->preview, &self->component);
	psy_ui_component_set_align(psy_ui_button_base(&self->preview),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_set_icon(&self->preview, psy_ui_ICON_MORE);
}

void fileline_init_name(FileLine* self)
{
	psy_Path extract;

	assert(self);

	psy_ui_button_init(&self->name, &self->component);
	psy_ui_component_set_encoding(psy_ui_button_base(&self->name),
		PSY_ENCODING_NONE);
	psy_ui_component_set_align(psy_ui_button_base(&self->name),
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_prevent_translation(&self->name);
	psy_ui_button_set_text_alignment(&self->name,
		psy_ui_ALIGNMENT_LEFT | psy_ui_ALIGNMENT_CENTER_VERTICAL);
	self->name.stoppropagation = FALSE;
	psy_ui_component_set_margin(&self->name.component, psy_ui_margin_make_em(
		0.0, 1.0, 0.0, 0.0));
	psy_path_init(&extract, self->path);
	psy_ui_button_set_text(&self->name, psy_path_filename(&extract));
	psy_path_dispose(&extract);
}

void fileline_init_size(FileLine* self)
{
	char str[256];

	assert(self);

	psy_ui_label_init(&self->size, &self->component);
	psy_ui_component_set_encoding(psy_ui_label_base(&self->size),
		PSY_ENCODING_NONE);
	psy_ui_component_set_margin(&self->size.component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_component_set_align(psy_ui_label_base(&self->size),
		psy_ui_ALIGN_RIGHT);
	psy_ui_label_prevent_translation(&self->size);
	fileline_size_str(self, str);
	psy_ui_label_set_text(&self->size, str);
}

void fileline_size_str(const FileLine* self, char* rv)
{
	uintptr_t size;

	assert(self);

	size = psy_file_size(self->path);
	if (size != psy_INDEX_INVALID) {
		if (size > 10240) {
			psy_snprintf(rv, 256, "%dk", (int)(ceil((double)size / 1024.0)));
		}
		else {
			psy_snprintf(rv, 256, "%d", (int)size);
		}
	}
	else {
		psy_snprintf(rv, 256, "%s", "");
	}
}


/* FileBox */

#define FILE_PANE FALSE
#define DIR_PANE  TRUE

/* prototypes */
static void filebox_on_destroyed(FileBox*);
static void filebox_init_signals(FileBox*);
static void filebox_init_pane(FileBox*);
static void filebox_init_scroller(FileBox*);
static void filebox_connect_input_handler(FileBox*, InputHandler*);
static void filebox_add(FileBox*, psy_ui_Component* parent, const char* path,
	bool is_dir, bool has_preview);
static void filebox_on_button(FileBox*, psy_ui_Button* sender);
static void filebox_on_dir_button(FileBox*, psy_ui_Button* sender);
static void filebox_on_preview_button(FileBox*, psy_ui_Button* sender);
static void filebox_on_timer(FileBox*, uintptr_t timer_id);
static psy_List* filebox_sort(psy_List* source, psy_fp_comp comp);
static intptr_t filebox_comp_filename(psy_List* p, psy_List* q);
static void filebox_on_mouse_down(FileBox*, psy_ui_MouseEvent* ev);
static bool filebox_on_input(FileBox*, InputHandler* sender);
static bool filebox_handle_command(FileBox*, uintptr_t cmd);
static void filebox_select(FileBox*, bool sel_dir, uintptr_t sel);
static void filebox_scroll_down(FileBox*);
static void filebox_scroll_up(FileBox*);
static FileLine* filebox_selected_line(FileBox*);
static const FileLine* filebox_selected_line_const(const FileBox*);
static psy_ui_Component* filebox_selected_pane(FileBox*);
FileLine* filebox_line_at(FileBox*, uintptr_t index);
const FileLine* filebox_line_at_const(const FileBox*, uintptr_t index);
static void filebox_select_dir(FileBox*, const char* path);

/* vtable */
static psy_ui_ComponentVtable filebox_vtable;
static bool filebox_vtable_initialized = FALSE;

static void filebox_vtable_init(FileBox* self)
{
	assert(self);

	if (!filebox_vtable_initialized) {
		filebox_vtable = *(self->scroller.component.vtable);
		filebox_vtable.on_destroyed =
			(psy_ui_fp_component)
			filebox_on_destroyed;
		filebox_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			filebox_on_mouse_down;
		filebox_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			filebox_on_timer;
		filebox_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->scroller.component, &filebox_vtable);
}

/* implementation */
void filebox_init(FileBox* self, psy_ui_Component* parent,
	InputHandler* input_handler)
{
	assert(self);

	psy_ui_scroller_init(&self->scroller, parent, NULL, NULL);
	self->scroller.prevent_mouse_down_propagation = FALSE;
	filebox_vtable_init(self);
	filebox_init_signals(self);
	self->selindex = psy_INDEX_INVALID;
	self->rebuild = FALSE;
	self->wildcard = psy_strdup("*.psy");
	self->dirsonly = FALSE;
	self->has_preview = TRUE;
	self->sel_dir = FALSE;
	self->read_from_file_list = FALSE;
	psy_path_init(&self->curr_dir, NULL);
#if defined(DIVERSALIS__OS__MICROSOFT)
	psy_path_set_prefix(&self->curr_dir, "C:\\");
#else	
	psy_path_set_prefix(&self->curr_dir, "/");
#endif	
	psy_ui_component_set_tab_index(filebox_base(self), 0);
	psy_ui_component_set_preferred_size(filebox_base(self),
		psy_ui_size_make_em(80.0, 0.0));
	filebox_init_pane(self);
	filebox_init_scroller(self);
	filebox_read(self, psy_path_prefix(&self->curr_dir));
	filebox_connect_input_handler(self, input_handler);
}

void filebox_on_destroyed(FileBox* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_preview);
	psy_signal_dispose(&self->signal_selected);
	psy_signal_dispose(&self->signal_dir_changed);
	psy_path_dispose(&self->curr_dir);
	free(self->wildcard);
	self->wildcard = NULL;
}

void filebox_init_signals(FileBox* self)
{
	assert(self);

	psy_signal_init(&self->signal_preview);
	psy_signal_init(&self->signal_selected);
	psy_signal_init(&self->signal_dir_changed);
}

void filebox_init_pane(FileBox* self)
{
	assert(self);

	psy_ui_component_init(&self->pane, &self->scroller.pane, NULL);
	psy_ui_component_set_overflow(&self->pane, psy_ui_OVERFLOW_SCROLL);
	psy_ui_component_set_wheel_scroll(&self->pane, 2);
	psy_ui_component_set_scroll_step(&self->pane,
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_component_init(&self->dirpane, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->dirpane, STYLE_FILEBOX_DIR_PANE);
	psy_ui_component_set_align(&self->dirpane, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->filepane, &self->pane, NULL);
	psy_ui_component_set_style_type(&self->filepane, STYLE_FILEBOX_FILE_PANE);
	psy_ui_component_set_align(&self->filepane, psy_ui_ALIGN_TOP);
}

void filebox_init_scroller(FileBox* self)
{
	assert(self);

	psy_ui_scroller_set_client(&self->scroller, &self->pane);
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_HCLIENT);
	psy_ui_component_set_padding(psy_ui_scroller_base(&self->scroller),
		psy_ui_margin_make_em(0.5, 0.5, 0.0, 1.0));
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
}

void filebox_connect_input_handler(FileBox* self, InputHandler* input_handler)
{
	assert(self);

	if (!input_handler) {
		return;
	}
	inputhandler_connect(input_handler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "tracker", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)filebox_on_input);
	/* SELECTMACHINE := SELECT FILE */
	inputhandler_connect(input_handler, INPUTHANDLER_FOCUS,
		psy_EVENTDRIVER_CMD, "edit", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)filebox_on_input);
}

void filebox_read(FileBox* self, const char* path)
{
	psy_List* p;
	psy_List* q;
	psy_List* sorted;
	psy_List* files;

	assert(self);

	self->selindex = psy_INDEX_INVALID;
	self->previewindex = psy_INDEX_INVALID;
	psy_ui_component_clear(&self->dirpane);
	psy_ui_component_clear(&self->filepane);

	if (self->read_from_file_list) {
		FILE* fp = fopen(path, "r");
		char buf[psy_MAX_PATH];
		int i = 0;

		if (!fp) {
			return;
		}
		while (fgets(buf, psy_MAX_PATH, fp)) {
			if (buf[strlen(buf) - 1] == '\n') {
				buf[strlen(buf) - 1] = '\0';
			}
			filebox_add(self, &self->filepane, buf, FALSE,
				self->has_preview);

		}
		fclose(fp);
	}
	else {
		files = psy_directories(path);
		sorted = filebox_sort(files, (psy_fp_comp)filebox_comp_filename);
		psy_list_deallocate(&files, (psy_fp_disposefunc)NULL);
		files = sorted;
		for (q = p = files; p != NULL; psy_list_next(&p)) {
			filebox_add(self, &self->dirpane, (const char*)p->entry, TRUE,
				FALSE);
		}
		psy_list_deallocate(&q, NULL);
		if (!self->dirsonly) {
			files = psy_files(path, self->wildcard, psy_ui_NONE_RECURSIVE);
			sorted = filebox_sort(files, (psy_fp_comp)filebox_comp_filename);
			psy_list_deallocate(&files, (psy_fp_disposefunc)NULL);
			files = sorted;
			for (q = p = files; p != NULL; psy_list_next(&p)) {
				filebox_add(self, &self->filepane, (const char*)p->entry, FALSE,
					self->has_preview);
			}
			psy_list_deallocate(&q, NULL);
		}
	}
	psy_path_set_prefix(&self->curr_dir, path);
	psy_signal_emit(&self->signal_dir_changed, self, 0);
}

void filebox_add(FileBox* self, psy_ui_Component* parent, const char* path,
	bool is_dir, bool has_preview)
{
	FileLine* file_line;

	assert(self);

	file_line = fileline_alloc_init(parent, path, is_dir, has_preview);
	if (parent == &self->dirpane) {
		psy_signal_connect(&file_line->name.signal_clicked,
			self, filebox_on_dir_button);
	}
	else {
		psy_signal_connect(&file_line->name.signal_clicked,
			self, filebox_on_button);
	}
	if (has_preview) {
		psy_signal_connect(&file_line->preview.signal_clicked,
			self, filebox_on_preview_button);
	}
}

psy_List* filebox_sort(psy_List* source, psy_fp_comp comp)
{
	psy_List* rv;

	rv = NULL;
	if (source) {
		uintptr_t num;
		psy_Table arrayptr;
		psy_List* p;
		uintptr_t i;

		num = psy_list_size(source);
		if (num == 0) {
			return NULL;
		}
		psy_table_init(&arrayptr);
		p = source;
		for (i = 0; p != NULL && i < num; p = p->next, ++i) {
			psy_table_insert(&arrayptr, i, p);
		}
		psy_qsort(&arrayptr,
			(psy_fp_set_index_double)psy_table_insert,
			(psy_fp_index_double)psy_table_at,
			0, (int)(num - 1), comp);
		for (i = 0; i < num; ++i) {
			psy_list_append(&rv,
				psy_strdup((const char*)(((psy_List*)
					(psy_table_at(&arrayptr, i)))->entry)));
		}
		psy_table_dispose(&arrayptr);
	}
	return rv;
}

intptr_t filebox_comp_filename(psy_List* p, psy_List* q)
{
	const char* left;
	const char* right;

	left = (const char*)p->entry;
	right = (const char*)q->entry;
	return strcmp(left, right);
}


uintptr_t filebox_selected(const FileBox* self)
{
	assert(self);

	return self->selindex;
}

void filebox_set_wildcard(FileBox* self, const char* wildcard)
{
	assert(self);

	psy_strreset(&self->wildcard, wildcard);
	filebox_read(self, psy_path_full(&self->curr_dir));
	psy_ui_component_align_invalidate(filebox_base(self));
}


void filebox_set_directory(FileBox* self, const char* path)
{
	assert(self);

#if defined DIVERSALIS__OS__POSIX	
	char norm[4096];

	psy_dir_normalize(path, norm);
	psy_path_set_prefix(&self->curr_dir, norm);
	filebox_read(self, norm);
#else	
	if (strcmp(path, "..") == 0) {
		psy_path_remove_dir(&self->curr_dir);
		filebox_read(self, psy_path_prefix(&self->curr_dir));
	}
	else {
		filebox_read(self, path);
	}
#endif	
	psy_ui_component_align_invalidate(filebox_base(self));
}

void filebox_on_dir_button(FileBox* self, psy_ui_Button* sender)
{
	assert(self);

	filebox_select_dir(self, psy_ui_button_text(sender));
}

void filebox_select_dir(FileBox* self, const char* path)
{
	assert(self);

	if (strcmp(path, psy_SLASHSTR"..") == 0 || (strcmp(path, "..") == 0)) {
		psy_path_remove_dir(&self->curr_dir);
#if defined DIVERSALIS__OS__POSIX
		if (psy_strlen(psy_path_prefix(&self->curr_dir)) == 0) {
			psy_path_set_prefix(&self->curr_dir, "/");
		}
#endif
		self->rebuild = TRUE;
		psy_ui_component_start_timer(filebox_base(self), 0, 50);
	}
	else {
		psy_path_append_dir(&self->curr_dir, path);
		psy_ui_component_start_timer(filebox_base(self), 0, 50);
		self->rebuild = TRUE;
	}
}

void filebox_on_preview_button(FileBox* self, psy_ui_Button* sender)
{
	assert(self);

	self->previewindex = psy_ui_component_index(
		psy_ui_component_parent(psy_ui_button_base(sender)));
	psy_signal_emit(&self->signal_preview, self, 0);
	self->previewindex = psy_INDEX_INVALID;
}

void filebox_on_timer(FileBox* self, uintptr_t timer_id)
{
	assert(self);

	if (self->rebuild) {
		self->rebuild = FALSE;
		psy_ui_component_stop_timer(filebox_base(self), 0);
		filebox_refresh(self);
	}
}

void filebox_refresh(FileBox* self)
{
	assert(self);

	filebox_read(self, psy_path_prefix(&self->curr_dir));
	psy_ui_component_align_invalidate(filebox_base(self));
}

void filebox_on_button(FileBox* self, psy_ui_Button* sender)
{
	assert(self);

	self->selindex = psy_ui_component_index(
		psy_ui_component_parent(psy_ui_button_base(sender)));
	psy_signal_emit(&self->signal_selected, self, 0);
}

void filebox_on_mouse_down(FileBox* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 2) {
		if (!self->read_from_file_list) {
			psy_path_remove_dir(&self->curr_dir);
#if defined DIVERSALIS__OS__POSIX
			if (psy_strlen(psy_path_prefix(&self->curr_dir)) == 0) {
				psy_path_set_prefix(&self->curr_dir, "/");
			}
#endif
			filebox_read(self, psy_path_prefix(&self->curr_dir));
			psy_ui_component_align_invalidate(filebox_base(self));
		}
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

const char* filebox_file_name(const FileBox* self)
{
	const FileLine* line;

	assert(self);

	line = filebox_selected_line_const(self);
	if (line) {
		return psy_ui_button_text(&line->name);
	}
	return "";
}

const char* filebox_preview_name(const FileBox* self)
{
	const FileLine* line;

	assert(self);

	line = filebox_line_at_const(self, self->previewindex);
	if (line) {
		return psy_ui_button_text(&line->name);
	}
	return "";
}

const char* filebox_directory(const FileBox* self)
{
	assert(self);

	return psy_path_full(&self->curr_dir);
}

void filebox_full_name(const FileBox* self, char* rv, uintptr_t maxlen)
{
	assert(self);

	if (self->read_from_file_list) {
		const FileLine* line;

		assert(self);

		line = filebox_selected_line_const(self);
		if (line) {
			psy_snprintf(rv, maxlen, line->path);
		}
		else {
			psy_snprintf(rv, maxlen, "");
		}
	}
	else {
		psy_snprintf(rv, maxlen, "%s"psy_SLASHSTR"%s",
			psy_path_prefix(&self->curr_dir),
			filebox_file_name(self));
	}
}

void filebox_full_preview_name(const FileBox* self, char* rv, uintptr_t maxlen)
{
	assert(self);

	psy_snprintf(rv, maxlen, "%s"psy_SLASHSTR"%s",
		psy_path_prefix(&self->curr_dir),
		filebox_preview_name(self));
}

void filebox_enable_preview(FileBox* self)
{
	assert(self);

	self->has_preview = TRUE;
}

void filebox_disable_preview(FileBox* self)
{
	assert(self);

	self->has_preview = FALSE;
}

void filebox_show_only_directories(FileBox* self)
{
	assert(self);

	self->dirsonly = TRUE;
}

void filebox_show_files_and_directories(FileBox* self)
{
	assert(self);

	self->dirsonly = FALSE;
}

bool filebox_on_input(FileBox* self, InputHandler* sender)
{
	psy_EventDriverCmd cmd;

	assert(self);

	cmd = inputhandler_cmd(sender);
	return filebox_handle_command(self, cmd.id);
}

bool filebox_handle_command(FileBox* self, uintptr_t cmd)
{
	assert(self);

	switch (cmd) {
	case CMD_NAVUP:
		if (self->selindex == psy_INDEX_INVALID) {
			filebox_select(self, FILE_PANE, 0);
		}
		else if (!self->sel_dir && self->selindex > 0) {
			filebox_select(self, FILE_PANE, self->selindex - 1);
		}
		else {
			if (!self->sel_dir) {
				psy_List* p;

				p = psy_ui_component_children(&self->dirpane,
					psy_ui_NONE_RECURSIVE);
				if (p && psy_list_size(p) > 0) {
					uintptr_t sel;

					sel = psy_list_size(p) - 1;
					filebox_select(self, DIR_PANE, sel);
				}
				free(p);
				p = NULL;
			}
			else if (self->selindex > 0) {
				filebox_select(self, DIR_PANE, self->selindex - 1);
			}
		}
		filebox_scroll_up(self);
		return TRUE;
	case CMD_NAVPAGEUP:
		return TRUE;
	case CMD_NAVDOWN: {
		if (self->selindex == psy_INDEX_INVALID) {
			psy_List* p;

			p = psy_ui_component_children(&self->filepane,
				psy_ui_NONE_RECURSIVE);
			if (p) {
				filebox_select(self, FILE_PANE, 0);
			}
			else {
				filebox_select(self, DIR_PANE, 0);
			}
			free(p);
			p = NULL;
		}
		else {
			if (self->sel_dir) {
				psy_List* p;

				p = psy_ui_component_children(&self->dirpane, psy_ui_NONE_RECURSIVE);
				if (p && psy_list_size(p) > 0) {
					if (self->selindex >= psy_list_size(p) - 1) {
						filebox_select(self, FILE_PANE, 0);
					}
					else {
						filebox_select(self, DIR_PANE, self->selindex + 1);
					}
				}
				free(p);
				p = NULL;
			}
			else {
				psy_List* p;

				p = psy_ui_component_children(&self->filepane, psy_ui_NONE_RECURSIVE);
				if (p && psy_list_size(p) > 0 && self->selindex < psy_list_size(p) - 1) {
					filebox_select(self, FILE_PANE, self->selindex + 1);
				}
				free(p);
				p = NULL;
			}
		}
		filebox_scroll_down(self);
		return TRUE; }
	case CMD_NAVPAGEDOWN:
		return TRUE;
	case CMD_NAVLEFT:
		if (!self->read_from_file_list) {
			filebox_select_dir(self, "..");
		}
		return TRUE;
	case CMD_NAVRIGHT:
		return TRUE;
	case CMD_SELECTMACHINE: {
		FileLine* line;

		line = filebox_selected_line(self);
		if (line) {
			if (!self->sel_dir) {
				psy_signal_emit(&self->signal_selected, self, 0);
			}
			else {
				filebox_select_dir(self, psy_ui_button_text(&line->name));
			}
		}
		return TRUE; }
	default:
		return FALSE;
	}
}

void filebox_scroll_down(FileBox* self)
{
	FileLine* line;

	assert(self);

	line = filebox_selected_line(self);
	if (line) {
		psy_ui_RealRectangle position;
		psy_ui_RealRectangle parent_position;
		psy_ui_RealSize client_size;
		psy_ui_RealRectangle client;
		double diff;

		position = psy_ui_component_position(fileline_base(line));
		parent_position = psy_ui_component_position(psy_ui_component_parent(
			fileline_base(line)));
		psy_ui_realrectangle_move(&position, psy_ui_realrectangle_topleft(
			&parent_position));
		client_size = psy_ui_component_clientsize_px(&self->pane);
		client = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scroll_left_px(&self->pane),
				psy_ui_component_scroll_top_px(&self->pane)),
			client_size);
		diff = client.bottom - position.bottom;
		if (diff < 0) {
			psy_ui_component_set_scroll_top_px(&self->pane,
				client.top - diff);
		}
		psy_ui_component_invalidate(&self->pane);
	}
}

void filebox_scroll_up(FileBox* self)
{
	FileLine* line;

	assert(self);

	line = filebox_selected_line(self);
	if (line) {
		psy_ui_RealRectangle position;
		psy_ui_RealRectangle parent_position;
		psy_ui_RealSize client_size;
		psy_ui_RealRectangle client;

		position = psy_ui_component_position(fileline_base(line));
		parent_position = psy_ui_component_position(psy_ui_component_parent(
			fileline_base(line)));
		psy_ui_realrectangle_move(&position, psy_ui_realrectangle_topleft(
			&parent_position));
		client_size = psy_ui_component_clientsize_px(&self->pane);
		client = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scroll_left_px(&self->pane),
				psy_ui_component_scroll_top_px(&self->pane)),
			client_size);
		if (position.top < client.top) {
			psy_ui_component_set_scroll_top_px(&self->pane,
				position.top);
		}
		psy_ui_component_invalidate(&self->pane);
	}
}

void filebox_select(FileBox* self, bool sel_dir, uintptr_t sel)
{
	FileLine* line;

	assert(self);

	line = filebox_selected_line(self);
	if (line) {
		psy_ui_component_remove_style_state(psy_ui_button_base(
			&line->name), psy_ui_STYLESTATE_SELECT);
	}
	self->selindex = sel;
	self->sel_dir = sel_dir;
	line = filebox_selected_line(self);
	if (line) {
		psy_ui_component_add_style_state(psy_ui_button_base(&line->name),
			psy_ui_STYLESTATE_SELECT);
	}
}

FileLine* filebox_selected_line(FileBox* self)
{
	assert(self);

	return filebox_line_at(self, self->selindex);
}

const FileLine* filebox_selected_line_const(const FileBox* self)
{
	assert(self);

	return filebox_line_at_const(self, self->selindex);
}

FileLine* filebox_line_at(FileBox* self, uintptr_t index)
{
	assert(self);

	if (index != psy_INDEX_INVALID) {
		return (FileLine*)psy_ui_component_at(filebox_selected_pane(self),
			index);
	}
	return NULL;
}

const FileLine* filebox_line_at_const(const FileBox* self, uintptr_t index)
{
	assert(self);

	return filebox_line_at((FileBox*)self, index);
}

psy_ui_Component* filebox_selected_pane(FileBox* self)
{
	assert(self);

	if (self->sel_dir) {
		return &self->dirpane;
	}
	return &self->filepane;
}
