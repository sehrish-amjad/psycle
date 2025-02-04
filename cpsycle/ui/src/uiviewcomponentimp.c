/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiviewcomponentimp.h"
/* local */
#include "uiapp.h"
#include "uicomponent.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"


/* prototypes */
static void view_dev_dispose(psy_ui_ViewComponentImp*);
static void view_dev_destroy(psy_ui_ViewComponentImp*);
static void view_dev_show(psy_ui_ViewComponentImp*);
static void view_dev_showstate(psy_ui_ViewComponentImp*, int state);
static void view_dev_hide(psy_ui_ViewComponentImp*);
static int view_dev_visible(psy_ui_ViewComponentImp*);
static int view_dev_draw_visible(psy_ui_ViewComponentImp*);
static void view_dev_move(psy_ui_ViewComponentImp*, psy_ui_Point origin);
static void view_dev_resize(psy_ui_ViewComponentImp*, psy_ui_Size);
static void view_dev_clientresize(psy_ui_ViewComponentImp*, intptr_t width,
	intptr_t height);
static psy_ui_RealRectangle view_dev_position(psy_ui_ViewComponentImp*);
static psy_ui_RealRectangle view_dev_screenposition(psy_ui_ViewComponentImp*);
static void view_dev_setposition(psy_ui_ViewComponentImp*,
	psy_ui_Point topleft, psy_ui_Size);
static psy_ui_Size view_dev_size(const psy_ui_ViewComponentImp*);
static void view_dev_updatesize(psy_ui_ViewComponentImp*);
static psy_ui_Size view_dev_framesize(psy_ui_ViewComponentImp*);
static void view_dev_scrollto(psy_ui_ViewComponentImp*, psy_ui_RealPoint delta,
	const psy_ui_RealRectangle*);
static psy_ui_Component* view_dev_parent(psy_ui_ViewComponentImp*);
static void view_dev_setparent(psy_ui_ViewComponentImp*,
	psy_ui_Component* parent);
static void view_dev_insert(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* child, psy_ui_ViewComponentImp* insertafter);
static void view_dev_remove(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* child);
static void view_dev_erase(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* child);
static void view_dev_setorder(psy_ui_ViewComponentImp*,
	psy_ui_ViewComponentImp* insertafter);
static void view_dev_capture(psy_ui_ViewComponentImp*);
static void view_dev_releasecapture(psy_ui_ViewComponentImp*);
static void view_dev_invalidate(psy_ui_ViewComponentImp*);
static void view_dev_invalidaterect(psy_ui_ViewComponentImp*,
	const psy_ui_RealRectangle*);
static void view_dev_update(psy_ui_ViewComponentImp*);
static void view_dev_setfont(psy_ui_ViewComponentImp*, psy_ui_Font*);
static void view_dev_showhorizontalscrollbar(psy_ui_ViewComponentImp*);
static void view_dev_hidehorizontalscrollbar(psy_ui_ViewComponentImp*);
static psy_List* view_dev_children(psy_ui_ViewComponentImp*, int recursive);
static void view_dev_enableinput(psy_ui_ViewComponentImp*);
static void view_dev_preventinput(psy_ui_ViewComponentImp*);
bool view_dev_inputprevented(const psy_ui_ViewComponentImp*);
static void view_dev_setcursor(psy_ui_ViewComponentImp*, psy_ui_CursorStyle);
static void view_dev_seticonressource(psy_ui_ViewComponentImp*,
	int ressourceid);
static const psy_ui_TextMetric* view_dev_textmetric(
	const psy_ui_ViewComponentImp*);
static void view_dev_setbackgroundcolour(psy_ui_ViewComponentImp*,
	psy_ui_Colour);
static void view_dev_settitle(psy_ui_ViewComponentImp*, const char* title);
const char* view_dev_title(const psy_ui_ViewComponentImp*);
static void view_dev_setfocus(psy_ui_ViewComponentImp*);
static int view_dev_hasfocus(psy_ui_ViewComponentImp*);
static uintptr_t view_dev_flags(const psy_ui_ComponentImp*);
static psy_ui_RealRectangle view_translation(psy_ui_ViewComponentImp*,
	const psy_ui_RealRectangle*);
static psy_ui_RealPoint translatecoords(psy_ui_ViewComponentImp*,
	psy_ui_Component* src, psy_ui_Component* dst);
bool view_intersection(psy_ui_ViewComponentImp*,
	const psy_ui_RealRectangle*, psy_ui_RealRectangle* rv);

static uintptr_t dev_platform_handle(psy_ui_ViewComponentImp* self)
{
	return psy_INDEX_INVALID;
}

/* vtable */
static psy_ui_ComponentImpVTable view_imp_vtable;
static bool view_imp_vtable_initialized = FALSE;

static void view_imp_vtable_init(psy_ui_ViewComponentImp* self)
{
	if (!view_imp_vtable_initialized) {
		view_imp_vtable = *self->imp.vtable;
		view_imp_vtable.dev_destroy =
			(psy_ui_fp_componentimp_dev_destroy)
			view_dev_destroy;
		view_imp_vtable.dev_dispose =
			(psy_ui_fp_componentimp_dev_dispose)
			view_dev_dispose;
		view_imp_vtable.dev_show =
			(psy_ui_fp_componentimp_dev_show)
			view_dev_show;
		view_imp_vtable.dev_showstate =
			(psy_ui_fp_componentimp_dev_showstate)
			view_dev_showstate;
		view_imp_vtable.dev_hide =
			(psy_ui_fp_componentimp_dev_hide)
			view_dev_hide;
		view_imp_vtable.dev_visible =
			(psy_ui_fp_componentimp_dev_visible)
			view_dev_visible;
		view_imp_vtable.dev_draw_visible =
			(psy_ui_fp_componentimp_dev_draw_visible)
			view_dev_draw_visible;
		view_imp_vtable.dev_move =
			(psy_ui_fp_componentimp_dev_move)
			view_dev_move;
		view_imp_vtable.dev_resize =
			(psy_ui_fp_componentimp_dev_resize)
			view_dev_resize;
		view_imp_vtable.dev_clientresize =
			(psy_ui_fp_componentimp_dev_clientresize)
			view_dev_clientresize;
		view_imp_vtable.dev_position =
			(psy_ui_fp_componentimp_dev_position)
			view_dev_position;
		view_imp_vtable.dev_screenposition =
			(psy_ui_fp_componentimp_dev_position)
			view_dev_screenposition;
		view_imp_vtable.dev_set_position =
			(psy_ui_fp_componentimp_dev_set_position)
			view_dev_setposition;
		view_imp_vtable.dev_size =
			(psy_ui_fp_componentimp_dev_size)
			view_dev_size;
		view_imp_vtable.dev_updatesize =
			(psy_ui_fp_componentimp_dev_updatesize)
			view_dev_updatesize;
		view_imp_vtable.dev_framesize =
			(psy_ui_fp_componentimp_dev_framesize)
			view_dev_framesize;
		view_imp_vtable.dev_scrollto =
			(psy_ui_fp_componentimp_dev_scrollto)
			view_dev_scrollto;
		view_imp_vtable.dev_parent =
			(psy_ui_fp_componentimp_dev_parent)
			view_dev_parent;
		view_imp_vtable.dev_setparent =
			(psy_ui_fp_componentimp_dev_setparent)
			view_dev_setparent;
		view_imp_vtable.dev_insert =
			(psy_ui_fp_componentimp_dev_insert)
			view_dev_insert;
		view_imp_vtable.dev_remove =
			(psy_ui_fp_componentimp_dev_remove)
			view_dev_remove;
		view_imp_vtable.dev_erase =
			(psy_ui_fp_componentimp_dev_erase)
			view_dev_erase;
		view_imp_vtable.dev_capture =
			(psy_ui_fp_componentimp_dev_capture)
			view_dev_capture;
		view_imp_vtable.dev_releasecapture =
			(psy_ui_fp_componentimp_dev_releasecapture)
			view_dev_releasecapture;
		view_imp_vtable.dev_invalidate =
			(psy_ui_fp_componentimp_dev_invalidate)
			view_dev_invalidate;
		view_imp_vtable.dev_invalidaterect =
			(psy_ui_fp_componentimp_dev_invalidaterect)
			view_dev_invalidaterect;
		view_imp_vtable.dev_update =
			(psy_ui_fp_componentimp_dev_update)
			view_dev_update;
		view_imp_vtable.dev_setfont =
			(psy_ui_fp_componentimp_dev_setfont)
			view_dev_setfont;
		view_imp_vtable.dev_children =
			(psy_ui_fp_componentimp_dev_children)
			view_dev_children;
		view_imp_vtable.dev_enableinput =
			(psy_ui_fp_componentimp_dev_enableinput)
			view_dev_enableinput;
		view_imp_vtable.dev_preventinput =
			(psy_ui_fp_componentimp_dev_preventinput)
			view_dev_preventinput;
		view_imp_vtable.dev_inputprevented =
			(psy_ui_fp_componentimp_dev_inputprevented)
			view_dev_inputprevented;
		view_imp_vtable.dev_set_cursor =
			(psy_ui_fp_componentimp_dev_set_cursor)
			view_dev_setcursor;
		view_imp_vtable.dev_seticonressource =
			(psy_ui_fp_componentimp_dev_seticonressource)
			view_dev_seticonressource;
		view_imp_vtable.dev_textmetric =
			(psy_ui_fp_componentimp_dev_textmetric)
			view_dev_textmetric;
		view_imp_vtable.dev_setbackgroundcolour =
			(psy_ui_fp_componentimp_dev_setbackgroundcolour)
			view_dev_setbackgroundcolour;
		view_imp_vtable.dev_settitle =
			(psy_ui_fp_componentimp_dev_settitle)
			view_dev_settitle;
		view_imp_vtable.dev_title =
			(psy_ui_fp_componentimp_dev_title)
			view_dev_title;
		view_imp_vtable.dev_setfocus =
			(psy_ui_fp_componentimp_dev_setfocus)
			view_dev_setfocus;
		view_imp_vtable.dev_hasfocus =
			(psy_ui_fp_componentimp_dev_hasfocus)
			view_dev_hasfocus;
		view_imp_vtable.dev_flags =
			(psy_ui_fp_componentimp_dev_flags)
			view_dev_flags;
		view_imp_vtable.dev_platform_handle =
			(psy_ui_fp_componentimp_dev_platform_handle)
			dev_platform_handle;
		view_imp_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_viewcomponentimp_init(psy_ui_ViewComponentImp* self,
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view,
	const char* classname,
	psy_ui_RealRectangle position,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_componentimp_init(&self->imp);
	view_imp_vtable_init(self);
	self->imp.vtable = &view_imp_vtable;
	self->view = view;
	self->component = component;
	self->parent = parent;
	self->visible = TRUE;
	self->title = NULL;
	if (parent && parent->imp) {
		parent->imp->vtable->dev_insert(parent->imp, &self->imp, NULL);
	}
	self->position = position;
	self->viewcomponents = NULL;
}

void view_dev_dispose(psy_ui_ViewComponentImp* self)
{
	if (self->component == psy_ui_app_capture(psy_ui_app())) {
		psy_ui_component_release_capture(self->view);
	}
	psy_ui_componentimp_dispose(&self->imp);
	self->parent = NULL;
	psy_list_free(self->viewcomponents);
	self->viewcomponents = NULL;
	free(self->title);
	self->title = NULL;
}

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_alloc(void)
{
	return (psy_ui_ViewComponentImp*)malloc(sizeof(psy_ui_ViewComponentImp));
}

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view,
	const char* classname,
	psy_ui_RealRectangle position,
	uint32_t dwStyle,
	int usecommand)
{
	psy_ui_ViewComponentImp* rv;

	rv = psy_ui_viewcomponentimp_alloc();
	if (rv) {
		psy_ui_viewcomponentimp_init(rv,
			component,
			parent,
			view,
			classname,
			position,
			dwStyle,
			usecommand
		);
	}
	return rv;
}

void view_dev_destroy(psy_ui_ViewComponentImp* self)
{
	psy_ui_Component* component;
	psy_ui_Component* parent;
	psy_List* c;
	psy_List* p;
	psy_List* q;

	component = self->component;
	if (!component) {
		return;
	}
	parent = psy_ui_component_parent(component);
	// psy_signal_emit(&component->signal_destroy,
	//	self->component, 0);
	// component->vtable->on_destroy(component);	
	if (parent) {
		psy_ui_component_erase(parent, component);
	}
	c = psy_ui_component_children(self->component, psy_ui_NONE_RECURSIVE);
	for (p = c; p != NULL; p = q) {
		psy_ui_Component* component;
		bool deallocate;

		q = p->next;
		component = (psy_ui_Component*)psy_list_entry(p);
		deallocate = component->deallocate;
		psy_ui_component_destroy(component);
		if (deallocate) {
			free(component);
		}
	}
	psy_list_free(c);
	psy_list_free(self->viewcomponents);
	self->viewcomponents = NULL;
	psy_ui_component_dispose(self->component);
}

void view_dev_show(psy_ui_ViewComponentImp* self)
{
	self->visible = TRUE;
}

void view_dev_showstate(psy_ui_ViewComponentImp* self, int state)
{
	self->visible = TRUE;
}

void view_dev_hide(psy_ui_ViewComponentImp* self)
{
	self->visible = FALSE;
}

int view_dev_visible(psy_ui_ViewComponentImp* self)
{
	return self->visible;
}

int view_dev_draw_visible(psy_ui_ViewComponentImp* self)
{
	psy_ui_Component* curr;
	bool rv;

	curr = self->component;
	rv = TRUE;
	while (curr) {
		if (!psy_ui_component_visible(curr)) {
			rv = FALSE;
			break;
		}
		curr = psy_ui_component_parent(curr);
	}
	return rv;
}

void view_dev_move(psy_ui_ViewComponentImp* self, psy_ui_Point origin)
{
	const psy_ui_TextMetric* tm;

	tm = view_dev_textmetric(self);
	psy_ui_realrectangle_set_topleft(&self->position,
		psy_ui_realpoint_make(
			psy_ui_value_px(&origin.x, tm, NULL),
			psy_ui_value_px(&origin.y, tm, NULL)));
	//	view_dev_invalidate(self);
}

void view_dev_resize(psy_ui_ViewComponentImp* self, psy_ui_Size size)
{
	psy_ui_RealPoint topleft;
	const psy_ui_TextMetric* tm;

	topleft = psy_ui_realrectangle_topleft(&self->position);
	tm = view_dev_textmetric(self);
	self->position = psy_ui_realrectangle_make(
		topleft,
		psy_ui_realsize_make(
			psy_ui_value_px(&size.width, tm, NULL),
			psy_ui_value_px(&size.height, tm, NULL)));
}

void view_dev_clientresize(psy_ui_ViewComponentImp* self, intptr_t width,
	intptr_t height)
{

}

psy_ui_RealRectangle view_dev_position(psy_ui_ViewComponentImp* self)
{
	return self->position;
}

psy_ui_RealRectangle view_dev_screenposition(psy_ui_ViewComponentImp* self)
{
	psy_ui_RealPoint translation;
	psy_ui_RealRectangle position;
	psy_ui_RealRectangle viewscreenposition;

	assert(self->view);

	translation = translatecoords(self, self->component, self->view);
	position = psy_ui_component_position(self->component);
	viewscreenposition = psy_ui_component_screenposition(self->view);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			viewscreenposition.left + position.left + translation.x,
			viewscreenposition.top + position.top + translation.y),
		psy_ui_realrectangle_size(&position));
}

void view_dev_setposition(psy_ui_ViewComponentImp* self, psy_ui_Point topleft,
	psy_ui_Size size)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Size parentsize;
	psy_ui_Size* pparentsize;

	tm = view_dev_textmetric(self);
	if (psy_ui_size_has_percent(&size)) {
		if (psy_ui_component_parent_const(self->component)) {
			parentsize = psy_ui_component_scroll_size(
				psy_ui_component_parent_const(self->component));
		}
		else {
			parentsize = psy_ui_component_scroll_size(self->component);
		}
		pparentsize = &parentsize;
	}
	else {
		pparentsize = NULL;
	}
	if (topleft.x.set) {
		self->position.left = psy_ui_value_px(&topleft.x, tm, pparentsize);
		self->position.right += self->position.left;
	}
	if (topleft.y.set) {
		self->position.top = psy_ui_value_px(&topleft.y, tm, pparentsize);
		self->position.bottom += self->position.top;
	}
	if (size.width.set) {
		self->position.right = self->position.left +
			psy_ui_value_px(&size.width, tm, pparentsize);
	}
	if (size.height.set) {
		self->position.bottom = self->position.top +
			psy_ui_value_px(&size.height, tm, pparentsize);
	}
}

psy_ui_Size view_dev_size(const psy_ui_ViewComponentImp* self)
{
	return psy_ui_size_make_px(self->position.right - self->position.left,
		self->position.bottom - self->position.top);
}

void view_dev_updatesize(psy_ui_ViewComponentImp* self)
{
}

psy_ui_Size view_dev_framesize(psy_ui_ViewComponentImp* self)
{
	return psy_ui_size_make_px(self->position.right - self->position.left,
		self->position.bottom - self->position.top);
}

psy_ui_Component* view_dev_parent(psy_ui_ViewComponentImp* self)
{
	return self->parent;
}

void view_dev_setparent(psy_ui_ViewComponentImp* self, psy_ui_Component* parent)
{
	if (self->parent) {
		psy_ui_component_erase(self->parent, self->component);
	}
	self->parent = parent;
	if (parent) {
		psy_ui_component_insert(parent, self->component, NULL);
	}
}

void view_dev_insert(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child,
	psy_ui_ViewComponentImp* insertafter)
{
	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
		psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_list_append(&self->viewcomponents, child->component);
		if (child->view != self->view) {
			psy_List* p;
			psy_List* q;

			child->view = self->view;
			child->component->view = self->view;
			q = p = psy_ui_component_children(child->component, psy_ui_RECURSIVE);
			for (; p != NULL; p = p->next) {
				psy_ui_Component* curr;
				psy_ui_ViewComponentImp* imp;

				curr = (psy_ui_Component*)p->entry;
				curr->view = child->view;
				if (child->imp.vtable->dev_flags(curr->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
					imp = (psy_ui_ViewComponentImp*)curr->imp;
					imp->view = child->view;
				}
			}
			psy_list_free(q);
		}
	}
}

void view_dev_remove(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child)
{
	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
		psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_List* p;

		p = psy_list_find_entry(self->viewcomponents, child->component);
		if (p) {
			psy_list_remove(&self->viewcomponents, p);
			if (child->component->deallocate) {
				psy_ui_component_deallocate(child->component);
			}
			else {
				psy_ui_component_dispose(child->component);
			}
		}
	}
	else {
		assert(0);
		/* todo */
	}
}

void view_dev_erase(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp* child)
{
	if ((child->imp.vtable->dev_flags(&child->imp) & psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) ==
		psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN) {
		psy_List* p;

		p = psy_list_find_entry(self->viewcomponents, child->component);
		if (p) {
			psy_list_remove(&self->viewcomponents, p);
		}
	}
	else {
		assert(0);
		/* todo */
	}
}

void view_dev_setorder(psy_ui_ViewComponentImp* self, psy_ui_ViewComponentImp*
	insertafter)
{
}

void view_dev_capture(psy_ui_ViewComponentImp* self)
{
	self->view->imp->vtable->dev_capture(self->view->imp);
}

void view_dev_releasecapture(psy_ui_ViewComponentImp* self)
{
	self->view->imp->vtable->dev_releasecapture(self->view->imp);
}


void view_dev_scrollto(psy_ui_ViewComponentImp* self, psy_ui_RealPoint delta,
	const psy_ui_RealRectangle* r)
{
	psy_ui_RealRectangle rc;

	if (r) {
		rc = view_translation(self, r);
	}
	else {
		rc = psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_component_scroll_size_px(self->component));
		rc = view_translation(self, &rc);
	}
	psy_ui_component_scroll_to(self->view, delta, &rc);
}


void view_dev_invalidate(psy_ui_ViewComponentImp* self)
{
	psy_ui_RealRectangle r;

	r = psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
		psy_ui_component_scroll_size_px(self->component));
	view_dev_invalidaterect(self, &r);
}

void view_dev_invalidaterect(psy_ui_ViewComponentImp* self,
	const psy_ui_RealRectangle* r)
{
	if (r) {
		psy_ui_RealRectangle rc;

		if (view_intersection(self, r, &rc)) {
			self->view->imp->vtable->dev_invalidaterect(self->view->imp, &rc);
		}
	}
}

psy_ui_RealRectangle view_translation(psy_ui_ViewComponentImp* self,
	const psy_ui_RealRectangle* r)
{
	psy_ui_RealPoint translation;
	psy_ui_RealRectangle position;
	psy_ui_RealMargin spacing;

	translation = translatecoords(self, self->component, self->view);
	position = psy_ui_component_position(self->component);
	spacing = psy_ui_component_spacing_px(self->view);
	return psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			position.left + r->left + translation.x + spacing.left,
			position.top + r->top + translation.y + spacing.top),
		psy_ui_realrectangle_size(r));
}

bool view_intersection(psy_ui_ViewComponentImp* self,
	const psy_ui_RealRectangle* rc, psy_ui_RealRectangle* rv)
{
	psy_ui_Component* curr;
	psy_ui_RealRectangle parent_position;
	psy_ui_RealRectangle client;

	assert(self);

	curr = self->component;
	parent_position = psy_ui_component_position(curr);
	client = psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(),
		psy_ui_realrectangle_size(&parent_position));
	if (rc) {
		*rv = *rc;
	}
	else {
		*rv = client;
	}
	if (!psy_ui_realrectangle_intersection(rv, &client)) {
		return FALSE;
	}
	while (curr && curr != self->view && psy_ui_component_parent(curr)) {
		psy_ui_RealRectangle client;

		psy_ui_realrectangle_move(rv, psy_ui_realrectangle_topleft(
			&parent_position));
		curr = psy_ui_component_parent(curr);
		parent_position = psy_ui_component_position(curr);
		client = psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_realrectangle_size(&parent_position));
		if (!psy_ui_realrectangle_intersection(rv, &client)) {
			return FALSE;
		}
	}
	return TRUE;
}

psy_ui_RealPoint translatecoords(psy_ui_ViewComponentImp* self,
	psy_ui_Component* src,
	psy_ui_Component* dst)
{
	psy_ui_RealPoint rv;
	psy_ui_Component* curr;
	psy_ui_RealRectangle r;

	curr = psy_ui_component_parent(src);
	psy_ui_realpoint_init(&rv);
	while (dst != curr && curr != NULL) {
		r = psy_ui_component_position(curr);
		psy_ui_realpoint_add(&rv, psy_ui_realrectangle_topleft(&r));
		curr = psy_ui_component_parent(curr);
	}
	return rv;
}

void view_dev_update(psy_ui_ViewComponentImp* self)
{
	psy_ui_component_update(self->view);
}

void view_dev_setfont(psy_ui_ViewComponentImp* self, psy_ui_Font* source)
{
}

void view_dev_enableinput(psy_ui_ViewComponentImp* self)
{
}

void view_dev_preventinput(psy_ui_ViewComponentImp* self)
{
}

bool view_dev_inputprevented(const psy_ui_ViewComponentImp* self)
{
	return FALSE;
}

const psy_ui_TextMetric* view_dev_textmetric(const psy_ui_ViewComponentImp*
	self)
{
	if (self->component) {
		const psy_ui_Font* font;

		font = psy_ui_component_font(self->component);
		if (font) {
			return psy_ui_font_textmetric(font);
		}
	}
	return NULL;
}

void view_dev_setcursor(psy_ui_ViewComponentImp* self, psy_ui_CursorStyle
	cursorstyle)
{
	psy_ui_component_set_cursor(self->view, cursorstyle);
}

void view_dev_seticonressource(psy_ui_ViewComponentImp* self, int ressourceid)
{
}

void view_dev_setbackgroundcolour(psy_ui_ViewComponentImp* self,
	psy_ui_Colour colour)
{
}

void view_dev_settitle(psy_ui_ViewComponentImp* self, const char* title)
{
	psy_strreset(&self->title, title);
}

const char* view_dev_title(const psy_ui_ViewComponentImp* self)
{
	if (self->title) {
		return self->title;
	}
	return "";
}

void view_dev_setfocus(psy_ui_ViewComponentImp* self)
{
	assert(self);
	assert(self->view);

	if (!psy_ui_component_has_focus(self->view)) {
		psy_ui_component_set_focus(self->view);
	}
}

int view_dev_hasfocus(psy_ui_ViewComponentImp* self)
{
	return psy_ui_app()->focus == self->component;
}

uintptr_t view_dev_flags(const psy_ui_ComponentImp* self)
{
	return psy_ui_COMPONENTIMPFLAGS_HANDLECHILDREN;
}

psy_List* view_dev_children(psy_ui_ViewComponentImp* self, int recursive)
{
	psy_List* rv = 0;
	psy_List* p;

	for (p = self->viewcomponents; p != NULL; psy_list_next(&p)) {
		psy_list_append(&rv, (psy_ui_Component*)psy_list_entry(p));
		if (recursive == psy_ui_RECURSIVE) {
			psy_List* r;
			psy_List* q;

			q = psy_ui_component_children((psy_ui_Component*)psy_list_entry(p),
				recursive);
			for (r = q; r != NULL; psy_list_next(&r)) {
				psy_list_append(&rv, (psy_ui_Component*)psy_list_entry(r));
			}
			psy_list_free(q);
		}
	}
	return rv;
}
