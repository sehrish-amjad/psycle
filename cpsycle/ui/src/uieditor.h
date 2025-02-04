/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_EDITOR_H
#define psy_ui_EDITOR_H

/* local */
#include "uitextarea.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct psy_ui_Editor
** @brief Editor component
*/
typedef struct psy_ui_Editor {
	/*! @extends  */
	psy_ui_Component component;
	psy_ui_TextArea text_area_;
} psy_ui_Editor;

void psy_ui_editor_init(psy_ui_Editor*, psy_ui_Component* parent);

void psy_ui_editor_load(psy_ui_Editor*, const char* path);
void psy_ui_editor_save(psy_ui_Editor*, const char* path);
void psy_ui_editor_set_text(psy_ui_Editor*, const char* text);
void psy_ui_editor_add_text(psy_ui_Editor*, const char* text);
char* psy_ui_editor_text(psy_ui_Editor*, uintptr_t maxlength, char* text);
uintptr_t psy_ui_editor_length(psy_ui_Editor*);
void psy_ui_editor_clear(psy_ui_Editor*);
void psy_ui_editor_set_colour(psy_ui_Editor*, psy_ui_Colour);
void psy_ui_editor_set_background_colour(psy_ui_Editor*, psy_ui_Colour);
void psy_ui_editor_enable_edit(psy_ui_Editor*);
void psy_ui_editor_prevent_edit(psy_ui_Editor*);
void psy_ui_editor_wrap(psy_ui_Editor*);
void psy_ui_editor_prevent_wrap(psy_ui_Editor*);
void psy_ui_editor_gotoline(psy_ui_Editor*, uintptr_t line);
uintptr_t psy_ui_editor_cursor_line(const psy_ui_Editor*);

INLINE psy_ui_Component* psy_ui_editor_base(psy_ui_Editor* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EDITOR_H */
