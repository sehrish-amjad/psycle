/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_EMPTY
#ifndef psy_ui_empty_COLORDIALOGIMP_H
#define psy_ui_empty_COLORDIALOGIMP_H

#include "../../uicolordialog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_empty_ColourDialogImp {
	psy_ui_ColourDialogImp imp;
	struct psy_ui_Component* parent;		
	psy_ui_Colour colour;
} psy_ui_empty_ColourDialogImp;

void psy_ui_empty_colourdialogimp_init(psy_ui_empty_ColourDialogImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_empty_COLORDIALOGIMP_H */
#endif /* PSYCLE_TK_EMPTY */
