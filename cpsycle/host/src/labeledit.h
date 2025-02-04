/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(LABELEDIT_H)
#define LABELEDIT_H

/* host */
#include "workspace.h"
/* ui */
#include <uilabel.h>
#include <uitext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LabelEdit {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Label desc;
	psy_ui_Text edit;	
} LabelEdit;

void labeledit_init(LabelEdit*, psy_ui_Component* parent,
	const char* desc);

LabelEdit* labeledit_alloc(void);
LabelEdit* labeltedit_alloc_init(psy_ui_Component* parent,
	const char* desc);

INLINE psy_ui_Component* labeledit_base(LabelEdit* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* LABELEDIT_H */
