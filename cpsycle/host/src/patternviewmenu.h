/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNVIEWMENU_H)
#define PATTERNVIEWMENU_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_PATTERN_VIEW

/* ui */
#include <uilabel.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

struct InputHandler;

/*!
** @struct PatternMenu
** @internal Context menu for Trackerview and Pianoroll
*/
typedef struct PatternMenu {
	/*! @extends */
	psy_ui_Scroller scroller;
	/*! @internal */	
	psy_ui_Component pane;	
	psy_ui_Label note_transposition;
	psy_ui_Component transpose;	
	/* references */	
	struct InputHandler* input_handler;
} PatternMenu;

void patternmenu_init(PatternMenu*, psy_ui_Component* parent,
	struct InputHandler*);

INLINE psy_ui_Component* patternmenu_base(PatternMenu* self)
{
	assert(self);

	return psy_ui_scroller_base(&self->scroller);
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_PATTERN_VIEW */

#endif /* PATTERNVIEWMENU_H */
