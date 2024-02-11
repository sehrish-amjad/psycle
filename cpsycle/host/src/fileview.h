/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(FILEVIEW_H)
#define FILEVIEW_H

/* local */
#include "filebox.h"
#include "propertiesview.h"
/* file */
#include <fileselect.h>
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uitabbar.h>
#include <uitext.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct FileViewFilter
** @brief Filter Type select for the ft2 fileview
*/
typedef struct FileViewFilter {
	/*! @extends */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_changed;
	/*! @internal */
	psy_ui_Label desc;
	psy_ui_Component items;	
	bool show_all_;
	psy_Property filter;	
} FileViewFilter;

void fileviewfilter_init(FileViewFilter*, psy_ui_Component* parent,
	const char* title);

const char* fileviewfilter_type(const FileViewFilter*);
void fileviewfilter_set_filter(FileViewFilter*, const psy_List* types);
void fileviewfilter_connect(FileViewFilter*, void* context, void* fp);
void fileviewfilter_show_all(FileViewFilter*);
void fileviewfilter_show_filter(FileViewFilter*);

INLINE psy_ui_Component* fileviewfilter_base(FileViewFilter* self)
{
	return &self->component;
}

typedef enum {
	FILEVIEWFILTER_ALL,
	FILEVIEWFILTER_PSY,
	FILEVIEWFILTER_MOD
} FileViewFilterType;


/*!
** @struct FileViewLinks
** @brief Links to directories and drives displayed in the ft2 fileview
*/
typedef struct FileViewLinks {
	/*! @extends */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_selected;
	/*! @internal */
	psy_Table locations_;
} FileViewLinks;

void fileviewlinks_init(FileViewLinks*, psy_ui_Component* parent);

void fileview_add_drives(FileViewLinks* self);
void fileviewlinks_add(FileViewLinks*, const char* label,
	const char* path);
const char* fileviewlinks_path(FileViewLinks*, uintptr_t index);
void fileviewlinks_change_path(FileViewLinks*, uintptr_t index,
	const char* path);

INLINE psy_ui_Component* fileviewlinks_base(FileViewLinks* self)
{
	return &self->component;
}

/*!
** @struct FileViewCommands
** @brief Command Buttons of the ft2 fileview
*/
typedef struct FileViewCommands {
	/*! extends */
	psy_ui_Component component;
	/*! internal */
	psy_ui_Button save_select;
	psy_ui_Button showall;
	psy_ui_Button refresh;
	psy_ui_Button mkdir;
	psy_ui_Text dir;
	psy_ui_Button exit;
} FileViewCommands;

void fileviewcommands_init(FileViewCommands*, psy_ui_Component* parent);

void fileviewcommands_set_select(FileViewCommands*);
void fileviewcommands_set_save(FileViewCommands*);


INLINE psy_ui_Component* fileviewcommands_base(FileViewCommands* self)
{
	return &self->component;
}

/*!
** @struct FileView
** @brief ft2 style fileview
*/
typedef struct FileView {
	/*! @extends */
	psy_ui_Component component;		
	/*! @internal */
	psy_ui_Label view_bar_;
	psy_ui_Component client_;
	FileBox file_box_;
	psy_ui_Component left_;
	psy_ui_Component options_;
	psy_ui_Component filters_;
	FileViewFilter load_filter_;
	FileViewFilter save_filter_;
	FileViewCommands buttons_;
	FileViewLinks links_;
	psy_ui_Component bottom_;
	psy_ui_Label dir_;
	psy_ui_Component file_bar_;
	psy_ui_Label file_desc_;
	psy_ui_Text file_name_;
	char* recent_files_;
	/* references */	
	psy_FileSelect select_load_;
	psy_FileSelect select_save_;
	psy_FileSelect select_dir_;
} FileView;

void fileview_init(FileView*, psy_ui_Component* parent,
	psy_ui_Component* viewbar_parent, struct InputHandler*,
	const char* recent_path);
	
FileView* fileview_alloc(void);
FileView* fileview_alloc_init(psy_ui_Component* parent, 
	psy_ui_Component* viewbar_parent, struct InputHandler*,	
	const char* recent_files);

void fileview_filename(const FileView*, char* filename, uintptr_t maxsize);
void fileview_set_directory(FileView*, const char* directory);
void fileview_set_load_filter(FileView*, const psy_List* types);
void fileview_set_save_filter(FileView*, const psy_List* types);
void fileview_add_link(FileView*, const char* label, const char* path);
	
void fileview_set_load_select(FileView*, const psy_FileSelect*);
void fileview_set_save_select(FileView*, const psy_FileSelect*);
void fileview_set_dir_select(FileView*, const psy_FileSelect*);

INLINE psy_ui_Component* fileview_base(FileView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* FILEVIEW_H */
