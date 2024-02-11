/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_GLYPH_H
#define psy_ui_GLYPH_H

/* local */
#include "uigeometry.h"
/* container */
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct psy_ui_Polygon
*/
typedef struct psy_ui_Polygon {
	psy_List* pts;
	bool fill;		
} psy_ui_Polygon;

void psy_ui_polygon_init(psy_ui_Polygon*);
void psy_ui_polygon_dispose(psy_ui_Polygon*);

void psy_ui_polygon_copy(psy_ui_Polygon*, const psy_ui_Polygon* src);

psy_ui_Polygon* psy_ui_polygon_alloc(void);
psy_ui_Polygon* psy_ui_polygon_alloc_init(void);
psy_ui_Polygon* psy_ui_polygon_clone(const psy_ui_Polygon*);

void psy_ui_polygon_add(psy_ui_Polygon*, psy_ui_RealPoint);


/*!
** @struct psy_ui_Arc
*/
typedef struct psy_ui_Arc {
	psy_ui_RealRectangle r;
	double angle_start;
	double angle_end;
	bool fill;
} psy_ui_Arc;

void psy_ui_arc_init(psy_ui_Arc*);
void psy_ui_arc_dispose(psy_ui_Arc*);

void psy_ui_arc_copy(psy_ui_Arc*, const psy_ui_Arc* src);

psy_ui_Arc* psy_ui_arc_alloc(void);
psy_ui_Arc* psy_ui_arc_alloc_init(void);
psy_ui_Arc* psy_ui_arc_clone(const psy_ui_Arc*);

void psy_ui_arc_set(psy_ui_Arc*, psy_ui_RealRectangle, double angle_start,
	double angle_end);


/*!
** @struct psy_ui_Glyph
*/
typedef struct psy_ui_Glyph {
	psy_List* polys;
	psy_List* arcs;
	psy_ui_Polygon* curr_poly;
	psy_ui_Arc* curr_arc;
	psy_ui_RealSize size;	
} psy_ui_Glyph;

void psy_ui_glyph_init(psy_ui_Glyph*, psy_ui_RealSize);
void psy_ui_glyph_dispose(psy_ui_Glyph*);

void psy_ui_glyph_copy(psy_ui_Glyph*, const psy_ui_Glyph* src);

psy_ui_Glyph* psy_ui_glyph_alloc(void);
psy_ui_Glyph* psy_ui_glyph_alloc_init(psy_ui_RealSize);
psy_ui_Glyph* psy_ui_glyph_clone(const psy_ui_Glyph*);


psy_ui_Polygon* psy_ui_glyph_add_poly(psy_ui_Glyph*);
psy_ui_Polygon* psy_ui_glyph_add_rect(psy_ui_Glyph*, psy_ui_RealRectangle,
	bool fill);
psy_ui_Arc* psy_ui_glyph_add_arc(psy_ui_Glyph*, psy_ui_RealRectangle,
	double angle_start, double angle_end, bool fill);
psy_ui_Arc* psy_ui_glyph_add_circle(psy_ui_Glyph*, psy_ui_RealPoint center,
	double radius, bool fill);

INLINE uintptr_t psy_ui_glyph_num_polys(const psy_ui_Glyph* self)
{
	return psy_list_size(self->polys);
}

INLINE psy_ui_RealSize psy_ui_glyph_size(const psy_ui_Glyph* self)
{
	return self->size;
}

INLINE bool psy_ui_glyph_empty(const psy_ui_Glyph* self)
{
	return (!self->arcs && !self->polys);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GLYPH_H */
