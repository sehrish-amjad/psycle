/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "buffer.h"
/* local */
#include "connections.h"
#include <operations.h>
/* dsp */
#include <rms.h>
/* std */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../../detail/trace.h"


/* implementation */
void psy_audio_buffer_init(psy_audio_Buffer* self, uintptr_t channels)
{
	self->rms = 0;
	self->samples = 0;	
	self->offset = 0;
	self->numsamples = 0;
	self->range = PSY_DSP_AMP_RANGE_NATIVE;
	self->volumedisplay = 0;
	self->preventmixclear = FALSE;
	self->writepos = 0;
	self->shared = TRUE;
	self->numchannels = 0;
	psy_audio_buffer_resize(self, channels);
}

void psy_audio_buffer_init_shared(psy_audio_Buffer* self, psy_audio_Buffer* src,
	uintptr_t offset)
{
	uintptr_t channel;
	
	psy_audio_buffer_init(self, src->numchannels);
	for (channel = 0; channel < src->numchannels; ++channel) {
		self->samples[channel] = src->samples[channel] + offset;
	}
}

void psy_audio_buffer_move(psy_audio_Buffer* self, uintptr_t offset)
{	
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		self->samples[channel] = self->samples[channel] + offset;
	}
}

void psy_audio_buffer_dispose(psy_audio_Buffer* self)
{
	if (!self->shared) {
		psy_audio_buffer_deallocsamples(self);
	}
	free(self->samples);
	psy_audio_buffer_disablerms(self);
}

void psy_audio_buffer_copy(psy_audio_Buffer* self, const psy_audio_Buffer* src)
{
	if (self != src && src) {		
		psy_audio_buffer_dispose(self);			
		psy_audio_buffer_init(self, src->numchannels);
		self->numsamples = src->numsamples;
		psy_audio_buffer_allocsamples(self, self->numsamples);		
		psy_audio_buffer_clearsamples(self, self->numsamples);			
		psy_audio_buffer_add(self, src, self->numsamples, 1.0f);		
		self->writepos = src->writepos;
		self->offset = src->offset;
		self->preventmixclear = src->preventmixclear;
		self->range = src->range;
		self->volumedisplay = src->volumedisplay;		
		self->rms = NULL;
		self->shared = FALSE;
	}
}

psy_audio_Buffer* psy_audio_buffer_alloc(void)
{
	return (psy_audio_Buffer*) malloc(sizeof(psy_audio_Buffer));
}

psy_audio_Buffer* psy_audio_buffer_allocinit(uintptr_t channels)
{
	psy_audio_Buffer* rv;

	rv = psy_audio_buffer_alloc();
	if (rv) {
		psy_audio_buffer_init(rv, channels);
	}
	return rv;
}

psy_audio_Buffer* psy_audio_buffer_clone(const psy_audio_Buffer* src)
{
	psy_audio_Buffer* rv;

	rv = psy_audio_buffer_allocinit(0);
	if (rv) {
		psy_audio_buffer_copy(rv, src);
	}
	return rv;
}

void psy_audio_buffer_deallocate(psy_audio_Buffer* self)
{	
	assert(self);

	psy_audio_buffer_dispose(self);
	free(self);	
}

void psy_audio_buffer_resize(psy_audio_Buffer* self, uintptr_t numchannels)
{
	float** old;	

	old = self->samples;
	if (numchannels > 0) {
		uintptr_t numold;

		numold = self->numchannels;	
		self->samples = (float**)malloc(sizeof(float*)*numchannels);
		if (self->samples) {
			uintptr_t c;

			memset(self->samples, 0, sizeof(float*) * numchannels);
			for (c = 0; c < numold; ++c) {
				self->samples[c] = old[c];				
			}
			if (!self->shared && self->numsamples > 0) {
				for (; c < numchannels; ++c) {
					self->samples[c] = dsp.memory_alloc(self->numsamples,
						sizeof(float));
				}
			}
		}
	}
	free(old);
	self->numchannels = numchannels;	
}

void psy_audio_buffer_set_offset(psy_audio_Buffer* self, uintptr_t offset)
{
	self->offset = offset;
}

uintptr_t psy_audio_buffer_offset(psy_audio_Buffer* self)
{
	return self->offset;
}

float* psy_audio_buffer_at(psy_audio_Buffer* self, uintptr_t channel)
{
	return self->samples[channel] + self->offset;
}

void psy_audio_buffer_clearsamples(psy_audio_Buffer* self, uintptr_t numsamples)
{
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		dsp.clear(self->samples[channel] + self->offset, numsamples);
	}
}

void psy_audio_buffer_add(psy_audio_Buffer* self,
	const psy_audio_Buffer* source,
	uintptr_t numsamples,
	double vol)
{
	if (source) {
		uintptr_t channel;
		double factor;

		factor = psy_audio_buffer_range_factor(source, self->range) * vol;	
		for (channel = 0; channel < source->numchannels && 
			channel < self->numchannels; ++channel) {
				dsp.add(
					source->samples[channel] + source->offset,
					self->samples[channel] + self->offset,
					numsamples,
					(float)factor);
				dsp.erase_all_nans_infinities_and_denormals(
					self->samples[channel] + self->offset, numsamples);
		}
	}
}

void psy_audio_buffer_mix(psy_audio_Buffer* self,
	psy_audio_Buffer* source, uintptr_t numsamples, double vol,
	const psy_audio_PinMapping* mapping)
{
	if (source) {
		double factor;
		psy_List* pinpair;

		factor = psy_audio_buffer_range_factor(source, self->range) * vol;
		for (pinpair = mapping->container; pinpair != 0;
				pinpair = pinpair->next) {
			psy_audio_PinConnection* connection;

			connection = (psy_audio_PinConnection*)(pinpair->entry);
			if (connection->dst < self->numchannels &&
				connection->src < source->numchannels) {
				dsp.add(
					source->samples[connection->src] + source->offset,
					self->samples[connection->dst] + self->offset,
					numsamples,
					(float)factor);
				dsp.erase_all_nans_infinities_and_denormals(
					self->samples[connection->dst] + self->offset, numsamples);
			}
		}		
	}
}

void psy_audio_buffer_mul(psy_audio_Buffer* self, uintptr_t numsamples,
	double mul)
{
	uintptr_t channel;
	
	for (channel = 0; channel < self->numchannels; ++channel) {		
		dsp.mul(self->samples[channel] + self->offset, numsamples, (float)mul);
	}	
}

void psy_audio_buffer_pan(psy_audio_Buffer* self, double pan, uintptr_t amount)
{
	uintptr_t channel;
	double vol[2];

	vol[1] = pan * 2.0;
	vol[0] = 2.0 - vol[1];
	if (vol[0] > 1.0) {
		vol[0] = 1.0;
	}
	if (vol[1] > 1.0) {
		vol[1] = 1.0;
	}
	for (channel = 0; channel < 2 && channel < self->numchannels; ++channel) {
		dsp.mul(self->samples[channel] + self->offset, amount, (float)vol[channel]);
	}
}

int psy_audio_buffer_mono(psy_audio_Buffer* self)
{
	return self->numchannels == 1;
}

void psy_audio_buffer_insertsamples(psy_audio_Buffer* self,
	psy_audio_Buffer* source, uintptr_t numsamples, uintptr_t numsourcesamples)
{	
	double rangefactor;

	rangefactor = psy_audio_buffer_range_factor(source, self->range);
	if (numsourcesamples < numsamples) {		
		uintptr_t diff;		
		uintptr_t channel;
				
		diff = numsamples - numsourcesamples;
		for (channel = 0; channel < self->numchannels; ++channel) {

			uintptr_t i;

			for (i = 0; i < diff; ++i) {
				self->samples[channel][numsamples - 1 - i] =
					self->samples[channel][numsamples - 1 - i - numsourcesamples];
			}				
			if (channel < source->numchannels) {
				dsp.movmul(source->samples[channel] + source->offset,
					self->samples[channel] + self->offset,
					numsourcesamples,
					(float)rangefactor);
			}									
		}
	} else {
		uintptr_t channel;

		for (channel = 0; channel < self->numchannels; ++channel) {
			if (channel < source->numchannels) {
				dsp.movmul(source->samples[channel] + source->offset,
					self->samples[channel] + self->offset,
					numsamples,
					(float)rangefactor);
			}
		}
	}
	if (self->rms) {
		if (self->numchannels > 1) {
			psy_dsp_rmsvol_tick(self->rms, self->samples[0],
				self->samples[1],
				(int)numsamples); /* \todo int cast? */
		}
	}
}

void psy_audio_buffer_writesamples(psy_audio_Buffer* self,
	psy_audio_Buffer* source, uintptr_t numsamples, uintptr_t numsourcesamples)
{
	uintptr_t channel;
	double rangefactor;

	rangefactor = psy_audio_buffer_range_factor(source, self->range);
	if (self->writepos + numsourcesamples < numsamples) {
		for (channel = 0; channel < self->numchannels; ++channel) {
			if (channel < source->numchannels) {
				dsp.movmul(source->samples[channel] + source->offset,
					self->samples[channel] + self->offset + self->writepos,
					numsourcesamples,
					(float)rangefactor);
			}
		}
		self->writepos += numsourcesamples;
	} else {
		uintptr_t tail;
		uintptr_t front;

		tail = numsamples - self->writepos;
		front = numsourcesamples - tail;
		if (tail > 0) {
			for (channel = 0; channel < self->numchannels; ++channel) {
				if (channel < source->numchannels) {
					dsp.movmul(source->samples[channel] + source->offset,
						self->samples[channel] + self->offset + self->writepos,
						tail,
						(float)rangefactor);
				}
			}
		}
		if (front > 0) {
			for (channel = 0; channel < self->numchannels; ++channel) {
				if (channel < source->numchannels) {
					dsp.movmul(source->samples[channel] + source->offset,
						self->samples[channel] + self->offset,
						front,
						(float)rangefactor);
				}
			}
		}
		self->writepos = front;
	}	
	if (self->rms) {
		if (self->numchannels > 1) {
			psy_dsp_rmsvol_tick(self->rms, self->samples[0],
				self->samples[1],
				(int)numsamples); /* /\todo int cast? */
		}
	}
}

void psy_audio_buffer_scale(psy_audio_Buffer* self, psy_dsp_amp_range_t range,
	uintptr_t numsamples)
{	
	if (self->range != range && range != PSY_DSP_AMP_RANGE_IGNORE) {
		uintptr_t c;
		double rangefactor;

		rangefactor = psy_audio_buffer_range_factor(self, range);
		for (c = 0; c < self->numchannels; ++c) {
			dsp.mul(self->samples[c] + self->offset, numsamples, (float)rangefactor);
		}
		self->range = range;
	}
}

double psy_audio_buffer_range_factor(const psy_audio_Buffer* self,
	psy_dsp_amp_range_t range)
{
	double rv;

	if (self->range != range && range != PSY_DSP_AMP_RANGE_IGNORE) {
		if (range == PSY_DSP_AMP_RANGE_NATIVE) {
			rv = 32768.0;		
		} else 
		if (range == PSY_DSP_AMP_RANGE_VST) {
			rv = 1.0 / 32768.0;
		} else {
			rv = 1.0;
		}
	} else {
		rv = 1.0;
	}
	return rv;
}

void psy_audio_buffer_trace(psy_audio_Buffer* self, uintptr_t channel,
	uintptr_t numsamples)
{
	uintptr_t i;

	for (i = 0; i < numsamples; ++i) {
		float* p = self->samples[channel] + self->offset + i;

		TRACE_FLOAT(*p);
	}
	TRACE("\n");
}

void psy_audio_buffer_enablerms(psy_audio_Buffer* self)
{
	if (self->rms == NULL) {
		self->rms = psy_dsp_rmsvol_allocinit();
	}	
}

void psy_audio_buffer_disablerms(psy_audio_Buffer* self)
{
	if (self->rms != NULL) {
		psy_dsp_rmsvol_deallocate(self->rms);
		self->rms = 0;
	}
}

double psy_audio_buffer_rmsvolume(psy_audio_Buffer* self)
{
	return self->rms ? psy_dsp_rmsvol_value(self->rms) : 0.f;
}

double psy_audio_buffer_rmsdisplay(psy_audio_Buffer* self)
{
	return psy_audio_buffer_rmsscale(self,
		psy_audio_buffer_rmsvolume(self));
}

double psy_audio_buffer_rmsscale(psy_audio_Buffer* self,
	double rms_volume)
{
	double temp;

	temp = rms_volume;
	if (temp == 0.0) {
		return 0.0;
	}
	if (self->range == PSY_DSP_AMP_RANGE_NATIVE) {
		temp /= 32767;
	}
	temp = 50.0 * log10(temp) + 100.0;
	if (temp > 97.0) {
		temp = 97.0;
	}
	return (temp > 0.0) ? temp / 97.0 : 0.0;
}

void psy_audio_buffer_make_monoaureal(psy_audio_Buffer* self,
	uintptr_t numsamples)
{
	if (psy_audio_buffer_num_channels(self) > 1) {
		dsp.add(psy_audio_buffer_at(self, 0),
			psy_audio_buffer_at(self, 1),
			numsamples,
			1.f);
	}
}

void psy_audio_buffer_allocsamples(psy_audio_Buffer* self, uintptr_t numframes)
{
	uintptr_t channel;

	psy_audio_buffer_deallocsamples(self);
	self->numsamples = numframes;	
	for (channel = 0; channel < self->numchannels; ++channel) {
		if (numframes) {
			self->samples[channel] = dsp.memory_alloc(numframes,
				sizeof(float));
		} else {
			self->samples[channel] = NULL;
		}
	}
	self->shared = FALSE;
}

void psy_audio_buffer_deallocsamples(psy_audio_Buffer* self)
{
	uintptr_t channel;

	for (channel = 0; channel < self->numchannels; ++channel) {
		if (self->samples[channel]) {
			dsp.memory_dealloc(self->samples[channel]);
			self->samples[channel] = NULL;
		}		
	}
	self->numsamples = 0;
	self->shared = TRUE;
}
