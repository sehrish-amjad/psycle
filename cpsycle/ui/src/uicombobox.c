/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicombobox.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void psy_ui_combobox_on_destroyed(psy_ui_ComboBox*);
static void psy_ui_combobox_on_property_changed(psy_ui_ComboBox*,
	psy_Property* sender);
static void psy_ui_combobox_before_property_destroyed(psy_ui_ComboBox*,
	psy_Property* sender);
static bool psy_ui_combobox_has_prev_entry(const psy_ui_ComboBox*);
static bool psy_ui_combobox_has_next_entry(const psy_ui_ComboBox*);
static void psy_ui_combobox_on_sel_change(psy_ui_ComboBox*,
	psy_ui_Component* sender, intptr_t index);
static void psy_ui_combobox_on_less(psy_ui_ComboBox*, psy_ui_Button* sender);
static void psy_ui_combobox_on_more(psy_ui_ComboBox*, psy_ui_Button* sender);
static void psy_ui_combobox_on_expand(psy_ui_ComboBox*, psy_ui_Button* sender);
static void psy_ui_combobox_on_text_field(psy_ui_ComboBox*,
	psy_ui_Label* sender, psy_ui_MouseEvent*);
static void psy_ui_combobox_expand(psy_ui_ComboBox*);
static void psy_ui_combobox_on_mouse_wheel(psy_ui_ComboBox*,
	psy_ui_MouseEvent*);
static void psy_ui_combobox_on_textfield_changed(psy_ui_ComboBox*,
	psy_ui_Text* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_ComboBox* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			psy_ui_combobox_on_destroyed;
		vtable.on_mouse_wheel =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_combobox_on_mouse_wheel;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(psy_ui_combobox_base(self),
		&vtable);
}

/* implementation */
void psy_ui_combobox_init(psy_ui_ComboBox* self, psy_ui_Component* parent)
{	
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->property_ = NULL;
	self->simple_ = FALSE;
	self->prevent_wheel_select_ = FALSE;
	psy_signal_init(&self->signal_selchanged);
	psy_table_init_keysize(&self->itemdata_, 33);
	psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_COMBOBOX);
	psy_ui_component_set_align_expand(psy_ui_combobox_base(self),
		psy_ui_HEXPAND);	
	/* dropdown */
	psy_ui_dropdownbox_init(&self->dropdown_, self->component.view);
	psy_ui_component_init(&self->pane_, &self->dropdown_.component,
		&self->dropdown_.component);
	psy_ui_component_set_padding(&self->pane_, psy_ui_margin_make_em(
		0.0, 0.2, 0.2, 0.2));
	psy_ui_component_set_align(&self->pane_, psy_ui_ALIGN_CLIENT);
	/* listbox */
	psy_ui_listbox_init(&self->listbox_, &self->pane_);
	self->listbox_.scroller.prevent_mouse_down_propagation = FALSE;
	psy_signal_connect(&self->listbox_.signal_selchanged, self,
		psy_ui_combobox_on_sel_change);
	psy_ui_component_set_align(psy_ui_listbox_base(&self->listbox_),
		psy_ui_ALIGN_CLIENT);
	/* text */
	psy_ui_text_init(&self->text_, &self->component);	
	psy_ui_component_set_style_type(psy_ui_text_base(&self->text_),
		psy_ui_STYLE_COMBOBOX_TEXT);
	psy_ui_component_set_align(psy_ui_text_base(&self->text_),
		psy_ui_ALIGN_CLIENT);	
	// psy_ui_text_set_char_number(&self->text_, 10.0);
	psy_signal_connect(&psy_ui_text_base(&self->text_)->signal_mouse_down,
		self, psy_ui_combobox_on_text_field);
	psy_ui_text_prevent(&self->text_);		
	/* buttons */
	psy_ui_component_init(&self->buttons_, &self->component, NULL);
	psy_ui_component_set_align_expand(&self->buttons_, psy_ui_HEXPAND);
	psy_ui_component_set_align(&self->buttons_, psy_ui_ALIGN_RIGHT);
	/* less */
	psy_ui_button_init_connect(&self->less_, &self->buttons_,
		self, psy_ui_combobox_on_less);
	psy_ui_component_set_align(psy_ui_button_base(&self->less_),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_set_icon(&self->less_, psy_ui_ICON_LESS);
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->less_));
	/* more */
	psy_ui_button_init_connect(&self->more_, &self->buttons_,
		self, psy_ui_combobox_on_more);
	psy_ui_button_set_icon(&self->more_, psy_ui_ICON_MORE);
	psy_ui_component_set_align(psy_ui_button_base(&self->more_),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->more_));
	/* expand */
	psy_ui_button_init(&self->expand_, &self->buttons_);
	psy_signal_connect(&self->expand_.signal_clicked,
		self, psy_ui_combobox_on_expand);
	psy_ui_component_set_align(psy_ui_button_base(&self->expand_),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_set_icon(&self->expand_, psy_ui_ICON_DOWN);
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->expand_));
}

void psy_ui_combobox_init_simple(psy_ui_ComboBox* self,
	psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->property_ = NULL;
	self->simple_ = TRUE;
	psy_signal_init(&self->signal_selchanged);
	psy_table_init_keysize(&self->itemdata_, 33);
	psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_COMBOBOX);
	psy_ui_component_set_align_expand(psy_ui_combobox_base(self),
		psy_ui_HEXPAND);	
	/* listbox */
	psy_ui_component_init(&self->pane_, &self->component, NULL);
	psy_ui_component_set_padding(&self->pane_, psy_ui_margin_make_em(
		0.0, 0.2, 0.2, 0.2));
	psy_ui_component_set_align(&self->pane_, psy_ui_ALIGN_CLIENT);
	psy_ui_listbox_init(&self->listbox_, &self->pane_);
	self->listbox_.scroller.prevent_mouse_down_propagation = FALSE;
	psy_signal_connect(&self->listbox_.signal_selchanged, self,
		psy_ui_combobox_on_sel_change);
	psy_ui_component_set_align(psy_ui_listbox_base(&self->listbox_),
		psy_ui_ALIGN_CLIENT);	
	/* text */
	psy_ui_component_init(&self->editpane_, &self->component, NULL);
	psy_ui_component_set_align(&self->editpane_, psy_ui_ALIGN_TOP);	
	psy_ui_text_init(&self->text_, &self->editpane_);	
	psy_ui_component_set_style_type(psy_ui_text_base(&self->text_),
		psy_ui_STYLE_COMBOBOX_TEXT);
	psy_ui_component_set_align(psy_ui_text_base(&self->text_),
		psy_ui_ALIGN_LEFT);
	psy_ui_text_set_char_number(&self->text_, 10.0);
	psy_signal_connect(&psy_ui_text_base(&self->text_)->signal_mouse_down,
		self, psy_ui_combobox_on_text_field);
	psy_signal_connect(&self->text_.signal_change,
		self, psy_ui_combobox_on_textfield_changed);
	/* buttons */
	psy_ui_component_init(&self->buttons_, &self->editpane_, NULL);
	psy_ui_component_set_align_expand(&self->buttons_, psy_ui_HEXPAND);
	psy_ui_component_set_align(&self->buttons_, psy_ui_ALIGN_LEFT);
	/* less */
	psy_ui_button_init_connect(&self->less_, &self->buttons_,
		self, psy_ui_combobox_on_less);
	psy_ui_component_set_align(psy_ui_button_base(&self->less_),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_set_icon(&self->less_, psy_ui_ICON_LESS);
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->less_));
	/* more */
	psy_ui_button_init_connect(&self->more_, &self->buttons_,
		self, psy_ui_combobox_on_more);
	psy_ui_button_set_icon(&self->more_, psy_ui_ICON_MORE);
	psy_ui_component_set_align(psy_ui_button_base(&self->more_),
		psy_ui_ALIGN_LEFT);
	psy_ui_component_prevent_app_focus_out(psy_ui_button_base(&self->more_));
}


void psy_ui_combobox_on_destroyed(psy_ui_ComboBox* self)
{
	assert(self);	
	
	if (self->property_) {
		psy_property_disconnect(self->property_, self);
	}
	psy_table_dispose(&self->itemdata_);
	psy_signal_dispose(&self->signal_selchanged);
	if (!self->simple_) {
		psy_ui_component_destroy(&self->dropdown_.component);
	}
}

psy_ui_ComboBox* psy_ui_combobox_alloc(void)
{
	return (psy_ui_ComboBox*)malloc(sizeof(psy_ui_ComboBox));
}

psy_ui_ComboBox* psy_ui_combobox_alloc_init(psy_ui_Component* parent)
{
	psy_ui_ComboBox* rv;

	rv = psy_ui_combobox_alloc();
	if (rv) {
		psy_ui_combobox_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void psy_ui_combobox_exchange(psy_ui_ComboBox* self,
	psy_Property* property)
{
	self->property_ = property;
	if (self->property_) {
		psy_List* p;

		p = psy_property_begin(self->property_);
		for (; p != NULL; p = p->next) {
			psy_Property* property;

			property = (psy_Property*)p->entry;
			psy_ui_combobox_add_text(self,
				(psy_property_translation_prevented(property))
				? psy_property_text(property)
				: psy_ui_translate(psy_property_text(property)));
		}
		psy_ui_combobox_on_property_changed(self, self->property_);
		psy_property_connect(self->property_, self,
			psy_ui_combobox_on_property_changed);
		psy_signal_connect(&self->property_->before_destroyed, self,
			psy_ui_combobox_before_property_destroyed);
	}
}

void psy_ui_combobox_on_property_changed(psy_ui_ComboBox* self,
	psy_Property* sender)
{
	if (psy_property_is_int(sender) || psy_property_is_choice(sender)) {
		psy_ui_combobox_select(self, psy_property_item_int(
			self->property_));
	}
}

void psy_ui_combobox_before_property_destroyed(psy_ui_ComboBox* self,
	psy_Property* sender)
{
	assert(self);

	self->property_ = NULL;
}

intptr_t psy_ui_combobox_add_text(psy_ui_ComboBox* self, const char* text)
{
	assert(self);

	return psy_ui_listbox_add_text(&self->listbox_, text);
}

void psy_ui_combobox_set_text(psy_ui_ComboBox* self, const char* text,
	intptr_t index)
{
	assert(self);

	psy_ui_listbox_set_text(&self->listbox_, text, index);
	if (index == psy_ui_combobox_cursel(self)) {
		char text[512];

		psy_ui_combobox_text(self, text);
		psy_ui_text_set_text(&self->text_, text);
	}
}

void psy_ui_combobox_text(psy_ui_ComboBox* self, char* rv)
{
	const char* edit_text;
	
	assert(self);

	edit_text = psy_ui_text_text(&self->text_);
	psy_snprintf(rv, 256, "%s", edit_text);
}

void psy_ui_combobox_text_at(psy_ui_ComboBox* self, char* text, intptr_t index)
{
	assert(self);

	psy_ui_listbox_text(&self->listbox_, text, index);
}

intptr_t psy_ui_combobox_count(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_listbox_count(&self->listbox_);
}

void psy_ui_combobox_clear(psy_ui_ComboBox* self)
{
	assert(self);

	psy_ui_listbox_clear(&self->listbox_);
	psy_table_clear(&self->itemdata_);
	psy_ui_text_set_text(&self->text_, "");
}

void psy_ui_combobox_select(psy_ui_ComboBox* self, intptr_t index)
{
	char text[512];

	assert(self);

	psy_ui_listbox_set_cur_sel(&self->listbox_, index);
	psy_ui_listbox_text(&self->listbox_, text, index);	
	psy_ui_text_set_text(&self->text_, text);
}

intptr_t psy_ui_combobox_cursel(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_listbox_cur_sel(&self->listbox_);
}

void psy_ui_combobox_set_char_number(psy_ui_ComboBox* self, double number)
{
	assert(self);

	psy_ui_text_set_char_number(&self->text_, number);
	if (number == 0.0) {
		psy_ui_component_set_align(psy_ui_text_base(&self->text_),
			psy_ui_ALIGN_CLIENT);
		psy_ui_component_set_align(&self->buttons_, psy_ui_ALIGN_RIGHT);
	} else {
		psy_ui_component_set_align(&self->buttons_, psy_ui_ALIGN_LEFT);
		psy_ui_component_set_align(psy_ui_text_base(&self->text_),
			psy_ui_ALIGN_LEFT);
	}	
}

void psy_ui_combobox_set_item_data(psy_ui_ComboBox* self, uintptr_t index,
	intptr_t data)
{
	assert(self);

	psy_table_insert(&self->itemdata_, index, (void*)data);
}

intptr_t psy_ui_combobox_itemdata(psy_ui_ComboBox* self, uintptr_t index)
{
	assert(self);

	if (psy_table_exists(&self->itemdata_, index)) {
		return (intptr_t)psy_table_at(&self->itemdata_, index);
	}
	return -1;
}

bool psy_ui_combobox_has_prev_entry(const psy_ui_ComboBox* self)
{
	assert(self);

	return psy_ui_combobox_cursel(self) > 0;
}

bool psy_ui_combobox_has_next_entry(const psy_ui_ComboBox* self)
{
	intptr_t count;

	assert(self);

	count = psy_ui_combobox_count(self);
	return (psy_ui_combobox_cursel(self) < count - 1);
}

void psy_ui_combobox_on_sel_change(psy_ui_ComboBox* self,
	psy_ui_Component* sender, intptr_t index)
{
	char text[512];

	assert(self);

	if (!self->simple_) {
		psy_ui_dropdownbox_hide(&self->dropdown_);
	}
	psy_ui_listbox_set_cur_sel(&self->listbox_, index);	
	psy_ui_listbox_text(&self->listbox_, text, index);
	psy_ui_text_set_text(&self->text_, text);
	if (self->property_ && index != psy_property_item_int(self->property_)) {
		psy_property_set_item_int(self->property_, index);
	}
	psy_signal_emit(&self->signal_selchanged, self, 1, index);
}

void psy_ui_combobox_on_less(psy_ui_ComboBox* self, psy_ui_Button* sender)
{
	intptr_t index;

	assert(self);

	index = psy_ui_combobox_cursel(self);
	if (index > 0) {
		psy_ui_combobox_select(self, index - 1);
		if (self->property_) {
			psy_property_set_item_int(self->property_, index - 1);
		}
		psy_signal_emit(&self->signal_selchanged, self, 1, index - 1);
	}
}

void psy_ui_combobox_on_more(psy_ui_ComboBox* self, psy_ui_Button* sender)
{
	intptr_t count;
	intptr_t index;

	assert(self);

	index = psy_ui_combobox_cursel(self);
	count = psy_ui_combobox_count(self);
	if (index < count - 1) {
		psy_ui_combobox_select(self, index + 1);
		if (self->property_) {
			psy_property_set_item_int(self->property_, index + 1);
		}
		psy_signal_emit(&self->signal_selchanged, self, 1, index + 1);
	}
}

void psy_ui_combobox_on_text_field(psy_ui_ComboBox* self, psy_ui_Label* sender,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_combobox_expand(self);
	psy_ui_mouseevent_stop_propagation(ev);
}

void psy_ui_combobox_on_expand(psy_ui_ComboBox* self, psy_ui_Button* sender)
{
	assert(self);
	
	psy_ui_combobox_expand(self);	
}

void psy_ui_combobox_expand(psy_ui_ComboBox* self)
{
	assert(self);

	if (self->simple_) {
		return;
	}
	if (!psy_ui_component_visible(&self->dropdown_.component)) {
		psy_ui_dropdownbox_show(&self->dropdown_, &self->component);
		psy_ui_component_capture(&self->dropdown_.component);
		self->dropdown_.component.capture_relative = TRUE;
	}
}

void psy_ui_combobox_on_mouse_wheel(psy_ui_ComboBox* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (self->prevent_wheel_select_) {		
		return;
	}
	if (psy_ui_mouseevent_delta(ev) != 0) {
		intptr_t index;

		index = psy_ui_combobox_cursel(self) +
			psy_sgn(psy_ui_mouseevent_delta(ev));
		if (index >= 0 && index < psy_ui_combobox_count(self)) {
			psy_ui_combobox_select(self, index);
			if (self->property_ && index != psy_property_item_int(
					self->property_)) {
				psy_property_set_item_int(self->property_, index);
			}
			psy_signal_emit(&self->signal_selchanged, self, 1, index);
		}
	}
	psy_ui_mouseevent_prevent_default(ev);
}

void psy_ui_combobox_on_textfield_changed(psy_ui_ComboBox* self,
	psy_ui_Text* sender)
{
	psy_ui_listbox_set_cur_sel(&self->listbox_, -1);
}
