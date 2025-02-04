/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(HELP_H)
#define HELP_H

/* ui */
#include <uilabel.h>
#include <uiscroller.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct Help
** @brief Help file view
**
** @detail
** Shows the psycle help files inside the psycle doc directory.
** Configure the used files with PSYCLE_HELPFILES in psyconf.h.
*/

typedef struct Help {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Scroller scroller_;
	psy_ui_Label text_;
	psy_ui_TabBar tabbar_;	
	psy_Table file_names_;
	psy_ui_AlignType last_align_;
	char* doc_path_;		
	bool dos_to_utf8_;	
} Help;

void help_init(Help*, psy_ui_Component* parent, const char* doc_path);

INLINE psy_ui_Component* help_base(Help* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* HELP_H */
