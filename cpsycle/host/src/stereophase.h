// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEREOPHASE_H)
#define STEREOPHASE_H

// host
#include "workspace.h"
// ui
#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

// StereoPhase
//
// Displays the stereo phase of the signal 

typedef struct StereoPhase {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_audio_Wire wire;	
	double invol;
	double mult;
	// float* pSamplesL;
	// float* pSamplesR;
	int scope_peak_rate;
	int scope_phase_rate;
	int hold;	
	// memories for vu-meter
	double peakL, peakR;
	int peakLifeL, peakLifeR;
	//Memories for phase
	double o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;
	psy_ui_Bitmap bg;
	// references
	Workspace* workspace;	
} StereoPhase;

void stereophase_init(StereoPhase*, psy_ui_Component* parent, psy_audio_Wire wire, Workspace*);
void stereophase_stop(StereoPhase*);

void stereophase_idle(StereoPhase*);

INLINE psy_ui_Component* stereophase__base(StereoPhase* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* STEREOPHASE_H */
