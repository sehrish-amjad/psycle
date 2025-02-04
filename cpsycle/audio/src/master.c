/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "master.h"
/* local */
#include "machines.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
/* dsp */
#include <convert.h>
#include <operations.h>
#include <valuemapper.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define psy_audio_MASTER_MINCOLS 6
#define psy_audio_MASTER_NUMROWS 3

/* prototypes */
static void master_generateaudio(psy_audio_Master*, psy_audio_BufferContext*);
static void master_seqtick(psy_audio_Master*, uintptr_t channel,
	const psy_audio_PatternEvent* ev);
static int master_mode(psy_audio_Master* self)
{
	return psy_audio_MACHMODE_MASTER;
}
static void master_dispose(psy_audio_Master*);
static const psy_audio_MachineInfo* info(psy_audio_Master*);
static int master_loadspecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static int master_savespecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static const char* master_editname(psy_audio_Master* self)
{
	return "Psycle Master and Minimixer";
}
static uintptr_t numinputs(psy_audio_Master* self) { return 2; }
static uintptr_t numoutputs(psy_audio_Master* self) { return 2; }
static uintptr_t slot(psy_audio_Master* self) { return psy_audio_MASTER_INDEX; }
/* Parameters */
static psy_audio_MachineParam* parameter(psy_audio_Master*, uintptr_t param);
static psy_audio_MachineParam* tweakparameter(psy_audio_Master*,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_Master*);
static uintptr_t numtweakparameters(psy_audio_Master*);
static uintptr_t numparametercols(psy_audio_Master*);
static uintptr_t numinputwires(psy_audio_Master*);
static uintptr_t paramstrobe(const psy_audio_Master*);
static void master_describeeditname(psy_audio_Master*, char* text,
	uintptr_t slot);
static void master_title_name(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, char* text);
static void master_title_describe(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);
static void master_slider_tweak(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, double value);
static void master_slider_normvalue(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, double* rv);
static void master_level_normvalue(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, double* rv);
static void master_level_describe(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);
static void onconnected(psy_audio_Master*, psy_audio_Connections*,
	uintptr_t outputslot, uintptr_t inputslot);
static void ondisconnected(psy_audio_Master*, psy_audio_Connections*,
	uintptr_t outputslot, uintptr_t inputslot);


static psy_audio_MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	psy_audio_MASTER | 32 | 64,
	psy_audio_MACHMODE_MASTER,
	"Master"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Master",
	"Psycledelics",
	"help",
	psy_audio_MASTER,
	0,
	0,
	"",
	"",
	"",
	psy_INDEX_INVALID,
	""
};

const psy_audio_MachineInfo* psy_audio_master_info(void) { return &MacInfo; }


/* vtable */
static MachineVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_audio_Master* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.generateaudio =
			(fp_machine_generateaudio)
			master_generateaudio;
		vtable.seqtick =
			(fp_machine_seqtick)
			master_seqtick;
		vtable.mode =
			(fp_machine_mode)
			master_mode;
		vtable.info =
			(fp_machine_info)
			info;
		vtable.dispose =
			(fp_machine_dispose)
			master_dispose;		
		vtable.numinputs =
			(fp_machine_numinputs)
			numinputs;
		vtable.numoutputs =
			(fp_machine_numoutputs)
			numoutputs;
		vtable.slot =
			(fp_machine_slot)
			slot;
		vtable.loadspecific =
			(fp_machine_loadspecific)
			master_loadspecific;
		vtable.savespecific =
			(fp_machine_savespecific)
			master_savespecific;
		vtable.editname =
			(fp_machine_editname)
			master_editname;		
		/* Parameter */
		vtable.parameter =
			(fp_machine_parameter)
			parameter;
		vtable.tweakparameter =
			(fp_machine_tweakparameter)
			tweakparameter;
		vtable.numparametercols =
			(fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters =
			(fp_machine_numparameters)
			numparameters;
		vtable.numtweakparameters =
			(fp_machine_numparameters)
			numtweakparameters;		
		vtable.paramstrobe =
			(fp_machine_paramstrobe)
			paramstrobe;
		vtable_initialized = TRUE;
	}
	self->custommachine.machine.vtable = &vtable;
}

/* implementation */
int psy_audio_master_init(psy_audio_Master* self,
	psy_audio_MachineCallback* callback)
{	
	int status;
	
	status = PSY_OK;
	memset(self, 0, sizeof(psy_audio_Master));
	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	psy_audio_custommachine_set_amp_range(&self->custommachine,
		PSY_DSP_AMP_RANGE_IGNORE);
	self->volume_ = 1.0;
	self->strobe_ = 0;
	psy_audio_custommachineparam_init(&self->param_info_,
		"", "", MPF_INFOLABEL | MPF_SMALL, 0, 0x1FFE);
	psy_signal_connect(&self->param_info_.machineparam.signal_name, self,
		master_title_name);
	psy_signal_connect(&self->param_info_.machineparam.signal_describe, self,
		master_title_describe);
	psy_audio_custommachineparam_init(&self->param_slider_,
		"Vol", "Vol", MPF_SLIDER | MPF_SMALL, 0, 0x1FE);
	psy_signal_connect(&self->param_slider_.machineparam.signal_tweak, self,
		master_slider_tweak);
	psy_signal_connect(&self->param_slider_.machineparam.signal_normvalue, self,
		master_slider_normvalue);
	psy_signal_connect(&self->param_slider_.machineparam.signal_describe, self,
		master_level_describe);
	psy_audio_custommachineparam_init(&self->param_level_,
		"", "", MPF_LEVEL | MPF_SMALL, 0, 0xFFFF);
	psy_signal_connect(&self->param_level_.machineparam.signal_normvalue, self,
		master_level_normvalue);
	psy_audio_machine_set_position(&self->custommachine.machine, 320, 200);	
	self->dostart_ = TRUE;
	return status;
}

void master_dispose(psy_audio_Master* self)
{			
	psy_audio_custommachineparam_dispose(&self->param_info_);
	psy_audio_custommachineparam_dispose(&self->param_slider_);
	psy_audio_custommachineparam_dispose(&self->param_level_);
	psy_audio_custommachine_dispose(&self->custommachine);
}

const psy_audio_MachineInfo* info(psy_audio_Master* self)
{
	return &MacInfo;
}

void master_generateaudio(psy_audio_Master* self, psy_audio_BufferContext* bc)
{	
	psy_audio_buffer_mul(bc->output_,
		psy_audio_buffercontext_num_samples(bc), self->volume_);
}

void master_seqtick(psy_audio_Master* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{
	if (ev->cmd == psy_audio_PATTERNCMD_SET_VOLUME) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		if (machines) {
			if (ev->inst == psy_audio_NOTECOMMANDS_INST_EMPTY &&
				ev->vol == psy_audio_MASTER_INDEX) {
				double nv;

				nv = ev->parameter / (double)0x1FE;
				self->volume_ = nv * nv * 4.0;				
				// self->volume_ = psy_dsp_map_255_1(ev->parameter) * 2;
			} else if (ev->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
				psy_audio_MachineSockets* sockets;

				sockets = psy_audio_connections_at(&machines->connections,
					ev->vol);
				if (sockets) {
					psy_audio_WireSocket* output_socket;
						
					output_socket = psy_audio_wiresockets_at(&sockets->outputs,
						ev->inst);
					if (output_socket) {
						double nv;
							
						nv = ev->parameter / (double)0x1FE;
						/* 
						** here ev->vol is used by the sequencer as src mac slot
						*/
						psy_audio_connections_set_wire_volume(
							&machines->connections, psy_audio_wire_make(
								ev->vol, output_socket->slot),
								(nv * nv * 4.0));
					}
				}
			}
		}
	}
}

void master_describeeditname(psy_audio_Master* self, char* text, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(psy_audio_machine_machines(
		&self->custommachine.machine), slot);
	psy_snprintf(text, 128, "%s", (machine != NULL) ?
		psy_audio_machine_edit_name(machine) : "");
}

void master_title_name(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, char* text)
{
	if (sender->index == 0) {
		psy_snprintf(text, 128, "%s", "Master");
	} else {
		psy_snprintf(text, 10, "m%d", (int)sender->index - 1);
	}
}

void master_title_describe(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, int* active, char* text)
{
	*active = 0;
	if (sender->index == 0) {
		*active = 0;
	} else {
		psy_audio_MachineSockets* sockets;		
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		sockets = psy_audio_connections_at(&machines->connections,
			psy_audio_MASTER_INDEX);
		if (sockets) {
			psy_audio_WireSocket* input_socket;

			input_socket = psy_audio_wiresockets_at(&sockets->inputs,
				sender->index - 1);
			if (input_socket) {
				master_describeeditname(self, text, input_socket->slot);
				*active = 1;
			}
		}
	}
}

void master_slider_tweak(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, double value)
{
	if (sender->index == 0) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		if (machines) {
			self->volume_ = value * value * 4.0;
		}
	} else {
		psy_audio_MachineSockets* sockets;		
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		sockets = psy_audio_connections_at(&machines->connections,
			psy_audio_MASTER_INDEX);
		if (sockets) {
			psy_audio_WireSocket* input_socket;

			input_socket = psy_audio_wiresockets_at(&sockets->inputs,
				sender->index - 1);
			if (input_socket) {
				input_socket->volume = (value * value * 4.0);
			}
		}
	}
}

void master_slider_normvalue(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, double* rv)
{
	*rv = 0.0;
	if (sender->index == 0) {		
		*rv = sqrt(self->volume_) * 0.5;
	} else {
		psy_audio_MachineSockets* sockets;		
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		sockets = psy_audio_connections_at(&machines->connections,
			psy_audio_MASTER_INDEX);
		if (sockets) {
			psy_audio_WireSocket* input_socket;

			input_socket = psy_audio_wiresockets_at(
				&sockets->inputs, sender->index - 1);
			if (input_socket) {
				*rv = sqrt(input_socket->volume) * 0.5;
			}
		}
	}
}

void master_level_normvalue(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, double* rv)
{
	*rv = 0.0;
	if (sender->index == 0) {		
		psy_audio_Machines* machines;
		psy_audio_Machine* machine;
		psy_audio_Buffer* memory;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		machine = psy_audio_machines_at(machines, psy_audio_MASTER_INDEX);
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			*rv = psy_audio_buffer_rmsdisplay(memory);
		}
	} else {
		psy_audio_MachineSockets* sockets;		
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		sockets = psy_audio_connections_at(&machines->connections,
			psy_audio_MASTER_INDEX);
		if (sockets) {
			psy_audio_WireSocket* input_socket;

			input_socket = psy_audio_wiresockets_at(&sockets->inputs,
				sender->index - 1);
			if (input_socket) {							
				psy_audio_Machine* machine;
				psy_audio_Buffer* memory;
				
				machine = psy_audio_machines_at(machines, input_socket->slot);
				if (machine) {
					memory = psy_audio_machine_buffermemory(machine);
					if (memory) {
						*rv = psy_audio_buffer_rmsdisplay(memory);
					}
				}
			}
		}
	}
}

void master_level_describe(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, int* active, char* text)
{
	if (sender->index == 0) {
		psy_snprintf(text, 10, "%.2f dB", psy_dsp_convert_amp_to_db(
			self->volume_));
		*active = 1;
	} else {
		
		psy_audio_MachineSockets* sockets;		
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->custommachine.machine);
		sockets = psy_audio_connections_at(&machines->connections,
			psy_audio_MASTER_INDEX);		
		*active = 0;
		if (sockets) {
			psy_audio_WireSocket* input_socket;

			input_socket = psy_audio_wiresockets_at(&sockets->inputs,
				sender->index - 1);
			if (input_socket) {				
				psy_snprintf(text, 10, "%.2f dB", psy_dsp_convert_amp_to_db(
					input_socket->volume));
				*active = 1;
			}
		}
	}
}

psy_audio_MachineParam* parameter(psy_audio_Master* self, uintptr_t id)
{
	uintptr_t col = id / psy_audio_MASTER_NUMROWS;
	uintptr_t row = id % psy_audio_MASTER_NUMROWS;

	if (col < numinputwires(self) + 1) {
		if (row == 0) {
			self->param_info_.index = col;
			return &self->param_info_.machineparam;
		} else if (row == 1) {
			self->param_slider_.index = col;
			return &self->param_slider_.machineparam;
		} else if (row == 2) {
			self->param_level_.index = col;
			return &self->param_level_.machineparam;
		}
	}
	return NULL;	
}

psy_audio_MachineParam* tweakparameter(psy_audio_Master* self, uintptr_t id)
{
	if (id == 0) {
		self->param_slider_.index = 0;		
		return &self->param_slider_.machineparam;
	}
	return NULL;
}

uintptr_t numtweakparameters(psy_audio_Master* self)
{
	return numparametercols(self);
}

uintptr_t numparameters(psy_audio_Master* self)
{
	return numparametercols(self) * psy_audio_MASTER_NUMROWS;
}

uintptr_t numparametercols(psy_audio_Master* self)
{	
	return psy_max(numinputwires(self) + 1, psy_audio_MASTER_MINCOLS);	
}

uintptr_t numinputwires(psy_audio_Master* self)
{
	psy_audio_MachineSockets* sockets;
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(&self->custommachine.machine);
	sockets = psy_audio_connections_at(&machines->connections,
		psy_audio_MASTER_INDEX);
	if (sockets) {
		return wiresockets_size(&sockets->inputs);
	}
	return 0;
}

uintptr_t paramstrobe(const psy_audio_Master* self)
{
	if (self->dostart_) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(
			&((psy_audio_Master*)self)->custommachine.machine);
		if (machines) {
			psy_signal_connect(&machines->connections.signal_connected,
				(psy_audio_Master*)self, onconnected);
			psy_signal_connect(&machines->connections.signal_disconnected,
				(psy_audio_Master*)self, ondisconnected);
			((psy_audio_Master*)self)->dostart_ = FALSE;
		}
	}
	return self->strobe_;
}

void onconnected(psy_audio_Master* self, psy_audio_Connections* connections,
	uintptr_t outputslot, uintptr_t inputslot)
{
	++self->strobe_;
}

void ondisconnected(psy_audio_Master* self, psy_audio_Connections* connections,
	uintptr_t outputslot, uintptr_t inputslot)
{
	++self->strobe_;
}

int master_loadspecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;
	int32_t outdry = 256;
	unsigned char decreaseOnClip = 0;
	int status;

	/* size of this part params to load */
	if ((status = psyfile_read(songfile->file, &size, sizeof(size)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &outdry, sizeof(outdry)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &decreaseOnClip,
		sizeof(decreaseOnClip)))) {
		return status;
	}
	self->volume_ = outdry / 256.0;
	return PSY_OK;
}

int master_savespecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t outdry;
	uint8_t decreaseOnClip = 0;
	int status;
				
	size = sizeof outdry + sizeof decreaseOnClip;
	outdry = (int32_t)(self->volume_ * 256);
	/* size of this part params to save */
	if ((status = psyfile_write(songfile->file, &size, sizeof(size)))) {
		return status;
	}
	if ((status = psyfile_write(songfile->file, &outdry, sizeof(outdry)))) {
		return status;
	}
	if ((status = psyfile_write(songfile->file, &decreaseOnClip,
			sizeof(decreaseOnClip)))) {
		return status;
	}
	return PSY_OK;
}
