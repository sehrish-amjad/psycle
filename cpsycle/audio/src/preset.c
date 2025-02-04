/*
** This source is free software; you can redistribute it and /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "preset.h"
/* platform */
#include "../../detail/portable.h"


/* implementation */
void psy_audio_preset_init(psy_audio_Preset* self)
{
	assert(self);

	self->name = strdup("");
	psy_table_init(&self->parameters);
	self->datasize = 0;
	self->data = 0;
	self->isfloat = 0;
	self->magic = 0;
	self->id = 0;
	self->version = 0;
}

void psy_audio_preset_dispose(psy_audio_Preset* self)
{
	assert(self);

	free(self->name);
	self->name = NULL;
	psy_table_dispose(&self->parameters);
	free(self->data);
	self->data = NULL;
	self->datasize = 0;	
}

void psy_audio_preset_clear(psy_audio_Preset* self)
{
	assert(self);

	psy_audio_preset_dispose(self);
	psy_audio_preset_init(self);
}

psy_audio_Preset* psy_audio_preset_alloc(void)
{
	return malloc(sizeof(psy_audio_Preset));
}

psy_audio_Preset* psy_audio_preset_alloc_init(void)
{
	psy_audio_Preset* rv;

	rv = psy_audio_preset_alloc();
	if (rv) {
		psy_audio_preset_init(rv);
	}
	return rv;
}

void psy_audio_preset_set_name(psy_audio_Preset* self, const char* name)
{
	assert(self);

	psy_strreset(&self->name, name);
}

const char* psy_audio_preset_name(psy_audio_Preset* self)
{
	assert(self);

	if (self->name) {
		return self->name;
	}
	return "";
}

void psy_audio_preset_set_value(psy_audio_Preset* self, uintptr_t param,
	intptr_t value)
{
	assert(self);

	psy_table_insert(&self->parameters, param, (void*)(uintptr_t)value);
}

intptr_t psy_audio_preset_value(psy_audio_Preset* self, uintptr_t param)
{
	assert(self);

	if (psy_table_exists(&self->parameters, param)) {
		return (intptr_t)(uintptr_t) psy_table_at(&self->parameters, param);
	} else {
		return 0;
	}
}

void psy_audio_preset_set_data_struct(psy_audio_Preset* self,
	uintptr_t num, const char* newname, int const* parameters, uintptr_t size,
	void* newdata)
{	
	assert(self);

	if (num > 0) {
		uintptr_t p;
		psy_table_dispose(&self->parameters);
		for (p = 0; p < num; ++p) {
			psy_audio_preset_set_value(self, p, (intptr_t)parameters[p]);
		}
	} else
	{
		psy_table_dispose(&self->parameters);
	}
	psy_audio_preset_put_data(self, size, newdata);
	free(self->name);
	self->name = strdup(newname);
}

void psy_audio_preset_put_data(psy_audio_Preset* self, uintptr_t size, void* newdata)
{	
	assert(self);

	if (size > 0)
	{
		free(self->data);
		self->data = (unsigned char*)malloc(size);
		if (self->data) {
			memcpy(self->data, newdata, size);
			self->datasize = size;
		} else {
			self->datasize = 0;
		}
	} else {
		free(self->data);
		self->data = 0;
		self->datasize = 0;
	}	
}
