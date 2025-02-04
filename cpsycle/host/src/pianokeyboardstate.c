/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianokeyboardstate.h"

void keyboardstate_init(KeyboardState* self, psy_ui_Orientation orientation,
	bool white_size_fixed)
{
	assert(self);

	self->orientation = orientation;
	self->keymin = 0;
	self->keymax = 119;
	self->white_size_fixed = white_size_fixed;
	if (self->orientation == psy_ui_VERTICAL) {
		self->default_key_extent = psy_ui_value_make_eh(1.0);
		self->key_align = psy_ui_ALIGN_LEFT;
	} else {
		self->default_key_extent = psy_ui_value_make_ew(1.0);
		self->key_align = psy_ui_ALIGN_BOTTOM;
	}
	self->align_keys = FALSE;	
	self->key_extent = self->default_key_extent;
	self->key_extent_px = 13;
	self->keyboard_extent_px = 13 * (self->keymax - self->keymin);
	self->notemode = psy_dsp_NOTESTAB_A220;
	self->drawpianokeys = TRUE;
	self->entry.keyrange.low = 255;
	self->entry.keyrange.high = 255;	
}
