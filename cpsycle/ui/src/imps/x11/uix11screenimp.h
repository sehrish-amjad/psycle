/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_x11_SCREENIMP_H
#define psy_ui_x11_SCREENIMP_H

#include "../../uiscreen.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_x11_ScreenImp {
	psy_ui_ScreenImp imp;	
} psy_ui_x11_ScreenImp;

void psy_ui_x11_screenimp_init(psy_ui_x11_ScreenImp* self);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_SCREENIMP_H */
#endif /* PSYCLE_TK_X11 */
