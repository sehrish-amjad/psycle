/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uialigner.h"
/* ui */
#include "uicomponent.h"
/* std */
#include <math.h>
#include <stdio.h>

/* prototypes */
static void psy_ui_aligner_dispose_virtual(psy_ui_Aligner* self) { }
static void psy_ui_aligner_align_virtual(psy_ui_Aligner* self, psy_ui_Component* component) { }
static void psy_ui_aligner_preferredsize_virtual(psy_ui_Aligner* self,
	psy_ui_Component* component, const psy_ui_Size* limit, psy_ui_Size* rv)
{
}

/* vtable */
static psy_ui_AlignerVTable aligner_vtable;
static bool aligner_vtable_initialized = FALSE;

static void aligner_vtable_init(void)
{
	if (!aligner_vtable_initialized) {
		aligner_vtable.dispose = psy_ui_aligner_dispose_virtual;
		aligner_vtable.align = psy_ui_aligner_align_virtual;
		aligner_vtable.preferredsize = psy_ui_aligner_preferredsize_virtual;
		aligner_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_aligner_init(psy_ui_Aligner* self)
{
	aligner_vtable_init();
	self->vtable = &aligner_vtable;
	self->alignexpandmode = psy_ui_NOEXPAND;
	self->insertaligntype = psy_ui_ALIGN_NONE;
	self->alignsorted = psy_ui_ALIGN_NONE;
	psy_ui_margin_init(&self->insertmargin);
}

/* implementation helper functions */
void psy_ui_aligner_adjust_minmax_size(
	psy_ui_Component* component, const psy_ui_TextMetric* tm,
	psy_ui_Size* componentsize, psy_ui_Size* parentsize)
{
	psy_ui_Size minsize;
	psy_ui_Size maxsize;

	minsize = psy_ui_component_minimumsize(component);
	maxsize = psy_ui_component_maximumsize(component);
	if (!psy_ui_size_iszero(&maxsize)) {
		if (!psy_ui_value_is_zero(&maxsize.width)) {
			if (psy_ui_value_comp(&maxsize.width,
				&componentsize->width, tm, parentsize) < 0) {
				componentsize->width = maxsize.width;
			}
		}
		if (!psy_ui_value_is_zero(&maxsize.height)) {
			if (psy_ui_value_comp(&maxsize.height,
				&componentsize->height, tm, parentsize) < 0) {
				componentsize->height = maxsize.height;
			}
		}
	}
	if (!psy_ui_size_iszero(&minsize)) {
		if (!psy_ui_value_is_zero(&minsize.width)) {
			if (psy_ui_value_comp(&minsize.width,
				&componentsize->width, tm, parentsize) > 0) {
				componentsize->width = minsize.width;
			}
		}
		if (!psy_ui_value_is_zero(&minsize.height)) {
			if (psy_ui_value_comp(&minsize.height,
				&componentsize->height, tm, parentsize) > 0) {
				componentsize->height = minsize.height;
			}
		}
	}
}

void psy_ui_aligner_add_border(psy_ui_Component* component,
	psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Margin bordermargin;

	tm = psy_ui_component_textmetric(component);
	bordermargin = psy_ui_border_margin(psy_ui_component_border(component));
	psy_ui_size_setpx(rv,
		psy_ui_value_px(&rv->width, tm, NULL) +
		psy_ui_margin_width_px(&bordermargin, tm, NULL),
		psy_ui_value_px(&rv->height, tm, NULL) +
		psy_ui_margin_height_px(&bordermargin, tm, NULL));
}

void psy_ui_aligner_add_spacing_and_border(psy_ui_Component* component,
	psy_ui_Size* rv)
{
	const psy_ui_TextMetric* tm;
	psy_ui_Margin bordermargin;
	psy_ui_RealMargin spacing;

	tm = psy_ui_component_textmetric(component);
	spacing = psy_ui_component_spacing_px(component);
	bordermargin = psy_ui_border_margin(psy_ui_component_border(component));
	psy_ui_size_setpx(rv,
		psy_ui_value_px(&rv->width, tm, NULL) +
		psy_ui_realmargin_width(&spacing) +
		psy_ui_margin_width_px(&bordermargin, tm, NULL),
		psy_ui_value_px(&rv->height, tm, NULL) +
		psy_ui_realmargin_height(&spacing) +
		psy_ui_margin_height_px(&bordermargin, tm, NULL));
}

void psy_ui_aligner_adjust_spacing(psy_ui_Component* component,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright)
{
	psy_ui_Margin padding;

	padding = psy_ui_component_padding(component);
	cp_topleft->x += psy_ui_value_px(&padding.left,
		psy_ui_component_textmetric(component), NULL);
	cp_topleft->y += psy_ui_value_px(&padding.top,
		psy_ui_component_textmetric(component), NULL);
	cp_bottomright->x -= psy_ui_value_px(&padding.right,
		psy_ui_component_textmetric(component), NULL);
	cp_bottomright->x = psy_max(0.0, cp_bottomright->x);
	cp_bottomright->y -= psy_ui_value_px(&padding.bottom,
		psy_ui_component_textmetric(component), NULL);
	cp_bottomright->y = psy_max(0.0, cp_bottomright->y);
}

void psy_ui_aligner_adjust_border(psy_ui_Component* component,
	psy_ui_RealPoint* cp_topleft, psy_ui_RealPoint* cp_bottomright)
{
	const psy_ui_TextMetric* tm;
	const psy_ui_Border* border;
	psy_ui_Margin bordermargin;

	tm = psy_ui_component_textmetric(component);
	border = psy_ui_component_border(component);
	bordermargin = psy_ui_border_margin(border);
	if (border->left.style == psy_ui_BORDER_SOLID) {
		cp_topleft->x += psy_ui_value_px(&bordermargin.left, tm, NULL);
	}
	if (border->top.style == psy_ui_BORDER_SOLID) {
		cp_topleft->y += psy_ui_value_px(&bordermargin.top, tm, NULL);
	}
	if (border->right.style == psy_ui_BORDER_SOLID) {
		cp_bottomright->x -= psy_ui_value_px(&bordermargin.right, tm, NULL);
	}
	if (border->bottom.style == psy_ui_BORDER_SOLID) {
		cp_bottomright->y -= psy_ui_value_px(&bordermargin.bottom, tm, NULL);
	}
}

void psy_ui_aligner_adjust_margin_size(psy_ui_Component* component,
	const psy_ui_Size* parentsize,
	psy_ui_Size* rv)
{
	psy_ui_Margin margin;

	assert(rv);

	margin = psy_ui_component_margin(component);
	rv->width = psy_ui_value_make_px(
		psy_ui_value_px(&rv->width, psy_ui_component_textmetric(component), parentsize) -
		psy_ui_value_px(&margin.left, psy_ui_component_textmetric(component), parentsize) -
		psy_ui_value_px(&margin.right, psy_ui_component_textmetric(component), parentsize));
	rv->height = psy_ui_value_make_px(
		psy_ui_value_px(&rv->height, psy_ui_component_textmetric(component), parentsize) -
		psy_ui_value_px(&margin.top, psy_ui_component_textmetric(component), parentsize) -
		psy_ui_value_px(&margin.bottom, psy_ui_component_textmetric(component), parentsize));
}
