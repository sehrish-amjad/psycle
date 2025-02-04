/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pluginscanthread.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/os.h"
#include "../../detail/portable.h"


#if defined DIVERSALIS__OS__MICROSOFT
static unsigned int __stdcall pluginscanthread(void* context)
#else
static unsigned int pluginscanthread(void* context)
#endif
{
	PluginScanThread* self;

	assert(context);

	self = (PluginScanThread*)context;
	psy_audio_plugincatcher_scan(self->plugincatcher);
	self->filescanned = 0;
	self->scantaskstart = 0;
	free(self->scanfilename);
	self->scanfilename = NULL;
	return 0;
}

/* prototypes */
static void pluginscanthread_connect_plugincatcher(PluginScanThread*);
static void pluginscanthread_on_scan_end(PluginScanThread*, psy_audio_PluginCatcher* sender);
static void pluginscanthread_on_scan_file(PluginScanThread*, psy_audio_PluginCatcher* sender,
	const char* path, int type);
static void pluginscanthread_on_scan_progress(PluginScanThread*, psy_audio_PluginCatcher* sender,
	int progress);
void pluginscanthread_on_scan_start(PluginScanThread*, psy_audio_PluginCatcher* sender);
void pluginscanthread_on_scan_end(PluginScanThread*, psy_audio_PluginCatcher* sender);
static void pluginscanthread_on_scan_task_start(PluginScanThread*,
	psy_audio_PluginCatcher* sender, psy_audio_PluginScanTask*);
static void pluginscanthread_on_plugin_cache_changed(PluginScanThread*,
	psy_audio_PluginCatcher* sender);

void pluginscanthread_init(PluginScanThread* self,
	psy_audio_PluginCatcher* plugincatcher,
	psy_Logger* progress)
{
	assert(self);
	assert(plugincatcher);

	psy_signal_init(&self->signal_plugincachechanged);
	psy_signal_init(&self->signal_scanprogress);
	psy_signal_init(&self->signal_scanfile);
	psy_signal_init(&self->signal_scanstart);
	psy_signal_init(&self->signal_scanend);
	psy_signal_init(&self->signal_scantaskstart);
	psy_thread_init(&self->pluginscanthread);
	psy_lock_init(&self->pluginscanlock);
	self->plugincatcher = plugincatcher;
	self->filescanned = 0;
	self->scanstart = 0;
	self->scanend = 0;
	self->scantaskstart = 0;
	self->plugincachechanged = 0;
	self->scanprogresschanged = 0;
	self->scanfilename = NULL;
	self->scanplugintype = psy_audio_UNDEFINED;
	self->progress = progress;
	pluginscanthread_connect_plugincatcher(self);	
}

void pluginscanthread_dispose(PluginScanThread* self)
{
	assert(self);
	
	psy_lock_dispose(&self->pluginscanlock);
	psy_thread_dispose(&self->pluginscanthread);	
	free(self->scanfilename);
	self->scanfilename = NULL;
	psy_signal_dispose(&self->signal_plugincachechanged);
	psy_signal_dispose(&self->signal_scanprogress);
	psy_signal_dispose(&self->signal_scanfile);
	psy_signal_dispose(&self->signal_scanstart);
	psy_signal_dispose(&self->signal_scanend);
	psy_signal_dispose(&self->signal_scantaskstart);
}

void pluginscanthread_connect_plugincatcher(PluginScanThread* self)
{
	assert(self);

	psy_signal_connect(&self->plugincatcher->signal_changed, self,
		pluginscanthread_on_plugin_cache_changed);
	psy_signal_connect(&self->plugincatcher->signal_scanprogress, self,
		pluginscanthread_on_scan_progress);
	psy_signal_connect(&self->plugincatcher->signal_scanfile, self,
		pluginscanthread_on_scan_file);
	psy_signal_connect(&self->plugincatcher->signal_scanstart, self,
		pluginscanthread_on_scan_start);
	psy_signal_connect(&self->plugincatcher->signal_scanend, self,
		pluginscanthread_on_scan_end);
	psy_signal_connect(&self->plugincatcher->signal_taskstart, self,
		pluginscanthread_on_scan_task_start);
}

void pluginscanthread_start(PluginScanThread* self)
{
	assert(self);
	
	if (!psy_audio_plugincatcher_scanning(self->plugincatcher)) {
		free(self->scanfilename);
		self->scanfilename = NULL;
		self->filescanned = 0;
		self->plugincachechanged = 0;
		self->scanstart = 0;
		self->scantaskstart = 0;
		free(self->scanfilename);
		self->scanplugintype = psy_audio_UNDEFINED;
#ifdef PSYCLE_THREAD_PLUGIN_SCAN		
		psy_thread_start(&self->pluginscanthread, self,
			pluginscanthread);		
#else		
		pluginscanthread(self);
#endif		
			
	}
}

void pluginscanthread_on_scan_progress(PluginScanThread* self, psy_audio_PluginCatcher* sender,
	int progress)
{
	psy_lock_enter(&self->pluginscanlock);
	self->scanprogresschanged = 1;
	self->scanprogress = progress;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_file(PluginScanThread* self, psy_audio_PluginCatcher* sender,
	const char* path, int type)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->filescanned = 1;	
	psy_strreset(&self->scanfilename, path);
	self->scanplugintype = type;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_start(PluginScanThread* self, psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->scanstart = 1;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_end(PluginScanThread* self, psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->scanend = 1;	
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_scan_task_start(PluginScanThread* self, psy_audio_PluginCatcher* sender,
	psy_audio_PluginScanTask* task)
{
	assert(self);
	assert(task);

	psy_lock_enter(&self->pluginscanlock);
	self->scantaskstart = 1;
	self->lastscantask = *task;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_on_plugin_cache_changed(PluginScanThread* self,
	psy_audio_PluginCatcher* sender)
{
	assert(self);

	psy_lock_enter(&self->pluginscanlock);
	self->plugincachechanged = 1;
	psy_lock_leave(&self->pluginscanlock);
}

void pluginscanthread_idle(PluginScanThread* self)
{
	if (self->scanstart) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanstart, self, 0);
		self->scanstart = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->scanend) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanend, self, 0);
		if (self->progress) {
			psy_logger_progress(self->progress, PSY_PROGRESS_STATE_END);
		}
		self->scanend = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->scantaskstart) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scantaskstart, self, 1,
			&self->lastscantask);
		self->scantaskstart = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->filescanned) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_scanfile, self, 2, self->scanfilename,
			self->scanplugintype);
		self->filescanned = 0;
		free(self->scanfilename);
		self->scanfilename = NULL;		
		psy_lock_leave(&self->pluginscanlock);
	}
	if (self->scanprogresschanged) {		
		psy_signal_emit(&self->signal_scanprogress, self, 1,
			self->scanprogress);
		if (self->scanprogress != 0 && self->progress) {
			psy_logger_progress(self->progress, PSY_PROGRESS_STATE_TICK);
		}
		self->scanprogresschanged = 0;
	}
	if (self->plugincachechanged) {
		psy_lock_enter(&self->pluginscanlock);
		psy_signal_emit(&self->signal_plugincachechanged, self, 0);
		self->plugincachechanged = 0;
		psy_lock_leave(&self->pluginscanlock);
	}
}
