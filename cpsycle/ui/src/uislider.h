/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_SLIDER_H
#define psy_ui_SLIDER_H

/* local */
#include "uicomponent.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Slider;

/* todo combine with scrollbarpane */
/* psy_ui_SliderPane */
typedef struct psy_ui_SliderPane {
	/*! @extends  */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_clicked;
	psy_Signal signal_changed;
	psy_Signal signal_describevalue;
	psy_Signal signal_tweakvalue;
	psy_Signal signal_value;
	psy_Signal signal_customdraw;
	/*! @internal */
	double value;
	double defaultvalue;
	double rulerstep;
	double wheelstep;
	double tweakbase;	
	double step;
	psy_ui_Size hslidersize;
	psy_ui_Size vslidersize;
	psy_ui_RealSize slidersizepx;
	bool poll;
	bool hover;
	uintptr_t timerinterval;
	psy_ui_Orientation orientation;
	/* references */
	struct psy_ui_Slider* slider;	
} psy_ui_SliderPane;

typedef void (*ui_slider_fpdescribe)(void*, struct psy_ui_Slider*, char* text);
typedef void (*ui_slider_fptweak)(void*, struct psy_ui_Slider*, double value);
typedef void (*ui_slider_fpvalue)(void*, struct psy_ui_Slider*, double* value);

void psy_ui_sliderpane_init(psy_ui_SliderPane*, psy_ui_Component* parent);
void psy_ui_sliderpane_set_value(psy_ui_SliderPane*, double value);
void psy_ui_sliderpane_set_default_value(psy_ui_SliderPane*, double value);
double psy_ui_sliderpane_value(psy_ui_SliderPane*);
void psy_ui_sliderpane_show_vertical(psy_ui_SliderPane*);
void psy_ui_sliderpane_show_horizontal(psy_ui_SliderPane*);
psy_ui_Orientation psy_ui_sliderpane_orientation(psy_ui_SliderPane*);
void psy_ui_sliderpane_connect(psy_ui_SliderPane*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);

INLINE psy_ui_Component* psy_ui_sliderpane_base(psy_ui_SliderPane* self)
{
	return &self->component;
}

/* psy_ui_Slider */
typedef struct psy_ui_Slider {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Label desc;
	psy_ui_SliderPane pane;
	psy_ui_Label value;
} psy_ui_Slider;

void psy_ui_slider_init(psy_ui_Slider*, psy_ui_Component* parent);
void psy_ui_slider_init_text(psy_ui_Slider*, psy_ui_Component* parent,
	const char* text);

void psy_ui_slider_connect(psy_ui_Slider*, void* context, ui_slider_fpdescribe,
	ui_slider_fptweak, ui_slider_fpvalue);
void psy_ui_slider_set_text(psy_ui_Slider*, const char* text);
void psy_ui_slider_set_char_number(psy_ui_Slider*, double number);
void psy_ui_slider_set_value_char_number(psy_ui_Slider*, double number);
void psy_ui_slider_show_vertical(psy_ui_Slider*);
void psy_ui_slider_show_horizontal(psy_ui_Slider*);
void psy_ui_slider_show_label(psy_ui_Slider*);
void psy_ui_slider_hide_label(psy_ui_Slider*);
void psy_ui_slider_show_value_label(psy_ui_Slider*);
void psy_ui_slider_hide_value_label(psy_ui_Slider*);
void psy_ui_slider_describe_value(psy_ui_Slider*);
void psy_ui_slider_start_poll(psy_ui_Slider*);
void psy_ui_slider_stop_poll(psy_ui_Slider*);
void psy_ui_slider_update(psy_ui_Slider*);
void psy_ui_slider_set_default_value(psy_ui_Slider*, double value);
void psy_ui_slider_set_ruler_step(psy_ui_Slider*, double value);
void psy_ui_slider_set_wheel_step(psy_ui_Slider*, double value);

INLINE psy_ui_Component* psy_ui_slider_base(psy_ui_Slider* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SLIDER_H */
