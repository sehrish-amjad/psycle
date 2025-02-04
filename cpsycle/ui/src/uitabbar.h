/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TABBAR_H
#define psy_ui_TABBAR_H

/* local */
#include "uicomponent.h"
/* container */
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_TABMODE_SINGLESEL,	
	psy_ui_TABMODE_LABEL
} TabMode;

typedef enum {
	TABCHECKSTATE_OFF = 0,
	TABCHECKSTATE_ON = 1
} psy_ui_TabCheckState;

/* psy_ui_Tab */
typedef struct psy_ui_Tab {	
	/*! @extends  */
	psy_ui_Component component;
	/* Signals */
	psy_Signal signal_clicked;
	/*! @internal */
	char* text;
	char* translation;
	char* key;
	TabMode mode;		
	bool istoggle;	
	psy_ui_TabCheckState checkstate;
	psy_ui_Bitmap bitmapicon;
	double bitmapident;
	uintptr_t index;
	bool prevent_translation;
	uintptr_t lightresourceid;
	uintptr_t darkresourceid;
	psy_ui_Colour bitmaptransparency;
	bool vertical_icon;
} psy_ui_Tab;

void psy_ui_tab_init(psy_ui_Tab*, psy_ui_Component* parent,
	const char* text, uintptr_t index);

psy_ui_Tab* psy_ui_tab_alloc(void);
psy_ui_Tab* psy_ui_tab_alloc_init(psy_ui_Component* parent,
	const char* text, uintptr_t index);

void psy_ui_tab_set_text(psy_ui_Tab*, const char* text);

INLINE const char* psy_ui_tab_text(const psy_ui_Tab* self)
{
	assert(self);

	return self->text;
}

void psy_ui_tab_set_key(psy_ui_Tab*, const char* key);

INLINE const char* psy_ui_tab_key(const psy_ui_Tab* self)
{
	assert(self);

	return self->key;
}

void psy_ui_tab_set_mode(psy_ui_Tab*, TabMode);
void psy_ui_tab_prevent_translation(psy_ui_Tab*);
void psy_ui_tab_load_resource(psy_ui_Tab*, uintptr_t lightresourceid,
	uintptr_t darkresourceid, psy_ui_Colour);

INLINE psy_ui_Component* psy_ui_tab_base(psy_ui_Tab* self)
{
	return &self->component;
}

/*!
** @struct psy_ui_TabBar
** @brief Shows tabs in a bar.
**
** @detail
** Can be used with a psy_ui_Notebook or independently.
**
** structure
** psy_ui_Component <>----<> psy_ui_ComponentImp
**       ^
**       |
**       |            psy_ui_Component
**       |                    ^
**       |                    |
** psy_ui_TabBar <@>------ psy_ui_Tab
**                       *
*/
typedef struct psy_ui_TabBar {
	/*! @extends  */
	psy_ui_Component component;
	/* Signals */
	psy_Signal signal_change;
	/*! @internal */
	uintptr_t selected;	
	uintptr_t num_tabs;
	bool prevent_translation;
	psy_ui_AlignType tab_alignment;	
} psy_ui_TabBar;

void psy_ui_tabbar_init(psy_ui_TabBar*, psy_ui_Component* parent);

psy_ui_Tab* psy_ui_tabbar_append(psy_ui_TabBar*, const char* label,
	uintptr_t target_id, uintptr_t light_resource_id,
	uintptr_t dark_resource_id, psy_ui_Colour transparency);
void psy_ui_tabbar_append_tabs(psy_ui_TabBar*, const char* label, ...);
void psy_ui_tabbar_clear(psy_ui_TabBar*);
bool psy_ui_tabbar_select_id(psy_ui_TabBar*, uintptr_t id);
void psy_ui_tabbar_select(psy_ui_TabBar*, uintptr_t tab_index);
void psy_ui_tabbar_mark(psy_ui_TabBar*, uintptr_t tab_index);
bool psy_ui_tabbar_mark_id(psy_ui_TabBar*, uintptr_t id);
void psy_ui_tabbar_unmark(psy_ui_TabBar*);
void psy_ui_tabbar_set_tab_align(psy_ui_TabBar*, psy_ui_AlignType);
void psy_ui_tabbar_prevent_translation(psy_ui_TabBar*);

INLINE uintptr_t psy_ui_tabbar_selected(const psy_ui_TabBar* self)
{	
	assert(self);

	return self->selected;	
}

void psy_ui_tabbar_set_tab_mode(psy_ui_TabBar*, uintptr_t tab, TabMode);
psy_ui_Tab* psy_ui_tabbar_tab(psy_ui_TabBar*, uintptr_t tab_index);
const psy_ui_Tab* psy_ui_tabbar_tab_const(const psy_ui_TabBar*,
	uintptr_t tab_index);

INLINE psy_ui_Component* psy_ui_tabbar_base(psy_ui_TabBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TABBAR_H */
