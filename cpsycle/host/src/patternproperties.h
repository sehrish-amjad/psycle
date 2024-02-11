/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNPROPERTIES)
#define PATTERNPROPERTIES

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_PATTERN_VIEW

/* host */
#include "closebar.h"
#include "patternviewstate.h"
#include "workspace.h"
/* audio */
#include <pattern.h>
/* ui */
#include <uibutton.h>
#include <uilabel.h>
#include <uinumberedit.h>
#include <uitext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PatternProperties {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	CloseBar close_bar;
	psy_ui_Label name_label;
	psy_ui_Label length_label;
	psy_ui_Text name_edit;
	psy_ui_NumberEdit length_edit;	
	psy_ui_NumberEdit timesig_numerator;
	psy_ui_NumberEdit timesig_denominator;
	/* references */	
	uintptr_t pattern_index;
	Workspace* workspace;
	PatternViewState* state;
} PatternProperties;

void patternproperties_init(PatternProperties*, psy_ui_Component* parent,
	PatternViewState*, Workspace* workspace);

INLINE psy_ui_Component* patternproperties_base(PatternProperties* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_PATTERN_VIEW */

#endif /* PATTERNPROPERTIES */

