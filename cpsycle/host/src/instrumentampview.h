/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(INSTRUMENTAMPVIEW_H)
#define INSTRUMENTAMPVIEW_H

/* host */
#include "adsrsliders.h"
#include "envelopeview.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct InstrumentVolumeView {
	/*! @extends  */
	psy_ui_Component component;	
	/*! @internal */
	psy_ui_Slider randomvolume;
	psy_ui_Slider volume_fade_speed;
	EnvelopeView envelopeview;
	AdsrSliders adsrsliders;
	/* references */	
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentVolumeView;

void instrumentvolumeview_init(InstrumentVolumeView*,
	psy_ui_Component* parent, psy_audio_Instruments*);

void instrumentvolumeview_set_instrument(InstrumentVolumeView*,
	psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTAMPVIEW_H */
