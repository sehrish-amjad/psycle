/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiglyph.h"
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"


/* psy_ui_Polygon */
void psy_ui_polygon_init(psy_ui_Polygon* self)
{
	assert(self);
	
	self->pts = 0;
	self->fill = TRUE;
}

void psy_ui_polygon_dispose(psy_ui_Polygon* self)
{
	assert(self);
	
	psy_list_deallocate(&self->pts, NULL);
}

void psy_ui_polygon_copy(psy_ui_Polygon* self, const psy_ui_Polygon* src)
{
	psy_List* p;
	
	assert(self);
	assert(src);
	
	if (self != src) {
		psy_ui_polygon_dispose(self);
		psy_ui_polygon_init(self);
		self->fill = src->fill;
		for (p = src->pts; p != NULL; p = p->next) {
			psy_ui_RealPoint* pt;
			
			pt = (psy_ui_RealPoint*)p->entry;
			psy_ui_polygon_add(self, *pt);
		}
	}
}

psy_ui_Polygon* psy_ui_polygon_alloc(void)
{
	return (psy_ui_Polygon*)malloc(sizeof(psy_ui_Polygon));
}

psy_ui_Polygon* psy_ui_polygon_alloc_init(void)
{
	psy_ui_Polygon* rv;
	
	rv = psy_ui_polygon_alloc();
	if (rv) {
		psy_ui_polygon_init(rv);
	}
	return rv;
}

psy_ui_Polygon* psy_ui_polygon_clone(const psy_ui_Polygon* src)
{
	psy_ui_Polygon* rv;
	
	rv = psy_ui_polygon_alloc_init();
	if (rv) {
		psy_ui_polygon_copy(rv, src);
	}
	return rv;
}

void psy_ui_polygon_add(psy_ui_Polygon* self, psy_ui_RealPoint pt)
{
	psy_ui_RealPoint* new_pt;
	
	assert(self);
	
	new_pt = (psy_ui_RealPoint*)malloc(sizeof(psy_ui_RealPoint));
	*new_pt = pt;
	psy_list_append(&self->pts, new_pt);
}


/* psy_ui_Arc */
void psy_ui_arc_init(psy_ui_Arc* self)
{
	assert(self);
	
	self->r = psy_ui_realrectangle_zero();
	self->angle_start = 0;
	self->angle_end = 360;
	self->fill = TRUE;
}

void psy_ui_arc_dispose(psy_ui_Arc* self)
{
	assert(self);
		
}

void psy_ui_arc_copy(psy_ui_Arc* self, const psy_ui_Arc* src)
{
	assert(self);
	assert(src);
	
	if (self != src) {
		*self = *src;
	}	
}

psy_ui_Arc* psy_ui_arc_alloc(void)
{
	return (psy_ui_Arc*)malloc(sizeof(psy_ui_Arc));
}

psy_ui_Arc* psy_ui_arc_alloc_init(void)
{
	psy_ui_Arc* rv;
	
	rv = psy_ui_arc_alloc();
	if (rv) {
		psy_ui_arc_init(rv);
	}
	return rv;
}

psy_ui_Arc* psy_ui_arc_clone(const psy_ui_Arc* src)
{
	psy_ui_Arc* rv;
	
	rv = psy_ui_arc_alloc_init();
	if (rv) {
		psy_ui_arc_copy(rv, src);
	}
	return rv;
}

void psy_ui_arc_set(psy_ui_Arc* self, psy_ui_RealRectangle r, double angle_start,
	double angle_end)
{
	assert(self);
	
	self->r = r;
	self->angle_start = angle_start;
	self->angle_end = angle_end;
}


/* psy_ui_Glyph */
void psy_ui_glyph_init(psy_ui_Glyph* self, psy_ui_RealSize size)
{
	assert(self);
	
	self->polys = NULL;
	self->arcs = NULL;
	self->curr_poly = NULL;
	self->curr_arc = NULL;
	self->size = size;	
}

void psy_ui_glyph_dispose(psy_ui_Glyph* self)
{
	assert(self);
	
	psy_list_deallocate(&self->polys, (psy_fp_disposefunc)
		psy_ui_polygon_dispose);	
	psy_list_deallocate(&self->arcs, (psy_fp_disposefunc)
		psy_ui_arc_dispose);	
}

void psy_ui_glyph_copy(psy_ui_Glyph* self, const psy_ui_Glyph* src)
{
	psy_List* p;
	
	assert(self);
	assert(src);
	
	psy_ui_glyph_dispose(self);
	psy_ui_glyph_init(self, src->size);	
	for (p = src->polys; p != NULL; p = p->next) {
		psy_ui_Polygon* src;
		
		src = (psy_ui_Polygon*)p->entry;
		self->curr_poly = psy_ui_polygon_clone(src);
		if (self->curr_poly) {
			psy_list_append(&self->polys, self->curr_poly);
		}
	}
	for (p = src->arcs; p != NULL; p = p->next) {
		psy_ui_Arc* src;
		
		src = (psy_ui_Arc*)p->entry;
		self->curr_arc = psy_ui_arc_clone(src);
		if (self->curr_arc) {
			psy_list_append(&self->arcs, self->curr_arc);
		}
	}
}

psy_ui_Glyph* psy_ui_glyph_alloc(void)
{
	return (psy_ui_Glyph*)malloc(sizeof(psy_ui_Glyph));
}

psy_ui_Glyph* psy_ui_glyph_alloc_init(psy_ui_RealSize size)
{
	psy_ui_Glyph* rv;
	
	rv = psy_ui_glyph_alloc();
	if (rv) {
		psy_ui_glyph_init(rv, size);
	}
	return rv;
}

psy_ui_Glyph* psy_ui_glyph_clone(const psy_ui_Glyph* src)
{
	psy_ui_Glyph* rv;
	
	assert(src);
	
	rv = psy_ui_glyph_alloc_init(src->size);
	if (rv) {
		psy_ui_glyph_copy(rv, src);
	}
	return rv;
}

psy_ui_Polygon* psy_ui_glyph_add_poly(psy_ui_Glyph* self)
{	
	assert(self);
		
	self->curr_poly = psy_ui_polygon_alloc_init();
	if (self->curr_poly) {
		psy_list_append(&self->polys, self->curr_poly);
	}	
	return self->curr_poly;
}

psy_ui_Polygon* psy_ui_glyph_add_rect(psy_ui_Glyph* self,
	psy_ui_RealRectangle r, bool fill)
{
	assert(self);
		
	self->curr_poly = psy_ui_polygon_alloc_init();
	if (self->curr_poly) {
		psy_list_append(&self->polys, self->curr_poly);
		psy_ui_polygon_add(self->curr_poly, psy_ui_realpoint_make(
			r.left, r.top));
		psy_ui_polygon_add(self->curr_poly, psy_ui_realpoint_make(
			r.right, r.top));
		psy_ui_polygon_add(self->curr_poly, psy_ui_realpoint_make(
			r.right, r.bottom));
		psy_ui_polygon_add(self->curr_poly, psy_ui_realpoint_make(
			r.left, r.bottom));
		psy_ui_polygon_add(self->curr_poly, psy_ui_realpoint_make(
			r.left, r.top));
		self->curr_poly->fill = fill;
	}	
	return self->curr_poly;
}

psy_ui_Arc* psy_ui_glyph_add_arc(psy_ui_Glyph* self, psy_ui_RealRectangle r,
	double angle_start, double angle_end, bool fill)
{
	assert(self);
		
	self->curr_arc = psy_ui_arc_alloc_init();
	if (self->curr_arc) {
		psy_list_append(&self->arcs, self->curr_arc);
		psy_ui_arc_set(self->curr_arc, r, angle_start, angle_end);
		self->curr_arc->fill = fill;
	}
	return self->curr_arc;
}

psy_ui_Arc* psy_ui_glyph_add_circle(psy_ui_Glyph* self, psy_ui_RealPoint center,
	double radius, bool fill)
{	
	return psy_ui_glyph_add_arc(self, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(center.x - radius, center.y - radius),
		psy_ui_realsize_make(radius * 2.0, radius * 2.0)),
		0, 360, fill);
}
