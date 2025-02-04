/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiemptysavedialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_EMPTY

#include "../../uiapp.h"
#include <stdlib.h>
#include "../../uiapp.h"
#include "uiemptycomponentimp.h"
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void dev_dispose(psy_ui_empty_SaveDialogImp*);
static int dev_execute(psy_ui_empty_SaveDialogImp*);
static const psy_Path* dev_path(psy_ui_empty_SaveDialogImp*);

/* vtable */
static psy_ui_SaveDialogImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_empty_SaveDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_fp_savedialogimp_dev_dispose)
			dev_dispose;
		imp_vtable.dev_execute =
			(psy_ui_fp_savedialogimp_dev_execute)
			dev_execute;
		imp_vtable.dev_path =
			(psy_ui_fp_savedialogimp_dev_path)
			dev_path;
		imp_vtable_initialized = TRUE;
	}
}
/* implementation */
void psy_ui_empty_savedialogimp_init(psy_ui_empty_SaveDialogImp* self,
	psy_ui_Component* parent)
{
	psy_ui_savedialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;	
	self->parent = parent;
	self->title = strdup("");
	self->filter = strdup("");
	self->defaultextension = strdup("");
	self->initialdir = strdup("");
	psy_path_init(&self->path, NULL);
}

void psy_ui_empty_savedialogimp_init_all(psy_ui_empty_SaveDialogImp* self,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_savedialogimp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->parent = parent;
	self->title = strdup(title);
	self->filter = strdup(filter);
	self->defaultextension = strdup(defaultextension);
	self->initialdir = strdup(initialdir);
	psy_path_init(&self->path, NULL);
}

void dev_dispose(psy_ui_empty_SaveDialogImp* self)
{
	free(self->title);
	free(self->filter);
	free(self->defaultextension);
	free(self->initialdir);
	psy_path_dispose(&self->path);
}

int dev_execute(psy_ui_empty_SaveDialogImp* self)
{	
	return FALSE;
}

const psy_Path* dev_path(psy_ui_empty_SaveDialogImp* self)
{
	return &self->path;
}

#endif /* PSYCLE_TK_EMPTY */
