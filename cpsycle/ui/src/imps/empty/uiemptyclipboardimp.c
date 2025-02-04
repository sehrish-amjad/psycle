/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiemptyclipboardimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_EMPTY

/* local */
#include "../../uiapp.h"
/* file */
#include <encoding.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void psy_ui_empty_clipboardimp_dispose(psy_ui_empty_ClipboardImp*);
static const char* psy_ui_empty_clipboardimp_read_text(psy_ui_empty_ClipboardImp*);
static void psy_ui_empty_clipboardimp_write_text(psy_ui_empty_ClipboardImp*,
	const char* text);

/* vtable */
static psy_ui_ClipboardImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_empty_ClipboardImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_clipboard_imp_fp_dispose)
			psy_ui_empty_clipboardimp_dispose;
		imp_vtable.dev_read_text = 
			(psy_ui_clipboard_imp_fp_read_text)
			psy_ui_empty_clipboardimp_read_text;
		imp_vtable.dev_write_text =
			(psy_ui_clipboard_imp_fp_write_text)
			psy_ui_empty_clipboardimp_write_text;
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}

/* implementation */
void psy_ui_empty_clipboardimp_init(psy_ui_empty_ClipboardImp* self)
{
	assert(self);

	psy_ui_clipboard_imp_init(&self->imp);
	imp_vtable_init(self);
	self->text = NULL;
}

void psy_ui_empty_clipboardimp_dispose(psy_ui_empty_ClipboardImp* self)
{
	assert(self);

	free(self->text);
	self->text = NULL;
}

const char* psy_ui_empty_clipboardimp_read_text(psy_ui_empty_ClipboardImp* self)
{
	assert(self);

	return self->text;
}

void psy_ui_empty_clipboardimp_write_text(psy_ui_empty_ClipboardImp* self,
	const char* text)
{
	assert(self);

	psy_strreset(&self->text, text);
}

#endif /* PSYCLE_TK_EMPTY */
