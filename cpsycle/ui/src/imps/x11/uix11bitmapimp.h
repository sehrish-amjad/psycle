/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11
#ifndef psy_ui_x11_BITMAPIMP_H
#define psy_ui_x11_BITMAPIMP_H

#include "uibitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>

typedef struct psy_ui_x11_BitmapImp {
	psy_ui_BitmapImp imp;
	Pixmap pixmap;
	Pixmap mask;
} psy_ui_x11_BitmapImp;

void psy_ui_x11_bitmapimp_init(psy_ui_x11_BitmapImp* self,
	psy_ui_RealSize size);
	
uint8_t* psy_ui_x11_bitmapimp_make_x11_wm_icon(
	psy_ui_x11_BitmapImp*, uintptr_t* amount);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_BITMAPIMP_H */
#endif /* PSYCLE_TK_X11 */
