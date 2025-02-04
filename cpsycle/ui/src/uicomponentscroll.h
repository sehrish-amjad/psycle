/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMPONENTSCROLL_H
#define psy_ui_COMPONENTSCROLL_H

/* local */
#include "uidef.h"


#ifdef __cplusplus
extern "C" {
#endif

/*! @struct psy_ui_ComponentScroll */
typedef struct psy_ui_ComponentScroll {	
	psy_ui_Size step;		
	psy_ui_IntPoint vrange;
	psy_ui_IntPoint hrange;
	uintptr_t wheel;
	psy_ui_Overflow overflow;
} psy_ui_ComponentScroll;

void psy_ui_componentscroll_init(psy_ui_ComponentScroll*);

psy_ui_ComponentScroll* psy_ui_componentscroll_alloc(void);
psy_ui_ComponentScroll* psy_ui_componentscroll_alloc_init(void);
void psy_ui_componentscroll_deallocate(psy_ui_ComponentScroll*);


INLINE void psy_ui_componentscroll_sethrange(psy_ui_ComponentScroll* self,
	psy_ui_IntPoint range)
{
	self->hrange = range;
}

INLINE psy_ui_IntPoint psy_ui_componentscroll_hrange(
	const psy_ui_ComponentScroll* self)
{
	return self->hrange;
}

INLINE void psy_ui_componentscroll_setvrange(psy_ui_ComponentScroll* self,
	psy_ui_IntPoint range)
{
	self->vrange = range;
}

INLINE psy_ui_IntPoint psy_ui_componentscroll_vrange(
	const psy_ui_ComponentScroll* self)
{
	return self->vrange;
}

INLINE void psy_ui_componentscroll_set_wheel(
	psy_ui_ComponentScroll* self, uintptr_t wheel)
{
	self->wheel = wheel;
}

INLINE uintptr_t psy_ui_componentscroll_wheel(
	const psy_ui_ComponentScroll* self)
{
	return self->wheel;
}

INLINE bool psy_ui_componentscroll_has_vertical_overflow(
	const psy_ui_ComponentScroll* self)
{
	return ((self->overflow & psy_ui_OVERFLOW_VSCROLL) ==
			psy_ui_OVERFLOW_VSCROLL);
}

INLINE bool psy_ui_componentscroll_has_vertical_center_overflow(
	const psy_ui_ComponentScroll* self)
{
	return ((self->overflow & psy_ui_OVERFLOW_VSCROLLCENTER) ==
		psy_ui_OVERFLOW_VSCROLLCENTER);
}

INLINE bool psy_ui_componentscroll_has_horizontal_overflow(
	const psy_ui_ComponentScroll* self)
{
	return ((self->overflow & psy_ui_OVERFLOW_HSCROLL) ==
		psy_ui_OVERFLOW_HSCROLL);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSCROLL_H */
