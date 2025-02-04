/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(LABELUI_H)
#define LABELUI_H

/* host */
#include "paramview.h"
/* ui */
#include <uicomponent.h>

/* LabelUi
**
** Label to display a MachineParameter (MPF_INFOLABEL)
*/

#ifdef __cplusplus
extern "C" {
#endif

struct ParamSkin;

/* LabelUi */
typedef struct LabelUi {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	uintptr_t paramidx;
	/* references */	
	struct psy_audio_Machine* machine;	
	struct psy_audio_MachineParam* param;
} LabelUi;

void labelui_init(LabelUi*, psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

LabelUi* labelui_alloc(void);
LabelUi* labelui_allocinit(psy_ui_Component* parent,
	struct psy_audio_Machine*, uintptr_t paramidx,
	struct psy_audio_MachineParam*);

INLINE psy_ui_Component* labelui_base(LabelUi* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* LABELUI_H */
