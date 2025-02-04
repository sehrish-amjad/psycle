/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uibutton.h"
/* local */
#include "uiapp.h"
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"


psy_ui_ButtonRepeat psy_ui_buttonrepeat_make(uintptr_t rate, uintptr_t first_rate)
{
	psy_ui_ButtonRepeat rv;

	rv.repeat_rate = rate;
	rv.first_repeat_rate = first_rate;
	rv.first_repeat = TRUE;
	psy_ui_mouseevent_init(&rv.repeat_event);
	return rv;
}


/* psy_ui_Button */

/* prototypes */
static void psy_ui_button_on_destroyed(psy_ui_Button*);
static void psy_ui_button_on_language_changed(psy_ui_Button*);
static void psy_ui_button_on_draw(psy_ui_Button*, psy_ui_Graphics*);
static void psy_ui_button_on_mouse_down(psy_ui_Button*, psy_ui_MouseEvent*);
static void psy_ui_button_on_mouse_up(psy_ui_Button*, psy_ui_MouseEvent*);
static void psy_ui_button_emit(psy_ui_Button*, psy_ui_MouseEvent*);
static void psy_ui_button_on_preferred_size(psy_ui_Button*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void button_on_key_down(psy_ui_Button*, psy_ui_KeyboardEvent*);
static psy_ui_RealSize psy_ui_button_size(psy_ui_Button*);
static void psy_ui_button_on_update_styles(psy_ui_Button*);
static void psy_ui_button_load_bitmaps(psy_ui_Button*);
static void psy_ui_button_on_property_changed(psy_ui_Button*,
	psy_Property* sender);
static void psy_ui_button_before_property_destroyed(psy_ui_Button*,
	psy_Property* sender);
static void psy_ui_button_on_repeat_timer(psy_ui_Button*, uintptr_t id);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Button* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(psy_ui_button_base(self)->vtable);
		super_vtable = *(psy_ui_button_base(self)->vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_button_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_button_on_draw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			psy_ui_button_on_preferred_size;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_button_on_mouse_down;
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_button_on_mouse_up;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			button_on_key_down;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_button_on_language_changed;
		vtable.onupdatestyles =
			(psy_ui_fp_component)
			psy_ui_button_on_update_styles;
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			psy_ui_button_on_repeat_timer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(psy_ui_button_base(self), &vtable);
}

/* implementation */
void psy_ui_button_init(psy_ui_Button* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(psy_ui_button_base(self), parent, NULL);
	vtable_init(self);
	self->property = NULL;
	psy_ui_component_set_aligner(&self->component, NULL);
	self->icon = psy_ui_ICON_NONE;
	self->charnumber = 0.0;
	self->linespacing = 1.0;
	self->bitmapident = 1.0;
	self->data = psy_INDEX_INVALID;
	self->textalignment = psy_ui_ALIGNMENT_CENTER;
	self->text = NULL;
	self->translation = NULL;
	self->translate = TRUE;
	self->shiftstate = FALSE;
	self->ctrlstate = FALSE;
	self->buttonstate = 1;
	self->allowrightclick = FALSE;
	self->stoppropagation = TRUE;
	self->lightresourceid = psy_INDEX_INVALID;
	self->darkresourceid = psy_INDEX_INVALID;
	self->light_path = NULL;
	self->dark_path = NULL;
	self->click_mode = psy_ui_CLICK_MODE_RELEASE;
	self->repeat = psy_ui_buttonrepeat_make(0, 0);
	self->text_setted = FALSE;
	psy_ui_colour_init(&self->bitmaptransparency);
	psy_ui_bitmap_init(&self->bitmapicon);
	psy_signal_init(&self->signal_clicked);
	psy_ui_component_set_style_types(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON, psy_ui_STYLE_BUTTON_HOVER,
		psy_ui_STYLE_BUTTON_SELECT, psy_INDEX_INVALID);
	psy_ui_component_set_style_type_active(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON_ACTIVE);
	psy_ui_component_set_style_type_focus(psy_ui_button_base(self),
		psy_ui_STYLE_BUTTON_FOCUS);
}

void psy_ui_button_init_text(psy_ui_Button* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);

	psy_ui_button_init(self, parent);
	psy_ui_button_set_text(self, text);
}

void psy_ui_button_init_connect(psy_ui_Button* self, psy_ui_Component* parent,
	void* context, void* fp)
{
	assert(self);

	psy_ui_button_init(self, parent);
	psy_signal_connect(&self->signal_clicked, context, fp);
}

void psy_ui_button_init_text_connect(psy_ui_Button* self, psy_ui_Component*
	parent, const char* text, void* context, void* fp)
{
	assert(self);

	psy_ui_button_init_connect(self, parent, context, fp);
	psy_ui_button_set_text(self, text);
}

void psy_ui_button_init_icon(psy_ui_Button* self, psy_ui_Component*
	parent, psy_ui_ButtonIcon icon)
{
	assert(self);

	psy_ui_button_init(self, parent);
	psy_ui_button_set_icon(self, icon);
}

void psy_ui_button_init_icon_connect(psy_ui_Button* self, psy_ui_Component*
	parent, psy_ui_ButtonIcon icon, void* context, void* fp)
{
	assert(self);

	psy_ui_button_init_connect(self, parent, context, fp);
	psy_ui_button_set_icon(self, icon);
}

psy_ui_Button* psy_ui_button_alloc(void)
{
	return (psy_ui_Button*)malloc(sizeof(psy_ui_Button));
}

psy_ui_Button* psy_ui_button_allocinit(psy_ui_Component* parent)
{
	psy_ui_Button* rv;

	rv = psy_ui_button_alloc();
	if (rv) {
		psy_ui_button_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_button_on_destroyed(psy_ui_Button* self)
{
	assert(self);

	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
	free(self->text);
	self->text = NULL;
	free(self->translation);
	self->translation = NULL;
	free(self->light_path);
	self->light_path = NULL;
	free(self->dark_path);
	self->dark_path = NULL;
	psy_signal_dispose(&self->signal_clicked);
	psy_ui_bitmap_dispose(&self->bitmapicon);
}

void psy_ui_button_connect(psy_ui_Button* self, void* context, void* fp)
{
	assert(self);
	assert(fp);

	psy_signal_connect(&self->signal_clicked, context, fp);
}

void psy_ui_button_on_language_changed(psy_ui_Button* self)
{
	assert(self);

	if (self->translate) {
		psy_strreset(&self->translation, psy_ui_translate(self->text));
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_button_on_draw(psy_ui_Button* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	psy_ui_RealRectangle r;
	char* text;
	double cpx;
	const psy_ui_TextMetric* tm;

	assert(self);

	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->translate && self->translation) {
		text = self->translation;
	}
	else {
		text = self->text;
	}
	size = psy_ui_component_size_px(psy_ui_button_base(self));
	r = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
	if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_HORIZONTAL) ==
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL) {
		cpx = (size.width - psy_ui_button_size(self).width) / 2.0;
	}
	else if ((self->textalignment & psy_ui_ALIGNMENT_RIGHT) ==
		psy_ui_ALIGNMENT_RIGHT) {
		cpx = size.width - psy_ui_button_size(self).width;
	}
	else {
		cpx = 0.0;
	}
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
		psy_ui_RealSize srcbpmsize;
		psy_ui_RealSize destbpmsize;
		double vcenter;
		double ratio;

		srcbpmsize = psy_ui_bitmap_size(&self->bitmapicon);
#if PSYCLE_USE_TK == PSYCLE_TK_X11
		destbpmsize = srcbpmsize;
#else		
		ratio = (tm->tmAscent - tm->tmDescent) / srcbpmsize.height;
		if (fabs(ratio - 1.0) < 0.15) {
			ratio = 1.0;
		}
		destbpmsize.width = srcbpmsize.width * ratio;
		destbpmsize.height = srcbpmsize.height * ratio;
#endif		
		vcenter = (size.height - destbpmsize.height) / 2.0;
		psy_ui_graphics_draw_stretched_bitmap(g, &self->bitmapicon,
			psy_ui_realrectangle_make(psy_ui_realpoint_make(cpx, vcenter),
				destbpmsize),
			psy_ui_realpoint_zero(),
			srcbpmsize);
		cpx += destbpmsize.width + tm->tmAveCharWidth * self->bitmapident;

	}
	if (self->icon != psy_ui_ICON_NONE) {
		const psy_ui_Glyph* glyph;
		double vcenter;

		glyph = psy_ui_icons_at(&psy_ui_app()->icons, self->icon);
		if (glyph) {
			if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
				psy_ui_ALIGNMENT_CENTER_VERTICAL) {
				vcenter = (size.height -
					psy_ui_glyph_size(glyph).height)
					/ 2.0;
			}
			else {
				vcenter = 0.0;
			}
			psy_ui_graphics_draw_glyph(g, glyph,
				psy_ui_realpoint_make(cpx, vcenter),
				psy_ui_component_colour(&self->component));
			cpx += psy_ui_glyph_size(glyph).width;
			cpx += tm->tmAveCharWidth * self->bitmapident;
		}
	}
	if (psy_strlen(text) > 0) {
		double vcenter;

		if ((self->textalignment & psy_ui_ALIGNMENT_CENTER_VERTICAL) ==
			psy_ui_ALIGNMENT_CENTER_VERTICAL) {
			vcenter = (size.height - tm->tmHeight) / 2.0;
		}
		else {
			vcenter = 0.0;
		}
		if (psy_ui_component_input_prevented(&self->component)) {
			psy_ui_graphics_set_text_colour(g, psy_ui_colour_make(0x00777777));
		}
		else {
			psy_ui_graphics_set_text_colour(g, psy_ui_component_colour(
				&self->component));
		}
		psy_ui_graphics_textout_rectangle(g, psy_ui_realpoint_make(cpx, vcenter),
			psy_ui_ETO_CLIPPED, r, text, strlen(text));
	}
}

psy_ui_RealSize psy_ui_button_size(psy_ui_Button* self)
{
	psy_ui_RealSize rv;
	char* text;
	const psy_ui_TextMetric* tm;

	assert(self);

	rv = psy_ui_realsize_zero();
	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->translate && self->translation) {
		text = self->translation;
	}
	else {
		text = self->text;
	}

	if (psy_strlen(text) > 0) {
		psy_ui_Size size;
		const psy_ui_Font* font;

		font = psy_ui_component_font(&self->component);
		if (font) {
			size = psy_ui_font_textsize(font, text, psy_strlen(text));
		}
		else {
			size = psy_ui_size_zero();
		}
		rv.width += psy_ui_value_px(&size.width, tm, NULL);
	}
	if (self->icon != psy_ui_ICON_NONE) {
		const psy_ui_Glyph* glyph;

		glyph = psy_ui_icons_at(&psy_ui_app()->icons, self->icon);
		if (glyph) {
			rv.width += psy_ui_glyph_size(glyph).width;
			if (psy_strlen(text) == 0) {
				rv.height = psy_ui_glyph_size(glyph).height;
			}
		}
		if (psy_strlen(text) > 0) {
			rv.width += tm->tmAveCharWidth * self->bitmapident;
		}
	}
	if (!psy_ui_bitmap_empty(&self->bitmapicon)) {
		psy_ui_RealSize srcbpmsize;
		double ratio;

		srcbpmsize = psy_ui_bitmap_size(&self->bitmapicon);
#if PSYCLE_USE_TK == PSYCLE_TK_X11
		ratio = 1.0;
#else		
		ratio = (tm->tmAscent - tm->tmDescent) / srcbpmsize.height;
		if (fabs(ratio - 1.0) < 0.15) {
			ratio = 1.0;
		}
#endif		
		rv.width += srcbpmsize.width * ratio;
		if (psy_strlen(text) > 0) {
			rv.width += tm->tmAveCharWidth * self->bitmapident;
		}
	}
	return rv;
}

void psy_ui_button_set_char_number(psy_ui_Button* self, double number)
{
	self->charnumber = psy_max(0.0, number);
}

void psy_ui_button_set_line_spacing(psy_ui_Button* self, double spacing)
{
	assert(self);

	self->linespacing = spacing;
}

void psy_ui_button_on_preferred_size(psy_ui_Button* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;

	assert(self);

	tm = psy_ui_component_textmetric(psy_ui_button_base(self));
	if (self->charnumber == 0) {
		psy_ui_RealSize button_size;

		button_size = psy_ui_button_size(self);
		rv->width = psy_ui_value_make_px(button_size.width);
		if (button_size.height == 0.0) {
			rv->height = psy_ui_value_make_eh(self->linespacing);
		}
		else {
			rv->height = psy_ui_value_make_px(button_size.height);
		}
	}
	else {
		rv->width = psy_ui_value_make_ew(self->charnumber);
		rv->height = psy_ui_value_make_eh(self->linespacing);
	}
}

void psy_ui_button_on_mouse_down(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	super_vtable.on_mouse_down(psy_ui_button_base(self), ev);
	if (self->stoppropagation) {
		psy_ui_mouseevent_stop_propagation(ev);
	}
	if (!psy_ui_component_input_prevented(&self->component)) {
		if ((self->allowrightclick || (psy_ui_mouseevent_button(ev) == 1)) &&
			((self->click_mode == psy_ui_CLICK_MODE_PRESS) ||
				(self->click_mode == psy_ui_CLICK_MODE_REPEAT))) {
			psy_ui_button_emit(self, ev);
			if (self->repeat.repeat_rate != 0) {
				self->repeat.repeat_event = *ev;
				self->repeat.first_repeat = TRUE;
				psy_ui_component_start_timer(&self->component, 0,
					self->repeat.first_repeat_rate);
			}
		}
		else {
			psy_ui_component_capture(psy_ui_button_base(self));
		}
	}
}

void psy_ui_button_on_mouse_up(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	super_vtable.on_mouse_up(psy_ui_button_base(self), ev);
	if (!psy_ui_component_input_prevented(&self->component)) {
		psy_ui_component_release_capture(psy_ui_button_base(self));
		if (psy_ui_component_input_prevented(&self->component)) {
			psy_ui_mouseevent_stop_propagation(ev);
			return;
		}
		self->buttonstate = psy_ui_mouseevent_button(ev);
		if (self->allowrightclick || psy_ui_mouseevent_button(ev) == 1) {
			psy_ui_RealRectangle client_position;

			client_position = psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
				psy_ui_component_scroll_size_px(psy_ui_button_base(self)));
			if (self->stoppropagation) {
				psy_ui_mouseevent_stop_propagation(ev);
			}
			if (self->click_mode == psy_ui_CLICK_MODE_RELEASE &&
				psy_ui_realrectangle_intersect(&client_position,
					psy_ui_mouseevent_offset(ev))) {
				psy_ui_button_emit(self, ev);
			}
		}
		if (self->repeat.repeat_rate != 0) {
			psy_ui_component_stop_timer(&self->component, 0);
		}
	}
}

void psy_ui_button_emit(psy_ui_Button* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	self->shiftstate = psy_ui_mouseevent_shift_key(ev);
	self->ctrlstate = psy_ui_mouseevent_ctrl_key(ev);
	psy_signal_emit(&self->signal_clicked, self, 0);
	if (self->property) {
		if (psy_property_is_choice_item(self->property)) {
			intptr_t index;

			if (!psy_property_parent(self->property)) {
				return;
			}
			index = psy_property_index(self->property);
			psy_property_set_item_int(psy_property_parent(self->property),
				index);
		}
		else if (psy_property_is_bool(self->property)) {
			psy_property_set_item_bool(self->property,
				!psy_property_item_bool(self->property));
		}
		else {
			psy_signal_emit(&self->property->changed, self->property, 0);
		}
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void psy_ui_button_set_text(psy_ui_Button* self, const char* text)
{
	assert(self);

	psy_strreset(&self->text, text);
	if (self->translate) {
		psy_strreset(&self->translation, psy_ui_translate(text));
	}
	self->text_setted = TRUE;
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

const char* psy_ui_button_text(const psy_ui_Button* self)
{
	return self->text;
}

void psy_ui_button_set_icon(psy_ui_Button* self, psy_ui_ButtonIcon icon)
{
	assert(self);

	self->icon = icon;
	psy_ui_component_invalidate(psy_ui_button_base(self));
}

void psy_ui_button_load_resource(psy_ui_Button* self,
	uintptr_t lightresourceid, uintptr_t darkresourceid,
	psy_ui_Colour transparency)
{
	assert(self);

	self->lightresourceid = lightresourceid;
	self->darkresourceid = darkresourceid;
	self->bitmaptransparency = transparency;
	psy_ui_button_load_bitmaps(self);
}

void psy_ui_button_load_bitmap(psy_ui_Button* self, const char* light_path,
	const char* dark_path, psy_ui_Colour transparency)
{
	assert(self);

	if (light_path) {
		psy_strreset(&self->light_path, light_path);
	}
	if (dark_path) {
		psy_strreset(&self->dark_path, dark_path);
	}
	self->bitmaptransparency = transparency;
	psy_ui_button_load_bitmaps(self);
}

void psy_ui_button_highlight(psy_ui_Button* self)
{
	assert(self);

	if (!psy_ui_button_highlighted(self)) {
		psy_ui_component_add_style_state(psy_ui_button_base(self),
			psy_ui_STYLESTATE_SELECT);
	}
}

void psy_ui_button_disable_highlight(psy_ui_Button* self)
{
	assert(self);

	if (psy_ui_button_highlighted(self)) {
		psy_ui_component_remove_style_state(psy_ui_button_base(self),
			psy_ui_STYLESTATE_SELECT);
	}
}

bool psy_ui_button_highlighted(const psy_ui_Button* self)
{
	assert(self);

	return (psy_ui_componentstyle_state(&self->component.style) &
		psy_ui_STYLESTATE_SELECT) == psy_ui_STYLESTATE_SELECT;
}

void psy_ui_button_settextcolour(psy_ui_Button* self, psy_ui_Colour colour)
{
	assert(self);

	psy_ui_component_set_colour(&self->component, colour);
}

void psy_ui_button_set_text_alignment(psy_ui_Button* self,
	psy_ui_Alignment alignment)
{
	assert(self);

	self->textalignment = alignment;
}

void psy_ui_button_prevent_translation(psy_ui_Button* self)
{
	assert(self);

	self->translate = FALSE;
	if (self->translation) {
		free(self->translation);
		self->translation = NULL;
	}
}

void psy_ui_button_set_click_mode(psy_ui_Button* self, psy_ui_ClickMode mode)
{
	assert(self);

	self->click_mode = mode;
}

void psy_ui_button_set_repeat(psy_ui_Button* self, uintptr_t rate, uintptr_t first_rate)
{
	assert(self);

	self->repeat = psy_ui_buttonrepeat_make(rate, first_rate);
	self->click_mode = psy_ui_CLICK_MODE_REPEAT;
}

void button_on_key_down(psy_ui_Button* self, psy_ui_KeyboardEvent* ev)
{
	assert(self);

	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN &&
		!psy_ui_component_input_prevented(&self->component)) {
		psy_signal_emit(&self->signal_clicked, self, 0);
		psy_ui_keyboardevent_stop_propagation(ev);
	}
}

void psy_ui_button_on_update_styles(psy_ui_Button* self)
{
	assert(self);

	psy_ui_button_load_bitmaps(self);
}

void psy_ui_button_load_bitmaps(psy_ui_Button* self)
{
	assert(self);

	if (psy_ui_app_has_dark_theme(psy_ui_app())) {
		if (self->darkresourceid != psy_INDEX_INVALID) {
			psy_ui_bitmap_load_resource(&self->bitmapicon,
				self->darkresourceid);
		}
		else if (psy_strlen(self->dark_path) > 0) {
			psy_ui_bitmap_load(&self->bitmapicon, self->dark_path);
		}
	}
	else {
		if (self->lightresourceid != psy_INDEX_INVALID) {
			psy_ui_bitmap_load_resource(&self->bitmapicon,
				self->lightresourceid);
		}
		else if (psy_strlen(self->light_path) > 0) {
			psy_ui_bitmap_load(&self->bitmapicon, self->light_path);
		}
	}
	if (!self->bitmaptransparency.mode.transparent) {
		psy_ui_bitmap_set_transparency(&self->bitmapicon,
			self->bitmaptransparency);
	}
}

void psy_ui_button_exchange(psy_ui_Button* self, psy_Property* property)
{
	assert(self);
	assert(property);

	assert(self);

	self->property = property;
	if (property) {
		psy_ui_button_on_property_changed(self, property);
		if (!self->text_setted) {
			psy_ui_button_set_text(self, psy_property_text(property));
		}
		psy_property_connect(property, self,
			psy_ui_button_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			psy_ui_button_before_property_destroyed);
	}
}

void psy_ui_button_on_property_changed(psy_ui_Button* self,
	psy_Property* sender)
{
	assert(self);

	if (psy_property_is_choice_item(sender)) {
		bool checked;

		checked = (psy_property_at_choice(psy_property_parent(sender))
			== sender);
		if (checked) {
			psy_ui_button_highlight(self);
		}
		else {
			psy_ui_button_disable_highlight(self);
		}
		return;
	}
	if (!psy_property_is_bool(sender)) {
		return;
	}
	if (psy_property_item_bool(sender)) {
		psy_ui_button_highlight(self);
	}
	else {
		psy_ui_button_disable_highlight(self);
	}
}

void psy_ui_button_before_property_destroyed(psy_ui_Button* self,
	psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}

void psy_ui_button_on_repeat_timer(psy_ui_Button* self, uintptr_t id)
{
	if (self->buttonstate == 0) {
		return;
	}
	if (self->repeat.first_repeat && self->repeat.first_repeat_rate !=
		self->repeat.repeat_rate) {
		psy_ui_component_start_timer(&self->component, id,
			self->repeat.repeat_rate);
		self->repeat.first_repeat = FALSE;
	}
	psy_ui_button_emit(self, &self->repeat.repeat_event);
}
