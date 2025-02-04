/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_NOTEBOOK_H
#define psy_ui_NOTEBOOK_H

/* local */
#include "uisplitbar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Notebook {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Splitter splitbar;	
	uintptr_t pageindex;
	uintptr_t page_not_found_index;
	int split;
	bool preventalign;	
} psy_ui_Notebook;

void psy_ui_notebook_init(psy_ui_Notebook*, psy_ui_Component* parent);

void psy_ui_notebook_select(psy_ui_Notebook*, uintptr_t page);
void psy_ui_notebook_select_by_component_id(psy_ui_Notebook*,
	uintptr_t component_id);
uintptr_t psy_ui_notebook_page_index(const psy_ui_Notebook*);

psy_ui_Component* psy_ui_notebook_active_page(psy_ui_Notebook*);

INLINE const psy_ui_Component* psy_ui_notebook_active_page_const(
	const psy_ui_Notebook* self)
{
	return psy_ui_notebook_active_page((psy_ui_Notebook*)self);
}

psy_ui_Component* psy_ui_notebook_page(psy_ui_Notebook*, uintptr_t pageindex);
void psy_ui_notebook_split(psy_ui_Notebook*, psy_ui_Orientation);
int psy_ui_notebook_splitactivated(psy_ui_Notebook*);
void psy_ui_notebook_full(psy_ui_Notebook*);
void psy_ui_notebook_connect_controller(psy_ui_Notebook*, 
	psy_Signal* controllersignal);

INLINE psy_ui_Component* psy_ui_notebook_base(psy_ui_Notebook* self)
{
	assert(self);

	return &self->component;
}

INLINE void psy_ui_notebook_set_page_not_found_index(psy_ui_Notebook* self,
	uintptr_t index)
{
	self->page_not_found_index = index;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_NOTEBOOK_H */
