/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOGRID_H)
#define PIANOGRID_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_PIANOROLL

/* host */
#include "patternhostcmds.h"
#include "pianogriddraw.h"
#include "pianokeyboard.h"
#include "pianoruler.h"
#include "workspace.h"
/* ui */
#include <uicomponent.h>
/* audio */
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct Pianogrid
** @brief Draws and edits pattern events of the song sequence
*/
typedef struct Pianogrid {
	/*! @extends */
	psy_ui_Component component;
	/*! @internal */
	psy_audio_PatternEntry* hoverpatternentry;	
	psy_audio_SequenceCursor old_cursor;
	psy_audio_SequenceCursor dragcursor;	
	psy_audio_SequenceCursor last_drag_cursor;
	bool prevent_context_menu;	
	bool preventscrollleft;	
	psy_audio_PatternEvent play_event;
	/* references */
	KeyboardState* keyboardstate;
	PianoGridState* state;	
	PatternCmds cmds;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardState*,
	PianoGridState*, InputHandler*, psy_DiskOp*);

void pianogrid_invalidate_line(Pianogrid*, intptr_t line);
void pianogrid_invalidate_cursor(Pianogrid*);
void pianogrid_invalidate_playbar(Pianogrid*);

INLINE const psy_audio_BlockSelection* pianogrid_selection(const Pianogrid* self)
{
	assert(self);

	return &self->state->pv->selection.block;
}

INLINE psy_ui_Component* pianogrid_base(Pianogrid* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_PIANOROLL */

#endif /* PIANOGRID_H */
