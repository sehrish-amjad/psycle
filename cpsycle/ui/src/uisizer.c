/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uisizer.h"
/* local */
#include "uiapp.h"


/* prototypes */
static void psy_ui_sizer_on_draw(psy_ui_Sizer*, psy_ui_Graphics*);
static void psy_ui_sizer_on_mouse_down(psy_ui_Sizer*,
	psy_ui_MouseEvent*);
static void psy_ui_sizer_on_mouse_move(psy_ui_Sizer*,
	psy_ui_MouseEvent*);
static void psy_ui_sizer_on_mouse_up(psy_ui_Sizer*,
	psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable psy_ui_sizer_vtable;
static bool psy_ui_sizer_vtable_initialized = FALSE;

static void psy_ui_sizer_vtable_init(psy_ui_Sizer* self)
{
	assert(self);

	if (!psy_ui_sizer_vtable_initialized) {
		psy_ui_sizer_vtable = *(self->component.vtable);
		psy_ui_sizer_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_sizer_on_draw;
		psy_ui_sizer_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sizer_on_mouse_down;
		psy_ui_sizer_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sizer_on_mouse_move;
		psy_ui_sizer_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_sizer_on_mouse_up;
		psy_ui_sizer_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &psy_ui_sizer_vtable);
}

/* implementation */
void psy_ui_sizer_init(psy_ui_Sizer* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_sizer_vtable_init(self);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(1.0, 1.0));
	self->resize_component_ = NULL;
	self->dragging_ = FALSE;
}


void psy_ui_sizer_set_resize_component(psy_ui_Sizer* self,
	psy_ui_Component* resize_component)
{
	assert(self);

	self->resize_component_ = resize_component;
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_sizer_on_draw(psy_ui_Sizer* self, psy_ui_Graphics* g)
{
	const psy_ui_Glyph* glyph;

	assert(self);

	if (!self->resize_component_) {
		return;
	}
	glyph = psy_ui_icons_at(&psy_ui_app()->icons, psy_ui_ICON_GRIP);
	if (glyph) {
		psy_ui_RealSize size;
		psy_ui_RealPoint cp;

		size = psy_ui_component_scroll_size_px(&self->component);
		cp = psy_ui_realpoint_make(
			(size.width - psy_ui_glyph_size(glyph).width) / 2.0,
			(size.height - psy_ui_glyph_size(glyph).height) / 2.0);
		psy_ui_graphics_draw_glyph(g, glyph, cp,
			psy_ui_component_colour(&self->component));
	}
}

void psy_ui_sizer_on_mouse_down(psy_ui_Sizer* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (!self->resize_component_) {
		return;
	}
	if (psy_ui_mouseevent_button(ev) == 1) {
		self->frame_drag_offset_ = psy_ui_mouseevent_offset(ev);
		self->dragging_ = TRUE;
		psy_ui_component_capture(&self->component);
	}
}

void psy_ui_sizer_on_mouse_move(psy_ui_Sizer* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealRectangle position;

	assert(self);

	if ((!self->dragging_) || (!self->resize_component_)) {
		return;
	}
	position = psy_ui_component_screenposition(self->resize_component_);
	{
		psy_ui_Size size;

		size = psy_ui_size_make_px(
			position.right - position.left + (psy_ui_mouseevent_offset(ev).x -
				self->frame_drag_offset_.x),
			position.bottom - position.top + (psy_ui_mouseevent_offset(ev).y -
				self->frame_drag_offset_.y));
		psy_ui_component_resize(self->resize_component_, size);
	}
}

void psy_ui_sizer_on_mouse_up(psy_ui_Sizer* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (!self->resize_component_) {
		return;
	}
	psy_ui_component_release_capture(&self->component);
	self->dragging_ = FALSE;
}
