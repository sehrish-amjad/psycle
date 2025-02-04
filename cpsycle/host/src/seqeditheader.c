/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditheader.h"

#ifdef PSYCLE_USE_SEQEDITOR

/* host */
#include "resources/resource.h"
#include "sequencetrackbox.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
/* platform */
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 10.0

/* SeqEditHeader */
void seqeditheader_init(SeqEditHeader* self, psy_ui_Component* parent,
	SeqEditState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_init(&self->pane, &self->component, NULL);	
	psy_ui_component_set_align(&self->pane, psy_ui_ALIGN_FIXED);
	/* timesig */
	seqedittimesigs_init(&self->timesig, &self->pane, state);
	psy_ui_component_hide(&self->timesig.component);
	psy_ui_component_set_align(&self->timesig.component, psy_ui_ALIGN_TOP);
	/* loops */
	seqeditloops_init(&self->loops, &self->pane, state);
	psy_ui_component_hide(&self->loops.component);
	psy_ui_component_set_align(&self->loops.component, psy_ui_ALIGN_TOP);
	/* ruler */
	seqeditruler_init(&self->ruler, &self->pane, state);
	psy_ui_component_set_align(&self->ruler.component, psy_ui_ALIGN_TOP);
}

void seqeditheader_hide_timesig(SeqEditHeader* self)
{
	psy_ui_component_hide(&self->timesig.component);
	psy_ui_component_invalidate(&self->component);
}

void seqeditheader_show_timesig(SeqEditHeader* self)
{
	psy_ui_component_show(&self->timesig.component);
	psy_ui_component_invalidate(&self->component);
}

void seqeditheader_hide_loops(SeqEditHeader* self)
{
	psy_ui_component_hide(&self->loops.component);
	psy_ui_component_invalidate(&self->component);
}

void seqeditheader_show_loops(SeqEditHeader* self)
{
	psy_ui_component_show(&self->loops.component);
	psy_ui_component_invalidate(&self->component);
}

#endif /* PSYCLE_USE_SEQEDITOR */
