/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQEDITTOOLBAR_H)
#define SEQEDITTOOLBAR_H

#include "../../detail/psyconf.h"

#ifdef PSYCLE_USE_SEQEDITOR

/* host */
#include "seqeditorentry.h"
#include "seqeditheader.h"
#include "zoombox.h"
/* ui */
#include <uicombobox.h>
#include <uicheckbox.h>
#include <uinumberedit.h>
#include <uitext.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct SeqEditToolBar
** @brief Toolbar of the sequence editor
*/
typedef struct SeqEditToolBar {
	/*! @extends */
	psy_ui_Component component;		
	psy_ui_Button move;
	psy_ui_Button reorder;
	psy_ui_Label desctype;
	psy_ui_ComboBox inserttype;
	psy_ui_Button assignsample;
	psy_ui_CheckBox usesamplerindex;	
	psy_ui_NumberEdit samplerindex;
	psy_ui_Button configure;
	psy_ui_Component alignbar;
	psy_ui_Button expand;
	psy_ui_Button view_float;
	psy_ui_Button timesig;
	psy_ui_Button loop;
	psy_ui_Label trackname;	
	psy_ui_Text trackedit;	
	/* references */	
	SeqEditState* state;
} SeqEditToolBar;

void seqedittoolbar_init(SeqEditToolBar*, psy_ui_Component* parent,
	SeqEditState*);

void seqeditortoolbar_setdragtype(SeqEditToolBar*, SeqEditorDragType);

void seqeditortoolbar_update_track_name(SeqEditToolBar*);

INLINE psy_ui_Component* seqedittoolbar_base(SeqEditToolBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PSYCLE_USE_SEQEDITOR */

#endif /* SEQEDITTOOLBAR_H */
