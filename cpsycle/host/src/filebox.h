/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEBOX_H)
#define FILEBOX_H

/* ui */
#include <uibutton.h>
#include <uilabel.h>
#include <uiscroller.h>
/* file */
#include <dir.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct FileLine
** @brief A file or dir item in the FileBox
*/
typedef struct FileLine {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Button preview;
	psy_ui_Button name;
	psy_ui_Label size;	
	char* path;
} FileLine;

void fileline_init(FileLine*, psy_ui_Component* parent, const char* path, bool is_dir,
	bool has_preview);

FileLine* fileline_alloc(void);
FileLine* fileline_alloc_init(psy_ui_Component* parent, const char* path, bool is_dir,
	bool has_preview);

INLINE psy_ui_Component* fileline_base(FileLine* self)
{
	return &self->component;
}


struct InputHandler;

/*!
** @struct FileBox
** @brief A file/dir listview
*/
typedef struct FileBox {
	/*! @extends */
	psy_ui_Scroller scroller;
	/* signal */
	psy_Signal signal_selected;
	psy_Signal signal_dir_changed;
	psy_Signal signal_preview;
	/*! @internal */	
	psy_ui_Component pane;
	psy_ui_Component filepane;
	psy_ui_Component dirpane;
	uintptr_t selindex;
	bool sel_dir;
	uintptr_t previewindex;
	psy_Path curr_dir;
	bool rebuild;
	char* wildcard;
	bool dirsonly;
	bool has_preview;
	bool read_from_file_list;
} FileBox;

void filebox_init(FileBox*, psy_ui_Component* parent, struct InputHandler*);

void filebox_read(FileBox*, const char* path);
uintptr_t filebox_selected(const FileBox*);
void filebox_set_wildcard(FileBox*, const char* wildcard);
void filebox_set_directory(FileBox*, const char* path);
const char* filebox_directory(const FileBox*);
const char* filebox_file_name(const FileBox*);
const char* filebox_preview_name(const FileBox*);
void filebox_full_name(const FileBox*, char* rv, uintptr_t maxlen);
void filebox_full_preview_name(const FileBox*, char* rv, uintptr_t maxlen);
void filebox_refresh(FileBox*);
void filebox_enable_preview(FileBox*);
void filebox_disable_preview(FileBox*);
void filebox_show_only_directories(FileBox*);
void filebox_show_files_and_directories(FileBox*);

INLINE psy_ui_Component* filebox_base(FileBox* self)
{
	return psy_ui_scroller_base(&self->scroller);
}

#ifdef __cplusplus
}
#endif

#endif /* FILEBOX_H */
