/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(KEYBOARDVIEW_H)
#define KEYBOARDVIEW_H

/* host */
#include "closebar.h"
#include "pianokeyboard.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct KeyboardView
*/
typedef struct KeyboardView {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	CloseBar close_bar;
	PianoKeyboard keyboard;
	KeyboardState state;
	Workspace* workspace;
} KeyboardView;

void keyboardview_init(KeyboardView*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* keyboardview_base(KeyboardView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* KEYBOARDVIEW_H */
