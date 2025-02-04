/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "spectrumanalyzer.h"

#include <songio.h>
#include "../../detail/portable.h"
#include <math.h>
#include <rms.h>
#include <exclusivelock.h>
#include <operations.h>
#include <string.h>

#define SCOPE_BUF_SIZE_LOG 13

#define SCOPE_BARS_WIDTH  (256 / SCOPE_SPEC_BANDS)
#define SCOPE_BUF_SIZE  (1 << SCOPE_BUF_SIZE_LOG)
static const uint32_t CLBARDC = 0x1010DC;
static const uint32_t CLBARPEAK = 0xC0C0C0;
static const uint32_t CLLEFT = 0xC06060;
static const uint32_t CLRIGHT = 0x60C060;
static const uint32_t CLBOTH = 0xC0C060;

static void spectrumanalyzer_on_destroyed(SpectrumAnalyzer*);
static void spectrumanalyzer_init_background(SpectrumAnalyzer*);
static void spectrumanalyzer_on_draw(SpectrumAnalyzer*, psy_ui_Graphics*);
static void spectrumanalyzer_draw_background(SpectrumAnalyzer*, psy_ui_Graphics*);
static void spectrumanalyzer_draw_spectrum(SpectrumAnalyzer*, psy_ui_Graphics*);	
static void FillLinearFromCircularBuffer(SpectrumAnalyzer*,
	float inBuffer[], float outBuffer[], float vol, uintptr_t writepos);
static psy_audio_Buffer* spectrumanalyzer_buffer(SpectrumAnalyzer*,
	uintptr_t* numsamples);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(SpectrumAnalyzer* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			spectrumanalyzer_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			spectrumanalyzer_on_draw;		
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void spectrumanalyzer_init(SpectrumAnalyzer* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
	psy_ui_component_set_preferred_size(&self->component,
		psy_ui_size_make_px(256.0, 128.0));
	self->wire = wire;	
	self->invol = 1.0;
	self->mult = 1.0;
	self->scope_peak_rate = 20;
	self->hold = 0;
	self->scope_spec_mode = 2;
	self->scope_spec_samples = 2048;
	self->scope_spec_rate = 20;	
	self->workspace = workspace;
	self->hold = FALSE;
	memset(self->db_left, 0, sizeof(self->db_left));
	memset(self->db_right, 0, sizeof(self->db_right));
	self->lastwritepos = 0;
	self->lastprocessed = 0;	
	fftclass_init(&self->fftSpec);
	fftclass_setup(&self->fftSpec, hann, self->scope_spec_samples,
		SCOPE_SPEC_BANDS);
	/* create background */
	spectrumanalyzer_init_background(self);
	psy_dsp_noopt_init(&self->dsp_noopt);	
}

void spectrumanalyzer_on_destroyed(SpectrumAnalyzer* self)
{	
	fftclass_dispose(&self->fftSpec);
	psy_ui_bitmap_dispose(&self->bg);
}

void spectrumanalyzer_init_background(SpectrumAnalyzer* self)
{
	psy_ui_Font font;
	psy_ui_FontInfo fontinfo;
	psy_ui_Graphics g;

	psy_ui_bitmap_init_size(&self->bg, psy_ui_realsize_make(256.0, 128.0));
	psy_ui_graphics_init_bitmap(&g, &self->bg);
	psy_ui_fontinfo_init(&fontinfo, "tahoma", 12);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_graphics_set_font(&g, &font);
	psy_ui_font_dispose(&font);
	spectrumanalyzer_draw_background(self, &g);
	psy_ui_graphics_dispose(&g);
}

void spectrumanalyzer_on_draw(SpectrumAnalyzer* self, psy_ui_Graphics* g)
{		
	psy_ui_graphics_draw_full_bitmap(g, &self->bg, psy_ui_realpoint_zero());
	spectrumanalyzer_draw_spectrum(self, g);	
}

void spectrumanalyzer_draw_background(SpectrumAnalyzer* self, psy_ui_Graphics* g)
{
	psy_ui_RealRectangle rect;
	char buf[64];
	int i;
	double invlog2;
	double thebar;
	psy_ui_Colour fg;
	psy_ui_Colour fg_medium;
	psy_ui_Font font;
	psy_ui_FontInfo font_info;

	fg = psy_ui_colour_make(0x505050);
	fg_medium = psy_ui_colour_make(0x606060);

	for (i = 0; i < SCOPE_SPEC_BANDS; i++)
	{
		self->bar_heights[i] = 256;
	}
	
	// psy_ui_graphics_set_background_mode(g, psy_ui_TRANSPARENT);
	rect = psy_ui_realrectangle_make(psy_ui_realpoint_zero(),
		psy_ui_realsize_make(256.0, 128.0));
	font_info = psy_ui_font_font_info(psy_ui_component_font(&self->component));
	font_info.lfHeight = 8;
	psy_ui_font_init(&font, &font_info);
	psy_ui_graphics_set_font(g, &font);
	psy_ui_font_dispose(&font);
	psy_ui_graphics_draw_solid_rectangle(g, rect, psy_ui_colour_make(0x121212));
	psy_ui_graphics_set_text_colour(g, fg);	
	psy_snprintf(buf, sizeof(buf), "db");
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(3, 0), buf, psy_strlen(buf));
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(256 - 13, 0), buf, psy_strlen(buf));
	for (i = 1; i < 6; i++) {
		rect.top = 20 * i;
		rect.bottom = rect.top + 1;
		psy_ui_graphics_draw_solid_rectangle(g, rect, fg);

		psy_snprintf(buf, sizeof(buf), "-%d0", i);
		psy_ui_graphics_textout(g, psy_ui_realpoint_make(0, rect.top - 10), buf, psy_strlen(buf));
		psy_ui_graphics_textout(g, psy_ui_realpoint_make(256 - 16, rect.top - 10), buf, psy_strlen(buf));
	}
	rect.left = 128;
	rect.right = 256;
	rect.top = 120;
	rect.bottom = rect.top + 1;
	psy_ui_graphics_draw_solid_rectangle(g, rect, fg);

	psy_snprintf(buf, sizeof(buf), "-60");
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(256 - 16, rect.top - 10), buf, psy_strlen(buf));

	rect.top = 0;
	rect.bottom = 128;
	invlog2 = (float)(1.0 / log10(2.0));
	thebar = 440.f * 2.f * 256.f / (float)psy_audio_player_sample_rate(workspace_player(self->workspace));
	if (self->scope_spec_mode == 1) rect.left = (int) thebar;
	else if (self->scope_spec_mode == 2) rect.left = (int)(16 * sqrt(thebar));
	else if (self->scope_spec_mode == 3) rect.left = (int)(32 * log10(1 + thebar) * invlog2);
	rect.right = rect.left + 1;
	psy_ui_graphics_draw_solid_rectangle(g, rect, fg_medium);
	psy_snprintf(buf, sizeof(buf), "440");
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(rect.left, 0), buf, psy_strlen(buf));
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(rect.left, 128 - 12), buf, psy_strlen(buf));

	thebar = 7000 * 2.f * 256.f / (float)psy_audio_player_sample_rate(workspace_player(self->workspace));
	if (self->scope_spec_mode == 1) rect.left = (int) thebar;
	else if (self->scope_spec_mode == 2) rect.left = (int) (16 * sqrt(thebar));
	else if (self->scope_spec_mode == 3) rect.left = (int) (32 * log10(1 + thebar) * invlog2);
	rect.right = rect.left + 1;
	psy_ui_graphics_draw_solid_rectangle(g, rect, fg_medium);
	psy_snprintf(buf, sizeof(buf), "7K");
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(rect.left, 0), buf, psy_strlen(buf));
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(rect.left, 128 - 12), buf, psy_strlen(buf));

	thebar = 16000 * 2.f * 256.f / (float)psy_audio_player_sample_rate(workspace_player(self->workspace));
	if (self->scope_spec_mode == 1) rect.left = (int) thebar;
	else if (self->scope_spec_mode == 2) rect.left = (int)(16 * sqrt(thebar));
	else if (self->scope_spec_mode == 3) rect.left = (int)(32 * log10(1 + thebar) * invlog2);
	rect.right = rect.left + 1;
	psy_ui_graphics_draw_solid_rectangle(g, rect, fg_medium);
	sprintf(buf, "16K");
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(rect.left, 0), buf, psy_strlen(buf));
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(rect.left, 128 - 12), buf, psy_strlen(buf));
	psy_snprintf(buf, sizeof(buf), "%d Samples Refresh %.2fhz",
		self->scope_spec_samples, 1000.0f / self->scope_spec_rate);
	//sprintf(buf,"%d Samples Refresh %.2fhz Window %d",scope_spec_samples,1000.0f/scope_spec_rate, FFTMethod);
	psy_ui_graphics_set_background_mode(g, psy_ui_TRANSPARENT);
	psy_ui_graphics_set_text_colour(g, fg);
	psy_ui_graphics_textout(g, psy_ui_realpoint_make(4, 128 - 14), buf, psy_strlen(buf));
}

void spectrumanalyzer_draw_spectrum(SpectrumAnalyzer* self, psy_ui_Graphics* g)
{	
	const double multleft = 1/32768.0 * self->invol * self->mult; // *srcMachine._lVol;
	const double multright = 1 / 32768.f * self->invol * self->mult; // *srcMachine._rVol;
	uintptr_t scopesamples;
	psy_audio_Buffer* buffer;
	int DCBar;
	psy_ui_RealRectangle rect;	
	uintptr_t writepos;
	float temp[SCOPE_BUF_SIZE];
	float tempout[SCOPE_BUF_SIZE >> 1];	
	int i;
	
	buffer = spectrumanalyzer_buffer(self, &scopesamples);
	//self->scope_spec_samples = scopesamples;
	if (!buffer) {
		return;
	}	
	writepos = buffer->writepos;
#if 0
	if (FFTMethod == 0)
#endif
	{
		if (!spectrumanalyzer_stopped(self) && writepos != self->lastprocessed) {
			FillLinearFromCircularBuffer(self, buffer->samples[0], temp, (float)multleft, self->lastprocessed);
			fftclass_calculatespectrum(&self->fftSpec, temp, tempout);
			fftclass_fillbandsfromfft(&self->fftSpec, tempout, self->db_left);

			FillLinearFromCircularBuffer(self, buffer->samples[1], temp, (float)multright, writepos);
			fftclass_calculatespectrum(&self->fftSpec, temp, tempout);			
			fftclass_fillbandsfromfft(&self->fftSpec, tempout, self->db_right);
			self->lastprocessed = writepos;
		}
	}	
#if 0
	else if (FFTMethod == 1)
	{
		//DM FFT
		float temp[SCOPE_BUF_SIZE];
		float tempout[SCOPE_BUF_SIZE];

		FillLinearFromCircularBuffer(pSamplesL, temp, multleft);
		helpers::dsp::dmfft::WindowFunc(3, scope_spec_samples, temp);
		helpers::dsp::dmfft::PowerSpectrum(scope_spec_samples, temp, tempout);
		fftSpec.FillBandsFromFFT(tempout, db_left);

		FillLinearFromCircularBuffer(pSamplesR, temp, multright);
		helpers::dsp::dmfft::WindowFunc(3, scope_spec_samples, temp);
		helpers::dsp::dmfft::PowerSpectrum(scope_spec_samples, temp, tempout);
		fftSpec.FillBandsFromFFT(tempout, db_right);
	}
	else if (FFTMethod == 2)
	{
		// Psycle's pseudo FT
		const float dbinvSamples = psycle::helpers::dsp::dB(1.0f / (scope_spec_samples >> 2)); // >>2, because else seems 12dB's off
		float aal[SCOPE_SPEC_BANDS] = { 0 };
		float aar[SCOPE_SPEC_BANDS] = { 0 };
		float bbl[SCOPE_SPEC_BANDS] = { 0 };
		float bbr[SCOPE_SPEC_BANDS] = { 0 };

		//scopeBufferIndex is the position where it will write new data.
		int index = (srcMachine._scopeBufferIndex == 0) SCOPE_BUF_SIZE - 1 : srcMachine._scopeBufferIndex - 1;
		for (int i = 0; i < scope_spec_samples; i++, index--, index &= (SCOPE_BUF_SIZE - 1))
		{
			const float wl = pSamplesL[index] * multleft;
			const float wr = pSamplesR[index] * multright;
			for (int h = 0; h < SCOPE_SPEC_BANDS; h++)
			{
				aal[h] += wl * cth[i][h];
				bbl[h] += wl * sth[i][h];
				aar[h] += wr * cth[i][h];
				bbr[h] += wr * sth[i][h];
			}
		}

		for (int h = 0; h < SCOPE_SPEC_BANDS; h++)
		{
			float out = aal[h] * aal[h] + bbl[h] * bbl[h];
			db_left[h] = helpers::dsp::powerdB(out + 0.0000001f) + dbinvSamples;
			out = aar[h] * aar[h] + bbr[h] * bbr[h];
			db_right[h] = helpers::dsp::powerdB(out + 0.0000001f) + dbinvSamples;
		}
	}
#endif
	// draw our bands
	DCBar = -1; // fftSpec.getDCBars();
	rect = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, 0),
		psy_ui_realsize_make(SCOPE_BARS_WIDTH, 0));
	for (i = 0; i < SCOPE_SPEC_BANDS;
		i++, rect.left += SCOPE_BARS_WIDTH, rect.right += SCOPE_BARS_WIDTH)
	{
		//Remember, 0 -> top of spectrum, 128 bottom of spectrum.
		int curpeak, halfpeak;
		uint32_t colour;
		int aml = (int) (-self->db_left[i] * 2); // Reducing visible range from 128dB to 64dB.
		int amr = (int) (-self->db_right[i] * 2); // Reducing visible range from 128dB to 64dB.
		//int aml = - db_left[i];
		//int amr = - db_right[i];
		aml = (aml < 0) ? 0 : (aml > 128) ? 128 : aml;
		amr = (amr < 0) ? 0 : (amr > 128) ? 128 : amr;
		if (i <= DCBar) {
			curpeak = psy_min(aml, amr);
			halfpeak = 128;
			colour = CLBARDC;
		}
		else if (aml < amr) {
			curpeak = aml;
			halfpeak = amr;
			colour = CLLEFT;
		}
		else {
			curpeak = amr;
			halfpeak = aml;
			colour = CLRIGHT;
		}
		if (curpeak < self->bar_heights[i]) { self->bar_heights[i] = curpeak; }
		else if (!self->hold && self->bar_heights[i] < 128)
		{
			self->bar_heights[i] += (int)(128 / self->scope_spec_rate * 0.5);//two seconds to decay
			if (self->bar_heights[i] > 128) { self->bar_heights[i] = 128; }
		}

		rect.top = curpeak;
		rect.bottom = halfpeak;
		psy_ui_graphics_draw_solid_rectangle(g, rect, psy_ui_colour_make(colour));

		rect.top = rect.bottom;
		rect.bottom = 128;
		psy_ui_graphics_draw_solid_rectangle(g, rect, psy_ui_colour_make(CLBOTH));

		rect.top = self->bar_heights[i];
		rect.bottom = self->bar_heights[i] + 1;
		psy_ui_graphics_draw_solid_rectangle(g, rect, psy_ui_colour_make(CLBARPEAK));
	}	
}


void spectrumanalyzer_idle(SpectrumAnalyzer* self)
{
	if (!self->hold) {				
		self->invol = psy_audio_connections_wire_volume(
			&workspace_song(self->workspace)->machines_.connections,
			self->wire);
		psy_ui_component_invalidate(&self->component);
	}
}

void spectrumanalyzer_stop(SpectrumAnalyzer* self)
{
}

void FillLinearFromCircularBuffer(SpectrumAnalyzer* self, float inBuffer[], float outBuffer[],
	float vol, uintptr_t writepos)
{
	psy_audio_Buffer* buffer;
	uintptr_t buffersize;
	
	buffer = spectrumanalyzer_buffer(self, &buffersize);	
	if (buffer) {		
		uintptr_t index;
		
		if (writepos > (uintptr_t) self->scope_spec_samples) {
			index = writepos - self->scope_spec_samples;
		} else {
			index = buffersize - 1 - (self->scope_spec_samples - writepos);
		}
		//scopeBufferIndex is the position where it will write new data. 
		if (index + self->scope_spec_samples < buffersize) {
			self->dsp_noopt.movmul(inBuffer + index, outBuffer, self->scope_spec_samples, vol);
		} else {
			uintptr_t tail;
			uintptr_t front;

			tail = buffersize - index;
			front = self->scope_spec_samples - tail;
			if (tail > 0) {
				self->dsp_noopt.movmul(inBuffer + index, outBuffer, tail, vol);
			}
			if (front > 0) {
				self->dsp_noopt.movmul(inBuffer, outBuffer, front, vol);
			}
		}		
	}
}

psy_audio_Buffer* spectrumanalyzer_buffer(SpectrumAnalyzer * self,
	uintptr_t * numsamples)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;

	machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines_, self->wire.src);
	if (!machine) {
		*numsamples = 0;
		return NULL;
	}	
	buffer = psy_audio_machine_buffermemory(machine);
	*numsamples = psy_audio_machine_buffermemory_size(machine);	
	return buffer;
} 

void spectrumanalyzer_continue(SpectrumAnalyzer* self)
{
	self->hold = FALSE;
}

void spectrumanalyzer_hold(SpectrumAnalyzer* self)
{
	self->hold = TRUE;
}


bool spectrumanalyzer_stopped(SpectrumAnalyzer* self)
{
	return self->hold;
}
