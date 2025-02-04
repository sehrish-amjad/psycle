/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQVIEWDURATION_H)
#define SEQVIEWDURATION_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_SEQVIEW

/* host */
#include "seqviewstate.h"
/* ui */
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SeqView;

/* SeqviewDuration */
typedef struct SeqviewDuration {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Label desc_;
	psy_ui_Label duration_;			
	psy_dsp_seconds_t duration_ms_;
	psy_dsp_beatpos_t duration_bts_;
	bool calculating_;
	/* references */
	Workspace* workspace_;
} SeqviewDuration;

void seqviewduration_init(SeqviewDuration*, psy_ui_Component* parent,
	Workspace*);

void seqviewduration_update(SeqviewDuration*, bool force);
void seqviewduration_stop_duration_calc(SeqviewDuration*);

void seqviewduration_idle(SeqviewDuration*);

INLINE psy_ui_Component* seqviewduration_base(SeqviewDuration* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_SEQVIEW */

#endif /* SEQVIEWDURATION_H */
