/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianogrid.h"

#ifdef PSYCLE_USE_PIANOROLL

/* local */
#include "patternnavigator.h"
#include "cmdsnotes.h"
/* audio */
#include <sequencecmds.h>

#define CMD_ENTER 1100


/* prototypes */
static void pianogrid_connect_input_handler(Pianogrid*, InputHandler*);
static void pianogrid_on_draw(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_on_preferred_size(Pianogrid* self,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void pianogrid_on_mouse_down(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_on_mouse_move(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_on_mouse_up(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_on_display(Pianogrid*, psy_Property* sender);
static psy_audio_SequenceCursor pianogrid_make_cursor(Pianogrid*,
	psy_ui_RealPoint);
static void pianogrid_on_focus(Pianogrid*);
static void pianogrid_on_focus_lost(Pianogrid*);
static bool pianogrid_on_edt_cmds(Pianogrid*, InputHandler* sender);
static bool pianogrid_on_roll_cmds(Pianogrid*, InputHandler* sender);
static bool pianogrid_on_note_cmds(Pianogrid*, InputHandler* sender);
static bool pianogrid_handle_command(Pianogrid*, uintptr_t cmd);
static void pianogrid_enter(Pianogrid*);
static void pianogrid_row_clear(Pianogrid*);
static void pianogrid_drag_selection(Pianogrid*, psy_audio_SequenceCursor);
static bool pianogrid_key_hit_test(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track, uint8_t cursorkey);
static bool pianogrid_scroll_right(Pianogrid*, psy_audio_SequenceCursor);
static bool pianogrid_scroll_left(Pianogrid*, psy_audio_SequenceCursor);
static bool pianogrid_scroll_up(Pianogrid*, psy_audio_SequenceCursor);
static bool pianogrid_scroll_down(Pianogrid*, psy_audio_SequenceCursor);
static void pianogrid_prev_line(Pianogrid*);
static void pianogrid_prev_lines(Pianogrid*, uintptr_t lines, bool wrap);
static void pianogrid_prev_keys(Pianogrid*, uint8_t lines, bool wrap);
static void pianogrid_advance_line(Pianogrid*);
static void pianogrid_advance_lines(Pianogrid*, uintptr_t lines, bool wrap);
static void pianogrid_advance_keys(Pianogrid*, uint8_t lines, bool wrap);
static void pianogrid_set_cursor(Pianogrid*, psy_audio_SequenceCursor);

/* vtable */
static psy_ui_ComponentVtable pianogrid_vtable;
static bool pianogrid_vtable_initialized = FALSE;

static void pianogrid_vtable_init(Pianogrid* self)
{
	assert(self);

	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			pianogrid_on_draw;
		pianogrid_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_down;
		pianogrid_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_up;
		pianogrid_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			pianogrid_on_mouse_move;
		pianogrid_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			pianogrid_on_preferred_size;
		pianogrid_vtable.on_focus =
			(psy_ui_fp_component)
			pianogrid_on_focus;
		pianogrid_vtable.on_focuslost =
			(psy_ui_fp_component)
			pianogrid_on_focus_lost;
		pianogrid_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(pianogrid_base(self), &pianogrid_vtable);
}

/* implementation */
void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, PianoGridState* state,
	InputHandler* input_handler, psy_DiskOp* disk_op)
{
	assert(self);
	assert(state);	

	psy_ui_component_init(pianogrid_base(self), parent, NULL);	
	pianogrid_vtable_init(self);	
	self->state = state;
	assert(self->state->pv->player);
	patterncmds_init(&self->cmds, state->pv, disk_op);
	self->keyboardstate = keyboardstate;	
	self->prevent_context_menu = FALSE;
	self->hoverpatternentry = NULL;		
	self->preventscrollleft = FALSE;	
	psy_ui_component_set_tab_index(&self->component, 0);	
	psy_ui_component_set_wheel_scroll(pianogrid_base(self), 4);	
	pianogrid_connect_input_handler(self, input_handler);
	psy_audio_sequencecursor_init(&self->old_cursor);
	psy_ui_component_set_overflow(pianogrid_base(self), psy_ui_OVERFLOW_SCROLL);
	psy_property_connect(&self->state->track_display, self,
		pianogrid_on_display);
}

void pianogrid_connect_input_handler(Pianogrid* self,
	InputHandler* input_handler)
{
	assert(self);
	
	inputhandler_connect(input_handler,
		INPUTHANDLER_FOCUS, psy_EVENTDRIVER_CMD, "edit", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)pianogrid_on_edt_cmds);
	inputhandler_connect(input_handler,
		INPUTHANDLER_FOCUS, psy_EVENTDRIVER_CMD, "pianoroll", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)pianogrid_on_roll_cmds);
	inputhandler_connect(input_handler,
		INPUTHANDLER_FOCUS, psy_EVENTDRIVER_CMD, "notes", psy_INDEX_INVALID,
		self, (fp_inputhandler_input)pianogrid_on_note_cmds);
}

void pianogrid_on_draw(Pianogrid* self, psy_ui_Graphics* g)
{
	assert(self);

	if (patternviewstate_sequence(self->state->pv)) {
		PianoGridDraw grid_draw;	
		
		pianogriddraw_init(&grid_draw, self->keyboardstate, self->state,		
			self->hoverpatternentry, psy_ui_component_scroll_size_px(
			pianogrid_base(self)));
		pianogriddraw_on_draw(&grid_draw, g);
	}
}

void pianogrid_on_preferred_size(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	assert(self);
	
	rv->height = psy_ui_value_make_px((self->keyboardstate->keymax -
		self->keyboardstate->keymin) * self->keyboardstate->key_extent_px);	
	rv->width = psy_ui_value_make_px(pianogridstate_beat_to_px(self->state,
		patternviewstate_length(self->state->pv)));		
}

void pianogrid_on_mouse_down(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_Sequence* sequence;

	assert(self);
	
	if (!self->state->pv->undo_redo) {
		return;
	}
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return;
	}
	self->dragcursor = pianogrid_make_cursor(self, psy_ui_mouseevent_offset(
		ev));		
	psy_audio_blockselection_init_all(&self->state->pv->selection.block,
		self->dragcursor, self->dragcursor);
	psy_audio_blockselection_disable(&self->state->pv->selection.block);
	if (psy_ui_mouseevent_button(ev) != 2) {
		patternselection_disable(&self->state->pv->selection);			
		patternselection_set(&self->state->pv->selection,
			self->dragcursor, self->dragcursor);		
	}
	if (psy_ui_mouseevent_button(ev) == 2) {
		/* right button */
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_Pattern* pattern;
				
		pattern = psy_audio_sequence_pattern(self->state->pv->sequence,
			psy_audio_sequencecursor_order_index(&self->dragcursor));
		node = psy_audio_pattern_find_node(
			pattern,
			self->state->pv->cursor.channel_,
			self->dragcursor.offset,
			pianogridstate_step(self->state), &prev);
		if (!node) {
			if (prev) {
				psy_audio_PatternEntry* preventry;

				preventry = psy_audio_patternnode_entry(prev);
				if (psy_audio_patternentry_track(preventry) != self->dragcursor.channel_) {
					prev = psy_audio_patternnode_prev_track(prev,
						self->dragcursor.channel_);
				}
			}
			node = prev;
		}
		if (node) {
			psy_audio_PatternNode* next;
			psy_audio_PatternEntry* patternentry;

			patternentry = psy_audio_patternnode_entry(node);
			if (!pianogrid_key_hit_test(self, node,
					self->dragcursor.channel_, self->dragcursor.key)) {
				pianogrid_invalidate_cursor(self);
				return;
			}
			if (psy_dsp_beatpos_equal(
					psy_audio_patternentry_offset(patternentry),
					self->dragcursor.offset)) {
				next = psy_audio_patternnode_next_track(node,
					self->dragcursor.channel_);
				if (self->hoverpatternentry ==
						psy_audio_patternnode_entry(node)) {
					self->hoverpatternentry = NULL;
				}
				self->preventscrollleft = TRUE;
				psy_undoredo_execute(self->state->pv->undo_redo,
					&removecommand_allocinit(pattern, self->dragcursor,
					sequence)->command);
				self->prevent_context_menu = TRUE;
				self->preventscrollleft = FALSE;
				if (next) {
					psy_audio_PatternEntry* nextentry;

					nextentry = psy_audio_patternnode_entry(next);
					if (psy_audio_patternentry_front(nextentry)->note
						== psy_audio_NOTECOMMANDS_RELEASE) {
						if (self->hoverpatternentry ==
								psy_audio_patternnode_entry(next)) {
							self->hoverpatternentry = NULL;
						}
						self->dragcursor.offset = 
							psy_audio_patternentry_offset(nextentry);
						self->preventscrollleft = TRUE;
						psy_undoredo_execute(self->state->pv->undo_redo,
							&removecommand_allocinit(pattern, self->dragcursor,
								sequence)->command);
						self->preventscrollleft = FALSE;
					}
				}
				pianogrid_set_cursor(self, self->dragcursor);
				psy_ui_component_invalidate(&self->component);
			} else {
				if (psy_audio_patternentry_front(patternentry)->note ==
						psy_audio_NOTECOMMANDS_RELEASE) {
					pianogrid_set_cursor(self, self->dragcursor);					
					return;
				} else {
					psy_audio_PatternEvent release;

					next = psy_audio_patternnode_next_track(node,
						self->dragcursor.channel_);
					if (next) {
						psy_audio_PatternEntry* nextentry;

						nextentry = psy_audio_patternnode_entry(next);
						if (psy_audio_patternentry_front(nextentry)->note ==
								psy_audio_NOTECOMMANDS_RELEASE) {
							psy_audio_SequenceCursor cursor;

							cursor = self->dragcursor;
							cursor.offset = psy_audio_patternentry_offset(
								nextentry);
							self->preventscrollleft = TRUE;
							psy_undoredo_execute(self->state->pv->undo_redo,
								&removecommand_allocinit(pattern, cursor,
									sequence)->command);
							self->preventscrollleft = FALSE;
						}
					}
					psy_audio_patternevent_clear(&release);
					release.note = psy_audio_NOTECOMMANDS_RELEASE;
					self->preventscrollleft = TRUE;
					psy_undoredo_execute(self->state->pv->undo_redo,
						&insertcommand_allocinit(patternviewstate_pattern(
							self->state->pv),
							self->dragcursor, release,
							psy_dsp_beatpos_zero(),
							sequence)->command);
					self->preventscrollleft = FALSE;
					self->prevent_context_menu = TRUE;
					self->dragcursor.key =
						psy_audio_patternentry_front(patternentry)->note;
					pianogrid_set_cursor(self, self->dragcursor);					
				}
			}
			patternselection_start_drag(&self->state->pv->selection,
				self->dragcursor);
			psy_ui_mouseevent_stop_propagation(ev);						
		}
	} else if (!psy_ui_mouseevent_ctrl_key(ev)  &&	
			(psy_ui_mouseevent_button(ev) == 1)) {
		if (!self->state->select_mode) {
			psy_audio_PatternEvent patternevent;
			psy_audio_Pattern* pattern;			

			pattern = psy_audio_sequence_pattern(self->state->pv->sequence,
				psy_audio_sequencecursor_order_index(&self->dragcursor));
			if (pattern) {
				patternevent = psy_audio_player_pattern_event(
					self->state->pv->player, self->dragcursor.key);
				patternevent.note = self->dragcursor.key;
				self->preventscrollleft = TRUE;			
				psy_undoredo_execute(self->state->pv->undo_redo,
					&insertcommand_allocinit(
						pattern,
						self->dragcursor, patternevent,
						self->state->pv->insert_duration,
						sequence)->command);
				self->play_event = patternevent;
				psy_audio_player_play_event(self->state->pv->player,
					&self->play_event, 0);
				self->preventscrollleft = FALSE;			
				pianogrid_set_cursor(self, self->dragcursor);
			}
		}
		psy_ui_component_invalidate(&self->component);
	}	
}

void pianogrid_on_mouse_move(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	psy_audio_SequenceCursor cursor;

	assert(self);
		
	if (patternviewstate_sequence(self->state->pv)) {
		psy_audio_PatternEntry* oldhover;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;
		psy_audio_Pattern* pattern;

		cursor = pianogrid_make_cursor(self, psy_ui_mouseevent_offset(ev));
		oldhover = self->hoverpatternentry;
		pattern = psy_audio_sequence_pattern(self->state->pv->sequence,
			psy_audio_sequencecursor_order_index(&self->dragcursor));
		if (!pattern) {
			return;
		}
		node = psy_audio_pattern_find_node(pattern,
			self->state->pv->cursor.channel_,				
				psy_dsp_beatpos_add(
					pianogridstate_px_to_beat(self->state,
						psy_ui_mouseevent_pt(ev).x -
						psy_ui_component_scroll_left_px(&self->component)),					
					((patternviewstate_single_mode(self->state->pv))
					? psy_audio_sequencecursor_seqoffset(
						&self->state->pv->cursor,
						patternviewstate_sequence(self->state->pv))
					: psy_dsp_beatpos_zero())),
				pianogridstate_step(self->state), &prev);
		if (!node) {
			if (prev) {
				psy_audio_PatternEntry* preventry;

				preventry = psy_audio_patternnode_entry(prev);
				if (psy_audio_patternentry_track(preventry) != self->state->pv->cursor.channel_) {
					prev = psy_audio_patternnode_prev_track(prev,
						self->state->pv->cursor.channel_);
				}
				if (psy_audio_patternentry_front(preventry)->note ==
						psy_audio_NOTECOMMANDS_RELEASE) {
					prev = NULL;
				}
			}
			node = prev;
		}
		if (node && pianogrid_key_hit_test(self, node,
				self->state->pv->cursor.channel_, keyboardstate_screen_to_key(
				self->keyboardstate, psy_ui_mouseevent_pt(ev), 0.0))) {
			self->hoverpatternentry = psy_audio_patternnode_entry(node);
		} else {
			self->hoverpatternentry = NULL;
		}
		if (self->hoverpatternentry != oldhover) {
			psy_ui_component_invalidate(&self->component);
		}
		if (((psy_ui_mouseevent_button(ev) == 1) && (
				self->state->select_mode || psy_ui_mouseevent_ctrl_key(ev)))) {
			cursor = pianogrid_make_cursor(self, psy_ui_mouseevent_offset(ev));				
			if (!psy_audio_sequencecursor_equal(&cursor,
					&self->last_drag_cursor)) {
				if (!psy_audio_blockselection_valid(
						&self->state->pv->selection.block)) {
					patternselection_start_drag(&self->state->pv->selection,
						cursor);					
				} else {
					pianogrid_drag_selection(self, cursor);					
				}
				psy_ui_component_invalidate(&self->component);
				self->last_drag_cursor = cursor;
			}
		}
	}
}

void pianogrid_drag_selection(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	assert(self);
	
	patternselection_drag(&self->state->pv->selection, cursor,
		self->state->pv->sequence);
	if (psy_dsp_beatpos_less(psy_audio_sequencecursor_offset(&cursor),
			psy_audio_sequencecursor_offset(&self->last_drag_cursor))) {
//		pianogrid_scroll_left(self, cursor);
	} else {
//		pianogrid_scroll_right(self, cursor);
	}
	if (psy_audio_sequencecursor_key(&cursor) <
			psy_audio_sequencecursor_key(&self->last_drag_cursor)) {
		pianogrid_scroll_down(self, cursor);
	} else {
		pianogrid_scroll_up(self, cursor);
	}	
}

void pianogrid_on_mouse_up(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);
		
	if (self->prevent_context_menu) {				
		self->prevent_context_menu = FALSE;			
	} else if (!self->state->pv->selection.block.valid_) {
		if (self->state->select_mode || ((psy_ui_mouseevent_button(ev) == 1 &&
				psy_ui_mouseevent_ctrl_key(ev)))) {
			pianogrid_set_cursor(self, self->dragcursor);			
		}
	}
	self->play_event.note = psy_audio_NOTECOMMANDS_RELEASE;
	psy_audio_player_play_event(self->state->pv->player, &self->play_event, 0);
	patternselection_stop_drag(&self->state->pv->selection);
	psy_ui_mouseevent_stop_propagation(ev);
}

void pianogrid_on_display(Pianogrid* self, psy_Property* sender)
{
	assert(self);
	
	psy_ui_component_invalidate(&self->component);
}

psy_audio_SequenceCursor pianogrid_make_cursor(Pianogrid* self,
	psy_ui_RealPoint pt)
{
	psy_audio_SequenceCursor rv;
	psy_audio_Sequence* sequence;
	psy_dsp_beatpos_t offset;	
	
	assert(self);
	
	rv = *patternviewstate_cursor(self->state->pv);
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return rv;
	}	
	offset = pianogridstate_px_to_beat(self->state, pt.x);
	if (!patternviewstate_single_mode(self->state->pv)) {
		psy_audio_OrderIndex order_index;
		psy_audio_SequenceEntry* seq_entry;
				
		order_index = psy_audio_orderindex_make(rv.order_index.track,
			psy_audio_sequence_order(sequence, rv.order_index.track, offset));		
		seq_entry = psy_audio_sequence_entry(sequence, order_index);
		if (seq_entry) {
			offset = psy_dsp_beatpos_sub(offset,
				psy_audio_sequenceentry_offset(seq_entry));			
		} else {
			return rv;
		}
		psy_audio_sequencecursor_set_order_index(&rv, order_index);
	}	
	psy_audio_sequencecursor_set_offset(&rv, offset);	
	rv.key = keyboardstate_screen_to_key(self->keyboardstate,
		psy_ui_realpoint_make(0, pt.y), 0.0);	
	return rv;
}

bool pianogrid_key_hit_test(Pianogrid* self, psy_audio_PatternNode* node,
	uintptr_t track, uint8_t cursorkey)
{
	psy_audio_PatternEntry* patternentry;
	bool noteoff;

	assert(self);

	noteoff = FALSE;
	patternentry = psy_audio_patternnode_entry(node);
	if (psy_audio_patternentry_front(patternentry)->note ==
			psy_audio_NOTECOMMANDS_RELEASE) {
		psy_audio_PatternNode* prevtrack;

		/*
		** determine noterelease key
		** noterelease event has no key but is drawn next to
		** the prev note or if there is none with middlec as key
		*/
		prevtrack = psy_audio_patternnode_prev_track(node, track);
		if (prevtrack) {
			/* compare cursor key with the previous note key */
			if (cursorkey == psy_audio_patternentry_front(
				psy_audio_patternnode_entry(prevtrack))->note) {
				noteoff = TRUE;
			}
		} else if (cursorkey == psy_audio_NOTECOMMANDS_MIDDLEC) {
			/* compare cursor key with middlec key */
			noteoff = TRUE;
		}
	}
	return !(psy_audio_patternentry_front(patternentry)->note !=
		cursorkey && !noteoff);
}

void pianogrid_enter(Pianogrid* self)
{
	psy_audio_PatternEvent patternevent;
	psy_audio_Sequence* sequence;

	assert(self);

	if (!self->state->pv->undo_redo) {
		return;
	}
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return;
	}
	psy_audio_patternevent_clear(&patternevent);
	patternevent.note = self->state->pv->cursor.key;
	psy_undoredo_execute(self->state->pv->undo_redo,
		&insertcommand_allocinit(patternviewstate_pattern(self->state->pv),
			self->state->pv->cursor, patternevent,
			self->state->pv->insert_duration,
			sequence)->command);
	pianogrid_advance_line(self);
}

void pianogrid_row_clear(Pianogrid* self)
{
	psy_audio_Pattern* pattern;
	psy_audio_Sequence* sequence;

	assert(self);

	if (!self->state->pv->undo_redo) {
		return;
	}
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return;
	}	
	pattern = patternviewstate_pattern(self->state->pv);
	if (pattern) {		
		psy_undoredo_execute(self->state->pv->undo_redo,
			&removecommand_allocinit(pattern, self->state->pv->cursor,
			sequence)->command);
		pianogrid_advance_line(self);
	}
}

void pianogrid_prev_line(Pianogrid* self)
{
	assert(self);

	pianogrid_prev_lines(self, patternviewstate_cursor_step(
		self->state->pv), TRUE);
}

void pianogrid_prev_lines(Pianogrid* self, uintptr_t lines, bool wrap)
{	
	assert(self);

	if (patternviewstate_sequence(self->state->pv)) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;

		patternlinenavigator_init(&navigator, self->state->pv, wrap);
		cursor = patternlinenavigator_up(&navigator, lines,
			patternviewstate_cursor(self->state->pv));
		if (!patternlinennavigator_wrap(&navigator)) {
			pianogrid_scroll_left(self, cursor);
		} else {
			pianogrid_scroll_right(self, cursor);
		}
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_prev_keys(Pianogrid* self, uint8_t lines, bool wrap)
{
	assert(self);

	if (patternviewstate_sequence(self->state->pv)) {
		PatternKeyNavigator navigator;	
		psy_audio_SequenceCursor cursor;
	
		patternkeynavigator_init(&navigator, self->keyboardstate, FALSE);	
		cursor = patternkeynavigator_up(&navigator, lines,
			patternviewstate_cursor(self->state->pv));
		if (patternkeynavigator_wrap(&navigator)) {	
			pianogrid_scroll_up(self, cursor);
		} else {
			pianogrid_scroll_down(self, cursor);
		}
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_advance_line(Pianogrid* self)
{
	assert(self);

	pianogrid_advance_lines(self, patternviewstate_cursor_step(
		self->state->pv), TRUE);
}

void pianogrid_advance_lines(Pianogrid* self, uintptr_t lines, bool wrap)
{	
	assert(self);	

	if (patternviewstate_sequence(self->state->pv)) {
		PatternLineNavigator navigator;
		psy_audio_SequenceCursor cursor;
	
		patternlinenavigator_init(&navigator, self->state->pv, wrap);
		cursor = patternlinenavigator_down(&navigator, lines,
			patternviewstate_cursor(self->state->pv));
		if (patternlinennavigator_wrap(&navigator)) {
			pianogrid_scroll_left(self, cursor);
		} else {
			pianogrid_scroll_right(self, cursor);
		}
		pianogrid_set_cursor(self, cursor);
	}
}

void pianogrid_advance_keys(Pianogrid* self, uint8_t lines, bool wrap)
{	
	assert(self);

	if (patternviewstate_sequence(self->state->pv)) {
		PatternKeyNavigator navigator;	
		psy_audio_SequenceCursor cursor;
	
		patternkeynavigator_init(&navigator, self->keyboardstate, FALSE);	
		cursor = patternkeynavigator_down(&navigator, lines,
			patternviewstate_cursor(self->state->pv));
		if (patternkeynavigator_wrap(&navigator)) {	
			pianogrid_scroll_down(self, cursor);
		} else {
			pianogrid_scroll_up(self, cursor);
		}
		pianogrid_set_cursor(self, cursor);
	}
}

bool pianogrid_scroll_left(Pianogrid* self, psy_audio_SequenceCursor cursor)
{		
	assert(self);
	
	if (psy_dsp_beatpos_greater(pianogridstate_px_to_beat(self->state,
		psy_ui_component_scroll_left_px(&self->component)),
			patternviewstate_draw_offset(self->state->pv, cursor.offset))) {
		psy_ui_component_set_scroll_left(&self->component, psy_ui_value_make_px(
			pianogridstate_beat_to_px(self->state, 
				patternviewstate_draw_offset(self->state->pv, cursor.offset))));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scroll_right(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;
	intptr_t visilines;
	psy_ui_RealSize clientsize;

	assert(self);

	clientsize = psy_ui_component_clientsize_px(&self->component);
	visilines = (intptr_t)((clientsize.width /
		(double)self->state->beat_convert.line_px));	
	--visilines;	
	line = pianogridstate_beat_to_line(self->state,
		patternviewstate_draw_offset(self->state->pv, cursor.offset));
	if (visilines < line - psy_ui_component_scroll_left_px(&self->component) /
		pianogridstate_steppx(self->state) + 2) {
		intptr_t dlines;
		
		dlines = (intptr_t)((line - psy_ui_component_scroll_left_px(
			&self->component) /
			pianogridstate_steppx(self->state) - visilines + 16));		
		psy_ui_component_set_scroll_left(&self->component,		
			psy_ui_value_make_px(
				psy_max(0.0, psy_ui_component_scroll_left_px(&self->component) +
					psy_ui_component_scroll_step_width_px(&self->component) *
					dlines)));		
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scroll_up(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	intptr_t line;	
	intptr_t dlines;	
	double linepx;

	assert(self);
	
	line = self->keyboardstate->keymax - cursor.key - 1;
	linepx = self->keyboardstate->key_extent_px * line;	
	dlines = (intptr_t)((psy_ui_component_scroll_top_px(&self->component) -
		linepx) / (self->keyboardstate->key_extent_px));
	if (dlines > 0) {		
		psy_ui_component_set_scroll_top(&self->component,
			psy_ui_value_make_px(
			psy_max(0, psy_ui_component_scroll_top_px(&self->component) -
				psy_ui_component_scroll_step_height_px(&self->component) *
				dlines)));
		return FALSE;
	}
	return TRUE;
}

bool pianogrid_scroll_down(Pianogrid* self, psy_audio_SequenceCursor cursor)
{	
	psy_ui_RealSize clientsize;	
	intptr_t topline;
	intptr_t line;
	intptr_t visilines;	
	intptr_t dlines;

	assert(self);

	clientsize = psy_ui_component_clientsize_px(&self->component);		
	visilines = (intptr_t)floor(clientsize.height /
		self->keyboardstate->key_extent_px);
	topline = (intptr_t)ceil(psy_ui_component_scroll_top_px(&self->component) /
		self->keyboardstate->key_extent_px);
	line = self->keyboardstate->keymax - cursor.key;
	dlines = (intptr_t)(line - topline - visilines);
	if (dlines > 0) {		
		psy_ui_component_set_scroll_top(&self->component,
			psy_ui_value_make_px(
				psy_ui_component_scroll_top_px(&self->component) +
				psy_ui_component_scroll_step_height_px(&self->component) *
				dlines));
		return FALSE;
	}
	return TRUE;
}

void pianogrid_invalidate_playbar(Pianogrid* self)
{		
	psy_audio_Sequence* sequence;
	psy_audio_HostSequencerTime* seqtime;
	psy_dsp_beatpos_t last_cursor;
	psy_dsp_beatpos_t curr_cursor;
	double last;
	double curr;
	double minval;
	double maxval;
	psy_ui_RealSize size;	

	assert(self);
	
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return;
	}
	seqtime = &self->state->pv->player->sequencer.hostseqtime;
	if (patternviewstate_single_mode(self->state->pv) && 
		!psy_audio_orderindex_equal(
			&seqtime->lastplaycursor.order_index,
			self->state->pv->cursor.order_index) &&
		!psy_audio_orderindex_equal(
			&seqtime->currplaycursor.order_index,
			self->state->pv->cursor.order_index)) {
		return;					
	}	
	last_cursor = seqtime->lastplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		last_cursor = psy_dsp_beatpos_add(last_cursor,
			psy_audio_sequencecursor_seqoffset(&seqtime->lastplaycursor,
			sequence));
	}
	last = pianogridstate_beat_to_px(self->state, last_cursor);
	curr_cursor = seqtime->currplaycursor.offset;
	if (!patternviewstate_single_mode(self->state->pv)) {
		curr_cursor = psy_dsp_beatpos_add(curr_cursor,
			psy_audio_sequencecursor_seqoffset(&seqtime->currplaycursor,
			sequence));
	}
	curr = pianogridstate_beat_to_px(self->state, curr_cursor);		
	minval = psy_min(last, curr);
	maxval = psy_max(last, curr);
	size = psy_ui_component_scroll_size_px(&self->component);
	psy_ui_component_invalidate_rect(&self->component, 
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(			
				minval,
				psy_ui_component_scroll_top_px(&self->component)),
			psy_ui_realsize_make(
				maxval - minval + pianogridstate_steppx(self->state),
				size.height)));
}

void pianogrid_invalidate_line(Pianogrid* self, intptr_t line)
{	
	double scrolltoppx;
	intptr_t seqline;
	psy_ui_RealRectangle r1;
	psy_ui_RealSize size;
	psy_ui_RealSize stepsize;

	assert(self);

	scrolltoppx = psy_ui_component_scroll_top_px(pianogrid_base(self));
	size = psy_ui_component_clientsize_px(&self->component);
	stepsize = psy_ui_realsize_make(pianogridstate_steppx(self->state),
		size.height);
	seqline = 0;
	if (patternviewstate_single_mode(self->state->pv)) {
		seqline = (intptr_t)(
			psy_dsp_beatpos_real(
				psy_audio_sequencecursor_seqoffset(&self->state->pv->cursor,
				patternviewstate_sequence(self->state->pv))) *
			self->state->pv->cursor.lpb);
	}
	r1 = psy_ui_realrectangle_make(psy_ui_realpoint_make(
		(line - seqline) * stepsize.width, scrolltoppx), stepsize);
	psy_ui_component_invalidate_rect(pianogrid_base(self), r1);	
}

void pianogrid_invalidate_cursor(Pianogrid* self)
{
	psy_audio_Sequence* sequence;
	
	assert(self);
	
	if (!psy_ui_component_draw_visible(pianogrid_base(self))) {
		return;
	}
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return;
	}	
	pianogrid_invalidate_line(self, psy_audio_sequencecursor_line_abs(
		&self->old_cursor, sequence));
	pianogrid_invalidate_line(self, psy_audio_sequencecursor_line_abs(
		&self->state->pv->cursor, sequence));
	self->old_cursor = self->state->pv->cursor;
}

void pianogrid_set_cursor(Pianogrid* self, psy_audio_SequenceCursor cursor)
{
	assert(self);
	
	if (patternviewstate_sequence(self->state->pv)) {
		bool restore;

		restore = self->preventscrollleft;
		self->preventscrollleft = TRUE;
		psy_audio_sequence_set_cursor(patternviewstate_sequence(
			self->state->pv), cursor);
		self->preventscrollleft = restore;
	}
}

void pianogrid_on_focus(Pianogrid* self)
{
	assert(self);
	
	psy_ui_component_add_style_state(
		psy_ui_component_parent(psy_ui_component_parent(
			psy_ui_component_parent(&self->component))),
		psy_ui_STYLESTATE_SELECT);
}

void pianogrid_on_focus_lost(Pianogrid* self)
{
	assert(self);
	
	psy_ui_component_remove_style_state(
		psy_ui_component_parent(psy_ui_component_parent(
			psy_ui_component_parent(&self->component))),
		psy_ui_STYLESTATE_SELECT);
}

bool pianogrid_on_note_cmds(Pianogrid* self, InputHandler* sender)
{
	psy_audio_Sequence* sequence;

	assert(self);
	
	sequence = patternviewstate_sequence(self->state->pv);
	if (!sequence) {
		return 0;
	}
	if (psy_audio_player_edit_mode(self->state->pv->player)) {
		psy_EventDriverCmd cmd;

		assert(self);

		cmd = inputhandler_cmd(sender);
		if (cmd.id != -1) {
			psy_audio_Pattern* pattern;
			bool chord;
			psy_audio_PatternEvent ev;

			pattern = patternviewstate_pattern(self->state->pv);
			if (!pattern) {
				return TRUE;
			}
			chord = FALSE;
			if (cmd.id >= CMD_NOTE_OFF_C_0 && cmd.id < 255) {
				ev = psy_audio_player_pattern_event(self->state->pv->player,
					(uint8_t)cmd.id);
				ev.note = psy_audio_NOTECOMMANDS_RELEASE;
				psy_audio_player_play_event(self->state->pv->player, &ev,
					self->state->pv->cursor.channel_);
				return 1;
			}
			else if (cmd.id == CMD_NOTE_CHORD_END) {
				self->state->pv->cursor.channel_ =
					self->state->pv->chord_begin;
				return 1;
			}
			else if (cmd.id >= CMD_NOTE_CHORD_C_0 && cmd.id < CMD_NOTE_STOP) {
				chord = TRUE;
				ev = psy_audio_player_pattern_event(self->state->pv->player,
					(uint8_t)cmd.id - (uint8_t)CMD_NOTE_CHORD_C_0);
			}
			else if (cmd.id < 256) {
				chord = FALSE;
				ev = psy_audio_player_pattern_event(self->state->pv->player,
					(uint8_t)cmd.id);
			}
			else {
				return 1;
			}
			psy_undoredo_execute(self->state->pv->undo_redo,
				&insertcommand_allocinit(pattern,
					self->state->pv->cursor, ev,
					self->state->pv->insert_duration,
					sequence)->command);
			if (chord != FALSE) {
				++self->state->pv->cursor.channel_;
			}
			else {
				self->state->pv->cursor.channel_ =
					self->state->pv->chord_begin;
			}
			if (ev.note < psy_audio_NOTECOMMANDS_RELEASE) {
				self->state->pv->cursor.key = ev.note;
			}
			pianogrid_set_cursor(self,
				*patternviewstate_cursor(self->state->pv));
			return 1;
		}
	}
	return 0;
}

bool pianogrid_on_edt_cmds(Pianogrid* self, InputHandler* sender)
{		
	assert(self);
	
	psy_EventDriverCmd cmd;
		
	cmd = inputhandler_cmd(sender);		
	if (cmd.id == CMD_ROWCLEAR) {
		pianogrid_row_clear(self);
		return TRUE;
	}	
	if (patterncmds_handle_edt_command(&self->cmds, cmd)) {
	if (cmd.id == CMD_ROWDELETE) {
			if (patternviewstate_ft2delete(self->state->pv)) {
				pianogrid_prev_line(self);
			}
		} else {			
			psy_ui_component_invalidate(&self->component);
		}
		return TRUE;
	}	
	return FALSE;
}

bool pianogrid_on_roll_cmds(Pianogrid* self, InputHandler* sender)
{	
	psy_EventDriverCmd cmd;
		
	assert(self);

	cmd = inputhandler_cmd(sender);		
	return pianogrid_handle_command(self, cmd.id);
}

bool pianogrid_handle_command(Pianogrid* self, uintptr_t cmd)
{
	bool handled;

	assert(self);

	handled = TRUE;
	switch (cmd) {
	case CMD_NAVUP:
		pianogrid_advance_keys(self, 1, FALSE);
		break;
	case CMD_NAVPAGEUP:
		pianogrid_prev_lines(self, patternviewstate_curr_pgup_down_step(
			self->state->pv), FALSE);
		break;
	case CMD_NAVPAGEUPKEYBOARD:
		pianogrid_advance_keys(self, 12, 0);
		break;
	case CMD_NAVDOWN:
		pianogrid_prev_keys(self, 1, FALSE);
		break;
	case CMD_NAVPAGEDOWN:
		pianogrid_advance_lines(self,
			patternviewstate_curr_pgup_down_step(self->state->pv), 0);
		break;
	case CMD_NAVPAGEDOWNKEYBOARD:
		pianogrid_prev_keys(self, 12, 0);
		break;
	case CMD_NAVLEFT:
		pianogrid_prev_line(self);
		break;
	case CMD_NAVRIGHT:
		pianogrid_advance_line(self);
		break;	
	case CMD_ENTER: {
		pianogrid_enter(self);
		break; }
	default:
		handled = FALSE;
		break;
	}
	return handled;
}

#endif /* PSYCLE_USE_PIANOROLL */
