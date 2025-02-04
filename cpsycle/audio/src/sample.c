/*
** This source is free software; you can redistribute it and /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sample.h"
/* local */
#include "waveio.h"
/* dsp */
#include <alignedalloc.h>
#include <operations.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"


/* implementation */
void psy_audio_sample_init(psy_audio_Sample* self, uintptr_t numchannels)
{
	assert(self);

	psy_signal_init(&self->signal_loop_changed);
	self->name = strdup("");
	self->numframes = 0;
	self->globalvolume = 1.f;
	self->defaultvolume = 0x80;
	psy_audio_sampleloop_init(&self->loop);
	psy_audio_sampleloop_init(&self->sustainloop);
	psy_audio_buffer_init(&self->channels, numchannels);
	self->channels.shared = FALSE;
	self->samplerate = (double)44100;
	self->zone.tune = 0;
	self->zone.finetune = 0;
	self->stereo = numchannels > 1;
	self->panenabled = TRUE;
	self->panfactor = psy_audio_PAN_CENTER;
	self->surround = FALSE;		
	psy_audio_vibrato_init(&self->vibrato);
}

void psy_audio_sample_dispose(psy_audio_Sample* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_loop_changed);
	psy_audio_buffer_dispose(&self->channels);
	self->numframes = 0;
	free(self->name);
	self->name = NULL;
}

void psy_audio_sample_copy(psy_audio_Sample* self,
	const psy_audio_Sample* src)
{	
	assert(self);
	assert(src);

	if (self != src) {
		self->samplerate = src->samplerate;
		self->defaultvolume = src->defaultvolume;
		self->globalvolume = src->globalvolume;
		self->loop.start = src->loop.start;
		self->loop.end = src->loop.end;
		self->loop.type = src->loop.type;
		self->sustainloop.start = src->sustainloop.start;
		self->sustainloop.end = src->sustainloop.end;
		self->sustainloop.type = src->sustainloop.type;
		self->zone = src->zone;		
		self->panfactor = src->panfactor;
		self->panenabled = src->panenabled;
		self->surround = src->surround;
		psy_strreset(&self->name, src->name);
		self->vibrato.attack = src->vibrato.attack;
		self->vibrato.depth = src->vibrato.depth;
		self->vibrato.speed = src->vibrato.speed;
		self->vibrato.type = src->vibrato.type;
		self->numframes = src->numframes;
		self->stereo = src->stereo;		
		psy_audio_buffer_dispose(&self->channels);
		psy_audio_buffer_init(&self->channels, src->channels.numchannels);
		psy_audio_buffer_allocsamples(&self->channels, src->numframes);
		psy_audio_buffer_clearsamples(&self->channels, src->numframes);
		psy_audio_buffer_add(&self->channels, &src->channels,
			src->numframes, 1.0f);
	}	
}

psy_audio_Sample* psy_audio_sample_alloc(void)
{
	return (psy_audio_Sample*)malloc(sizeof(psy_audio_Sample));
}

psy_audio_Sample* psy_audio_sample_alloc_init(uintptr_t numchannels)
{
	psy_audio_Sample* rv;	

	rv = psy_audio_sample_alloc();
	if (rv) {
		psy_audio_sample_init(rv, numchannels);
	}
	return rv;
}

psy_audio_Sample* psy_audio_sample_clone(const psy_audio_Sample* src)
{
	psy_audio_Sample* rv;

	assert(src);

	rv = psy_audio_sample_alloc_init(0);
	if (rv) {
		psy_audio_sample_copy(rv, src);
	}
	return rv;
}

void psy_audio_sample_deallocate(psy_audio_Sample* self)
{
	assert(self);

	psy_audio_sample_dispose(self);
	free(self);
}

psy_audio_WaveDataController* psy_audio_sample_alloc_iterator(
	psy_audio_Sample* self, psy_dsp_ResamplerQuality quality)
{
	psy_audio_WaveDataController* rv;

	rv = psy_audio_wavedatacontroller_alloc();
	if (rv) {
		psy_audio_wavedatacontroller_init(rv);
		psy_audio_wavedatacontroller_initcontroller(rv, self,
			quality);
	}
	return rv;
}

void psy_audio_sample_alloc_wave_data(psy_audio_Sample* self)
{
	assert(self);

	psy_audio_buffer_allocsamples(&self->channels, self->numframes);
	psy_audio_buffer_clearsamples(&self->channels, self->numframes);
}

int psy_audio_sample_load(psy_audio_Sample* self, const char* filename)
{	
	int status;
	psy_Path path;
	
	assert(self);
	
	status = psy_audio_wave_load(self, filename);	
	psy_path_init(&path, filename);
	psy_audio_sample_set_name(self, psy_path_filename(&path));	
	psy_path_dispose(&path);
	return status;
}

void psy_audio_sample_save(psy_audio_Sample* self, const char* path)
{
	assert(self);

	psy_audio_wave_save(self, path);
}

void psy_audio_sample_set_name(psy_audio_Sample* self, const char* name)
{
	assert(self);

	psy_strreset(&self->name, name);	
}
