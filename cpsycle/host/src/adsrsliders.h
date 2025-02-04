/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(ADSRSLIDERS_H)
#define ADSRSLIDERS_H

/* host */
#include "envelopeview.h"
/* dsp */
#include <envelope.h>
/* ui */
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct AdsrSliders
** @brief Sliders for attack, decay. sustain and release.
*/
typedef struct AdsrSliders {
	/*! @extends */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_tweaked;
	/*! @internal */
	psy_ui_Slider attack;
	psy_ui_Slider decay;
	psy_ui_Slider sustain;
	psy_ui_Slider release;
	/* references */
	psy_dsp_Envelope* envelope;
} AdsrSliders;

void adsrsliders_init(AdsrSliders*, psy_ui_Component* parent);

void adsrsliders_set_envelope(AdsrSliders*, psy_dsp_Envelope*);
void adsrsliders_update(AdsrSliders*);
psy_dsp_seconds_t adsrsliders_max_time(const AdsrSliders*);

INLINE psy_ui_Component* adsrsliders_base(AdsrSliders* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* ADSRSLIDERS_H */
