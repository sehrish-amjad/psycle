// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "machinefactory.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "audiorecorder.h"
#include "plugin.h"
#include "luaplugin.h"
#include "sampler.h"
#include "xmsampler.h"
#ifdef PSYCLE_USE_VST2
#include "vstplugin.h"
#endif
#ifdef PSYCLE_USE_LADSPA
#include "ladspaplugin.h"
#endif
#ifdef PSYCLE_USE_LV2
#include "lv2plugin.h"
#endif
#include "machineproxy.h"
#include "virtualgenerator.h"

#include <stdlib.h>
#include <string.h>

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

void psy_audio_machinefactory_init(psy_audio_MachineFactory* self,
	psy_audio_MachineCallback* callback,
	psy_audio_PluginCatcher* catcher,
	psy_Logger* logger)
{
	assert(self);
	
	self->machinecallback = callback;
	self->catcher = catcher;
	self->logger = logger;
#ifdef PSYCLE_USE_MACHINEPROXY
	self->createasproxy = TRUE;
#else
	self->createasproxy = FALSE;
#endif
	self->loadnewgamefxblitz = 0;
	self->errstr = strdup("");
}

void psy_audio_machinefactory_dispose(psy_audio_MachineFactory* self)
{
	free(self->errstr);
	self->errstr = NULL;
}

void psy_audio_machinefactory_set_callback(psy_audio_MachineFactory* self,
	psy_audio_MachineCallback* callback)
{
	assert(self);
	
	self->machinecallback = callback;
}

void psy_audio_machinefactory_set_plugin_catcher(psy_audio_MachineFactory* self,
	psy_audio_PluginCatcher* plugin_catcher)
{
	assert(self);
	
	self->catcher = plugin_catcher;
}

void psy_audio_machinefactory_seterrorstr(psy_audio_MachineFactory* self, const char* str)
{
	if (str != self->errstr) {
		free(self->errstr);
		self->errstr = strdup(str);
	}
}

void psy_audio_machinefactory_createasproxy(psy_audio_MachineFactory* self)
{
	self->createasproxy = TRUE;
}

void psy_audio_machinefactory_createwithoutproxy(psy_audio_MachineFactory* self)
{
	self->createasproxy = FALSE;
}

psy_audio_Machine* psy_audio_machinefactory_make_machine(psy_audio_MachineFactory* self,
	psy_audio_MachineType type, const char* plugincatchername, uintptr_t instindex)
{
	char fullpath[_MAX_PATH];

	if (!self->catcher) {
		return 0;
	}
	return psy_audio_machinefactory_makemachinefrompath(self, type,
		psy_audio_plugincatcher_modulepath(self->catcher, type,
		self->loadnewgamefxblitz,
		plugincatchername, fullpath),
		psy_audio_plugincatcher_extractshellidx(plugincatchername),
		instindex);
}

psy_audio_Machine* psy_audio_machinefactory_make_info(psy_audio_MachineFactory* self,
	const psy_audio_MachineInfo* machineinfo)
{
	return psy_audio_machinefactory_makemachinefrompath(self,
		machineinfo->type,
		machineinfo->modulepath,
		machineinfo->shellidx,
		psy_INDEX_INVALID);
}

psy_audio_Machine* psy_audio_machinefactory_makemachinefrompath(
	psy_audio_MachineFactory* self, psy_audio_MachineType type,
	const char* path, uintptr_t shellidx, uintptr_t instindex)
{
	psy_audio_Machine* rv;

	rv = NULL;
	switch (type) {
	case psy_audio_MASTER: {
		psy_audio_Master* master;
		
		master = (psy_audio_Master*)malloc(sizeof(psy_audio_Master));
		if (master) {
			int status;
			
			status = psy_audio_master_init(master, self->machinecallback);
			if (status == PSY_OK) {
				rv = psy_audio_master_base(master);
			} else {
				psy_audio_machine_dispose(psy_audio_master_base(master));
				free(master);
			}
		}		
		break; }
	case psy_audio_DUMMY: {
		psy_audio_DummyMachine* dummy;
		
		dummy = (psy_audio_DummyMachine*)malloc(sizeof(psy_audio_DummyMachine));
		if (dummy) {
			psy_audio_dummymachine_init(dummy, self->machinecallback);
			rv = (psy_audio_Machine*) dummy;
		}		
		break; }
	case psy_audio_DUPLICATOR: {
		psy_audio_Duplicator* duplicator;
		
		duplicator = (psy_audio_Duplicator*)malloc(sizeof(psy_audio_Duplicator));
		if (duplicator) {
			psy_audio_duplicator_init(duplicator, self->machinecallback);
			rv = (psy_audio_Machine*) duplicator;
		}		
		break; }
	case psy_audio_DUPLICATOR2: {
		psy_audio_Duplicator2* duplicator2;
		
		duplicator2 = (psy_audio_Duplicator2*)malloc(sizeof(psy_audio_Duplicator2));
		if (duplicator2) {
			psy_audio_duplicator2_init(duplicator2, self->machinecallback);
			rv = (psy_audio_Machine*) duplicator2;
		}		
		break; }
	case psy_audio_MIXER: {
		psy_audio_Mixer* mixer;
		
		mixer = (psy_audio_Mixer*)malloc(sizeof(psy_audio_Mixer));
		if (mixer) {
			psy_audio_mixer_init(mixer, self->machinecallback);
			rv = (psy_audio_Machine*)mixer;
		}		
		break; }
	case psy_audio_RECORDER: {
		psy_audio_AudioRecorder* recorder;
		
		recorder = (psy_audio_AudioRecorder*)malloc(sizeof(psy_audio_AudioRecorder));
		if (recorder) {
			psy_audio_audiorecorder_init(recorder, self->machinecallback);
			rv = (psy_audio_Machine*)recorder;
		}		
		break; }
	case psy_audio_SAMPLER: {
		psy_audio_Sampler* sampler;			

		sampler = psy_audio_sampler_allocinit(self->machinecallback);
		if (sampler) {
			rv = psy_audio_sampler_base(sampler);
		}
		break; }
	case psy_audio_XMSAMPLER: {
		psy_audio_XMSampler* sampler;
		
		sampler = psy_audio_xmsampler_allocinit(self->machinecallback);
		if (sampler) {				
			rv = psy_audio_xmsampler_base(sampler);
		}		
		break; }
#ifdef PSYCLE_USE_VST2
	case psy_audio_VST:
	case psy_audio_VSTFX: {			
		psy_audio_VstPlugin* plugin;			

		plugin = (psy_audio_VstPlugin*)malloc(sizeof(psy_audio_VstPlugin));
		if (plugin) {
			int status;
			
			status = psy_audio_vstplugin_init(plugin, self->machinecallback,
				path);
			if (status == PSY_OK && psy_audio_machine_info(
					psy_audio_vstplugin_base(plugin))) {
				rv = psy_audio_vstplugin_base(plugin);
			} else {
				psy_audio_machine_dispose(psy_audio_vstplugin_base(plugin));
				free(plugin);
			}
		} else {
			rv = NULL;
		}
		break; }
#endif			
	case psy_audio_PLUGIN: {
		psy_audio_Machine* plugin;

		plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_Plugin));
		if (plugin) {
			int status;
			
			status = psy_audio_plugin_init((psy_audio_Plugin*)plugin,
				self->machinecallback, path,
				self->catcher->native_root_);
			if (status == PSY_OK && psy_audio_machine_info(plugin)) {
				rv = plugin;
			} else {
				psy_audio_machine_dispose(plugin);
				free(plugin);
			}
		}
		break; }
	case psy_audio_LUA: {
		psy_audio_LuaPlugin* plugin;

		plugin = (psy_audio_LuaPlugin*)malloc(sizeof(psy_audio_LuaPlugin));
		if (plugin) {
			int status;
			
			status = psy_audio_luaplugin_init(plugin, self->machinecallback,
				path, self->logger);				
			if (status == PSY_OK || psy_audio_machine_info(
					psy_audio_luaplugin_base(plugin))) {
				rv = psy_audio_luaplugin_base(plugin);
			} else {
				psy_audio_machinefactory_seterrorstr(self, plugin->script.errstr);
				psy_audio_machine_dispose(psy_audio_luaplugin_base(plugin));
				free(plugin);
			}
		}
		break; }
#ifdef PSYCLE_USE_LADSPA		
	case psy_audio_LADSPA: {
		psy_audio_Machine* plugin;

		plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_LadspaPlugin));
		if (plugin) {
			int status;
			
			status = psy_audio_ladspaplugin_init((psy_audio_LadspaPlugin*)
				plugin, self->machinecallback, path, shellidx);
			if (status == PSY_OK && psy_audio_machine_info(plugin)) {
				rv = plugin;
			} else {
				psy_audio_machine_dispose(plugin);
				free(plugin);
			}
		}
		break; }
#endif		
	case psy_audio_VIRTUALGENERATOR: {
		psy_audio_VirtualGenerator* virtualgenerator;

		virtualgenerator = (psy_audio_VirtualGenerator*)malloc(sizeof(psy_audio_VirtualGenerator));
		if (virtualgenerator) {
			psy_audio_virtualgenerator_init(virtualgenerator,
				self->machinecallback, shellidx, instindex);
			rv = &virtualgenerator->custommachine.machine;
		}
		break; }
#ifdef PSYCLE_USE_LV2			
	case psy_audio_LV2: {
		psy_audio_Machine* plugin;

		plugin = (psy_audio_Machine*)malloc(sizeof(psy_audio_LV2Plugin));
		if (plugin) {
			int status;
			
			status = psy_audio_lv2plugin_init((psy_audio_LV2Plugin*)plugin,
				self->machinecallback, path, shellidx);
			if (status == PSY_OK && psy_audio_machine_info(plugin)) {
				rv = plugin;
			} else {
				psy_audio_machine_dispose(plugin);
				free(plugin);
			}
		}
		break; }
#endif						
	default:			
		break;
	}
	if (rv && self->createasproxy) {
		psy_audio_MachineProxy* proxy;

		proxy = psy_audio_machineproxy_allocinit(rv);
		if (proxy) {
			rv = psy_audio_machineproxy_base(proxy);
		} else {
			machine_base_dispose(rv);
			free(rv);
			rv = 0;
		}		
	}	
	return rv;
}

void psy_audio_machinefactory_loadnewgamefxandblitzifversionunknown(
	psy_audio_MachineFactory* self)
{
	self->loadnewgamefxblitz = TRUE;
}

void psy_audio_machinefactory_loadoldgamefxandblitzifversionunknown(
	psy_audio_MachineFactory* self)
{
	self->loadnewgamefxblitz = FALSE;
}
