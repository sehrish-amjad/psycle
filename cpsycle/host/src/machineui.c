/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machineui.h"
/* host */
#include "paramview.h"
#include "wireview.h"
#include "effectui.h"
#include "generatorui.h"
#include "masterui.h"
#include "styles.h"
/* ui */
#include <uiapp.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"


static void editnameui_editname(EditnameUi*, char* rv, uintptr_t strlen);


void editnameui_init(EditnameUi* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, uintptr_t style)
{
	psy_ui_label_init(&self->label, parent);
	self->machine = machine;
	psy_ui_label_prevent_translation(&self->label);
	psy_ui_label_prevent_wrap(&self->label);		
	psy_ui_component_set_style_type(psy_ui_label_base(&self->label), style);	
	editnameui_update(self);	
}

void editnameui_update(EditnameUi* self)
{
	char editname[130];
	
	editnameui_editname(self, editname, 130);
	psy_ui_label_set_text(&self->label, editname);
}

void editnameui_editname(EditnameUi* self, char* rv, uintptr_t strlen)
{
	assert(self);

	rv[0] = '\0';
	if (self->machine && psy_audio_machine_edit_name(self->machine)) {
		if (!machineui_maxindex_prevented()) {
			psy_snprintf(rv, strlen, "%.2X:%s", (int)
				psy_audio_machine_slot(self->machine),
				psy_audio_machine_edit_name(self->machine));
		} else {
			psy_snprintf(rv, strlen, "%s",
				psy_audio_machine_edit_name(self->machine));
		}
	}
}

/* prototypes */
static void vuui_ondraw(VuUi*, psy_ui_Graphics*);
static void vuui_drawdisplay(VuUi*, psy_ui_Graphics*);
static void vuui_drawpeak(VuUi*, psy_ui_Graphics*);

/* vtable */
static psy_ui_ComponentVtable vuui_vtable;
static bool vuui_vtable_initialized = FALSE;

static void vuui_vtable_init(VuUi* self)
{
	assert(self);

	if (!vuui_vtable_initialized) {
		vuui_vtable = *(self->component.vtable);		
		vuui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			vuui_ondraw;		
		vuui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vuui_vtable);
}

void vuui_init(VuUi* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	uintptr_t vu_style, uintptr_t vu0_style, uintptr_t vupeak_style)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vuui_vtable_init(self);
	psy_ui_component_set_style_type(&self->component, vu_style);
	self->machine = machine;
	self->vu0_style = vu0_style;
	self->vupeak_style = vupeak_style;
	vudisplay_init(&self->vu);
}

bool vuui_update(VuUi* self)
{
	return vudisplay_update(&self->vu, psy_audio_machine_buffermemory(
		self->machine));
}

void vuui_ondraw(VuUi* self, psy_ui_Graphics* g)
{
	if (!machineui_vumeter_prevented()) {		
		vuui_drawdisplay(self, g);
		vuui_drawpeak(self, g);
	}
}

void vuui_drawdisplay(VuUi* self, psy_ui_Graphics* g)
{
	psy_ui_Style* style;
	psy_ui_RealRectangle r;
	psy_ui_RealSize bg_size;
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(&self->component);
	style = psy_ui_style(self->vu0_style);
	bg_size = psy_ui_size_px(&style->background.size, tm, 0);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_value_px(&style->padding.left, tm, 0),
			psy_ui_value_px(&style->padding.top, tm, 0)),
		psy_ui_realsize_make(
			self->vu.vuvalues.volumedisplay * bg_size.width,
			bg_size.height));
	if (!psy_ui_bitmap_empty(&style->background.bitmap)) {
		psy_ui_graphics_draw_bitmap(g, &style->background.bitmap, r,
			psy_ui_realpoint_make(
				-style->background.position.x,
				-style->background.position.y));
	} else {
		psy_ui_graphics_draw_solid_rectangle(g, r, psy_ui_style_background_colour(style));
	}
}

void vuui_drawpeak(VuUi* self, psy_ui_Graphics* g)
{
	if (self->vu.vuvalues.volumemaxdisplay > 0.01f) {
		const psy_ui_TextMetric* tm;
		psy_ui_Style* vu0_style;
		psy_ui_Style* vupeak_style;
		psy_ui_RealSize bg_vu0_size;
		psy_ui_RealSize bg_vupeak_size;
		psy_ui_RealRectangle r;

		tm = psy_ui_component_textmetric(&self->component);
		vu0_style = psy_ui_style(self->vu0_style);
		vupeak_style = psy_ui_style(self->vupeak_style);
		bg_vu0_size = psy_ui_size_px(&vu0_style->background.size, tm, 0);
		bg_vupeak_size = psy_ui_size_px(&vupeak_style->background.size, tm, 0);
		r = psy_ui_realrectangle_make(
					psy_ui_realpoint_make(
						psy_ui_value_px(&vu0_style->padding.left, tm, 0) +
						self->vu.vuvalues.volumemaxdisplay * bg_vu0_size.width,
						psy_ui_value_px(&vu0_style->padding.top, tm, 0)),
					bg_vupeak_size);
		if (!psy_ui_bitmap_empty(&vu0_style->background.bitmap)) {
			psy_ui_graphics_draw_bitmap(g, &vu0_style->background.bitmap,
				r, psy_ui_realpoint_make(
					-vupeak_style->background.position.x,
					-vupeak_style->background.position.y));
		} else {
			psy_ui_graphics_draw_solid_rectangle(g,
				r, psy_ui_style_background_colour(vupeak_style));
		}
	}
}

static void panui_on_mouse_down(PanUi*, psy_ui_MouseEvent*);
static void panui_on_mouse_up(PanUi*, psy_ui_MouseEvent*);
static void panui_onmousemove(PanUi*, psy_ui_MouseEvent*);
static void panui_onalign(PanUi*);

/* vtable */
static psy_ui_ComponentVtable panui_vtable;
static bool panui_vtable_initialized = FALSE;

static void panui_vtable_init(PanUi* self)
{
	assert(self);

	if (!panui_vtable_initialized) {
		panui_vtable = *(self->component.vtable);		
		panui_vtable.onalign =
			(psy_ui_fp_component)
			panui_onalign;
		panui_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			panui_on_mouse_down;
		panui_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			panui_on_mouse_up;
		panui_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			panui_onmousemove;
		panui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &panui_vtable);
}

void panui_init(PanUi* self, psy_ui_Component* parent, psy_audio_Machine*
	machine, uintptr_t pan_style, uintptr_t pan_slider_style)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	panui_vtable_init(self);
	self->machine = machine;
	self->drag = FALSE;
	self->dragoffset = 0;
	psy_ui_component_set_style_type(&self->component, pan_style);
	psy_ui_component_init(&self->slider, &self->component, NULL);
	psy_ui_component_set_style_type(&self->slider, pan_slider_style);
}

void panui_on_mouse_down(PanUi* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_target(ev) == &self->slider) {
		psy_ui_component_capture(&self->component);
		self->drag = TRUE;
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void panui_on_mouse_up(PanUi* self, psy_ui_MouseEvent* ev)
{
	if (self->drag) {
		psy_ui_component_release_capture(&self->component);
		self->drag = FALSE;
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void panui_onmousemove(PanUi* self, psy_ui_MouseEvent* ev)
{
	if (self->drag) {
		psy_ui_RealSize size;
		psy_ui_RealSize slidersize;
		double panvalue;

		size = psy_ui_component_size_px(&self->component);
		slidersize = psy_ui_component_size_px(&self->slider);
		panvalue = (double)((ev->offset_.x) /
			(((double)size.width)));
		psy_audio_machine_set_panning(self->machine, psy_min(panvalue, 1.0));
		psy_ui_component_align(&self->component);
		psy_ui_component_invalidate(&self->component);
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void panui_onalign(PanUi* self)
{
	psy_ui_Style* style;

	style = psy_ui_componentstyle_currstyle(&self->slider.style);
	if (style) {		
		psy_ui_Rectangle position;
		psy_ui_RealSize size;
		psy_ui_RealSize slidersize;

		size = psy_ui_component_size_px(&self->component);
		slidersize = psy_ui_component_size_px(&self->slider);
		position = *style->position.rectangle;
		position.topleft.x = psy_ui_value_make_px(
			psy_audio_machine_panning(self->machine) *
			((double)size.width - (slidersize.width)));
		psy_ui_component_set_position(&self->slider, position);		
	}
}


static bool vumeter_prevented = FALSE;
static bool macindex_prevented = FALSE;

static void drawmachineline(psy_ui_Graphics*, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge);

/* VuValues*/
void vuvalues_init(VuValues* self)
{
	self->volumedisplay = 0.0;
	self->volumemaxdisplay = 0.0;
	self->volumemaxcounterlife = 0;
}

bool vuvalues_update(VuValues* self, psy_audio_Buffer* buffer)
{		
	double temp;

	temp = self->volumedisplay;
	self->volumedisplay = (buffer)
		? psy_audio_buffer_rmsdisplay(buffer)
		: 0.0;
	vuvalues_tickcounter(self);
	return (fabs(temp - self->volumedisplay) >= 0.01);
}

void vuvalues_tickcounter(VuValues* self)
{
	--self->volumemaxcounterlife;
	if ((self->volumedisplay > self->volumemaxdisplay)
		|| (self->volumemaxcounterlife <= 0)) {
		self->volumemaxdisplay = self->volumedisplay - 1 / 32768.f;
		self->volumemaxcounterlife = 60;
	}
}

/* VuDisplay */
/* prototypes */
static void vudisplay_drawdisplay(VuDisplay*, psy_ui_Graphics*, psy_ui_Style* vu);
static void vudisplay_drawpeak(VuDisplay*, psy_ui_Graphics*, psy_ui_Style* vu,
	psy_ui_Style* vupeak);

/* implementation */
void vudisplay_init(VuDisplay* self)
{	
	vuvalues_init(&self->vuvalues);	
}

bool vudisplay_update(VuDisplay* self, psy_audio_Buffer* buffer)
{
	return vuvalues_update(&self->vuvalues, buffer);
}

void vudisplay_draw(VuDisplay* self, psy_ui_Graphics* g, psy_ui_Style* vu, psy_ui_Style* vupeak)
{
	assert(self);
	
	vudisplay_drawdisplay(self, g, vu);
	vudisplay_drawpeak(self, g, vu, vupeak);	
}

void vudisplay_drawdisplay(VuDisplay* self, psy_ui_Graphics* g, 
	psy_ui_Style* vu)
{			
	psy_ui_RealRectangle r;
	psy_ui_RealSize bg_vu_size;
	const psy_ui_TextMetric* tm;	
	tm = 0;
	
	bg_vu_size = psy_ui_size_px(&vu->background.size, tm, 0);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			psy_ui_value_px(&vu->padding.left, tm, 0),
			psy_ui_value_px(&vu->padding.top, tm, 0)),
		psy_ui_realsize_make(
			self->vuvalues.volumedisplay * bg_vu_size.width,
			bg_vu_size.height));		
	psy_ui_graphics_draw_bitmap(g, &vu->background.bitmap, r,
		psy_ui_realpoint_make(
			-vu->background.position.x,
			-vu->background.position.y));
}

void vudisplay_drawpeak(VuDisplay* self, psy_ui_Graphics* g,
	psy_ui_Style* vu, psy_ui_Style* vupeak)
{
	if (self->vuvalues.volumemaxdisplay > 0.01f) {		
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize bg_vu_size;
		psy_ui_RealSize bg_vu_peak_size;
		
		tm = 0;
		bg_vu_size = psy_ui_size_px(&vu->background.size, tm, 0);
		bg_vu_peak_size = psy_ui_size_px(&vupeak->background.size, tm, 0);
		psy_ui_graphics_draw_bitmap(g, &vu->background.bitmap,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					psy_ui_value_px(&vu->padding.left, tm, 0) +
					self->vuvalues.volumemaxdisplay * bg_vu_size.width,
					psy_ui_value_px(&vu->padding.top, tm, 0)),
				bg_vu_peak_size),
			psy_ui_realpoint_make(
				-vupeak->background.position.x,
				-vupeak->background.position.y));
	}
}

psy_ui_Component* machineui_create(psy_audio_Machine* machine, 
	psy_ui_Component* parent, ParamViews* paramviews, bool machine_pos,
	psy_audio_Machines* machines, psy_Configuration* config)
{	
	psy_ui_Component* rv;	
	const psy_audio_MachineInfo* info;
	uintptr_t mac_id;
	
	assert(machine);	
	assert(machines);	

	rv = NULL;	
	info = psy_audio_machine_info(machine);
	if (!info) {
		return NULL;
	}	
	mac_id = psy_audio_machine_slot(machine);
	if (mac_id == psy_audio_MASTER_INDEX) {
		MasterUi* ui;

		ui = masterui_alloc_init(parent, paramviews, machines, config);
		if (ui) {
			if (!machine_pos) {
				masterui_prevent_machine_pos(ui);
			}
			rv = masterui_base(ui);
		}
	} else if (mac_id >= 0x40 && mac_id < 0x80) {
		EffectUi* ui;

		ui = effectui_alloc_init(parent, mac_id, paramviews, machines,
			config);
		if (ui) {
			if (!machine_pos) {
				effectui_prevent_machine_pos(ui);
			}
			rv = effectui_base(ui);
		}
	} else {
		GeneratorUi* ui;

		ui = generatorui_alloc_init(parent, mac_id, paramviews, machines,
			config);
		if (ui) {
			if (!machine_pos) {
				generatorui_prevent_machine_pos(ui);
			}
			rv = generatorui_base(ui);
		}
	}
	if (rv) {
		psy_ui_component_set_id(rv, mac_id);
		psy_ui_component_set_align(rv, psy_ui_ALIGN_NONE);
		psy_ui_component_align(rv);
	}	
	return rv;	
}

void machineui_drawhighlight(psy_ui_Graphics* g, psy_ui_RealRectangle position)
{
	static double d = PSYCLE_MACHINE_BORDER_DISTANCE;
	static psy_ui_RealPoint dirs[] = {
		{ 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 }
	};
	psy_ui_RealPoint edges[4];
	psy_ui_RealPoint origin;
	double width;
	double height;
	
	width = psy_ui_realrectangle_width(&position);
	height = psy_ui_realrectangle_height(&position);
	edges[0] = psy_ui_realpoint_make(-d, -d);
	edges[1] = psy_ui_realpoint_make(width + d, -d);
	edges[2] = psy_ui_realpoint_make(width + d, height + d);
	edges[3] = psy_ui_realpoint_make(-d, height + d);
	origin = psy_ui_origin(g);
	psy_ui_setorigin(g,
		psy_ui_realpoint_make(
			origin.x - position.left,
			origin.y - position.top));
	drawmachineline(g, dirs[0], edges[0]);
	drawmachineline(g, dirs[1], edges[0]);
	drawmachineline(g, dirs[1], edges[1]);
	drawmachineline(g, dirs[2], edges[1]);
	drawmachineline(g, dirs[2], edges[2]);
	drawmachineline(g, dirs[3], edges[2]);
	drawmachineline(g, dirs[3], edges[3]);
	drawmachineline(g, dirs[0], edges[3]);
	psy_ui_setorigin(g, origin);
}

void drawmachineline(psy_ui_Graphics* g, psy_ui_RealPoint dir,
	psy_ui_RealPoint edge)
{
	static intptr_t hlength = 9; // the length of the selected machine highlight	

	psy_ui_drawline(g, edge, psy_ui_realpoint_make(
		edge.x + dir.x * hlength, edge.y + dir.y * hlength));
}

void machineui_enable_vumeter(void)
{
	vumeter_prevented = FALSE;
}

void machineui_prevent_vumeter(void)
{
	vumeter_prevented = TRUE;
}

bool machineui_vumeter_prevented(void)
{
	return vumeter_prevented;
}

void machineui_enable_macindex(void)
{
	macindex_prevented = FALSE;
}

void machineui_prevent_macindex(void)
{
	macindex_prevented = TRUE;
}

bool machineui_maxindex_prevented(void)
{
	return macindex_prevented;
}
