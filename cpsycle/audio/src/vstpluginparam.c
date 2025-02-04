/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "vstpluginparam.h"

#ifdef PSYCLE_USE_VST2

#include "../../detail/os.h"
/* local */
#if defined(DIVERSALIS__OS__UNIX)
#define _inline static inline
#endif
#include "aeffectx.h"
#include "plugin_interface.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"


/* prototypes */
static int vstpluginparam_name(psy_audio_VstPluginParam*, char* text);
static int vstpluginparam_label(psy_audio_VstPluginParam*, char* text);
static void vstpluginparam_tweak(psy_audio_VstPluginParam*, double val);
static int vstpluginparam_describe(psy_audio_VstPluginParam*,
	char* text);
static double vstpluginparam_norm_value(psy_audio_VstPluginParam*);
static void vstpluginparam_range(psy_audio_VstPluginParam*,
	intptr_t* minval, intptr_t* maxval);

/* vtable */
static MachineParamVtable vstpluginparam_vtable;
static bool vstpluginparam_vtable_initialized = FALSE;

static void vstpluginparam_vtable_init(psy_audio_VstPluginParam* self)
{
	if (!vstpluginparam_vtable_initialized) {
		vstpluginparam_vtable =
			*(self->custommachineparam.machineparam.vtable);
		vstpluginparam_vtable.name =
			(fp_machineparam_name)
			vstpluginparam_name;
		vstpluginparam_vtable.label =
			(fp_machineparam_label)
			vstpluginparam_label;
		vstpluginparam_vtable.tweak =
			(fp_machineparam_tweak)
			vstpluginparam_tweak;
		vstpluginparam_vtable.normvalue =
			(fp_machineparam_norm_value)
			vstpluginparam_norm_value;
		vstpluginparam_vtable.range =
			(fp_machineparam_range)
			vstpluginparam_range;
		vstpluginparam_vtable.describe =
			(fp_machineparam_describe)
			vstpluginparam_describe;
	}
	self->custommachineparam.machineparam.vtable =
		&vstpluginparam_vtable;
}

/* implementation */
void psy_audio_vstpluginparam_init(psy_audio_VstPluginParam* self,
	struct AEffect* effect, uintptr_t index)
{
	psy_audio_custommachineparam_init(&self->custommachineparam,
		"",
		"",
		MPF_STATE,
		0,
		0xFFFF);
	vstpluginparam_vtable_init(self);	
	self->effect = effect;
	self->index = index;
}

void psy_audio_vstpluginparam_dispose(psy_audio_VstPluginParam* self)
{
	psy_audio_custommachineparam_dispose(&self->custommachineparam);
}

psy_audio_VstPluginParam* psy_audio_vstpluginparam_alloc(void)
{
	return (psy_audio_VstPluginParam*)malloc(sizeof(
		psy_audio_VstPluginParam));
}

psy_audio_VstPluginParam* psy_audio_vstpluginparam_allocinit(
	struct AEffect* effect, uintptr_t index)
{
	psy_audio_VstPluginParam* rv;

	rv = psy_audio_vstpluginparam_alloc();
	if (rv) {
		psy_audio_vstpluginparam_init(rv, effect, index);
	}
	return rv;
}

int vstpluginparam_label(psy_audio_VstPluginParam* self, char* text)
{
	text[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamLabel,
		(VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

int vstpluginparam_name(psy_audio_VstPluginParam* self, char* text)
{
	text[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamName,
		(VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

void vstpluginparam_tweak(psy_audio_VstPluginParam* self, double value)
{	
	self->effect->setParameter(self->effect, (VstInt32)self->index,
		(float)value);
}

int vstpluginparam_describe(psy_audio_VstPluginParam* self, char* text)
{
	text[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamDisplay,
		(VstInt32)self->index, 0, text, 0);
	return *text != '\0';
}

double vstpluginparam_norm_value(psy_audio_VstPluginParam* self)
{
	return (double)self->effect->getParameter(self->effect,
		(VstInt32)self->index);
}

void vstpluginparam_range(psy_audio_VstPluginParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = 0;
	*maxval = 0xFFFF;
}

#endif /* PSYCLE_USE_VST2 */
