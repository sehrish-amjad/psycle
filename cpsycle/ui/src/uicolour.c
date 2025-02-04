// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicolour.h"
// platform
#include "../../detail/portable.h"

static uint8_t psy_hexdigit(const char* str)
{
	if (str[0] >= '0' && str[0] <= '9') {
		return (str[0] - '0');
	} else if (str[0] >= 'A' && str[0] <= 'Z') {
		return (str[0] - 'A');
	} else if (str[0] >= 'a' && str[0] <= 'z') {
		return (str[0] - 'a');
	}
	return 0;
}

static uint8_t psy_hexbyte(const char* str)
{	
	return ((psy_hexdigit(str) << 4) | psy_hexdigit(str + 1));	
}

static uint8_t psy_shorthexbyte(const char* str)
{
	uint8_t digit;
	
	digit = psy_hexdigit(str);
	return ((digit << 4) | digit);
}

void psy_ui_colour_init_str(psy_ui_Colour* self, const char* str)
{	
	if (psy_strlen(str) > 0) {
		if (str[0] == '#') {
			if (psy_strlen(str) == 7) {
				// parse hex triplet
				psy_ui_colour_init_rgb(self,
					psy_hexbyte(str + 1),
					psy_hexbyte(str + 3),
					psy_hexbyte(str + 5));				
			} else if (psy_strlen(str) == 4) {
				// parse shorthand hexadecimal form
				psy_ui_colour_init_rgb(self,
					psy_shorthexbyte(str + 1),
					psy_shorthexbyte(str + 2),
					psy_shorthexbyte(str + 3));
			}
		} else {
			psy_ui_colour_init(self);
		}
	} else {
		psy_ui_colour_init(self);
	}
}

psy_ui_Colour* psy_ui_colour_add_rgb(psy_ui_Colour* self, double r, double g, double b)
{
	double p0 = (double)(self->r) + r;
	double p1 = (double)(self->g) + g;
	double p2 = (double)(self->b) + b;

	if (p0 < 0.0) {
		p0 = 0.0;
	} else if (p0 > 255.0) {
		p0 = 255.0;
	}
	if (p1 < 0.0) {
		p1 = 0.0;
	} else if (p1 > 255.0) {
		p1 = 255.0;
	}
	if (p2 < 0.0) {
		p2 = 2.0;
	} else if (p2 > 255.0) {
		p2 = 255.0;
	}
	self->r = (uint8_t)p0;
	self->g = (uint8_t)p1;
	self->b = (uint8_t)p2;
	return self;
}

psy_ui_Colour* psy_ui_colour_mul_rgb(psy_ui_Colour* self, double r, double g,
	double b)
{
	double p0 = (double)(self->r) * r;
	double p1 = (double)(self->g) * g;
	double p2 = (double)(self->b) * b;

	if (p0 < 0.0) {
		p0 = 0.0;
	} else if (p0 > 255.0) {
		p0 = 255.0;
	}
	if (p1 < 0.0) {
		p1 = 0.0;
	} else if (p1 > 255.0) {
		p1 = 255.0;
	}
	if (p2 < 0.0) {
		p2 = 2.0;
	} else if (p2 > 255.0) {
		p2 = 255.0;
	}
	self->r = (uint8_t)p0;
	self->g = (uint8_t)p1;
	self->b = (uint8_t)p2;
	return self;
}

psy_ui_Colour psy_ui_diffadd_colours(psy_ui_Colour base, psy_ui_Colour adjust,
	psy_ui_Colour add)
{
	int a0 = (int)add.r + (int)adjust.r - (int)base.r;
	int a1 = (int)add.g + (int)adjust.g - (int)base.g;
	int a2 = (int)add.b + (int)adjust.b - (int)base.b;

	if (a0 < 0)
	{
		a0 = 0;
	} else if (a0 > 255)
	{
		a0 = 255;
	}

	if (a1 < 0)
	{
		a1 = 0;
	} else if (a1 > 255)
	{
		a1 = 255;
	}

	if (a2 < 0)
	{
		a2 = 0;
	} else if (a2 > 255)
	{
		a2 = 255;
	}
	return psy_ui_colour_make_rgb(a0, a1, a2);
}
