/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiicons.h"
#include "uiapp.h"
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"

#define NO_FILL FALSE
#define FILL TRUE

/* prototypes */
static void psy_ui_icons_make_common(psy_ui_Icons*);
static void psy_ui_icons_make_modes(psy_ui_Icons*);
static void psy_ui_icons_make_notes(psy_ui_Icons*);
static void psy_ui_icons_add_note_head(psy_ui_Glyph*, bool fill);
static void psy_ui_icons_add_stem(psy_ui_Glyph*);
static void psy_ui_icons_add_dot(psy_ui_Glyph*);


/* implementation */
void psy_ui_icons_init(psy_ui_Icons* self)
{
	assert(self);
	
	psy_table_init(&self->icons);
	psy_ui_icons_make_common(self);
	psy_ui_icons_make_modes(self);
	psy_ui_icons_make_notes(self);
}

void psy_ui_icons_dispose(psy_ui_Icons* self)
{
	assert(self);
	
	psy_table_dispose_all(&self->icons, (psy_fp_disposefunc)
		psy_ui_glyph_dispose);
}

void psy_ui_icons_add(psy_ui_Icons* self, uintptr_t key, psy_ui_Glyph* glyph)
{
	assert(self);
	
	psy_table_insert(&self->icons, key, glyph);
}

const psy_ui_Glyph* psy_ui_icons_at(psy_ui_Icons* self, uintptr_t key)
{
	assert(self);
	
	return (const psy_ui_Glyph*)psy_table_at_const(&self->icons, key);
}

void psy_ui_icons_make_common(psy_ui_Icons* self)
{
	psy_ui_Glyph* glyph;
	psy_ui_Polygon* p;
	
	assert(self);		
	
	/* 	psy_ui_ICON_UP */
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(9, 5));
	p = psy_ui_glyph_add_poly(glyph);	
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 0));		
	psy_ui_icons_add(self, psy_ui_ICON_UP, glyph);
	/* psy_ui_ICON_DOWN */
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(9, 5));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 0));	
	psy_ui_icons_add(self, psy_ui_ICON_DOWN, glyph);
	/* psy_ui_ICON_MORE */
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(5, 9));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 8));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 0));	
	psy_ui_icons_add(self, psy_ui_ICON_MORE, glyph);
	/* psy_ui_ICON_LESS */
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(5, 9));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 8));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 0));
	psy_ui_icons_add(self, psy_ui_ICON_LESS, glyph);
	/* psy_ui_ICON_CLOSE */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(5, 5));	
	psy_ui_glyph_add_rect(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(), psy_ui_realsize_make(4, 4)),
		FILL);	
	psy_ui_icons_add(self, psy_ui_ICON_CLOSE, glyph);	
	/* psy_ui_ICON_GRIP */
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(5, 9));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 8));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 8));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(4, 0));
	psy_ui_icons_add(self, psy_ui_ICON_GRIP, glyph);
}

void psy_ui_icons_make_modes(psy_ui_Icons* self)
{
	psy_ui_Glyph* glyph;
	psy_ui_Polygon* p;
	
	assert(self);
	
	/* psy_ui_ICON_SELECT */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 36));
	psy_ui_glyph_add_rect(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(8, 5), psy_ui_realsize_make(1, 25)),
		FILL);	
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(1, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 5));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(7, 5));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(1, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 0));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(7, 4));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(7, 5));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 5));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 0));
	p = psy_ui_glyph_add_poly(glyph);	
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 35));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(1, 35));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 31));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 30));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(7, 31));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(1, 35));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(0, 35));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 35));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 35));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(7, 31));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(7, 30));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(8, 30));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 35));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 35));
	psy_ui_icons_add(self, psy_ui_ICON_SELECT, glyph);
}

void psy_ui_icons_make_notes(psy_ui_Icons* self)
{
	psy_ui_Glyph* glyph;
	psy_ui_Polygon* p;
	
	assert(self);
		
	/* psy_ui_ICON_ENDLESS */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 35));	
	psy_ui_glyph_add_rect(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, 25), psy_ui_realsize_make(7, 10)),
		FILL);
	psy_ui_glyph_add_rect(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(8, 25), psy_ui_realsize_make(7, 10)),
		FILL);	
	psy_ui_icons_add(self, psy_ui_ICON_ENDLESS, glyph);
	/* psy_ui_ICON_SEMIBREVE */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 35));
	psy_ui_icons_add_note_head(glyph, NO_FILL);
	psy_ui_icons_add(self, psy_ui_ICON_SEMIBREVE, glyph);
	/* psy_ui_ICON_MINIM */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 35));	
	psy_ui_icons_add_note_head(glyph, NO_FILL);
	psy_ui_icons_add_stem(glyph);	
	psy_ui_icons_add(self, psy_ui_ICON_MINIM, glyph);
	/* psy_ui_ICON_MINIM_DOT */			
	glyph = psy_ui_glyph_clone(glyph);	
	psy_ui_icons_add_dot(glyph);	
	psy_ui_icons_add(self, psy_ui_ICON_MINIM_DOT, glyph);
	/* psy_ui_ICON_CROTCHET */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 35));
	psy_ui_icons_add_note_head(glyph, FILL);
	psy_ui_icons_add_stem(glyph);
	psy_ui_icons_add(self, psy_ui_ICON_CROTCHET, glyph);
	/* psy_ui_ICON_CROTCHET_DOT */
	glyph = psy_ui_glyph_clone(glyph);
	psy_ui_icons_add_dot(glyph);	
	psy_ui_icons_add(self, psy_ui_ICON_CROTCHET_DOT, glyph);
	/* psy_ui_ICON_QUAVER */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 35));
	psy_ui_icons_add_note_head(glyph, FILL);
	psy_ui_icons_add_stem(glyph);
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 10));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 15));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 15));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 10));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 1));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 0));
	psy_ui_icons_add(self, psy_ui_ICON_QUAVER, glyph);
	/* psy_ui_ICON_QUAVER_DOT */
	glyph = psy_ui_glyph_clone(glyph);
	psy_ui_icons_add_dot(glyph);	
	psy_ui_icons_add(self, psy_ui_ICON_QUAVER_DOT, glyph);
	/* psy_ui_ICON_SEMIQUAVER */	
	glyph = psy_ui_glyph_alloc_init(psy_ui_realsize_make(16, 35));
	psy_ui_icons_add_note_head(glyph, FILL);
	psy_ui_icons_add_stem(glyph);
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 0));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 10));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 30));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 30));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(14, 10));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 2));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 0));
	p = psy_ui_glyph_add_poly(glyph);
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 10));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(15, 20));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(13, 20));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 7));
	psy_ui_polygon_add(p, psy_ui_realpoint_make(10, 5));	
	psy_ui_icons_add(self, psy_ui_ICON_SEMIQUAVER, glyph);
}

void psy_ui_icons_add_note_head(psy_ui_Glyph* glyph, bool fill)
{	
	assert(glyph);
		
	psy_ui_glyph_add_arc(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, 25), psy_ui_realsize_make(10, 10)),
		0, 360, fill);	
}

void psy_ui_icons_add_dot(psy_ui_Glyph* glyph)
{	
	assert(glyph);
		
	psy_ui_glyph_add_rect(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(13, 32), psy_ui_realsize_make(2, 2)),
		FILL);	
}

void psy_ui_icons_add_stem(psy_ui_Glyph* glyph)
{	
	assert(glyph);
	
	psy_ui_glyph_add_rect(glyph, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(9, 0), psy_ui_realsize_make(1, 30)),
		FILL);	
}
