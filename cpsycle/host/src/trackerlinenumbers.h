/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERLINENUMBERS_H)
#define TRACKERLINENUMBERS_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_TRACKERVIEW

/* host */
#include "trackergridstate.h"
/* ui */
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** TrackerLineNumbers
** 
** Draws the tracker linenumbers
*/


typedef struct TrackerLineNumbers {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	bool draw_cursor;	
	bool show_in_hex;
	bool show_beat;
	const char* format;
	const char* format_seqstart;
	psy_audio_SequenceCursor old_cursor;
	psy_ui_RealSize size;	
	psy_ui_Colour draw_restore_fg_colour;
	psy_ui_Colour draw_restore_bg_colour;	
	/* references */
	TrackerState* state;	
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, psy_ui_Component* parent,
	TrackerState*);

void trackerlinenumbers_invalidate_cursor(TrackerLineNumbers*);
void trackerlinenumbers_invalidate_playbar(TrackerLineNumbers*);
void trackerlinenumbers_update_format(TrackerLineNumbers*);
void trackerlinenumbers_show_cursor(TrackerLineNumbers*);
void trackerlinenumbers_hide_cursor(TrackerLineNumbers*);
void trackerlinenumbers_show_in_hex(TrackerLineNumbers*);
void trackerlinenumbers_show_in_decimal(TrackerLineNumbers*);

INLINE void trackerlinenumbers_show_beat(TrackerLineNumbers* self)
{
	self->show_beat = TRUE;
	trackerlinenumbers_update_format(self);
}

INLINE void trackerlinenumbers_hide_beat(TrackerLineNumbers* self)
{
	self->show_beat = FALSE;
	trackerlinenumbers_update_format(self);
}

INLINE psy_ui_Component* trackerlinenumbers_base(TrackerLineNumbers* self)
{
	assert(self);

	return &self->component;
}


/* TrackerLineNumberView */

typedef struct TrackerLineNumberView {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	TrackerLineNumbers linenumbers;	
} TrackerLineNumberView;

void trackerlinenumberview_init(TrackerLineNumberView*,
	psy_ui_Component* parent, TrackerState*);

void trackerlinenumberview_set_scroll_top(TrackerLineNumberView*,
	psy_ui_Value top);

INLINE psy_ui_Component* trackerlinenumberview_base(TrackerLineNumberView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_TRACKERVIEW */

#endif /* TRACKERLINENUMBERS_H */
