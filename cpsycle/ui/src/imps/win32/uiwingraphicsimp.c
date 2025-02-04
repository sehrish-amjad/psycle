/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwingraphicsimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "uiwinfontimp.h"
#include "uiwinbitmapimp.h"
#include "../../uiapp.h"
#include "../../uicomponent.h"
/* file */
#include <encoding.h>
/* portable */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_win_g_imp_dispose(psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp*,
	double x, double y, const char*, uintptr_t len);
static void psy_ui_win_g_imp_textout_rectangle(psy_ui_win_GraphicsImp*,
	double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* text, uintptr_t len);
static void psy_ui_win_g_imp_drawrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle);
static void psy_ui_win_g_imp_drawroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle,
	psy_ui_RealSize cornersize);
static psy_ui_Size psy_ui_win_g_imp_textsize(psy_ui_win_GraphicsImp*, const char*, uintptr_t count);
static void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_Colour colour);
static void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp*, const psy_ui_RealRectangle r,
	psy_ui_RealSize cornersize, psy_ui_Colour colour);
static void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp*, psy_ui_RealPoint*,
	uintptr_t numpoints, uint32_t inner, uint32_t outter);
static void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp*, double x1, double y1, double x2, double y2);
static void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, double x, double y);
static void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc);
static void psy_ui_win_g_imp_drawstretchedbitmap(psy_ui_win_GraphicsImp*, psy_ui_Bitmap*, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc);
static void psy_ui_win_g_imp_setbackgroundcolour(psy_ui_win_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp*, uintptr_t mode);
static void psy_ui_win_g_imp_settextcolour(psy_ui_win_GraphicsImp*, psy_ui_Colour colour);
static void psy_ui_win_g_imp_settextalign(psy_ui_win_GraphicsImp*, uintptr_t align);
static void psy_ui_win_g_imp_setcolour(psy_ui_win_GraphicsImp*, psy_ui_Colour colour);
static psy_ui_Colour psy_ui_win_g_imp_textcolour(const psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp*, psy_ui_Font* font);
static const psy_ui_Font* psy_ui_win_g_imp_font(const psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp*, psy_ui_RealPoint pt);
static void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp*, psy_ui_RealPoint control_p1,
	psy_ui_RealPoint control_p2, psy_ui_RealPoint p);
static void psy_ui_win_g_imp_devdrawarc(psy_ui_win_GraphicsImp*,
	psy_ui_RealRectangle, double angle_start, double angle_end);
static void psy_ui_win_g_imp_devdrawsolidarc(psy_ui_win_GraphicsImp*,
	psy_ui_RealRectangle, double angle_start, double angle_end,
	uint32_t inner, uint32_t outter);
static void psy_ui_win_g_devsetlinewidth(psy_ui_win_GraphicsImp*, uintptr_t width);
static unsigned int psy_ui_win_g_devlinewidth(psy_ui_win_GraphicsImp*);
static psy_ui_TextMetric psy_ui_win_g_textmetric(const psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_devsetorigin(psy_ui_win_GraphicsImp*, double x, double y);
static psy_ui_RealPoint psy_ui_win_g_devorigin(const psy_ui_win_GraphicsImp*);
static uintptr_t psy_ui_win_g_dev_gc(psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_dev_setcliprect(psy_ui_win_GraphicsImp*, psy_ui_RealRectangle);
static psy_ui_RealRectangle psy_ui_win_g_dev_cliprect(const psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_dispose(psy_ui_win_GraphicsImp*);
static void psy_ui_win_g_imp_set_encoding(psy_ui_win_GraphicsImp*, psy_Encoding encoding);

static psy_ui_TextMetric convert_text_metric(const TEXTMETRIC*);

/* vtable */
static psy_ui_GraphicsImpVTable win_imp_vtable;
static bool win_imp_vtable_initialized = FALSE;

static void win_imp_vtable_init(psy_ui_win_GraphicsImp* self)
{
	if (!win_imp_vtable_initialized) {
		win_imp_vtable = *self->imp.vtable;
		win_imp_vtable.dev_dispose =
			(psy_ui_fp_graphicsimp_dev_dispose)
			psy_ui_win_g_imp_dispose;
		win_imp_vtable.dev_textout =
			(psy_ui_fp_graphicsimp_dev_textout)
			psy_ui_win_g_imp_textout;
		win_imp_vtable.dev_textoutrectangle =
			(psy_ui_fp_graphicsimp_dev_textoutrectangle)
			psy_ui_win_g_imp_textout_rectangle;
		win_imp_vtable.dev_drawrectangle =
			(psy_ui_fp_graphicsimp_dev_drawrectangle)
			psy_ui_win_g_imp_drawrectangle;
		win_imp_vtable.dev_drawroundrectangle =
			(psy_ui_fp_graphicsimp_dev_drawroundrectangle)
			psy_ui_win_g_imp_drawroundrectangle;
		win_imp_vtable.dev_textsize =
			(psy_ui_fp_graphicsimp_dev_textsize)
			psy_ui_win_g_imp_textsize;
		win_imp_vtable.dev_drawsolidrectangle =
			(psy_ui_fp_graphicsimp_dev_drawsolidrectangle)
			psy_ui_win_g_imp_drawsolidrectangle;
		win_imp_vtable.dev_drawsolidroundrectangle =
			(psy_ui_fp_graphicsimp_dev_drawsolidroundrectangle)
			psy_ui_win_g_imp_drawsolidroundrectangle;
		win_imp_vtable.dev_drawsolidpolygon =
			(psy_ui_fp_graphicsimp_dev_drawsolidpolygon)
			psy_ui_win_g_imp_drawsolidpolygon;
		win_imp_vtable.dev_drawline =
			(psy_ui_fp_graphicsimp_dev_drawline)
			psy_ui_win_g_imp_drawline;
		win_imp_vtable.dev_drawfullbitmap =
			(psy_ui_fp_graphicsimp_dev_drawfullbitmap)
			psy_ui_win_g_imp_drawfullbitmap;
		win_imp_vtable.dev_drawbitmap =
			(psy_ui_fp_graphicsimp_dev_drawbitmap)
			psy_ui_win_g_imp_drawbitmap;
		win_imp_vtable.dev_drawstretchedbitmap =
			(psy_ui_fp_graphicsimp_dev_drawstretchedbitmap)
			psy_ui_win_g_imp_drawstretchedbitmap;
		win_imp_vtable.dev_setbackgroundcolour =
			(psy_ui_fp_graphicsimp_dev_setbackgroundcolour)
			psy_ui_win_g_imp_setbackgroundcolour;
		win_imp_vtable.dev_setbackgroundmode =
			(psy_ui_fp_graphicsimp_dev_setbackgroundmode)
			psy_ui_win_g_imp_setbackgroundmode;
		win_imp_vtable.dev_settextcolour =
			(psy_ui_fp_graphicsimp_dev_settextcolour)
			psy_ui_win_g_imp_settextcolour;
		win_imp_vtable.dev_textcolour =
			(psy_ui_fp_graphicsimp_dev_colour)
			psy_ui_win_g_imp_textcolour;
		win_imp_vtable.dev_settextalign =
			(psy_ui_fp_graphicsimp_dev_settextalign)
			psy_ui_win_g_imp_settextalign;
		win_imp_vtable.dev_setcolour =
			(psy_ui_fp_graphicsimp_dev_setcolour)
			psy_ui_win_g_imp_setcolour;
		win_imp_vtable.dev_setfont =
			(psy_ui_fp_graphicsimp_dev_setfont)
			psy_ui_win_g_imp_setfont;
		win_imp_vtable.dev_font =
			(psy_ui_fp_graphicsimp_dev_font)
			psy_ui_win_g_imp_font;
		win_imp_vtable.dev_moveto =
			(psy_ui_fp_graphicsimp_dev_moveto)
			psy_ui_win_g_imp_moveto;
		win_imp_vtable.dev_curveto =
			(psy_ui_fp_graphicsimp_dev_curveto)
			psy_ui_win_g_imp_devcurveto;
		win_imp_vtable.dev_drawarc =
			(psy_ui_fp_graphicsimp_dev_drawarc)
			psy_ui_win_g_imp_devdrawarc;
		win_imp_vtable.dev_drawsolidarc =
			(psy_ui_fp_graphicsimp_dev_drawsolidarc)
			psy_ui_win_g_imp_devdrawsolidarc;
		win_imp_vtable.dev_setlinewidth =
			(psy_ui_fp_graphicsimp_dev_setlinewidth)
			psy_ui_win_g_devsetlinewidth;
		win_imp_vtable.dev_linewidth =
			(psy_ui_fp_graphicsimp_dev_linewidth)
			psy_ui_win_g_devlinewidth;
		win_imp_vtable.dev_textmetric =
			(psy_ui_fp_graphicsimp_dev_textmetric)
			psy_ui_win_g_textmetric;
		win_imp_vtable.dev_setorigin =
			(psy_ui_fp_graphicsimp_dev_setorigin)
			psy_ui_win_g_devsetorigin;
		win_imp_vtable.dev_origin =
			(psy_ui_fp_graphicsimp_dev_origin)
			psy_ui_win_g_devorigin;
		win_imp_vtable.dev_gc =
			(psy_ui_fp_graphicsimp_dev_gc)
			psy_ui_win_g_dev_gc;
		win_imp_vtable.dev_setcliprect =
			(psy_ui_fp_graphicsimp_dev_setcliprect)
			psy_ui_win_g_dev_setcliprect;
		win_imp_vtable.dev_cliprect =
			(psy_ui_fp_graphicsimp_dev_cliprect)
			psy_ui_win_g_dev_cliprect;
		win_imp_vtable.dev_set_encoding =
			(psy_ui_fp_graphicsimp_dev_set_encoding)
			psy_ui_win_g_imp_set_encoding;
		win_imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &win_imp_vtable;
}
/* implementation */
void psy_ui_win_graphicsimp_init(psy_ui_win_GraphicsImp* self, HDC hdc)
{
	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);
	self->hdc = hdc;
	self->shareddc = TRUE;
	self->colour = psy_ui_style_colour(psy_ui_style_const(psy_ui_STYLE_ROOT));
	self->pen = CreatePen(PS_SOLID, 1, psy_ui_colour_colorref(&self->colour));
	self->brush = 0;
	self->hBrushPrev = 0;
	self->oldbmp = 0;	
	self->penprev = SelectObject(self->hdc, self->pen);
	self->hFontPrev = SelectObject(self->hdc,		
		((psy_ui_win_FontImp*)psy_ui_style_const(psy_ui_STYLE_ROOT)->font.imp)->hfont);
	psy_ui_realpoint_init(&self->org);
	SetStretchBltMode(self->hdc, STRETCH_HALFTONE);
	self->font = NULL;
	self->encoding = PSY_ENCODING_UTF8;	
}

void psy_ui_win_graphicsimp_init_bitmap(psy_ui_win_GraphicsImp* self, psy_ui_Bitmap* bitmap)
{
	HDC hdc;
	psy_ui_win_BitmapImp* imp;

	psy_ui_graphics_imp_init(&self->imp);
	win_imp_vtable_init(self);
	self->imp.vtable = &win_imp_vtable;
	hdc = GetDC(NULL);
	SaveDC(hdc);
	self->hdc = CreateCompatibleDC(hdc);
	self->shareddc = FALSE;
	RestoreDC(hdc, -1);
	ReleaseDC(NULL, hdc);
	imp = (psy_ui_win_BitmapImp*)bitmap->imp_;
	self->oldbmp = SelectObject(self->hdc, imp->bitmap);
	self->colour = psy_ui_style_colour(psy_ui_style_const(psy_ui_STYLE_ROOT));
	self->pen = CreatePen(PS_SOLID, 1, psy_ui_colour_colorref(&self->colour));
	self->brush = 0;
	self->hBrushPrev = 0;
	self->penprev = SelectObject(self->hdc, self->pen);
	self->hFontPrev = SelectObject(self->hdc,
		((psy_ui_win_FontImp*)psy_ui_style_const(psy_ui_STYLE_ROOT)->font.imp)->hfont);	
	psy_ui_realpoint_init(&self->org);
	SetStretchBltMode(self->hdc, STRETCH_HALFTONE);
	self->font = NULL;
	self->encoding = PSY_ENCODING_UTF8;
}

void psy_ui_win_g_imp_dispose(psy_ui_win_GraphicsImp* self)
{
	SelectObject(self->hdc, self->penprev);
	SelectObject(self->hdc, self->hFontPrev);
	if (self->pen) {
		DeleteObject(self->pen);
	}
	if (self->oldbmp) {
		SelectObject(self->hdc, self->oldbmp);
	}
	if (!self->shareddc) {
		DeleteDC(self->hdc);
	}
}

void psy_ui_win_g_imp_textout(psy_ui_win_GraphicsImp* self, double x, double y,
	const char* str, uintptr_t len)
{
	char_dyn_t* out;
	intptr_t outlen;
	intptr_t inlen;

	if (len == 0) {
		return;
	}
	switch (self->encoding) {
	case PSY_ENCODING_NONE:
		TextOut(self->hdc,
			(int)x - (int)(self->org.x),
			(int)y - (int)(self->org.y),
			str, (int)len);
		return;
	case PSY_ENCODING_UTF8:
		out = (char_dyn_t*)malloc((len + 1) * sizeof(char_dyn_t));
		inlen = psy_min(psy_strlen(str), len);
		outlen = inlen;
		if (UTF8Toisolat1(out, &outlen, str, &inlen) == PSY_OK) {
			if (out != NULL && outlen > 0) {
				TextOut(self->hdc,
					(int)x - (int)(self->org.x),
					(int)y - (int)(self->org.y),
					out, (int)outlen);
			}
		}
		free(out);
		out = NULL;
		break;	
	default:
		break;
	}
}

void psy_ui_win_g_imp_textout_rectangle(psy_ui_win_GraphicsImp* self,
	double x, double y, uintptr_t options,
	psy_ui_RealRectangle r, const char* str, uintptr_t len)
{
	RECT rect;
	uintptr_t woptions = 0;
	char_dyn_t* out;
	intptr_t outlen;
	intptr_t inlen;

	if ((options & psy_ui_ETO_OPAQUE) == psy_ui_ETO_OPAQUE) {
		woptions |= ETO_OPAQUE;
	}
	if ((options & psy_ui_ETO_CLIPPED) == psy_ui_ETO_CLIPPED) {
		woptions |= ETO_CLIPPED;
	}
	SetRect(&rect,
		(int)r.left - (int)(self->org.x),
		(int)r.top - (int)(self->org.y),
		(int)r.right - (int)(self->org.x),
		(int)r.bottom - (int)(self->org.y));	
	if (self->encoding == PSY_ENCODING_NONE) {
		ExtTextOut(self->hdc,
			(int)x - (int)(self->org.x),
			(int)y - (int)(self->org.y),
			(UINT)woptions, &rect, str, (int)len, NULL);
	} else {
		out = (char_dyn_t*)malloc((len + 1) * sizeof(char_dyn_t));
		if (out == NULL) {
			assert(0);
			return;
		}
		inlen = psy_min(psy_strlen(str), len);
		outlen = inlen;
		if (UTF8Toisolat1(out, &outlen, str, &inlen) == PSY_OK) {
			ExtTextOut(self->hdc,
				(int)x - (int)(self->org.x),
				(int)y - (int)(self->org.y),
				(UINT)woptions, &rect, out, (int)outlen, NULL);
		}
		free(out);
		out = NULL;
	}
}

psy_ui_Size psy_ui_win_g_imp_textsize(psy_ui_win_GraphicsImp* self, const char* text, uintptr_t count)
{
	psy_ui_Size	rv;

	if (text) {
		SIZE size;

		GetTextExtentPoint(self->hdc, text, (int)count, &size);
		rv.width = psy_ui_value_make_px(size.cx);
		rv.height = psy_ui_value_make_px(size.cy);
	} else {
		rv.width = psy_ui_value_make_px(0);
		rv.height = psy_ui_value_make_px(0);
	}
	return rv;
}

void psy_ui_win_g_imp_drawrectangle(psy_ui_win_GraphicsImp* self,
	const psy_ui_RealRectangle r)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject(NULL_BRUSH);
	hOldBrush = SelectObject(self->hdc, hBrush);
	Rectangle(self->hdc,
		(int)r.left - (int)(self->org.x),
		(int)r.top - (int)(self->org.y),
		(int)r.right - (int)(self->org.x),
		(int)r.bottom - (int)(self->org.y));
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawroundrectangle(psy_ui_win_GraphicsImp* self,
	const psy_ui_RealRectangle r, psy_ui_RealSize cornersize)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject(NULL_BRUSH);
	hOldBrush = SelectObject(self->hdc, hBrush);
	RoundRect(self->hdc,
		(int)r.left - (int)(self->org.x),
		(int)r.top - (int)(self->org.y),
		(int)r.right - (int)(self->org.x),
		(int)r.bottom - (int)(self->org.y),
		(int)cornersize.width, (int)cornersize.height);
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_imp_drawsolidrectangle(psy_ui_win_GraphicsImp* self,
	const psy_ui_RealRectangle r, psy_ui_Colour colour)
{
	if (colour.mode.gc) {
		colour = self->colour;
	}
	if (!colour.mode.transparent) {
		HBRUSH hBrush;
		RECT   rect;
		
		hBrush = CreateSolidBrush(psy_ui_colour_colorref(&colour));
		SetRect(&rect,
			(int)r.left - (int)(self->org.x),
			(int)r.top - (int)(self->org.y),
			(int)r.right - (int)(self->org.x),
			(int)r.bottom - (int)(self->org.y));		
		FillRect(self->hdc, &rect, hBrush);
		DeleteObject(hBrush);
	}
}

void psy_ui_win_g_imp_drawsolidroundrectangle(psy_ui_win_GraphicsImp* self,
	const psy_ui_RealRectangle r, psy_ui_RealSize cornersize,
	psy_ui_Colour colour)
{
	if (colour.mode.gc) {
		colour = self->colour;
	}
	if (!colour.mode.transparent) {
		HBRUSH hBrush;
		HBRUSH hOldBrush;
		HPEN hPen;
		HPEN hOldPen;
		psy_ui_TextMetric tm;
		TEXTMETRIC win_tm;

		hBrush = CreateSolidBrush(psy_ui_colour_colorref(&colour));
		hOldBrush = SelectObject(self->hdc, hBrush);
		hPen = CreatePen(PS_SOLID, 1, psy_ui_colour_colorref(&colour));
		hOldPen = SelectObject(self->hdc, hPen);
		GetTextMetrics(self->hdc, &win_tm);
		tm = convert_text_metric(&win_tm);
		RoundRect(self->hdc,
			(int)r.left - (int)(self->org.x),
			(int)r.top - (int)(self->org.y),
			(int)r.right - (int)(self->org.x),
			(int)r.bottom - (int)(self->org.y),
			(int)cornersize.width, (int)cornersize.height);
		SelectObject(self->hdc, hOldBrush);
		SelectObject(self->hdc, hOldPen);
		DeleteObject(hBrush);
		DeleteObject(hPen);
	}
}

void psy_ui_win_g_imp_drawsolidpolygon(psy_ui_win_GraphicsImp* self,
	psy_ui_RealPoint* pts, uintptr_t numpoints, uint32_t inner,
	uint32_t outter)
{
	if (numpoints > 0) {
		HBRUSH hBrush;
		HBRUSH hBrushPrev;		
		HPEN hPenPrev;
		POINT* wpts;
		uintptr_t i;

		wpts = (POINT*)malloc(sizeof(POINT) * numpoints);
		if (wpts) {
			for (i = 0; i < numpoints; ++i) {
				wpts[i].x = (int)pts[i].x - (int)(self->org.x);
				wpts[i].y = (int)pts[i].y - (int)(self->org.y);
			}
			hBrush = CreateSolidBrush(inner);
			hBrushPrev = SelectObject(self->hdc, hBrush);			
			hPenPrev = (HPEN)SelectObject(self->hdc, GetStockObject(NULL_PEN));			
			Polygon(self->hdc, wpts, (int)numpoints);
			SelectObject(self->hdc, hBrushPrev);
			SelectObject(self->hdc, hPenPrev);
			DeleteObject(hBrush);
			free(wpts);
		}
	}
}

void psy_ui_win_g_imp_drawfullbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y)
{
	HDC hdcmem;
	psy_ui_RealSize size;
	HBITMAP wbitmap;

	assert(self);

	hdcmem = CreateCompatibleDC(self->hdc);
	if (hdcmem == NULL) {
		return;
	}
	wbitmap = (HBITMAP)psy_ui_bitmap_native(bitmap);
	if (wbitmap) {
		SelectObject(hdcmem, wbitmap);
		size = psy_ui_bitmap_size(bitmap);		
		BitBlt(self->hdc,
			(int)x - (int)(self->org.x),
			(int)y - (int)(self->org.y),
			(int)size.width,
			(int)size.height, hdcmem, 0, 0, SRCCOPY);
	}
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_drawbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y, double width,
	double height, double xsrc, double ysrc)
{
	HDC hdcmem;
	HBITMAP mask;
	HBITMAP wbitmap;
	DWORD rop;

	assert(self);
		
	hdcmem = CreateCompatibleDC(self->hdc);
	if (hdcmem == NULL) {
		return;
	}
	mask = (HBITMAP)psy_ui_bitmap_native_mask(bitmap);
	if (mask) {
		uint32_t restoretextcolour;

		/*
		** We are going to paint the two DDB's in sequence to the destination.
		** 1st the monochrome bitmap will be blitted using an AND operation to
		** cut a hole in the destination. The color image will then be ORed
		** with the destination, filling it into the hole, but leaving the
		** surrounding area untouched.
		*/
		SelectObject(hdcmem, mask);
		restoretextcolour = GetTextColor(self->hdc);
		SetTextColor(self->hdc, RGB(0, 0, 0));
		SetBkColor(self->hdc, RGB(255, 255, 255));
		BitBlt(self->hdc,
			(int)x - (int)(self->org.x), (int)y - (int)(self->org.y), 
			(int)width, (int)height, 
			hdcmem, 
			(int)xsrc, (int)ysrc, 
			SRCAND);		
		/* Also note the use of SRCPAINT rather than SRCCOPY. */
		rop = SRCPAINT;
		SetTextColor(self->hdc, restoretextcolour);		
	} else {
		rop = SRCCOPY;		
	}
	wbitmap = (HBITMAP)psy_ui_bitmap_native(bitmap);
	if (wbitmap) {
		SelectObject(hdcmem, wbitmap);
		BitBlt(self->hdc,
			(int)x - (int)(self->org.x), (int)y - (int)(self->org.y),
			(int)width, (int)height,
			hdcmem,
			(int)xsrc, (int)ysrc,
			rop);
	}
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_drawstretchedbitmap(psy_ui_win_GraphicsImp* self,
	psy_ui_Bitmap* bitmap, double x, double y, double width,
	double height, double xsrc, double ysrc, double wsrc, double hsrc)
{
	HDC hdcmem;
	HBITMAP wbitmap;
	HBITMAP mask;	
	DWORD rop;

	assert(self);
	
	hdcmem = CreateCompatibleDC(self->hdc);
	if (hdcmem == NULL) {
		return;
	}
	mask = (HBITMAP)psy_ui_bitmap_native_mask(bitmap);
	if (mask) {
		uint32_t restoretextcolour;
		/*
		** We are going to paint the two DDB's in sequence to the destination.
		**  1st the monochrome bitmap will be blitted using an AND operation to
		**  cut a hole in the destination. The color image will then be ORed
		**  with the destination, filling it into the hole, but leaving the
		**  surrounding area untouched.
		*/
		restoretextcolour = GetTextColor(self->hdc);
		SelectObject(hdcmem, mask);
		SetTextColor(self->hdc, RGB(0, 0, 0));
		SetBkColor(self->hdc, RGB(255, 255, 255));
		StretchBlt(self->hdc,
			(int)x - (int)(self->org.x), (int)y - (int)(self->org.y), 
			(int)width, (int)height, 
			hdcmem, 
			(int)xsrc, (int)ysrc, (int)wsrc, (int)hsrc,
			SRCAND);		
		SetTextColor(self->hdc, restoretextcolour);
		/* Also note the use of SRCPAINT rather than SRCCOPY. */
		rop = SRCPAINT;		
	} else {
		rop = SRCCOPY;
	}
	wbitmap = (HBITMAP)psy_ui_bitmap_native(bitmap);
	if (wbitmap) {
		SelectObject(hdcmem, wbitmap);
		StretchBlt(self->hdc,
			(int)x - (int)(self->org.x), (int)y - (int)(self->org.y),
			(int)width, (int)height,
			hdcmem,
			(int)xsrc, (int)ysrc,
			(int)wsrc, (int)hsrc,
			rop);
	}
	DeleteDC(hdcmem);
}

void psy_ui_win_g_imp_setcolour(psy_ui_win_GraphicsImp* self, psy_ui_Colour colour)
{
	LOGPEN currpen;
	HPEN pen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	self->colour = colour;
	currpen.lopnColor = psy_ui_colour_colorref(&colour);
	pen = CreatePenIndirect(&currpen);
	SelectObject(self->hdc, pen);	
	if (self->pen) {
		DeleteObject(self->pen);
	}
	self->pen = pen;
}

void psy_ui_win_g_imp_setbackgroundmode(psy_ui_win_GraphicsImp* self, uintptr_t mode)
{
	if (mode == psy_ui_TRANSPARENT) {
		SetBkMode(self->hdc, TRANSPARENT);
	} else
		if (mode == psy_ui_OPAQUE) {
			SetBkMode(self->hdc, OPAQUE);
		}
}

void psy_ui_win_g_imp_setbackgroundcolour(psy_ui_win_GraphicsImp* self, psy_ui_Colour colour)
{
	SetBkColor(self->hdc, psy_ui_colour_colorref(&colour));
}

void psy_ui_win_g_imp_settextcolour(psy_ui_win_GraphicsImp* self, psy_ui_Colour colour)
{	
	SetTextColor(self->hdc, psy_ui_colour_colorref(&colour));
}

psy_ui_Colour psy_ui_win_g_imp_textcolour(const psy_ui_win_GraphicsImp* self) 
{
	COLORREF colorref;
	
	colorref = GetTextColor(self->hdc);
	return psy_ui_colour_make(colorref);
}

void psy_ui_win_g_imp_settextalign(psy_ui_win_GraphicsImp* self, uintptr_t align)
{
	SetTextAlign(self->hdc, (UINT)align);
}

void psy_ui_win_g_imp_setfont(psy_ui_win_GraphicsImp* self, psy_ui_Font* font)
{
	self->font = font;
	if (font && ((psy_ui_win_FontImp*)font->imp)->hfont) {
		SelectObject(self->hdc, ((psy_ui_win_FontImp*)font->imp)->hfont);
	}
}

const psy_ui_Font* psy_ui_win_g_imp_font(const psy_ui_win_GraphicsImp* self)
{
	return self->font;
}

void psy_ui_win_g_imp_drawline(psy_ui_win_GraphicsImp* self,
	double x1, double y1, double x2, double y2)
{
	MoveToEx(self->hdc, (int)x1 - (int)(self->org.x), (int)y1 - (int)(self->org.y), NULL);
	LineTo(self->hdc, (int)x2 - (int)(self->org.x), (int)y2 - (int)(self->org.y));
}

void psy_ui_win_g_imp_moveto(psy_ui_win_GraphicsImp* self,
	psy_ui_RealPoint pt)
{
	MoveToEx(self->hdc, (int)pt.x - (int)(self->org.x),
		(int)pt.y - (int)(self->org.y), NULL);
}

void psy_ui_win_g_imp_devcurveto(psy_ui_win_GraphicsImp* self,
	psy_ui_RealPoint control_p1, psy_ui_RealPoint control_p2,
	psy_ui_RealPoint p)
{
	POINT pts[3];

	pts[0].x = (int)control_p1.x - (int)(self->org.x);
	pts[0].y = (int)control_p1.y - (int)(self->org.y);
	pts[1].x = (int)control_p2.x - (int)(self->org.x);
	pts[1].y = (int)control_p2.y - (int)(self->org.y);
	pts[2].x = (int)p.x - (int)(self->org.x);
	pts[2].y = (int)p.y - (int)(self->org.y);
	PolyBezierTo(self->hdc, pts, 3);
}

void psy_ui_win_g_imp_devdrawarc(psy_ui_win_GraphicsImp* self,
	psy_ui_RealRectangle r, double angle_start, double angle_end)
{
	int x, y, w, h, x3, y3, x4, y4;
	double mul;

	x = (int)r.left - (int)(self->org.x);
	y = (int)r.right - (int)(self->org.y);
	w = (int)(r.right - r.left);
	h = (int)(r.bottom - r.top);
	mul = 1 / 180 * 3.14159265358979323846;
	x3 = x + w / 2 + (int)(w * cos(angle_start * mul));
	y3 = y + h / 2 - (int)(h * sin(angle_start * mul));
	x4 = x + w / 2 + (int)(w * cos(angle_end * mul));
	y4 = y + h / 2 - (int)(h * sin(angle_end * mul));
	Arc(self->hdc,
		(int)(r.left) - (int)(self->org.x),
		(int)(r.top) - (int)(self->org.y),
		(int)(r.right) - (int)(self->org.x),
		(int)(r.bottom) - (int)(self->org.y),
		x3, y3, x4, y4);
}

void psy_ui_win_g_imp_devdrawsolidarc(psy_ui_win_GraphicsImp* self,
	psy_ui_RealRectangle r, double angle_start, double angle_end,
	uint32_t inner, uint32_t outter)
{
	int x, y, w, h, x3, y3, x4, y4;
	double mul;
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	x = (int)r.left - (int)(self->org.x);
	y = (int)r.right - (int)(self->org.y);
	w = (int)(r.right - r.left);
	h = (int)(r.bottom - r.top);
	mul = 1 / 180 * 3.14159265358979323846;
	x3 = x + w / 2 + (int)(w * cos(angle_start * mul));
	y3 = y + h / 2 - (int)(h * sin(angle_start * mul));
	x4 = x + w / 2 + (int)(w * cos(angle_end * mul));
	y4 = y + h / 2 - (int)(h * sin(angle_end * mul));
	hBrush = CreateSolidBrush(inner);
	hOldBrush = SelectObject(self->hdc, hBrush);
	Chord(self->hdc,
		(int)(r.left) - (int)(self->org.x),
		(int)(r.top) - (int)(self->org.y),
		(int)(r.right) - (int)(self->org.x),
		(int)(r.bottom) - (int)(self->org.y),
		x3, y3, x4, y4);
	SelectObject(self->hdc, hOldBrush);
}

void psy_ui_win_g_devsetlinewidth(psy_ui_win_GraphicsImp* self, uintptr_t width)
{
	LOGPEN currpen;
	HPEN pen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	currpen.lopnWidth.x = (int)width;
	pen = CreatePenIndirect(&currpen);
	SelectObject(self->hdc, pen);
	if (self->pen) {
		DeleteObject(self->pen);
	}
	self->pen = pen;
}

unsigned int psy_ui_win_g_devlinewidth(psy_ui_win_GraphicsImp* self)
{
	LOGPEN currpen;

	GetObject(self->pen, sizeof(LOGPEN), &currpen);
	return currpen.lopnWidth.x;
}

psy_ui_TextMetric psy_ui_win_g_textmetric(const psy_ui_win_GraphicsImp* self)
{
	psy_ui_TextMetric rv;
	TEXTMETRIC tm;
		
	GetTextMetrics(self->hdc, &tm);	
	rv.tmHeight = tm.tmHeight;
	rv.tmAscent = tm.tmAscent;
	rv.tmDescent = tm.tmDescent;
	rv.tmInternalLeading = tm.tmInternalLeading;
	rv.tmExternalLeading = tm.tmExternalLeading;
	rv.tmAveCharWidth = tm.tmAveCharWidth;
	rv.tmMaxCharWidth = tm.tmMaxCharWidth;
	rv.tmWeight = tm.tmWeight;
	rv.tmOverhang = tm.tmOverhang;
	rv.tmDigitizedAspectX = tm.tmDigitizedAspectX;
	rv.tmDigitizedAspectY = tm.tmDigitizedAspectY;
	rv.tmFirstChar = tm.tmFirstChar;
	rv.tmLastChar = tm.tmLastChar;
	rv.tmDefaultChar = tm.tmDefaultChar;
	rv.tmBreakChar = tm.tmBreakChar;
	rv.tmItalic = tm.tmItalic;
	rv.tmUnderlined = tm.tmUnderlined;
	rv.tmStruckOut = tm.tmStruckOut;
	rv.tmPitchAndFamily = tm.tmPitchAndFamily;
	rv.tmCharSet = tm.tmCharSet;
	return rv;
}

psy_ui_TextMetric convert_text_metric(const TEXTMETRIC* tm)
{
	psy_ui_TextMetric rv;

	rv.tmHeight = tm->tmHeight;
	rv.tmAscent = tm->tmAscent;
	rv.tmDescent = tm->tmDescent;
	rv.tmInternalLeading = tm->tmInternalLeading;
	rv.tmExternalLeading = tm->tmExternalLeading;
	rv.tmAveCharWidth = tm->tmAveCharWidth;
	rv.tmMaxCharWidth = tm->tmMaxCharWidth;
	rv.tmWeight = tm->tmWeight;
	rv.tmOverhang = tm->tmOverhang;
	rv.tmDigitizedAspectX = tm->tmDigitizedAspectX;
	rv.tmDigitizedAspectY = tm->tmDigitizedAspectY;
	rv.tmFirstChar = tm->tmFirstChar;
	rv.tmLastChar = tm->tmLastChar;
	rv.tmDefaultChar = tm->tmDefaultChar;
	rv.tmBreakChar = tm->tmBreakChar;
	rv.tmItalic = tm->tmItalic;
	rv.tmUnderlined = tm->tmUnderlined;
	rv.tmStruckOut = tm->tmStruckOut;
	rv.tmPitchAndFamily = tm->tmPitchAndFamily;
	rv.tmCharSet = tm->tmCharSet;
	return rv;
}

void psy_ui_win_g_devsetorigin(psy_ui_win_GraphicsImp* self, double x, double y)
{
	self->org.x = x;
	self->org.y = y;
}

psy_ui_RealPoint psy_ui_win_g_devorigin(const psy_ui_win_GraphicsImp* self)
{
	return self->org;
}

uintptr_t psy_ui_win_g_dev_gc(psy_ui_win_GraphicsImp* self)
{
	return (uintptr_t)self->hdc;
}

void psy_ui_win_g_dev_setcliprect(psy_ui_win_GraphicsImp* self, psy_ui_RealRectangle clip)
{
	self->clip = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			clip.left - (int)(self->org.x),
			clip.top - (int)(self->org.y)),
		psy_ui_realsize_make(
			clip.right - clip.left,
			clip.bottom - clip.top));
	if ((((int)clip.right - (int)clip.left) == 0) ||
		(((int)clip.bottom - (int)clip.top) == 0)) {
		SelectClipRgn(self->hdc, NULL);
	} else {
		HRGN rgn;

		rgn = CreateRectRgn(
			(int)clip.left - (int)(self->org.x),
			(int)clip.top - (int)(self->org.y),
			(int)clip.right - (int)(self->org.x),
			(int)clip.bottom - (int)(self->org.y));
		SelectClipRgn(self->hdc, rgn);
		DeleteObject(rgn);
	}
}

psy_ui_RealRectangle psy_ui_win_g_dev_cliprect(const psy_ui_win_GraphicsImp* self)
{
	psy_ui_RealRectangle rv;
	HRGN rgn;

	rv = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(
			self->clip.left + (int)(self->org.x),
			self->clip.top + (int)(self->org.y)),
		psy_ui_realsize_make(
			self->clip.right - self->clip.left,
			self->clip.bottom - self->clip.top));
	return rv;

	rgn = CreateRectRgn(0, 0, 0, 0);
	if (GetClipRgn(self->hdc, rgn) != -1) {
		RECT rc;

		GetRgnBox(rgn, &rc);
		rv = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				rc.left + (int)(self->org.x),
				rc.top + (int)(self->org.y)),
			psy_ui_realsize_make(
				rc.right - rc.left,
				rc.bottom - rc.top));
		rv = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				self->clip.left + (int)(self->org.x),
				self->clip.top + (int)(self->org.y)),
			psy_ui_realsize_make(
				self->clip.right - self->clip.left,
				self->clip.bottom - self->clip.top));
		return rv;
	} else {
		rv = psy_ui_realrectangle_zero();
	}
	DeleteObject(rgn);
	return rv;
}

void psy_ui_win_g_imp_set_encoding(psy_ui_win_GraphicsImp* self, psy_Encoding encoding)
{
	self->encoding = encoding;
}

#endif /* PSYCLE_USE_TK == PSYCLE_TK_WIN32 */
