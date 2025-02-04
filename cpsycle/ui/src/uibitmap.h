/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_BITMAP_H
#define psy_ui_BITMAP_H

#include "../../detail/psydef.h"
/* local */
#include "uidef.h"
/* std */
#include <assert.h>

/*
** Graphics Bitmap Interface
** Pattern: Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_Bitmap
** Implementor			psy_ui_BitmapImp
** Concrete Implementor	psy_ui_win_BitmapImp
**
** psy_ui_Bitmap <>-------- psy_ui_BitmapImp
**                                ^
**      ...                       |
**                          psy_ui_win_Bitmap
*/

#ifdef __cplusplus
extern "C" {
#endif	

struct psy_ui_Bitmap;

typedef void (*psy_ui_bitmap_fp_dispose)(struct psy_ui_Bitmap*);
typedef int (*psy_ui_bitmap_fp_load)(struct psy_ui_Bitmap*, const char* path);
typedef int (*psy_ui_bitmap_fp_loadresource)(struct psy_ui_Bitmap*, uintptr_t resourceid);
typedef psy_ui_RealSize (*psy_ui_bitmap_fp_size)(const struct psy_ui_Bitmap*);
typedef bool (*psy_ui_bitmap_fp_empty)(const struct psy_ui_Bitmap*);
typedef void (*psy_ui_bitmap_fp_settransparency)(struct psy_ui_Bitmap*,
	psy_ui_Colour);
	
typedef struct psy_ui_BitmapVTable {
	psy_ui_bitmap_fp_dispose dispose;
	psy_ui_bitmap_fp_load load;
	psy_ui_bitmap_fp_loadresource loadresource;
	psy_ui_bitmap_fp_size size;
	psy_ui_bitmap_fp_empty empty;
	psy_ui_bitmap_fp_settransparency settransparency;
} psy_ui_BitmapVTable;

struct psy_ui_BitmapImp;

typedef struct psy_ui_BitmapAnimate {
	bool enabled;
	uintptr_t interval;
	psy_ui_RealSize framesize;
	bool horizontal;
} psy_ui_BitmapAnimate;

void psy_ui_bitmapanimate_init(psy_ui_BitmapAnimate*);

typedef struct psy_ui_Bitmap {
	psy_ui_BitmapVTable* vtable;
	struct psy_ui_BitmapImp* imp_;		
} psy_ui_Bitmap;

void psy_ui_bitmap_init(psy_ui_Bitmap*);
void psy_ui_bitmap_init_size(psy_ui_Bitmap*, psy_ui_RealSize);

/* vtable calls	*/
INLINE void psy_ui_bitmap_dispose(psy_ui_Bitmap* self)
{
	self->vtable->dispose(self);
}

INLINE int psy_ui_bitmap_load(psy_ui_Bitmap* self, const char* path)
{
	assert(self);
	assert(path);

	return self->vtable->load(self, path);
}

INLINE int psy_ui_bitmap_load_resource(psy_ui_Bitmap* self,
	uintptr_t resourceid)
{
	return self->vtable->loadresource(self, resourceid);
}

INLINE psy_ui_RealSize psy_ui_bitmap_size(const psy_ui_Bitmap* self)
{
	return self->vtable->size(self);
}

INLINE bool psy_ui_bitmap_empty(const psy_ui_Bitmap* self)
{
	return self->vtable->empty(self);
}

INLINE void psy_ui_bitmap_set_transparency(psy_ui_Bitmap* self, psy_ui_Colour colour)
{
	self->vtable->settransparency(self, colour);
}

void psy_ui_bitmap_copy(psy_ui_Bitmap*, const psy_ui_Bitmap* other);

/* psy_ui_BitmapImp	*/
typedef void (*psy_ui_bitmap_imp_fp_dispose)(struct psy_ui_BitmapImp*);
typedef int (*psy_ui_bitmap_imp_fp_load)(struct psy_ui_BitmapImp*, struct psy_ui_Bitmap*, const char* path);
typedef int (*psy_ui_bitmap_imp_fp_loadresource)(struct psy_ui_BitmapImp*, uintptr_t resourceid);
typedef psy_ui_RealSize (*psy_ui_bitmap_imp_fp_size)(const struct psy_ui_BitmapImp*);
typedef bool (*psy_ui_bitmap_imp_fp_empty)(const struct psy_ui_BitmapImp*);
typedef void (*psy_ui_bitmap_imp_fp_settransparency)(struct psy_ui_BitmapImp*,
	psy_ui_Colour);
typedef void (*psy_ui_bitmap_imp_fp_copy)(struct psy_ui_BitmapImp*,
	const struct psy_ui_BitmapImp*);
typedef uintptr_t (*psy_ui_bitmap_imp_fp_native)(struct psy_ui_BitmapImp*);

typedef struct psy_ui_BitmapImpVTable {
	psy_ui_bitmap_imp_fp_dispose dev_dispose;
	psy_ui_bitmap_imp_fp_load dev_load;
	psy_ui_bitmap_imp_fp_loadresource dev_loadresource;
	psy_ui_bitmap_imp_fp_size dev_size;
	psy_ui_bitmap_imp_fp_empty dev_empty;
	psy_ui_bitmap_imp_fp_settransparency dev_settransparency;
	psy_ui_bitmap_imp_fp_copy dev_copy;
	psy_ui_bitmap_imp_fp_native dev_native;
	psy_ui_bitmap_imp_fp_native dev_native_mask;
} psy_ui_BitmapImpVTable;

typedef struct psy_ui_BitmapImp {
	psy_ui_BitmapImpVTable* vtable;
} psy_ui_BitmapImp;

void psy_ui_bitmap_imp_init(psy_ui_BitmapImp*);

uintptr_t psy_ui_bitmap_native(psy_ui_Bitmap*);
uintptr_t psy_ui_bitmap_native_mask(psy_ui_Bitmap*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BITMAP_H */
