/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinbitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

/* local */
#include "../../uiapp.h"
#include "uiwinapp.h"
#ifdef PSYCLE_TK_WIN32_USE_BPM_READER
#include "../../uibmpreader.h"
#endif

/* prototypes */
static void dev_dispose(psy_ui_win_BitmapImp*);
static int dev_load(psy_ui_win_BitmapImp*, struct psy_ui_Bitmap*, const char* path);
static int dev_loadresource(psy_ui_win_BitmapImp*, uintptr_t resourceid);
static psy_ui_RealSize dev_size(const psy_ui_win_BitmapImp*);
static bool dev_empty(const psy_ui_win_BitmapImp*);
static void dev_settransparency(psy_ui_win_BitmapImp*, psy_ui_Colour);
static void dev_preparemask(psy_ui_win_BitmapImp*, psy_ui_Colour clrtrans);
static void dev_copy(psy_ui_win_BitmapImp* self, const psy_ui_win_BitmapImp* other);
static uintptr_t dev_native(psy_ui_win_BitmapImp*);
static uintptr_t dev_native_mask(psy_ui_win_BitmapImp*);
	
/* vtable */
static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_win_BitmapImp* self)
{
	assert(self);

	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_bitmap_imp_fp_dispose)
			dev_dispose;
		imp_vtable.dev_load =
			(psy_ui_bitmap_imp_fp_load)
			dev_load;
		imp_vtable.dev_loadresource =
			(psy_ui_bitmap_imp_fp_loadresource)
			dev_loadresource;
		imp_vtable.dev_size =
			(psy_ui_bitmap_imp_fp_size)
			dev_size;
		imp_vtable.dev_empty =
			(psy_ui_bitmap_imp_fp_empty)
			dev_empty;
		imp_vtable.dev_settransparency =
			(psy_ui_bitmap_imp_fp_settransparency)
			dev_settransparency;
		imp_vtable.dev_copy =
			(psy_ui_bitmap_imp_fp_copy)
			dev_copy;
		imp_vtable.dev_native =
			(psy_ui_bitmap_imp_fp_native)
			dev_native;
		imp_vtable.dev_native_mask =
			(psy_ui_bitmap_imp_fp_native)
			dev_native_mask;
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}

/* implementation */
void psy_ui_win_bitmapimp_init(psy_ui_win_BitmapImp* self, psy_ui_RealSize size)
{
	assert(self);

	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);	
	self->mask = 0;
	if (size.width == 0 && size.height == 0) {
		self->bitmap = 0;
	} else {
		HDC hdc;

		hdc = GetDC(NULL);
		SaveDC(hdc);
		self->bitmap = CreateCompatibleBitmap(hdc,
			(int)size.width, (int)size.height);
		RestoreDC(hdc, -1);
		ReleaseDC(NULL, hdc);
	}	
}

void dev_dispose(psy_ui_win_BitmapImp* self)
{
	assert(self);

	if (self->bitmap) {
		DeleteObject(self->bitmap);
		self->bitmap = 0;
	}
	if (self->mask) {
		DeleteObject(self->mask);
		self->mask = 0;
	}
}

int dev_load(psy_ui_win_BitmapImp* self, struct psy_ui_Bitmap* bitmap, const char* path)
{
#ifdef PSYCLE_TK_WIN32_USE_BPM_READER
	int rv;

	assert(path);

	psy_ui_BmpReader bmpreader;

	psy_ui_bmpreader_init(&bmpreader, bitmap);
	rv = psy_ui_bmpreader_load(&bmpreader, path);
	psy_ui_bmpreader_dispose(&bmpreader);
	return rv;
#else
	assert(self);

	dev_dispose(self);
	self->mask = 0;
	self->bitmap = (HBITMAP)LoadImage(NULL,
		(LPCTSTR)path,
		IMAGE_BITMAP,
		0, 0,
		LR_DEFAULTSIZE | LR_LOADFROMFILE);	
	return (self->bitmap == 0);
#endif
}

int dev_loadresource(psy_ui_win_BitmapImp* self, uintptr_t resourceid)
{
	psy_ui_WinApp* winapp;	

	assert(self);
	
	dev_dispose(self);
	self->mask = 0;
	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	self->bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	return (self->bitmap == 0);
}

psy_ui_RealSize dev_size(const psy_ui_win_BitmapImp* self)
{
	assert(self);

	if (self->bitmap) {
		BITMAP bitmap;

		GetObject(self->bitmap, sizeof(BITMAP), &bitmap);
		return psy_ui_realsize_make(bitmap.bmWidth, bitmap.bmHeight);
	}
	return psy_ui_realsize_zero();	
}

bool dev_empty(const psy_ui_win_BitmapImp* self)
{
	assert(self);

	return self->bitmap == 0;
}

void dev_settransparency(psy_ui_win_BitmapImp* self, psy_ui_Colour colour)
{
	dev_preparemask(self, colour);
}

void dev_preparemask(psy_ui_win_BitmapImp* self, psy_ui_Colour clrtrans)
{
	BITMAP bm;
	HDC hdcsrc;
	HDC hdcdst;
	HBITMAP hbmsrct;
	HBITMAP hbmdstt;

	/* Get the dimensions of the source bitmap */
	GetObject(self->bitmap, sizeof(BITMAP), &bm);	
	/* Create the mask bitmap */
	if (self->mask) {
		DeleteObject(self->mask);
	}
	self->mask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);		
	/*
	** We will need two DCs to work with. One to hold the Image
	** (the source), and one to hold the mask (destination).
	** When blitting onto a monochrome bitmap from a color, pixels
	** in the source color bitmap that are equal to the background
	** color are blitted as white. All the remaining pixels are
	** blitted as black.
	*/
	hdcsrc = CreateCompatibleDC(NULL);	
	hdcdst = CreateCompatibleDC(NULL);
	/* Load the bitmaps into memory DC */
	hbmsrct = SelectObject(hdcsrc, (HGDIOBJ)self->bitmap);
	hbmdstt = SelectObject(hdcdst, (HGDIOBJ)self->mask);
	/* Change the background to trans color */
	SetBkColor(hdcsrc, psy_ui_colour_colorref(&clrtrans));
	/* This call sets up the mask bitmap. */
	BitBlt(hdcdst, 0, 0, bm.bmWidth, bm.bmHeight, hdcsrc, 0, 0, SRCCOPY);
	/*
	** Now, we need to paint onto the original image, making
	** sure that the "transparent" area is set to black. What
	** we do is AND the monochrome image onto the color Image
	** first. When blitting from mono to color, the monochrome
	** pixel is first transformed as follows:
	** if  1 (black) it is mapped to the color set by SetTextColor().
	** if  0 (white) is is mapped to the color set by SetBkColor().
	** Only then is the raster operation performed.
	*/
	SetTextColor(hdcsrc, RGB(255, 255, 255));
	SetBkColor(hdcsrc, RGB(0, 0, 0));
	BitBlt(hdcsrc, 0, 0, bm.bmWidth, bm.bmHeight, hdcdst, 0, 0, SRCAND);
	/* Clean up by deselecting any objects, and delete the DC's. */
	SelectObject(hdcsrc, hbmsrct);
	SelectObject(hdcdst, hbmdstt);
	DeleteDC(hdcsrc);
	DeleteDC(hdcdst);
}	

void dev_copy(psy_ui_win_BitmapImp* self, const psy_ui_win_BitmapImp* other)
{
	dev_dispose(self);
	self->bitmap = (HBITMAP)CopyImage(other->bitmap, IMAGE_BITMAP, 0, 0,
		LR_DEFAULTSIZE);
}

uintptr_t dev_native(psy_ui_win_BitmapImp* self)
{
	return (uintptr_t)self->bitmap;
}

uintptr_t dev_native_mask(psy_ui_win_BitmapImp* self)
{
	return (uintptr_t)self->mask;
}

#endif /* PSYCLE_TK_WIN32 */
