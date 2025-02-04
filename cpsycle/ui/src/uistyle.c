/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uistyle.h"
/* local */
#include "uiapp.h"
#include "uicolours.h"
#include "uicomponent.h"
/* platform */
#include "../../detail/portable.h"


/* psy_ui_Background */
void psy_ui_background_init(psy_ui_Background* self)
{
	psy_ui_colour_init(&self->colour);	
	self->image_id = psy_INDEX_INVALID;
	self->image_path = NULL;
	psy_ui_bitmap_init(&self->bitmap);
	self->repeat = psy_ui_REPEAT;
	self->align = psy_ui_ALIGNMENT_NONE;
	psy_ui_bitmapanimate_init(&self->animation);
	self->position = psy_ui_realpoint_zero();
	self->position_set = FALSE;
	self->size = psy_ui_size_zero();
	self->size_set = FALSE;
}

void psy_ui_background_dispose(psy_ui_Background* self)
{
	free(self->image_path);
	self->image_path = NULL;
	psy_ui_bitmap_dispose(&self->bitmap);
}

void psy_ui_background_copy(psy_ui_Background* self, const psy_ui_Background*
	other)
{
	self->colour = other->colour;
	self->image_id = other->image_id;
	psy_strreset(&self->image_path, other->image_path);
	psy_ui_bitmap_copy(&self->bitmap, &other->bitmap);
	self->repeat = other->repeat;
	self->align = other->align;
	self->animation = other->animation;	
	self->position = other->position;
	self->position_set = other->position_set;
	self->size = other->size;
	self->size_set = other->size_set;	
}

/* psy_ui_Style */
void psy_ui_style_init(psy_ui_Style* self)
{
	self->name = psy_strdup("");
	psy_ui_colour_init(&self->colour);
	self->colour.mode.inherit = TRUE;
	psy_ui_font_init(&self->font, NULL);	
	psy_ui_background_init(&self->background);		
	psy_ui_border_init(&self->border);
	psy_ui_margin_init(&self->margin);
	self->margin_set = FALSE;
	psy_ui_margin_init(&self->padding);
	self->padding_set = FALSE;
	psy_ui_position_init(&self->position);
	self->display = psy_ui_DISPLAY_NONE;
	self->display_set = FALSE;
	self->dbgflag = 0;	
}

void psy_ui_style_init_default(psy_ui_Style* self, uintptr_t styletype)
{	
	psy_ui_style_init_copy(self, psy_ui_style_const(styletype));	
}

void psy_ui_style_init_copy(psy_ui_Style* self, const psy_ui_Style* other)
{	
	psy_ui_style_init(self);
	psy_ui_style_copy(self, other);		
}

void psy_ui_style_init_colours(psy_ui_Style* self, psy_ui_Colour colour,
	psy_ui_Colour background)
{
	psy_ui_style_init(self);
	self->colour = colour;
	self->colour.mode.inherit = TRUE;
	self->background.colour = background;
}

void psy_ui_style_dispose(psy_ui_Style* self)
{		
	free(self->name);
	self->name = NULL;
	psy_ui_font_dispose(&self->font);
	psy_ui_background_dispose(&self->background);
	psy_ui_position_dispose(&self->position);
}

void psy_ui_style_copy(psy_ui_Style* self, const psy_ui_Style* other)
{
	psy_strreset(&self->name, other->name);
	self->colour = other->colour;	
	psy_ui_background_copy(&self->background, &other->background);	
	self->border = other->border;
	self->margin = other->margin;
	self->margin_set = other->margin_set;
	self->padding = other->padding;
	self->padding_set = other->padding_set;
	self->display = other->display;
	self->display_set = other->display_set;
	psy_ui_position_dispose(&self->position);
	psy_ui_position_init(&self->position);
	if (psy_ui_position_is_active(&other->position)) {
		psy_ui_position_set_rectangle(&self->position,
			*other->position.rectangle);
	}	
	psy_ui_font_copy(&self->font, &other->font);	
}

psy_ui_Style* psy_ui_style_alloc(void)
{
	return (psy_ui_Style*)malloc(sizeof(psy_ui_Style));
}

psy_ui_Style* psy_ui_style_allocinit(void)
{
	psy_ui_Style* rv;

	rv = psy_ui_style_alloc();
	if (rv) {
		psy_ui_style_init(rv);
	}
	return rv;
}

psy_ui_Style* psy_ui_style_clone(const psy_ui_Style* other)
{
	if (other) {
		psy_ui_Style* rv;

		rv = psy_ui_style_allocinit();
		if (rv) {
			psy_ui_style_copy(rv, other);
		}
		return rv;
	}
	return NULL;
}

void psy_ui_style_deallocate(psy_ui_Style* self)
{
	psy_ui_style_dispose(self);
	free(self);
}

void psy_ui_style_set_name(psy_ui_Style* self, const char* name)
{
	psy_strreset(&self->name, name);
}

void psy_ui_style_set_font(psy_ui_Style* self, const char* family, double size)
{	
	psy_ui_FontInfo font_info;
	
	psy_ui_font_dispose(&self->font);
	psy_ui_fontinfo_init(&font_info, family, size);
	psy_ui_font_init(&self->font, &font_info);
}

void psy_ui_style_set_font_string(psy_ui_Style* self, const char* str)
{
	psy_ui_FontInfo font_info;
	
	psy_ui_font_dispose(&self->font);
	psy_ui_fontinfo_init_string(&font_info, str);
	psy_ui_font_init(&self->font, &font_info);
}

void psy_ui_style_set_font_info(psy_ui_Style* self,
	psy_ui_FontInfo font_info)
{			
	psy_ui_font_dispose(&self->font);	
	psy_ui_font_init(&self->font, &font_info);
}

void psy_ui_style_set_background_id(psy_ui_Style* self,
	uintptr_t id)
{	
	free(self->background.image_path);
	self->background.image_path = NULL;
	self->background.image_id = id;	
	psy_ui_bitmap_load_resource(&self->background.bitmap, id);
}

int psy_ui_style_setbackgroundpath(psy_ui_Style* self,
	const char* path)
{		
	psy_strreset(&self->background.image_path, path);
	return psy_ui_bitmap_load(&self->background.bitmap, path);
}
