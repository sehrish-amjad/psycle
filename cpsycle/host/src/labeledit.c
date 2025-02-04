/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "labeledit.h"
/* platform */
#include "../../detail/portable.h"


/* implementation */
void labeledit_init(LabelEdit* self, psy_ui_Component* parent,
	const char* desc)
{
	assert(self);

	psy_ui_component_init(labeledit_base(self), parent, NULL);
	psy_ui_component_set_align_expand(labeledit_base(self), psy_ui_HEXPAND);	
	psy_ui_label_init_text(&self->desc, labeledit_base(self), desc);
	psy_ui_component_set_align(psy_ui_label_base(&self->desc), psy_ui_ALIGN_LEFT);
	psy_ui_label_set_char_number(&self->desc, 12.0);
	psy_ui_text_init(&self->edit, labeledit_base(self));	
	psy_ui_component_set_align(psy_ui_text_base(&self->edit), psy_ui_ALIGN_CLIENT);				
}

LabelEdit* labeledit_alloc(void)
{
	return (LabelEdit*)malloc(sizeof(LabelEdit));
}

LabelEdit* labeledit_alloc_init(psy_ui_Component* parent, const char* desc)
{
	LabelEdit* rv;

	rv = labeledit_alloc();
	if (rv) {
		labeledit_init(rv, parent, desc);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}
