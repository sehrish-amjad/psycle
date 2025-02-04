/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "vstplugininterface.h"

#ifdef PSYCLE_USE_VST2

#include "../../detail/os.h"

/* local */
#if defined(DIVERSALIS__OS__UNIX)
#define _inline static inline
#endif
#include "aeffectx.h"
#include "buffer.h"
#include "buffercontext.h"
/* std */
#include <stdlib.h>


/* implementation */
void psy_audio_vstinterface_init(psy_audio_VstInterface* self,
	AEffect* effect, void* user)
{
	self->effect = effect;
	if (self->effect) {
		self->effect->user = user;
	}
}

void psy_audio_vstinterface_open(psy_audio_VstInterface* self)
{
	if (self->effect) {
		self->effect->dispatcher(self->effect, effOpen, 0, 0, 0, 0);
	}
}

void psy_audio_vstinterface_close(psy_audio_VstInterface* self)
{
	if (self->effect) {
		self->effect->dispatcher(self->effect, effClose, 0, 0, 0, 0);
	}
}

void psy_audio_vstinterface_setsamplerate(psy_audio_VstInterface* self,
	double samplerate)
{
	self->effect->dispatcher(self->effect, effSetSampleRate, 0, 0, 0,
		(float)samplerate);
}

void psy_audio_vstinterface_setprocessprecision32(psy_audio_VstInterface* self)
{
	self->effect->dispatcher(self->effect, effSetProcessPrecision, 0,
		kVstProcessPrecision32, 0, 0);
}

void psy_audio_vstinterface_setblocksize(psy_audio_VstInterface* self,
	int32_t blocksize)
{
	self->effect->dispatcher(self->effect, effSetBlockSize, 0, blocksize, 0,
		0);
}

void psy_audio_vstinterface_mainschanged(psy_audio_VstInterface* self)
{
	self->effect->dispatcher(self->effect, effMainsChanged, 0, 1, 0, 0);
}

void psy_audio_vstinterface_startprocess(psy_audio_VstInterface* self)
{
	self->effect->dispatcher(self->effect, effStartProcess, 0, 0, 0, 0);
}

void psy_audio_vstinterface_work(psy_audio_VstInterface* self,
	psy_audio_BufferContext* bc)
{
	self->effect->processReplacing(self->effect,
		bc->output_->samples, bc->output_->samples,
		(VstInt32)psy_audio_buffercontext_num_samples(bc));
}

void psy_audio_vstinterface_tick(psy_audio_VstInterface* self,
	struct VstEvents* vstevents)
{
	vstevents->reserved = 0;
	self->effect->dispatcher(self->effect, effProcessEvents, 0, 0,
		vstevents, 0);
}

psy_audio_MachineMode psy_audio_vstinterface_mode(psy_audio_VstInterface* self)
{	
	return ((self->effect->flags & effFlagsIsSynth) == effFlagsIsSynth)
		? psy_audio_MACHMODE_GENERATOR
		: psy_audio_MACHMODE_FX;
}

void psy_audio_vstinterface_beginprogram(psy_audio_VstInterface* self)
{	
	self->effect->dispatcher(self->effect, effBeginSetProgram, 0, 0, 0, 0);	
}

void psy_audio_vstinterface_setprogram(psy_audio_VstInterface* self,
	uintptr_t prgidx)
{
	VstIntPtr prg;

	if (prgidx == psy_INDEX_INVALID) {
		prg = -1;
	} else {
		prg = (VstIntPtr)prgidx;
	}
	self->effect->dispatcher(self->effect, effSetProgram, 0, prg, 0, 0);
}

void psy_audio_vstinterface_endprogram(psy_audio_VstInterface* self)
{
	self->effect->dispatcher(self->effect, effEndSetProgram, 0, 0, 0, 0);
}

uint8_t psy_audio_vstinterface_program(psy_audio_VstInterface* self)
{
	return (uint8_t)self->effect->dispatcher(self->effect,
		effGetProgram, 0, 0, 0, 0);
}

uintptr_t psy_audio_vstinterface_numprograms(const psy_audio_VstInterface* self)
{
	return (uintptr_t)self->effect->numPrograms;
}

bool psy_audio_vstinterface_hasprogramchunk(const psy_audio_VstInterface* self)
{
	return (self->effect->flags & effFlagsProgramChunks) != 0;
}

uintptr_t psy_audio_vstinterface_numinputs(const psy_audio_VstInterface* self)
{
	if (self->effect->numInputs > 0) {
		return (uintptr_t)self->effect->numInputs;
	}
	return 0;
}

uintptr_t psy_audio_vstinterface_numoutputs(const psy_audio_VstInterface* self)
{
	if (self->effect->numOutputs > 0) {
		return (uintptr_t)self->effect->numOutputs;
	}
	return 0;
}

float psy_audio_vstinterface_parametervalue(const psy_audio_VstInterface* self,
	uintptr_t index)
{
	return (float)self->effect->getParameter(self->effect, (VstInt32)index);
}

void psy_audio_vstinterface_setparametervalue(psy_audio_VstInterface* self,
	uintptr_t index, float value)
{
	self->effect->setParameter(self->effect, (VstInt32)index, value);
}

uintptr_t psy_audio_vstinterface_numparameters(const
	psy_audio_VstInterface* self)
{
	if (self->effect->numParams > 0) {
		return (uintptr_t)self->effect->numParams;
	}
	return 0;
}

void* psy_audio_vstinterface_chunkdata(psy_audio_VstInterface* self,
	bool onlycurrprogram, uintptr_t* rv_size)
{	
	VstIntPtr size;

	assert(self);
	assert(rv_size);

	if (self->effect->flags & effFlagsProgramChunks) {		
		void* ptr;

		ptr = NULL;
		size = self->effect->dispatcher(self->effect, effGetChunk,
			onlycurrprogram, 0, &ptr, 0);
		*rv_size = (uintptr_t)size;
		return ptr;
	}
	return NULL;
}

bool psy_audio_vstinterface_setchunkdata(psy_audio_VstInterface* self,
	bool onlycurrprogram, void* data, uintptr_t size)
{
	if (self->effect->flags & effFlagsProgramChunks) {
		self->effect->dispatcher(self->effect,
			effSetChunk, onlycurrprogram, (VstIntPtr)size, data, 0);
		return TRUE;
	}
	return FALSE;
}

bool psy_audio_vstinterface_has_editor(const psy_audio_VstInterface* self)
{
	return (self->effect->flags & effFlagsHasEditor) == effFlagsHasEditor;
}

void psy_audio_vstinterface_open_editor(psy_audio_VstInterface* self,
	void* handle)
{
	self->effect->dispatcher(self->effect, effEditOpen, 0, 0, handle, 0);
}

void psy_audio_vstinterface_close_editor(psy_audio_VstInterface* self)
{
	self->effect->dispatcher(self->effect, effEditClose, 0, 0, 0, 0);
}

void psy_audio_vstinterface_editor_idle(psy_audio_VstInterface* self)
{	
	self->effect->dispatcher(self->effect, effEditIdle, 0, 0, 0, 0);
}

#endif /* PSYCLE_USE_VST2 */
