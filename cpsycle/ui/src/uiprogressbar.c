/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiprogressbar.h"

/* prototypes */
static void psy_ui_progressbar_on_draw(psy_ui_ProgressBar*, psy_ui_Graphics*);
static void psy_ui_progressbar_on_progress(psy_ui_ProgressBar*, psy_ProgressState);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_ProgressBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_progressbar_on_draw;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}


/* logger vtable */
static psy_LoggerVTable logger_vtable;
static bool logger_initialized = FALSE;

static void logger_vtable_init(psy_ui_ProgressBar* self)
{
	if (!logger_initialized) {
		logger_vtable = *(self->logger.vtable);
		logger_vtable.progress =
			(fp_progress)
			psy_ui_progressbar_on_progress;
		logger_initialized = TRUE;
	}
	self->logger.vtable = &logger_vtable;
	self->logger.context_ = (void*)self;
}

/* implementation */
void psy_ui_progressbar_init(psy_ui_ProgressBar* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_logger_init(&self->logger);
	logger_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, psy_ui_STYLE_PROGRESSBAR);
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_em(10.0, 0.0));
	self->progress = 0.0;
}

void psy_ui_progressbar_on_draw(psy_ui_ProgressBar* self, psy_ui_Graphics* g)
{	
	psy_ui_RealSize size;	
	psy_ui_RealRectangle bar;
				
	size = psy_ui_component_size_px(&self->component);	
	psy_ui_realrectangle_init_all(&bar,
		psy_ui_realpoint_zero(),
		psy_ui_realsize_make(self->progress * size.width, size.height));
	psy_ui_graphics_draw_solid_rectangle(g, bar,
		psy_ui_component_colour(progressbar_base(self)));
}

void psy_ui_progressbar_set_progress(psy_ui_ProgressBar* self, double progress)
{
	self->progress = progress;
	psy_ui_component_invalidate(progressbar_base(self));
	psy_ui_component_update(progressbar_base(self));
}

void psy_ui_progressbar_tick(psy_ui_ProgressBar* self)
{
	if (self->progress + 0.05 > 1.0) {
		self->progress = 0.0;
	}
	psy_ui_progressbar_set_progress(self, self->progress + 0.05);	
}

void psy_ui_progressbar_on_progress(psy_ui_ProgressBar* self, psy_ProgressState state)
{
	switch (state) {
	case PSY_PROGRESS_STATE_TICK:
		psy_ui_progressbar_tick(self);
		break;
	case PSY_PROGRESS_STATE_END:
		psy_ui_progressbar_set_progress(self, 0.0);
		break;
	default:
		break;
	}
	
}
