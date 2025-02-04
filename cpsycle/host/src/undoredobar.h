/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(UNDOREDOBAR_H)
#define UNDOREDOBAR_H

/* host */
#include "workspace.h"
/* ui */
#include "uibutton.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct UndoRedoBar
** @brief ToolBar for the workspace undo redo handler.
*/
typedef struct UndoRedoBar {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Button undo_;
	psy_ui_Button redo_;
	/* references */
	Workspace* workspace_;
} UndoRedoBar;

void undoredobar_init(UndoRedoBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* undoredobar_base(UndoRedoBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* UNDOREDOBAR_H */
