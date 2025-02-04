/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "cpuview.h"
/* host */
#include "styles.h"

#if defined(DIVERSALIS__OS__MICROSOFT)
#include <windows.h>
#endif

#include "../../detail/portable.h"
#include "../../detail/os.h"


/* prototypes */
static void cpumoduleview_on_draw(CPUModuleView*, psy_ui_Graphics*);
static void cpumoduleview_on_preferred_size(CPUModuleView*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

/* vtable */
static psy_ui_ComponentVtable cpumoduleview_vtable;
static bool cpumoduleview_vtable_initialized = FALSE;

static void cpumoduleview_vtable_init(CPUModuleView* self)
{
	if (!cpumoduleview_vtable_initialized) {
		cpumoduleview_vtable = *(self->component.vtable);
		cpumoduleview_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			cpumoduleview_on_draw;
		cpumoduleview_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			cpumoduleview_on_preferred_size;
		cpumoduleview_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &cpumoduleview_vtable);
}

/* implementation */
void cpumoduleview_init(CPUModuleView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	cpumoduleview_vtable_init(self);		
	self->workspace = workspace;	
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_set_scroll_step(&self->component, psy_ui_size_make_em(
		0.0, 1.0));
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);	
}

void cpumoduleview_on_draw(CPUModuleView* self, psy_ui_Graphics* g)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		uintptr_t slot;
		int cpy = 0;
		const psy_ui_TextMetric* tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_scroll_size(&self->component), tm, NULL);
		psy_ui_graphics_set_background_mode(g, psy_ui_TRANSPARENT);
		psy_ui_graphics_set_text_colour(g, psy_ui_colour_make(0x00D1C5B6));
		for (slot = 0; slot <= psy_audio_machines_maxindex(
				&workspace_song(self->workspace)->machines_); ++slot) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines_,
				slot);
			if (machine) {
				psy_ui_Value scrolltop;

				scrolltop = psy_ui_component_scroll_top(&self->component);
				if ((cpy - psy_ui_value_px(&scrolltop, tm, NULL)) >= 0) {
					char text[40];
					const psy_audio_MachineInfo* info;					

					info = psy_audio_machine_info(machine);
					if (info) {						
						psy_snprintf(text, 20, "%d", (int)slot);
						psy_ui_graphics_textout(g, psy_ui_realpoint_make(0, cpy), text,
							psy_strlen(text));
						psy_ui_graphics_textout(g, psy_ui_realpoint_make(tm->tmAveCharWidth * 5, cpy),
							psy_audio_machine_edit_name(machine),
							psy_min(psy_strlen(psy_audio_machine_edit_name(machine)), 14));
						psy_ui_graphics_textout(g, psy_ui_realpoint_make(tm->tmAveCharWidth * 21, cpy),
							info->name, psy_strlen(info->name));
						if (psy_audio_player_measuring_cpu_usage(workspace_player(self->workspace))) {
							psy_snprintf(text, 40, "%.1f%%", 100.0f * psy_audio_machine_cpu_time(machine).perc);						
						} else {
							psy_snprintf(text, 40, "N/A");
						}
						psy_ui_graphics_textout(g, psy_ui_realpoint_make(tm->tmAveCharWidth * 60, cpy),
							text, psy_strlen(text));
					}
					if ((cpy - psy_ui_value_px(&scrolltop, tm, NULL)) > size.height) {
						break;
					}
				}
				cpy += tm->tmHeight;
			}
		}
	}
}

void cpumoduleview_on_preferred_size(CPUModuleView* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	uintptr_t currlines;
	psy_ui_Size size;
	
	assert(self);

	if (workspace_song(self->workspace)) {
		currlines = psy_audio_machines_size(&workspace_song(
			self->workspace)->machines_);
	} else {
		currlines = 0;
	}
	size = psy_ui_component_scroll_size(&self->component);	
	rv->height = psy_ui_mul_value_real(
		psy_ui_component_scrollstep_height(&self->component),
		(double)currlines);
	rv->width = psy_ui_value_make_ew(80.0);
}


/* CPUView */

/* prototypes */
static void cpuview_init_core_info(CPUView*);
static void cpuview_init_title(CPUView*);
static void cpuview_init_resources(CPUView*);
static void cpuview_init_performance(CPUView*);
static void cpuview_init_modules(CPUView*, Workspace* workspace);
static void cpuview_on_timer(CPUView*, psy_ui_Component* sender,
	uintptr_t timerid);
static void cpuview_on_cpu_performance(CPUView*, psy_ui_CheckBox* sender);


/* implementation */
void cpuview_init(CPUView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_Margin margin;

	assert(self);

	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.5, 2.0);		
	psy_ui_component_init(&self->component, parent, NULL);	
	self->lastnummachines = psy_INDEX_INVALID;	
	self->workspace = workspace;
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_style_type(&self->client, STYLE_SIDE_VIEW);
	psy_ui_margin_init(&self->topmargin);		
	psy_ui_component_init_align(&self->top, &self->client, NULL,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_margin(&self->top, margin);
	cpuview_init_title(self);
	cpuview_init_core_info(self);	
	cpuview_init_resources(self);
	cpuview_init_performance(self);
	cpuview_init_modules(self, workspace);
	psy_signal_connect(&self->component.signal_timer, self,
		cpuview_on_timer);
	psy_ui_component_start_timer(&self->component, 0, 200);
}

void cpuview_init_title(CPUView* self)
{	
	titlebar_init(&self->titlebar, &self->component,
		"Psycle DSP/CPU Performance Monitor");	
	closebar_set_property(&self->titlebar.close_bar_,
		psy_configuration_at(
			psycleconfig_general(workspace_cfg(self->workspace)),
			"bench.showcpu"));	
}

void cpuview_init_core_info(CPUView* self)
{
	assert(self);

	psy_ui_label_init_text(&self->coreinfo, &self->top, "Core Info");
	psy_ui_component_set_align(&self->coreinfo.component, psy_ui_ALIGN_TOP);
}

void cpuview_init_resources(CPUView* self)
{	
	assert(self);

	psy_ui_component_init(&self->resources, &self->top, NULL);
	psy_ui_component_set_default_align(&self->resources, psy_ui_ALIGN_TOP,
		self->topmargin);
	psy_ui_component_set_align(&self->resources, psy_ui_ALIGN_LEFT);
	psy_ui_label_init_text(&self->resourcestitle, &self->resources,
		"Available Resources");		
	labelpair_init(&self->resources_win, &self->resources, "cpu.resources", 25.0);
	labelpair_init(&self->resources_mem, &self->resources, "cpu.ram", 25.0);
	labelpair_init(&self->resources_swap, &self->resources, "cpu.swap", 25.0);
	labelpair_init(&self->resources_vmem, &self->resources, "cpu.vram", 25.0);
}

void cpuview_init_performance(CPUView* self)
{	
	assert(self);

	psy_ui_component_init_align(&self->performance, &self->top, NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_component_set_default_align(&self->performance, psy_ui_ALIGN_TOP,
		self->topmargin);	
	psy_ui_checkbox_init_text(&self->cpucheck, &self->performance,
		"cpu.performance");
	psy_signal_connect(&self->cpucheck.signal_clicked, self,
		cpuview_on_cpu_performance);
	labelpair_init(&self->audiothreads, &self->performance, "cpu.audio-threads", 25.0);
	labelpair_init(&self->totaltime, &self->performance, "cpu.total", 25.0);
	labelpair_init(&self->machines, &self->performance, "cpu.machines", 25.0);
	labelpair_init(&self->routing, &self->performance, "cpu.routing", 25.0);
}

void cpuview_init_modules(CPUView* self, Workspace* workspace)
{
	psy_ui_Margin margin;

	cpumoduleview_init(&self->modules, &self->client, workspace);
	psy_ui_scroller_init(&self->scroller, &self->client, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->modules.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_em(&margin, 1.0, 0.0, 0.0, 2.0);
	psy_ui_component_set_margin(&self->scroller.component, margin);
}

void cpuview_on_timer(CPUView* self, psy_ui_Component* sender,
	uintptr_t timerid)
{
	uintptr_t nummachines;
#if defined(DIVERSALIS__OS__MICROSOFT) && WINVER >= 0x600
	MEMORYSTATUSEX lpBuffer;
	char buffer[128];

	if (!psy_ui_component_draw_visible(&self->component)) {
		return;
	}
	lpBuffer.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&lpBuffer);

	psy_snprintf(buffer, 128, "%d%%", 100 - lpBuffer.dwMemoryLoad);
	psy_ui_label_set_text(&self->resources_win.second, buffer);

	psy_snprintf(buffer, 128, "%.0fM (of %.0fM)",
		lpBuffer.ullAvailPhys / (float)(1 << 20),
		lpBuffer.ullTotalPhys / (float)(1 << 20));
	psy_ui_label_set_text(&self->resources_mem.second, buffer);

	psy_snprintf(buffer, 128, "%.0fM (of %.0fM)",
		(lpBuffer.ullAvailPageFile / (float)(1 << 20)),
		(lpBuffer.ullTotalPageFile / (float)(1 << 20)));
	psy_ui_label_set_text(&self->resources_swap.second, buffer);
#if defined _WIN64
	psy_snprintf(buffer, 128, "%.0fG (of %.0fG)",
		(lpBuffer.ullAvailVirtual / (float)(1 << 30)),
		(lpBuffer.ullTotalVirtual / (float)(1 << 30)));
#elif defined _WIN32
	psy_snprintf(buffer, 128, "%.0fM (of %.0fM)",
		(lpBuffer.ullAvailVirtual / (float)(1 << 20)),
		(lpBuffer.ullTotalVirtual / (float)(1 << 20)));
#endif
	psy_ui_label_set_text(&self->resources_vmem.second, buffer);
#endif
	if (workspace_song(self->workspace)) {
		nummachines = psy_audio_machines_size(&workspace_song(self->workspace)->machines_);
	} else {
		nummachines = 0;
	}
	if (nummachines != self->lastnummachines) {
		self->lastnummachines = nummachines;
		psy_ui_component_update_overflow(&self->modules.component);
	}
	psy_ui_component_invalidate(&self->modules.component);
}


void cpuview_on_cpu_performance(CPUView* self, psy_ui_CheckBox* sender)
{
	assert(self);

	if (psy_ui_checkbox_checked(sender) != 0) {
		psy_audio_player_measure_cpu_usage(workspace_player(self->workspace));
	} else {
		psy_audio_player_stop_measure_cpu_usage(workspace_player(self->workspace));
	}
}
