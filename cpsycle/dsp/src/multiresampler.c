// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "multiresampler.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

static const char* names[] = {
	"Zero Hold",
	"Linear",
	"Spline",
	"Sinc",
	"soxr"
};

static void psy_dsp_multiresampler_initresamplers(psy_dsp_MultiResampler*);
static void psy_dsp_multiresampler_dispose(psy_dsp_MultiResampler*);

static void setspeed(psy_dsp_MultiResampler*, double speed);
static float work(psy_dsp_MultiResampler*, 
	const int16_t* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length);
static float work_unchecked(psy_dsp_MultiResampler*,
	int16_t const* data,
	uint32_t res);
static float work_float(psy_dsp_MultiResampler*,
	const float* data,
	float offset,
	uint64_t length,
	const float* loopBeg,
	const float* loopEnd);
static float work_float_unchecked(psy_dsp_MultiResampler*,
	float const* data, uint32_t res);

static resampler_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_MultiResampler* self)
{
	if (!vtable_initialized) {
		vtable = *self->resampler.vtable;
		vtable.dispose = (psy_dsp_fp_resampler_dispose)psy_dsp_multiresampler_dispose;
		vtable.setspeed = (psy_dsp_fp_resampler_setspeed)setspeed;
		vtable.work = (psy_dsp_fp_resampler_work)work;		
		vtable.work_unchecked = (psy_dsp_fp_resampler_work_unchecked)work_unchecked;
		vtable.work_float = (psy_dsp_fp_resampler_work_float)work_float;
		vtable.work_float_unchecked = (psy_dsp_fp_resampler_work_float_unchecked)work_float_unchecked;
		vtable_initialized = 1;
	}	
}

void psy_dsp_multiresampler_init(psy_dsp_MultiResampler* self, psy_dsp_ResamplerQuality type)
{
	psy_dsp_resampler_init(&self->resampler);
	vtable_init(self);
	self->resampler.vtable = &vtable;
	self->use_sse2_if_available = FALSE;
	// todo: sse2 let sampulse crash with sinc_sse2
	// e.g.: Sampulsedemo.psy 10:0B (sinc_sse2_float_internal) around
	// float* psinc = &sinc_sse2_windowed_table[res];
	// release win32 build
	psy_dsp_multiresampler_initresamplers(self);
	self->selected = psy_dsp_RESAMPLERQUALITY_LINEAR;	
	psy_dsp_multiresampler_setquality(self, type);
}

void psy_dsp_multiresampler_initresamplers(psy_dsp_MultiResampler* self)
{
	psy_dsp_linearresampler_init(&self->linear);
#ifdef PSYCLE_USE_SSE
	psy_dsp_spline_sse2_resampler_init(&self->spline_sse2);
	psy_dsp_sinc_sse2_resampler_init(&self->sinc_sse2);
#endif
	psy_dsp_spline_resampler_init(&self->spline);
	psy_dsp_sinc_resampler_init(&self->sinc);	
}

void psy_dsp_multiresampler_dispose(psy_dsp_MultiResampler* self)
{
	self->linear.resampler.vtable->dispose(&self->linear.resampler);
	self->spline.resampler.vtable->dispose(&self->spline.resampler);	
#ifdef PSYCLE_USE_SSE
	self->sinc_sse2.resampler.vtable->dispose(&self->sinc_sse2.resampler);
#endif
	self->sinc.resampler.vtable->dispose(&self->sinc.resampler);
}

void psy_dsp_multiresampler_setquality(psy_dsp_MultiResampler* self,
	psy_dsp_ResamplerQuality type)
{
	psy_dsp_Resampler* base;

	self->selected = type;
	switch (self->selected) {
		case psy_dsp_RESAMPLERQUALITY_LINEAR:
			base = psy_dsp_linearresampler_base(&self->linear);
		break;

		case psy_dsp_RESAMPLERQUALITY_SPLINE:	
			if (self->use_sse2_if_available) {
#ifdef PSYCLE_USE_SSE	
				base = psy_dsp_spline_sse2_resampler_base(&self->spline_sse2);
#else
				base = psy_dsp_spline_resampler_base(&self->spline);
#endif
			} else {
				base = psy_dsp_spline_resampler_base(&self->spline);
			}
		break;
		case psy_dsp_RESAMPLERQUALITY_SINC:
			if (self->use_sse2_if_available) {
#ifdef PSYCLE_USE_SSE	
			base = psy_dsp_sinc_sse2_resampler_base(&self->sinc_sse2);
#else
			base = psy_dsp_sinc_resampler_base(&self->sinc);
#endif
			} else {
				base = psy_dsp_sinc_resampler_base(&self->sinc);
			}
		break;		
		default:
			base = NULL;
		break;
	}	
	self->selectedresampler = base;
}

psy_dsp_ResamplerQuality psy_dsp_multiresampler_quality(psy_dsp_MultiResampler* self)
{
	return self->selected;
}

const char* psy_dsp_multiresampler_name(psy_dsp_ResamplerQuality type)
{
	return names[(int)type];
}

uintptr_t psy_dsp_multiresampler_count(void)
{
	return psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS;
}

float work(psy_dsp_MultiResampler* self,
	int16_t const* data,
	uint64_t offset,
	uint32_t res,
	uint64_t length)
{
	return self->selectedresampler 
	? self->selectedresampler->vtable->work(self->selectedresampler,
		data, offset, res, length)
	: *data;
}

float work_unchecked(psy_dsp_MultiResampler* self,
	const int16_t* data,
	uint32_t res)
{
	return self->selectedresampler
		? self->selectedresampler->vtable->work_unchecked(self->selectedresampler,
			data,
			res)
		: *data;
}

float work_float(psy_dsp_MultiResampler* self,
	const float* data,
	float offset,
	uint64_t length,
	const float* loopBeg,
	const float* loopEnd)
{
	return self->selectedresampler
		? self->selectedresampler->vtable->work_float(self->selectedresampler,
			data,
			offset,
			length,
			loopBeg,
			loopEnd)
		: data[(uint64_t)floor(offset)];
}

float work_float_unchecked(psy_dsp_MultiResampler* self,
	float const* data, uint32_t res)
{
	return self->selectedresampler
		? self->selectedresampler->vtable->work_float_unchecked(self->selectedresampler,
			data,
			res)
		: *data;
}

void setspeed(psy_dsp_MultiResampler* self, double speed)
{ 
	if (self->selectedresampler) {
		self->selectedresampler->vtable->setspeed(self->selectedresampler,
			speed);
	}	
}
