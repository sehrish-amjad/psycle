/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VUSCOPE_H)
#define VUSCOPE_H

/* host */
#include "workspace.h"
/* ui */
#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** VuScope
**
** Display a blue left and green right volumebar in decibels.
*/

/* VuScope */
typedef struct VuScope {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_audio_Wire wire;
	double leftavg;
	double rightavg;
	double invol;
	double mult;
	int scope_peak_rate;
	int hold;
	int running;	
	double peakL, peakR;
	int peakLifeL, peakLifeR;
	Workspace* workspace;	
} VuScope;

void vuscope_init(VuScope*, psy_ui_Component* parent, psy_audio_Wire wire, Workspace*);
void vuscope_start(VuScope*);
void vuscope_stop(VuScope*);

void vuscope_idle(VuScope*);

INLINE psy_ui_Component* vuscope_base(VuScope* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* VUSCOPE_H */
