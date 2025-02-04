/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "instrumentfilterview.h"
/* ui */
#include <uiapp.h>
/* dsp */
#include <valuemapper.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void instrumentfilterview_update_slider(InstrumentFilterView*);
static void instrumentfilterview_fill_filter_combobox(InstrumentFilterView*);
static void instrumentfilterview_on_describe(InstrumentFilterView*,
	psy_ui_Slider*, char* text);
static void instrumentfilterview_on_tweak(InstrumentFilterView*,
	psy_ui_Slider*, double value);
static void instrumentfilterview_on_value(InstrumentFilterView*,
	psy_ui_Slider*, double* value);
static void instrumentfilterview_on_filter_combobox_changed(
	InstrumentFilterView*, psy_ui_ComboBox* sender, intptr_t index);
static void instrumentfilterview_on_tweaked(InstrumentFilterView*,
	psy_ui_Component*, uintptr_t pointindex);
static void instrumentfilterview_on_envelopeview_tweaked(InstrumentFilterView*,
	psy_ui_Component* sender, uintptr_t pointindex);

/* implementation */
void instrumentfilterview_init(InstrumentFilterView* self,
	psy_ui_Component* parent, psy_audio_Instruments* instruments)
{
	psy_ui_Margin margin;
	int i;
	psy_ui_Slider* sliders[] = {		
		&self->cutoff,
		&self->res,
		&self->modamount,
		&self->randomcutoff,
		& self->randomresonance
	};

	self->instruments = instruments;
	self->instrument = NULL;
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_set_default_align(&self->top, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_component_init(&self->filter, &self->top, NULL);
	margin = psy_ui_defaults_vmargin(psy_ui_defaults());
	margin.top = psy_ui_value_make_eh(1.0);	
	psy_ui_label_init_text(&self->filtertypeheader, &self->filter,
		"instview.filter-type");	
	psy_ui_component_set_align(&self->filtertypeheader.component,
		psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);		
	psy_ui_component_set_margin(&self->filtertypeheader.component, margin);
	psy_ui_combobox_init(&self->filtertype, &self->filter);
	psy_ui_combobox_set_char_number(&self->filtertype, 20);
	psy_ui_component_set_align(&self->filtertype.component, psy_ui_ALIGN_LEFT);
	instrumentfilterview_fill_filter_combobox(self);
	psy_ui_combobox_select(&self->filtertype, (int)F_NONE);
	psy_signal_connect(&self->filtertype.signal_selchanged, self,
		instrumentfilterview_on_filter_combobox_changed);
	psy_ui_slider_init(&self->randomcutoff, &self->top);
	psy_ui_slider_set_text(&self->randomcutoff, "Random Cutoff");
	psy_ui_slider_set_default_value(&self->randomcutoff, 0.0);
	psy_ui_slider_init(&self->randomresonance, &self->top);
	psy_ui_slider_set_default_value(&self->randomresonance, 0.0);
	psy_ui_slider_set_text(&self->randomresonance, "Random Res");
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_TOP,
		psy_ui_margin_zero());
	envelopeview_init(&self->envelopeview, &self->component);
	psy_ui_component_set_align(&self->envelopeview.component,
		psy_ui_ALIGN_CLIENT);
	envelopeview_settext(&self->envelopeview, "instview.filter-envelope");
	psy_ui_component_set_margin(&self->envelopeview.component,
		psy_ui_margin_make_em(1.0, 0.0, 1.0, 0.0));	
	psy_ui_component_init(&self->bottom, &self->component, NULL);
	psy_ui_component_set_align(&self->bottom, psy_ui_ALIGN_BOTTOM);		
	psy_ui_component_set_default_align(&self->bottom, psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	adsrsliders_init(&self->adsrsliders, &self->bottom);	
	psy_ui_slider_set_default_value(&self->adsrsliders.decay, 0.370 /
		adsrsliders_max_time(&self->adsrsliders));
	psy_ui_slider_set_default_value(&self->adsrsliders.sustain, 0.5);
	psy_ui_slider_set_default_value(&self->adsrsliders.release, 0.370 /
		adsrsliders_max_time(&self->adsrsliders));	
	psy_signal_connect(&self->adsrsliders.signal_tweaked, self,
		instrumentfilterview_on_tweaked);
	psy_signal_connect(&self->envelopeview.envelopebox.signal_tweaked, self,
		instrumentfilterview_on_tweaked);
	psy_ui_slider_init(&self->cutoff, &self->bottom);
	psy_ui_slider_set_default_value(&self->cutoff, 1.0);
	psy_ui_slider_set_text(&self->cutoff, "instview.cut-off");
	psy_ui_slider_init(&self->res, &self->bottom);
	psy_ui_slider_set_default_value(&self->res, 0.0);
	psy_ui_slider_set_text(&self->res, "instview.res");	
	psy_ui_slider_init(&self->modamount, &self->bottom);
	psy_ui_slider_set_text(&self->modamount, "instview.mod");
	psy_ui_slider_set_default_value(&self->modamount, 1.0);
	for (i = 0; i < 5; ++i) {				
		psy_ui_slider_set_char_number(sliders[i], 18);
		psy_ui_slider_set_value_char_number(sliders[i], 15);
		psy_ui_slider_connect(sliders[i], self,
			(ui_slider_fpdescribe)instrumentfilterview_on_describe,
			(ui_slider_fptweak)instrumentfilterview_on_tweak,
			(ui_slider_fpvalue)instrumentfilterview_on_value);
	}
	instrumentfilterview_update_slider(self);
	psy_signal_connect(&self->envelopeview.signal_tweaked, self,
		instrumentfilterview_on_envelopeview_tweaked);
}

void instrumentfilterview_update_slider(InstrumentFilterView* self)
{	
	psy_ui_slider_update(&self->randomcutoff);
	psy_ui_slider_update(&self->randomresonance);
	psy_ui_slider_update(&self->cutoff);
	psy_ui_slider_update(&self->res);
	psy_ui_slider_update(&self->modamount);
}

void instrumentfilterview_fill_filter_combobox(InstrumentFilterView* self)
{
	uintptr_t ft;

	for (ft = 0; ft < filter_numfilters(); ++ft) {
		psy_ui_combobox_add_text(&self->filtertype,
			filter_name((psy_dsp_FilterType)ft));
	}
}

void instrumentfilterview_on_filter_combobox_changed(InstrumentFilterView* self,
	psy_ui_ComboBox* sender, intptr_t index)
{
	self->instrument->filtertype = (psy_dsp_FilterType)index;
}

void instrumentfilterview_set_instrument(InstrumentFilterView* self,
	psy_audio_Instrument* instrument)
{	
	self->instrument = instrument;
	if (self->instrument) {
		adsrsliders_set_envelope(&self->adsrsliders,
			&self->instrument->filterenvelope);
		envelopeview_setenvelope(&self->envelopeview,
			&instrument->filterenvelope);		
		envelopeview_setmodamount(&self->envelopeview,
			instrument->filtermodamount);
		psy_ui_combobox_select(&self->filtertype,
			(intptr_t)instrument->filtertype);
	} else {
		adsrsliders_set_envelope(&self->adsrsliders, NULL);
		envelopeview_setenvelope(&self->envelopeview, NULL);
		psy_ui_combobox_select(&self->filtertype, (intptr_t)F_NONE);
	}
	instrumentfilterview_update_slider(self);	
}

void instrumentfilterview_on_describe(InstrumentFilterView* self,
	psy_ui_Slider* slider, char* text)
{
	if (!self->instrument) {		
		if (slider == &self->cutoff) {
			psy_snprintf(text, 10, "0");
		} else if (slider == &self->res) {
			psy_snprintf(text, 10, "50%%");
		} else if (slider == &self->modamount) {
			psy_snprintf(text, 10, "0%");
		} else if (slider == &self->randomcutoff) {
			psy_snprintf(text, 10, "0%");
		} else if (slider == &self->randomresonance) {
			psy_snprintf(text, 10, "off");
		}
		return;
	}
	if (slider == &self->cutoff) {		
		psy_snprintf(text, 20, "%d",
			(int)(self->instrument->filtercutoff * 11665 + 2333));				
	} else if (slider == &self->res) {		
		psy_snprintf(text, 20, "%d%%",
			(int)(self->instrument->filterres * 100));				
	} else if (slider == &self->modamount) {		
		psy_snprintf(text, 20, "%d%%",
			(int)(self->instrument->filtermodamount * 100));		
	} else if (slider == &self->randomcutoff) {
		if (self->instrument->randomcutoff == 0.f) {
			psy_snprintf(text, 10, "off");
		} else {
			psy_snprintf(text, 20, "%d%%",
				(int)(self->instrument->randomcutoff * 100));
		}
	} else if (slider == &self->randomresonance) {
		if (self->instrument->randomresonance == 0.f) {
			psy_snprintf(text, 10, "off");
		} else {
			psy_snprintf(text, 20, "%d%%",
				(int)(self->instrument->randomresonance * 100));
		}
	}
}

void instrumentfilterview_on_tweak(InstrumentFilterView* self,
	psy_ui_Slider* slider, double value)
{
	if (!self->instrument) {
		return;
	}	

	if (slider == &self->cutoff) {
		self->instrument->filtercutoff = value;
	} else if (slider == &self->res) {
		self->instrument->filterres = value;
	} else if (slider == &self->modamount) {
		self->instrument->filtermodamount = (value - 0.5) * 2.0;
	} else if (slider == &self->randomcutoff) {
		self->instrument->randomcutoff = value;
	} else if (slider == &self->randomresonance) {
		self->instrument->randomresonance = value;
	}
	envelopeview_setmodamount(&self->envelopeview,
		self->instrument->filtermodamount);
	envelopeview_update(&self->envelopeview);
}

void instrumentfilterview_on_value(InstrumentFilterView* self,
	psy_ui_Slider* slider, double* value)
{
	if (slider == &self->cutoff) {
		*value = self->instrument
			? self->instrument->filtercutoff
			: 0.5f;
	} else if (slider == &self->res) {
		*value = self->instrument
			? self->instrument->filterres
			: 0.5f;
	} else if (slider == &self->modamount) {
		*value = self->instrument
			? self->instrument->filtermodamount / 2 + 0.5f
			: 0.f;
	} else if (slider == &self->randomcutoff) {
		*value = self->instrument
			? self->instrument->randomcutoff
			: 0.0f;
	} else if (slider == &self->randomresonance) {
		*value = self->instrument
			? self->instrument->randomresonance
			: 0.0f;
	}
}

void instrumentfilterview_on_tweaked(InstrumentFilterView* self,
	psy_ui_Component* sender, uintptr_t pointindex)
{
	if (self->instrument) {
		envelopeview_setmodamount(&self->envelopeview,
			self->instrument->filtermodamount);
		envelopeview_update(&self->envelopeview);				
	}
}

void instrumentfilterview_on_envelopeview_tweaked(InstrumentFilterView* self,
	psy_ui_Component* sender, uintptr_t pointindex)
{
	adsrsliders_update(&self->adsrsliders);
}
