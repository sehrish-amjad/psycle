/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PROPERTIESVIEW)
#define PROPERTIESVIEW

/* host */
#include "inputdefiner.h"
/* ui */
#include <uicombobox.h>
#include <uilabel.h>
#include <uiscroller.h>
#include <uinotebook.h>
#include <uisizer.h>
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Displays and edits properties */

struct PropertiesRenderLine;
struct Workspace;

typedef struct PropertiesRenderState {
	psy_ui_Size size_col0;	
	uintptr_t numcols;	
	uintptr_t mainsectionstyle;
	uintptr_t mainsectionheaderstyle;
	uintptr_t keystyle;
	uintptr_t keystyle_hover;
	uintptr_t linestyle_select;
	bool do_build;
	/* references */		
	struct PropertiesRenderLine* property_line_selected;
	psy_ui_Component* view;
	psy_ui_Component* renderer;
	psy_ui_Notebook* notebook;
	struct Workspace* workspace;
} PropertiesRenderState;

void propertiesrenderstate_init(PropertiesRenderState*, uintptr_t numcols,
	bool lazy);
void propertiesrenderstate_dispose(PropertiesRenderState*);




/* PropertiesRenderLine */

typedef struct PropertiesRenderLine {
	/*! @extends  */
	psy_ui_Component component;	
	/*! @internal */		
	uintptr_t level;
	/* references */
	psy_ui_Component* section;
	psy_Property* property;
	PropertiesRenderState* state;	
} PropertiesRenderLine;

void propertiesrenderline_init(PropertiesRenderLine*,
	psy_ui_Component* parent, PropertiesRenderState*, psy_Property*,
	uintptr_t level, psy_ui_AlignType align);

PropertiesRenderLine* propertiesrenderline_alloc(void);
PropertiesRenderLine* propertiesrenderline_alloc_init(
	psy_ui_Component* parent, PropertiesRenderState*, psy_Property*,
	uintptr_t level, psy_ui_AlignType);

typedef struct PropertiesRenderer {
	/*! @extends  */
	psy_ui_Component component;
	psy_ui_Component client;
	/* signals */
	psy_Signal signal_selected;
	/*! @internal */	
	uintptr_t currlinestatecount;			
	PropertiesRenderState state;
	/* references */
	psy_Property* properties;	
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Property*, uintptr_t numcols, bool lazy, struct Workspace*);

void propertiesrenderer_set_style(PropertiesRenderer*,
	uintptr_t mainsection,
	uintptr_t mainsectionheader,
	uintptr_t keystyle,
	uintptr_t keystyle_hover,
	uintptr_t linestyle_select);

INLINE const psy_Property* propertiesrenderer_properties(const
	PropertiesRenderer* self)
{
	return self->properties;
}

void propertiesrenderer_build(PropertiesRenderer*);
void propertiesrenderer_maximize_sections(PropertiesRenderer*);

INLINE psy_ui_Component* propertiesrenderer_base(PropertiesRenderer* self)
{
	return &self->component;
}

struct InputHandler;

typedef struct PropertiesView {
	/*! @extends  */
	psy_ui_Component component;
	/* signals */	
	psy_Signal signal_selected;
	/* intern */	
	psy_ui_Component sectionfloated;	
	psy_ui_Component viewtabbar;
	psy_ui_TabBar tabbar;
	psy_ui_Component sidebar;
	psy_ui_Sizer sizer;
	psy_ui_Scroller tabbar_scroller;
	psy_ui_Notebook notebook;	
	psy_ui_Component mainview;
	PropertiesRenderer renderer;
	psy_ui_Scroller scroller;	
	bool maximize_main_sections;		
	psy_ui_Component sizerbar;
	/* references */
	struct Workspace* workspace;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property*, uintptr_t numcols,
	bool lazy, struct InputHandler*, struct Workspace*);

void propertiesview_reload(PropertiesView*);
void propertiesview_mark(PropertiesView*, psy_Property*);
void propertiesview_select(PropertiesView*, psy_Property*);
psy_Property* propertiesview_selected(PropertiesView*);
void propertiesview_prevent_maximize_main_sections(PropertiesView*);

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */
