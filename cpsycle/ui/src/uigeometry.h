/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_GEOMETRY_H
#define psy_ui_GEOMETRY_H

#include "uivalue.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void psy_ui_geometry_init(void);


typedef struct psy_ui_IntPoint {
	intptr_t x;
	intptr_t y;
} psy_ui_IntPoint;

INLINE void psy_ui_intpoint_init(psy_ui_IntPoint* self)
{
	self->x = 0;
	self->y = 0;
}

INLINE psy_ui_IntPoint psy_ui_intpoint_make(intptr_t x, intptr_t y)
{
	psy_ui_IntPoint rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

INLINE psy_ui_IntPoint psy_ui_intpoint_zero(void)
{
	psy_ui_IntPoint rv;

	rv.x = 0;
	rv.y = 0;
	return rv;
}

typedef struct psy_ui_RealPoint {
	double x;
	double y;
} psy_ui_RealPoint;

INLINE void psy_ui_realpoint_init(psy_ui_RealPoint* self)
{
	/* assume IEEE754 */
	memset(self, 0, sizeof(psy_ui_RealPoint));
}

INLINE void psy_ui_realpoint_init_all(psy_ui_RealPoint* self, double x, double y)
{
	/* assume IEEE754 */
	self->x = x;
	self->y = y;
}

INLINE psy_ui_RealPoint psy_ui_realpoint_make(double x, double y)
{
	psy_ui_RealPoint rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

INLINE psy_ui_RealPoint* psy_ui_realpoint_move(psy_ui_RealPoint* self,
	double x, double y)
{	
	self->x = x;
	self->y = y;
	return self;
}

void psy_ui_realpoint_floor(psy_ui_RealPoint* self);


INLINE psy_ui_RealPoint psy_ui_realpoint_zero(void)
{
	extern psy_ui_RealPoint psy_ui_internal_realpoint_zero;

	return psy_ui_internal_realpoint_zero;
}

INLINE bool psy_ui_realpoint_equal(const psy_ui_RealPoint* self, const psy_ui_RealPoint* other)
{
	return self->x == other->x && self->y == other->y;
}

INLINE void psy_ui_realpoint_add(psy_ui_RealPoint* self, psy_ui_RealPoint other)
{
	self->x += other.x;
	self->y += other.y;
}

INLINE void psy_ui_realpoint_sub(psy_ui_RealPoint* self, psy_ui_RealPoint other)
{
	self->x -= other.x;
	self->y -= other.y;
}

typedef struct psy_ui_RealSize {
	double width;
	double height;
} psy_ui_RealSize;

INLINE void psy_ui_realsize_init(psy_ui_RealSize* self)
{
	self->width = 0.0;
	self->height = 0.0;
}

INLINE psy_ui_RealSize psy_ui_realsize_make(double width, double height)
{
	psy_ui_RealSize rv;

	rv.width = width;
	rv.height = height;
	return rv;
}

INLINE psy_ui_RealSize psy_ui_realsize_zero(void)
{
	return psy_ui_realsize_make(0.0, 0.0);
}

INLINE bool psy_ui_realsize_equals(const psy_ui_RealSize* self,
	psy_ui_RealSize other)
{
	return (self->height == other.height && self->width == other.width);
}

INLINE double psy_ui_realsize_width(const psy_ui_RealSize* self)
{
	return self->width;
}


INLINE double psy_ui_realsize_height(const psy_ui_RealSize* self)
{
	return self->height;
}

/* psy_ui_RealRectangle */

struct psy_List;

typedef struct psy_ui_RealRectangle {
	double left;
	double top;
	double right;
	double bottom;	
} psy_ui_RealRectangle;

INLINE void psy_ui_realrectangle_init(psy_ui_RealRectangle* self)
{
	/* assume IEEE754 */
	memset(self, 0, sizeof(psy_ui_RealRectangle));	
}

psy_ui_RealRectangle* psy_ui_realrectangle_alloc(void);
psy_ui_RealRectangle* psy_ui_realrectangle_alloc_init(void);


INLINE void psy_ui_realrectangle_init_all(psy_ui_RealRectangle* self,
	psy_ui_RealPoint topleft,
	psy_ui_RealSize size)
{
	self->left = topleft.x;
	self->top = topleft.y;
	self->right = topleft.x + size.width;
	self->bottom = topleft.y + size.height;	
}

INLINE psy_ui_RealRectangle psy_ui_realrectangle_make(psy_ui_RealPoint topleft,
	psy_ui_RealSize size)
{
	psy_ui_RealRectangle rv;

	rv.left = topleft.x;
	rv.top = topleft.y;
	rv.right = topleft.x + size.width;
	rv.bottom = topleft.y + size.height;
	return rv;
}

INLINE psy_ui_RealRectangle psy_ui_realrectangle_zero(void)
{
	return psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
		psy_ui_realsize_zero());
}

INLINE double psy_ui_realrectangle_width(const psy_ui_RealRectangle* self)
{
	return self->right - self->left;
}

INLINE void psy_ui_realrectangle_setwidth(psy_ui_RealRectangle* self,
	double width)
{
	self->right = self->left + width;
}

INLINE void psy_ui_realrectangle_resize(psy_ui_RealRectangle* self, double width,
	double height)
{
	self->right = self->left + width;
	self->bottom = self->top + height;
}

INLINE void psy_ui_realrectangle_set_left_resize(psy_ui_RealRectangle* self,
	double left)
{
	double width;

	width = psy_ui_realrectangle_width(self);
	self->left = left;
	self->right = left + width;
}

INLINE double psy_ui_realrectangle_height(const psy_ui_RealRectangle* self)
{
	return self->bottom - self->top;
}

INLINE void psy_ui_realrectangle_set_top_resize(psy_ui_RealRectangle* self, double top)
{
	double height;

	height = psy_ui_realrectangle_height(self);
	self->top = top;
	self->bottom = top + height;
}

INLINE void psy_ui_realrectangle_set_left(psy_ui_RealRectangle* self, double left)
{	
	self->left = left;	
}

INLINE double psy_ui_realrectangle_left(const psy_ui_RealRectangle* self)
{
	return self->left;
}

INLINE void psy_ui_realrectangle_setright_resize(psy_ui_RealRectangle* self, double right)
{
	double width;

	width = psy_ui_realrectangle_width(self);
	self->left = self->right - width;
	self->right = right;
}

INLINE void psy_ui_realrectangle_setright(psy_ui_RealRectangle* self, double right)
{
	self->right = right;
}

INLINE double psy_ui_realrectangle_right(const psy_ui_RealRectangle* self)
{
	return self->right;
}

INLINE void psy_ui_realrectangle_setheight(psy_ui_RealRectangle* self, double height)
{
	self->bottom = self->top + height;
}

INLINE psy_ui_RealPoint psy_ui_realrectangle_topleft(const psy_ui_RealRectangle* self)
{
	return psy_ui_realpoint_make(self->left, self->top);
}

INLINE psy_ui_RealSize psy_ui_realrectangle_size(const psy_ui_RealRectangle* self)
{
	return psy_ui_realsize_make(
		psy_ui_realrectangle_width(self),
		psy_ui_realrectangle_height(self));
}

INLINE bool psy_ui_realrectangle_intersect(const psy_ui_RealRectangle* self,
	psy_ui_RealPoint pt)
{
	return (pt.x >= self->left && pt.x < self->right&&
		pt.y >= self->top && pt.y < self->bottom);
}

void psy_ui_realrectangle_trace(const psy_ui_RealRectangle*);

bool psy_ui_realrectangle_intersect_segment(const psy_ui_RealRectangle*,
	double x1, double y1, double x2, double y2);
int psy_ui_realrectangle_intersect_rectangle(const psy_ui_RealRectangle*,
	const psy_ui_RealRectangle* other);
void psy_ui_realrectangle_union(psy_ui_RealRectangle*,
	const psy_ui_RealRectangle* other);
bool psy_ui_realrectangle_intersection(psy_ui_RealRectangle*,
	const psy_ui_RealRectangle* other);
void psy_ui_realrectangle_expand(psy_ui_RealRectangle*,
	double top, double right, double bottom, double left);
void psy_ui_realrectangle_expand_all(psy_ui_RealRectangle*, double margin);
void psy_ui_realrectangle_move(psy_ui_RealRectangle*, psy_ui_RealPoint);
void psy_ui_realrectangle_set_topleft(psy_ui_RealRectangle*,
	psy_ui_RealPoint topleft);
struct psy_List* psy_ui_realrectangle_diff(const psy_ui_RealRectangle*,
	const psy_ui_RealRectangle* other);

void psy_ui_error(const char* err, const char* shorterr);

INLINE bool psy_ui_realrectangle_equal(psy_ui_RealRectangle* self,
	psy_ui_RealRectangle* other)
{
	return memcmp(self, other, sizeof(psy_ui_RealRectangle)) == 0;
}

INLINE bool psy_ui_realrectangle_empty(const psy_ui_RealRectangle* self)
{
	return ((self->right - self->left) == 0.0) ||
		((self->bottom - self->top) == 0.0);
}

typedef struct psy_ui_IntSize {
	intptr_t width;
	intptr_t height;
} psy_ui_IntSize;

INLINE psy_ui_IntSize psy_ui_intsize_make(intptr_t width, intptr_t height)
{
	psy_ui_IntSize rv;

	rv.width = width;
	rv.height = height;
	return rv;
}


typedef struct psy_ui_Point {
	psy_ui_Value x;
	psy_ui_Value y;
} psy_ui_Point;

INLINE void psy_ui_point_init(psy_ui_Point* self)
{
	self->x = psy_ui_value_zero();
	self->y = psy_ui_value_zero();
}

INLINE void psy_ui_point_init_all(psy_ui_Point* self, psy_ui_Value x, psy_ui_Value y)
{
	self->x = x;
	self->y = y;
}

INLINE psy_ui_Point psy_ui_point_make(psy_ui_Value x, psy_ui_Value y)
{
	psy_ui_Point rv;

	rv.x = x;
	rv.y = y;
	return rv;
}

INLINE psy_ui_Point psy_ui_point_make_em(double x, double y)
{
	psy_ui_Point rv;

	rv.x = psy_ui_value_make_ew(x);
	rv.y = psy_ui_value_make_eh(y);
	return rv;
}

INLINE psy_ui_Point psy_ui_point_make_real(psy_ui_RealPoint pt)
{
	psy_ui_Point rv;

	rv.x = psy_ui_value_make_px(pt.x);
	rv.y = psy_ui_value_make_px(pt.y);
	return rv;
}

INLINE psy_ui_Point psy_ui_point_make_px(double x, double y)
{
	psy_ui_Point rv;

	rv.x = psy_ui_value_make_px(x);
	rv.y = psy_ui_value_make_px(y);
	return rv;
}

INLINE psy_ui_Point psy_ui_point_makeem(double x, double y)
{
	psy_ui_Point rv;

	rv.x = psy_ui_value_make_ew(x);
	rv.y = psy_ui_value_make_eh(y);
	return rv;
}

INLINE psy_ui_Point psy_ui_point_zero(void)
{
	extern psy_ui_Point psy_ui_internal_point_zero;

	return psy_ui_internal_point_zero;
}

typedef struct psy_ui_Size {
	psy_ui_Value width;
	psy_ui_Value height;
} psy_ui_Size;

INLINE void psy_ui_size_setroundmode(psy_ui_Size* self, psy_ui_Round round)
{
	psy_ui_value_setroundmode(&self->width, round);
	psy_ui_value_setroundmode(&self->height, round);
}

INLINE void psy_ui_size_init(psy_ui_Size* self)
{
	psy_ui_value_init(&self->width);
	psy_ui_value_init(&self->height);
	psy_ui_size_setroundmode(self, psy_ui_ROUND_FLOOR);
}

INLINE void psy_ui_size_init_all(psy_ui_Size* self, psy_ui_Value width,
	psy_ui_Value height)
{
	self->width = width;
	self->height = height;
}

INLINE void psy_ui_size_init_em(psy_ui_Size* self, double width, double height)
{
	self->width = psy_ui_value_make_ew(width);
	self->height = psy_ui_value_make_eh(height);
	psy_ui_size_setroundmode(self, psy_ui_ROUND_FLOOR);
}

INLINE void psy_ui_size_init_perc(psy_ui_Size* self, double width, double height)
{
	self->width = psy_ui_value_make_pw(width);
	self->height = psy_ui_value_make_ph(height);
	psy_ui_size_setroundmode(self, psy_ui_ROUND_FLOOR);
}

INLINE void psy_ui_size_init_px(psy_ui_Size* self, double width, double height)
{
	self->width = psy_ui_value_make_px(width);
	self->height = psy_ui_value_make_px(height);
	psy_ui_size_setroundmode(self, psy_ui_ROUND_FLOOR);
}

INLINE psy_ui_Size psy_ui_size_make(psy_ui_Value width, psy_ui_Value height)
{
	psy_ui_Size rv;

	rv.width = width;
	rv.height = height;
	return rv;
}

INLINE psy_ui_Size psy_ui_size_make_px(double width, double height)
{
	psy_ui_Size rv;

	psy_ui_size_init_px(&rv, width, height);
	return rv;
}

INLINE psy_ui_Size psy_ui_size_make_em(double width, double height)
{
	psy_ui_Size rv;

	psy_ui_size_init_em(&rv, width, height);
	return rv;
}

INLINE psy_ui_Size psy_ui_size_make_perc(double width, double height)
{
	psy_ui_Size rv;

	psy_ui_size_init_perc(&rv, width, height);
	return rv;
}

INLINE psy_ui_Size psy_ui_size_make_real(psy_ui_RealSize size)
{
	psy_ui_Size rv;

	rv.width = psy_ui_value_make_px(size.width);
	rv.height = psy_ui_value_make_px(size.height);
	psy_ui_size_setroundmode(&rv, psy_ui_ROUND_FLOOR);
	return rv;
}

INLINE void psy_ui_size_setpx(psy_ui_Size* self, double width, double height)
{
	psy_ui_value_setpx(&self->width, width);
	psy_ui_value_setpx(&self->height, height);	
}

INLINE void psy_ui_size_setreal(psy_ui_Size* self, psy_ui_RealSize size)
{	
	psy_ui_value_setpx(&self->width, size.width);
	psy_ui_value_setpx(&self->height, size.height);	
}

INLINE void psy_ui_size_setem(psy_ui_Size* self, double width, double height)
{
	psy_ui_value_setew(&self->width, width);
	psy_ui_value_seteh(&self->height, height);
}

INLINE psy_ui_Size psy_ui_size_zero(void)
{
	return psy_ui_size_make_em(0.0, 0.0);
}

INLINE bool psy_ui_size_iszero(const psy_ui_Size* self)
{		
	return psy_ui_value_is_zero(&self->width) &&
		psy_ui_value_is_zero(&self->height);
}

INLINE bool psy_ui_issizezero(psy_ui_Size size)
{
	return psy_ui_size_iszero(&size);
}

INLINE psy_ui_RealSize psy_ui_size_px(const psy_ui_Size* self,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	psy_ui_RealSize rv;

	rv.width = psy_ui_value_px(&self->width, tm, pesize);
	rv.height = psy_ui_value_px(&self->height, tm, pesize);
	return rv;
}

INLINE psy_ui_IntSize psy_ui_intsize_init_size(psy_ui_Size size,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	psy_ui_IntSize rv;

	rv.width = (intptr_t)psy_ui_value_px(&size.width, tm, pesize);
	rv.height = (intptr_t)psy_ui_value_px(&size.height, tm, pesize);
	return rv;
}

INLINE psy_ui_Size psy_ui_max_size(psy_ui_Size lhs, psy_ui_Size rhs,
	const psy_ui_TextMetric* tm, const psy_ui_Size* pesize)
{
	psy_ui_Size rv;

	if (psy_ui_value_comp(&lhs.width, &rhs.width, tm, pesize) > 0) {
		rv.width = lhs.width;
	} else {
		rv.width = rhs.width;
	}
	if (psy_ui_value_comp(&lhs.height, &rhs.height, tm, pesize) > 0) {
		rv.height = lhs.height;
	} else {
		rv.height = rhs.height;
	}
	return rv;
}

INLINE bool psy_ui_size_has_percent(const psy_ui_Size* size)
{
	return (size->width.unit == psy_ui_UNIT_PW ||
		size->width.unit == psy_ui_UNIT_PH ||
		size->height.unit == psy_ui_UNIT_PH ||
		size->height.unit == psy_ui_UNIT_PW);
}

/* psy_ui_Rectangle */
typedef struct psy_ui_Rectangle {
	psy_ui_Point topleft;
	psy_ui_Size size;
} psy_ui_Rectangle;

INLINE void psy_ui_rectangle_init(psy_ui_Rectangle* self)
{
	psy_ui_point_init(&self->topleft);
	psy_ui_size_init(&self->size);		
}

INLINE psy_ui_Rectangle psy_ui_rectangle_zero(void)
{
	psy_ui_Rectangle rv;

        psy_ui_rectangle_init(&rv);
        return rv;	
}

INLINE void psy_ui_rectangle_deactivate(psy_ui_Rectangle* self)
{
	self->topleft.x.set = FALSE;
	self->topleft.y.set = FALSE;
	self->size.width.set = FALSE;
	self->size.height.set = FALSE;
}

INLINE void psy_ui_rectangle_activate(psy_ui_Rectangle* self)
{
	self->topleft.x.set = TRUE;
	self->topleft.y.set = TRUE;
	self->size.width.set = TRUE;
	self->size.height.set = TRUE;
}

INLINE bool psy_ui_rectangle_is_all_active(const psy_ui_Rectangle* self)
{
	return (self->topleft.x.set &&
		self->topleft.y.set &&
		self->size.width.set &&
		self->size.height.set);
}

INLINE psy_ui_Rectangle psy_ui_rectangle_make(psy_ui_Point topleft,
	psy_ui_Size size)
{
	psy_ui_Rectangle rv;

	rv.topleft = topleft;
	rv.size = size;
	return rv;
}

INLINE psy_ui_Rectangle psy_ui_rectangle_make_px(const psy_ui_RealRectangle* r)
{
	psy_ui_Rectangle rv;

	rv.topleft = psy_ui_point_make_px(r->left, r->top);	
	rv.size = psy_ui_size_make(
		psy_ui_value_make_px(psy_ui_realrectangle_width(r)),
		psy_ui_value_make_px(psy_ui_realrectangle_height(r)));
	return rv;
}

INLINE psy_ui_Size psy_ui_rectangle_size(const psy_ui_Rectangle* self)
{
	return self->size;
}

typedef struct psy_ui_Position {
	psy_ui_Rectangle* rectangle;	
} psy_ui_Position;

void psy_ui_position_init(psy_ui_Position*);
void psy_ui_position_dispose(psy_ui_Position*);

void psy_ui_position_set_rectangle(psy_ui_Position*, psy_ui_Rectangle);
void psy_ui_position_set_topleft(psy_ui_Position*, psy_ui_Point);
void psy_ui_position_set_size(psy_ui_Position*, psy_ui_Size);
psy_ui_Size psy_ui_position_size(const psy_ui_Position*);

INLINE psy_ui_Rectangle psy_ui_position_rectangle(const psy_ui_Position* self)
{
	return *self->rectangle;
}

bool psy_ui_position_is_active(const psy_ui_Position*);

typedef struct psy_ui_Margin {
	psy_ui_Value top;
	psy_ui_Value right;
	psy_ui_Value bottom;
	psy_ui_Value left;
} psy_ui_Margin;

void psy_ui_margin_init(psy_ui_Margin*);
void psy_ui_margin_init_all(psy_ui_Margin*, psy_ui_Value top, psy_ui_Value right,
	psy_ui_Value bottom, psy_ui_Value left);
void psy_ui_margin_init_em(psy_ui_Margin*, double top,
	double right, double bottom, double left);
void psy_ui_margin_init_px(psy_ui_Margin*, double top,
	double right, double bottom, double left);
void psy_ui_margin_init_perc(psy_ui_Margin*, double top,
	double right, double bottom, double left);
void psy_ui_margin_set_top(psy_ui_Margin*, psy_ui_Value value);
void psy_ui_margin_set_right(psy_ui_Margin*, psy_ui_Value value);
void psy_ui_margin_set_bottom(psy_ui_Margin*, psy_ui_Value value);
void psy_ui_margin_set_left(psy_ui_Margin*, psy_ui_Value value);
psy_ui_Value psy_ui_margin_width(psy_ui_Margin*, const psy_ui_TextMetric*,
	const psy_ui_Size* pesize);
double psy_ui_margin_width_px(psy_ui_Margin*, const psy_ui_TextMetric*,
	const psy_ui_Size* pesize);
psy_ui_Value psy_ui_margin_height(psy_ui_Margin*, const psy_ui_TextMetric*,
	const psy_ui_Size* pesize);
double psy_ui_margin_height_px(psy_ui_Margin*, const psy_ui_TextMetric*,
	const psy_ui_Size* pesize);

INLINE bool psy_ui_margin_is_zero(const psy_ui_Margin* self)
{
	return psy_ui_value_is_zero(&self->left) &&
		psy_ui_value_is_zero(&self->top) &&
		psy_ui_value_is_zero(&self->right) &&
		psy_ui_value_is_zero(&self->bottom);
}

INLINE psy_ui_Margin psy_ui_margin_make(psy_ui_Value top, psy_ui_Value right,
	psy_ui_Value bottom, psy_ui_Value left)
{
	psy_ui_Margin rv;

	psy_ui_margin_init_all(&rv, top, right, bottom, left);
	return rv;
}

INLINE psy_ui_Margin psy_ui_margin_make_em(double top,
	double right, double bottom, double left)
{
	psy_ui_Margin rv;

	psy_ui_margin_init_em(&rv, top, right, bottom, left);
	return rv;
}

INLINE psy_ui_Margin psy_ui_margin_make_px(double top,
	double right, double bottom, double left)
{
	psy_ui_Margin rv;

	psy_ui_margin_init_px(&rv, top, right, bottom, left);
	return rv;
}

INLINE psy_ui_Margin psy_ui_margin_make_perc(double top,
	double right, double bottom, double left)
{
	psy_ui_Margin rv;

	psy_ui_margin_init_perc(&rv, top, right, bottom, left);
	return rv;
}

INLINE psy_ui_Margin psy_ui_margin_zero(void)
{
	psy_ui_Margin rv;

	psy_ui_margin_init(&rv);
	return rv;
}

INLINE void psy_ui_margin_set_round_mode(psy_ui_Margin* self,
	psy_ui_Round round)
{
	psy_ui_value_setroundmode(&self->top, round);
	psy_ui_value_setroundmode(&self->right, round);
	psy_ui_value_setroundmode(&self->bottom, round);
	psy_ui_value_setroundmode(&self->left, round);
}

INLINE void psy_ui_margin_deactivate(psy_ui_Margin* self)
{
	self->top.set = FALSE;
	self->right.set = FALSE;
	self->bottom.set = FALSE;
	self->left.set = FALSE;
}

INLINE void psy_ui_margin_activate(psy_ui_Margin* self)
{
	self->top.set = TRUE;
	self->right.set = TRUE;
	self->bottom.set = TRUE;
	self->left.set = TRUE;
}

INLINE bool psy_ui_margin_is_set(const psy_ui_Margin* self)
{
	return (psy_ui_value_is_set(&self->top) ||
		psy_ui_value_is_set(&self->right) ||
		psy_ui_value_is_set(&self->bottom) ||
		psy_ui_value_is_set(&self->left));
}

typedef struct psy_ui_RealMargin {
	double top;
	double right;
	double bottom;
	double left;
} psy_ui_RealMargin;

INLINE void psy_ui_realmargin_init_margin(psy_ui_RealMargin* self,
	const psy_ui_Margin* other,
	const psy_ui_TextMetric* tm,
	const psy_ui_Size* pesize)
{
	self->top = psy_ui_value_px(&other->top, tm, pesize);
	self->right = psy_ui_value_px(&other->right, tm, pesize);
	self->bottom = psy_ui_value_px(&other->bottom, tm, pesize);
	self->left = psy_ui_value_px(&other->left, tm, pesize);
}

void psy_ui_realmargin_floor(psy_ui_RealMargin* self);

INLINE double psy_ui_realmargin_width(const psy_ui_RealMargin* self)
{
	return self->left + self->right;
}

INLINE double psy_ui_realmargin_height(const psy_ui_RealMargin* self)
{
	return self->top + self->bottom;
}

INLINE bool psy_ui_realmargin_iszero(const psy_ui_RealMargin* self)
{
	return (self->left == 0.0 &&
		self->top == 0.0 &&
		self->right == 0.0 &&
		self->bottom == 0.0);
}




#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GEOMETRY_H */
