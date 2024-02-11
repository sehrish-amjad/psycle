/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_ICONS_H
#define psy_ui_ICONS_H

/* local */
#include "uiglyph.h"
/* container */
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_ICON_NONE,
	psy_ui_ICON_LESS,
	psy_ui_ICON_MORE,	
	psy_ui_ICON_UP,
	psy_ui_ICON_DOWN,
	psy_ui_ICON_CLOSE,
	psy_ui_ICON_SELECT,
	psy_ui_ICON_ENDLESS,
	psy_ui_ICON_SEMIBREVE,
	psy_ui_ICON_MINIM,
	psy_ui_ICON_MINIM_DOT,
	psy_ui_ICON_CROTCHET,
	psy_ui_ICON_CROTCHET_DOT,
	psy_ui_ICON_QUAVER,
	psy_ui_ICON_QUAVER_DOT,
	psy_ui_ICON_SEMIQUAVER,
	psy_ui_ICON_GRIP
} psy_ui_ButtonIcon;

/*!
** @struct psy_ui_Icons
*/
typedef struct psy_ui_Icons {
	psy_Table icons;	
} psy_ui_Icons;

void psy_ui_icons_init(psy_ui_Icons*);
void psy_ui_icons_dispose(psy_ui_Icons*);

void psy_ui_icons_add(psy_ui_Icons*, uintptr_t key, psy_ui_Glyph*);
const psy_ui_Glyph* psy_ui_icons_at(psy_ui_Icons* self, uintptr_t key);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_ICONS_H */
