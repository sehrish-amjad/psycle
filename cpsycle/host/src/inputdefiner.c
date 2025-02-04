/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "inputdefiner.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* container */
#include <hashtbl.h>
/* driver */
#include "../../driver/eventdriver.h"
/* platform */
#include "../../detail/portable.h"


static bool valid_key_code(uintptr_t keycode)
{
	return (keycode >= 0x30 ||
		keycode == psy_ui_KEY_SPACE ||
		keycode == psy_ui_KEY_INSERT ||
		keycode == psy_ui_KEY_PAUSE ||
		keycode == psy_ui_KEY_LEFT ||
		keycode == psy_ui_KEY_RIGHT ||
		keycode == psy_ui_KEY_UP ||
		keycode == psy_ui_KEY_DOWN ||
		keycode == psy_ui_KEY_TAB ||
		keycode == psy_ui_KEY_BACK ||
		keycode == psy_ui_KEY_DELETE ||
		keycode == psy_ui_KEY_HOME ||
		keycode == psy_ui_KEY_END ||
		keycode == psy_ui_KEY_RETURN ||
		keycode == psy_ui_KEY_PRIOR ||
		keycode == psy_ui_KEY_NEXT);
}

/* InputDefinerKeyNames */

/* prototypes */
static void inputdefinerkeynames_add_keys(InputDefinerKeyNames*);

/* implementation */
void inputdefinerkeynames_init(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_init(&self->container);
	inputdefinerkeynames_add_keys(self);
}

void inputdefinerkeynames_dispose(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)NULL);
}

void inputdefinerkeynames_add_keys(InputDefinerKeyNames* self)
{
	uintptr_t key;

	assert(self);
	
	inputdefinerkeynames_add(self, psy_ui_KEY_LEFT, "LEFT");
	inputdefinerkeynames_add(self, psy_ui_KEY_RIGHT, "RIGHT");
	inputdefinerkeynames_add(self, psy_ui_KEY_UP, "UP");
	inputdefinerkeynames_add(self, psy_ui_KEY_DOWN, "DOWN");
	inputdefinerkeynames_add(self, psy_ui_KEY_PRIOR, "PGUP");
	inputdefinerkeynames_add(self, psy_ui_KEY_NEXT, "PGDOWN");
	inputdefinerkeynames_add(self, psy_ui_KEY_TAB, "TAB");
	inputdefinerkeynames_add(self, psy_ui_KEY_BACK, "BACKSPACE");
	inputdefinerkeynames_add(self, psy_ui_KEY_DELETE, "DELETE");
	inputdefinerkeynames_add(self, psy_ui_KEY_HOME, "HOME");
	inputdefinerkeynames_add(self, psy_ui_KEY_END, "END");
	inputdefinerkeynames_add(self, psy_ui_KEY_RETURN, "RETURN");
	inputdefinerkeynames_add(self, psy_ui_KEY_INSERT, "INSERT");
	inputdefinerkeynames_add(self, psy_ui_KEY_PAUSE, "PAUSE");
	inputdefinerkeynames_add(self, psy_ui_KEY_SPACE, "SPACE");
	inputdefinerkeynames_add(self, psy_ui_KEY_ADD, "+");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_COMMA, ",");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_PLUS, "=");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_1, ".");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_2, "/");	
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_3, "`");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_4, "[");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_5, "\\");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_6, "]");
	inputdefinerkeynames_add(self, psy_ui_KEY_OEM_7, "'");
	for (key = psy_ui_KEY_F1; key <= psy_ui_KEY_F12; ++key) {
		char keystr[5];

		psy_snprintf(keystr, 5, "F%d", key - psy_ui_KEY_F1 + 1);
		inputdefinerkeynames_add(self, key, keystr);
	}
	for (key = 0x30 /*psy_ui_KEY_0*/; key <= 127 /*psy_ui_KEY_Z*/; ++key) {
		char keystr[5];

		psy_snprintf(keystr, 5, "%c", key);
		if (strlen(keystr)) {
			inputdefinerkeynames_add(self, key, keystr);
		}
	}
	for (key = 128; key <= 191; ++key) {
		char keystr[5];

		psy_snprintf(keystr, 5, "\xc2%c", key);
		if (strlen(keystr)) {
			inputdefinerkeynames_add(self, key, keystr);
		}
	}
	for (key = 192; key <= 255; ++key) {
		char keystr[5];

		psy_snprintf(keystr, 5, "\xc3%c", key);
		if (strlen(keystr)) {
			inputdefinerkeynames_add(self, key, keystr);
		}
	}
}

void inputdefinerkeynames_add(InputDefinerKeyNames* self,
	uintptr_t keycode, const char* name)
{
	assert(self);

	if (!psy_table_exists(&self->container, keycode)) {
		psy_table_insert(&self->container, (uintptr_t)keycode,
			psy_strdup(name));
	}
}

const char* inputdefinerkeynames_at(const InputDefinerKeyNames* self,
	uintptr_t keycode)
{
	assert(self);

	return (psy_table_exists(&self->container, keycode))
		? (const char*)psy_table_at_const(&self->container, keycode)
		: "";
}

/* static InputDefinerKeyNames definition */
static int refcount = 0;
static InputDefinerKeyNames keynames;

static void keynames_init(void);
static void keynames_release(void);

void keynames_init(void)
{	
	if (refcount == 0) {
		inputdefinerkeynames_init(&keynames);
	}
	++refcount;
}

void keynames_release(void)
{
	--refcount;
	if (refcount == 0) {		
		inputdefinerkeynames_dispose(&keynames);
	}
}

/* InputDefiner */

/* prototypes */
static void inputdefiner_on_destroyed(InputDefiner*);
static void inputdefiner_on_key_down(InputDefiner*, psy_ui_KeyboardEvent*);
static void inputdefiner_on_key_up(InputDefiner*, psy_ui_KeyboardEvent*);
static void inputdefiner_on_focus_lost(InputDefiner*);
static void inputdefiner_on_clear(InputDefiner*, psy_ui_Component* sender);
static void inputdefiner_on_property_changed(InputDefiner*,
	psy_Property* sender);
static void inputdefiner_before_property_destroyed(InputDefiner*,
	psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(InputDefiner* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			inputdefiner_on_destroyed;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			inputdefiner_on_key_down;
		vtable.onkeyup = 
			(psy_ui_fp_component_on_key_event)
			inputdefiner_on_key_up;		
		vtable.on_focuslost =
			(psy_ui_fp_component)
			inputdefiner_on_focus_lost;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(inputdefiner_base(self), &vtable);
}

/* implementation */
void inputdefiner_init(InputDefiner* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->property = NULL;
	psy_ui_component_set_style_type_focus(inputdefiner_base(self),
		psy_ui_STYLE_EDIT_FOCUS);
	psy_ui_component_set_tab_index(inputdefiner_base(self), 0);
	psy_ui_label_init(&self->label, &self->component);
	psy_ui_component_set_align(psy_ui_label_base(&self->label),
		psy_ui_ALIGN_CLIENT);
	psy_ui_button_init_connect(&self->clear, &self->component,
		self, inputdefiner_on_clear);
	psy_ui_button_set_text(&self->clear, "settings.none");
	psy_ui_component_set_align(psy_ui_button_base(&self->clear),
		psy_ui_ALIGN_RIGHT);
	psy_signal_init(&self->signal_accept);
	keynames_init();
	self->input = 0;
	self->regular_key = 0;
	self->changed = FALSE;
	inputdefiner_set_input(self, self->input);
}

void inputdefiner_on_destroyed(InputDefiner* self)
{	
	assert(self);

	psy_signal_dispose(&self->signal_accept);	
	keynames_release();
	if (self->property) {
		psy_property_disconnect(self->property, self);
	}
}

InputDefiner* inputdefiner_alloc(void)
{
	return (InputDefiner*)malloc(sizeof(InputDefiner));
}

InputDefiner* inputdefiner_alloc_init(psy_ui_Component* parent)
{
	InputDefiner* rv;

	rv = inputdefiner_alloc();
	if (rv) {
		inputdefiner_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void inputdefiner_exchange(InputDefiner* self, psy_Property* property)
{
	assert(self);
	assert(property);

	self->property = property;
	if (property) {
		inputdefiner_on_property_changed(self, property);
		psy_property_connect(property, self,
			inputdefiner_on_property_changed);
		psy_signal_connect(&self->property->before_destroyed, self,
			inputdefiner_before_property_destroyed);
	}
}

void inputdefiner_set_input(InputDefiner* self, uint32_t input)
{
	char text[64];

	assert(self);
	
	self->input = input;	
	inputdefiner_text(self, text);
	psy_ui_label_set_text(&self->label, text);	
}

void inputdefiner_text(InputDefiner* self, char* text)
{	
	assert(self);

	inputdefiner_input_to_text(self->input, text);	
}

void inputdefiner_input_to_text(uint32_t input, char* text)
{
	uint32_t keycode;
	bool shift;
	bool ctrl;
	bool alt;
	bool up;

	text[0] = '\0';
	if (input == 0) {
		return;
	}
	psy_audio_decode_input(input, &keycode, &shift, &ctrl, &alt, &up);	
	if (alt) {
		strcat(text, "Alt + ");
	}
	if (shift) {
		strcat(text, "Shift + ");
	}
	if (ctrl) {
		strcat(text, "Ctrl + ");
	}
	if (up) {
		strcat(text, "UP + ");
	}
	keynames_init();
	strcat(text, inputdefinerkeynames_at(&keynames, keycode));		
	keynames_release();
}

void inputdefiner_on_key_down(InputDefiner* self, psy_ui_KeyboardEvent* ev)
{
	bool alt;
	bool shift;
	bool ctrl;
	bool up;

	assert(self);

	shift = psy_ui_keyboardevent_shift_key(ev);
	ctrl = psy_ui_keyboardevent_ctrl_key(ev);
	alt = psy_ui_keyboardevent_altkey(ev);
	up = 0;	
	self->old_input = self->input;
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_SHIFT ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_CONTROL ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_MENU) {
		if (self->regular_key == 0) {
			self->input = psy_audio_encodeinput(0, shift, ctrl, alt, up);
		} else {
			self->input = psy_audio_encodeinput(self->regular_key, shift, ctrl,
				alt, up);
		}
	}
	if (valid_key_code(psy_ui_keyboardevent_keycode(ev))) {
		self->regular_key = psy_ui_keyboardevent_keycode(ev);
		self->input = psy_audio_encodeinput(self->regular_key, shift, ctrl, alt,
			up);
	}
	inputdefiner_set_input(self, self->input);
	psy_ui_keyboardevent_stop_propagation(ev);
}

void inputdefiner_on_key_up(InputDefiner* self, psy_ui_KeyboardEvent* ev)
{
	bool alt;
	bool shift;
	bool ctrl;
	bool up;
	uint32_t inputkeycode;
	bool inputshift;
	bool inputctrl;
	bool inputalt;
	bool inputup;

	assert(self);

	alt = psy_ui_keyboardevent_altkey(ev);
    shift = psy_ui_keyboardevent_shift_key(ev);
    ctrl = psy_ui_keyboardevent_ctrl_key(ev);
	up = 0;	
	psy_audio_decode_input(self->input, &inputkeycode, &inputshift, &inputctrl, &inputalt, &inputup);
	if (self->regular_key) {
		if (inputalt) {
			self->input = psy_audio_encodeinput(inputkeycode, inputshift, inputctrl, inputalt, inputup);
		} else {
			self->input = psy_audio_encodeinput(inputkeycode, shift, ctrl, alt, inputup);
		}
	}
	if (valid_key_code(psy_ui_keyboardevent_keycode(ev))) {
		self->regular_key = 0;
	}
	if (!valid_key_code(inputkeycode)) {
		self->input = psy_audio_encodeinput(0, shift, ctrl, alt, up);
	}
	if (self->input != self->old_input) {
		self->changed = TRUE;
	}
	inputdefiner_set_input(self, self->input);
	if (self->changed) {		
		self->changed = FALSE;
		if (self->property) {
			psy_property_set_item_int(self->property,
				inputdefiner_input(self));			
		}
		psy_signal_emit(&self->signal_accept, self, 0);	
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void inputdefiner_on_focus_lost(InputDefiner* self)
{
	assert(self);

	super_vtable.on_focuslost(&self->component);
	if (self->changed) {
		self->changed = FALSE;
		if (self->property) {
			psy_property_set_item_int(self->property,
				inputdefiner_input(self));			
		}
		psy_signal_emit(&self->signal_accept, self, 0);
	}
}

void inputdefiner_on_clear(InputDefiner* self, psy_ui_Component* sender)
{
	inputdefiner_set_input(self, 0);
}

void inputdefiner_on_property_changed(InputDefiner* self, psy_Property* sender)
{
	inputdefiner_set_input(self, (uint32_t)psy_property_item_int(
		self->property));
}

void inputdefiner_before_property_destroyed(InputDefiner* self, psy_Property* sender)
{
	assert(self);

	self->property = NULL;
}
