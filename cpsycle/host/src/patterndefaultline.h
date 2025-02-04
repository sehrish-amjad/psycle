/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNDEFAULTLINE_H)
#define PATTERNDEFAULTLINE_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_TRACKERVIEW

/* host */
#include "patternviewstate.h"
#include "trackerview.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternDefaultLine
*/
typedef struct PatternDefaultLine {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Label desc;
	psy_ui_Component pane;
	TrackerGrid grid;
	PatternViewState pvstate_default_line;
	TrackerState state;
	/* references */
	Workspace* workspace;
	psy_Configuration* config;
	PatternViewState* pv_state;
} PatternDefaultLine;

void patterndefaultline_init(PatternDefaultLine*, psy_ui_Component* parent,
	PatternViewState*, TrackConfig*, psy_Configuration*, Workspace*);

void patterndefaultline_update_song_tracks(PatternDefaultLine*);

INLINE psy_ui_Component* patterndefaultline_base(PatternDefaultLine* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_TRACKERVIEW */

#endif /* PATTERNDEFAULTLINE_H */
