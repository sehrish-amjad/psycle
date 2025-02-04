/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sliderui.h"
/* host */
#include "machineparamconfig.h"
#include "styles.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <machine.h>
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"

/* SliderUi */

/* prototypes */
static void sliderui_ondraw(SliderUi*, psy_ui_Graphics*);
static void sliderui_onpreferredsize(SliderUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sliderui_on_mouse_down(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_on_mouse_up(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_on_mouse_move(SliderUi*, psy_ui_MouseEvent*);
static void sliderui_update_param(SliderUi*);

/* vtable */
static psy_ui_ComponentVtable sliderui_vtable;
static bool sliderui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* sliderui_vtable_init(SliderUi* self)
{
	assert(self);

	if (!sliderui_vtable_initialized) {
		sliderui_vtable = *(self->component.vtable);				
		sliderui_vtable.ondraw = (psy_ui_fp_component_ondraw)sliderui_ondraw;				
		sliderui_vtable.onpreferredsize = (psy_ui_fp_component_on_preferred_size)
			sliderui_onpreferredsize;
		sliderui_vtable.on_mouse_down = (psy_ui_fp_component_on_mouse_event)sliderui_on_mouse_down;
		sliderui_vtable.on_mouse_up = (psy_ui_fp_component_on_mouse_event)sliderui_on_mouse_up;
		sliderui_vtable.on_mouse_move = (psy_ui_fp_component_on_mouse_event)sliderui_on_mouse_move;
		sliderui_vtable_initialized = TRUE;
	}
	return &sliderui_vtable;
}

/* implementation */
void sliderui_init(SliderUi* self, psy_ui_Component* parent,	
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	assert(self);	

	psy_ui_component_init(&self->component, parent, NULL);
	sliderui_vtable_init(self);
	self->component.vtable = &sliderui_vtable;
	psy_ui_component_set_style_type(&self->component, STYLE_MACPARAM_SLIDER);
	self->machine = machine;
	self->paramidx = paramidx;
	self->param = param;
	paramtweak_init(&self->paramtweak);
}

SliderUi* sliderui_alloc(void)
{
	return (SliderUi*)malloc(sizeof(SliderUi));
}

SliderUi* sliderui_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t paramidx,
	psy_audio_MachineParam* param)
{
	SliderUi* rv;

	rv = sliderui_alloc();
	if (rv) {
		sliderui_init(rv, parent, machine, paramidx, param);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void sliderui_ondraw(SliderUi* self, psy_ui_Graphics* g)
{	
	double yoffset;
	double value;		
	psy_ui_Style* slider_style;
	psy_ui_Style* sliderknob_style;
	psy_ui_Background* bg;
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize bg_slider_size;
	psy_ui_RealSize bg_sliderknob_size;
	
	
	slider_style = psy_ui_style(STYLE_MACPARAM_SLIDER);
	sliderknob_style = psy_ui_style(STYLE_MACPARAM_SLIDERKNOB);	
	sliderui_update_param(self);
	if (self->param) {
		if (self->machine) {
			value = psy_audio_machine_parameter_norm_value(self->machine,
				self->param);
		} else {
			value = psy_audio_machineparam_norm_value(self->param);
		}
	} else {
		value = 0.f;
	}
	tm = psy_ui_component_textmetric(&self->component);
	bg_slider_size = psy_ui_size_px(&slider_style->background.size,
		tm, NULL);
	bg_sliderknob_size = psy_ui_size_px(&sliderknob_style->background.size,
		tm, NULL);
	yoffset = ((1.0 - value) *
		(bg_slider_size.height - bg_sliderknob_size.height));	
	bg = &sliderknob_style->background;
	psy_ui_graphics_draw_bitmap(g, &bg->bitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_value_px(&sliderknob_style->padding.left, tm, 0),
				yoffset + psy_ui_value_px(
					&sliderknob_style->padding.top, tm, 0)),
			bg_sliderknob_size),
		psy_ui_realpoint_make(-bg->position.x, -bg->position.y));	
}

void sliderui_onpreferredsize(SliderUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_Style* style;

	style = psy_ui_style(STYLE_MACPARAM_SLIDER);
	*rv = style->background.size;
}

void sliderui_on_mouse_down(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		paramtweak_begin(&self->paramtweak, self->machine, self->paramidx,
			self->param);		
		paramtweak_on_mouse_down(&self->paramtweak, ev);
		psy_ui_component_capture(&self->component);		
	}
}

void sliderui_on_mouse_move(SliderUi* self, psy_ui_MouseEvent* ev)
{
	if ((paramtweak_active(&self->paramtweak))) {		
		paramtweak_on_mouse_move(&self->paramtweak, ev);
		psy_ui_component_invalidate(&self->component);
	}
}

void sliderui_on_mouse_up(SliderUi* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_release_capture(&self->component);
	if ((paramtweak_active(&self->paramtweak))) {
		paramtweak_end(&self->paramtweak);		
		psy_ui_component_invalidate(&self->component);
	}
}

void sliderui_update_param(SliderUi* self)
{
	if (self->machine && self->paramidx != psy_INDEX_INVALID) {
		self->param = psy_audio_machine_parameter(self->machine,
			self->paramidx);
	}
}
