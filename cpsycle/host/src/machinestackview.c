/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machinestackview.h"
/* host */
#include "arrowui.h"
#include "switchui.h"
#include "knobui.h"
#include "resources/resource.h"
#include "slidergroupui.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <plugin_interface.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"


/* RouteMachineParam */

/* prototypes */
static void outputrouteparam_tweak(OutputRouteParam*, double val);
static double outputrouteparam_norm_value(OutputRouteParam*);
static int outputrouteparam_describe(OutputRouteParam* self, char* text);
static int outputrouteparam_name(OutputRouteParam*, char* text);
static int outputrouteparam_type(OutputRouteParam*);
static void outputrouteparam_range(OutputRouteParam*,
	intptr_t* minval, intptr_t* maxval);
static psy_audio_Wire outputrouteparam_wire(OutputRouteParam*);
static uintptr_t outputrouteparam_busid(OutputRouteParam*, psy_audio_Wire);
static psy_List* outputrouteparam_buses(OutputRouteParam*);

/* vtable */
static MachineParamVtable outputrouteparam_vtable;
static bool outputrouteparam_vtable_initialized = FALSE;

static void outputrouteparam_vtable_init(OutputRouteParam* self)
{
	if (!outputrouteparam_vtable_initialized) {
		outputrouteparam_vtable = *(self->machineparam.vtable);
		outputrouteparam_vtable.normvalue =
			(fp_machineparam_norm_value)
			outputrouteparam_norm_value;
		outputrouteparam_vtable.tweak =
			(fp_machineparam_tweak)
			outputrouteparam_tweak;
		outputrouteparam_vtable.describe =
			(fp_machineparam_describe)
			outputrouteparam_describe;
		outputrouteparam_vtable.type =
			(fp_machineparam_type)
			outputrouteparam_type;
		outputrouteparam_vtable.range =
			(fp_machineparam_range)
			outputrouteparam_range;
		outputrouteparam_vtable.name =
			(fp_machineparam_name)
			outputrouteparam_name;
		outputrouteparam_vtable_initialized = TRUE;
	}
}
/* implementation */
void outputrouteparam_init(OutputRouteParam* self, psy_audio_Machines* machines,
	uintptr_t column, MachineStackState* state)
{
	psy_audio_machineparam_init(&self->machineparam);
	outputrouteparam_vtable_init(self);
	self->machineparam.vtable = &outputrouteparam_vtable;
	self->column = column;
	self->machines = machines;
	self->state = state;
}

void outputrouteparam_dispose(OutputRouteParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

void outputrouteparam_tweak(OutputRouteParam* self, double value)
{
	intptr_t minval;
	intptr_t maxval;
	intptr_t selidx;
	intptr_t src;
	MachineStackColumn* column;

	column = machinestackstate_column(self->state, self->column);
	if (!column) {
		return;
	}
	outputrouteparam_range(self, &minval, &maxval);
	selidx = (intptr_t)(value * (maxval - minval) + 0.5f) + minval;
	self->state->oldwire = outputrouteparam_wire(self);
	src = machinestackcolumn_lastbeforeroute(column);
	if (self->state->oldwire.dst != psy_audio_MASTER_INDEX) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(self->state->machines,
			self->state->oldwire.dst);
		if (!(machine && psy_audio_machine_is_bus(machine))) {
			psy_audio_wire_invalidate(&self->state->oldwire);
		}
	}
	if ((selidx == 0) && (!psy_audio_wire_valid(&self->state->oldwire))) {
		self->state->rewire = FALSE;
		return;
	}
	else if (selidx == 1) {
		/* Master Connection */
		self->state->newwire = psy_audio_wire_make(src, psy_audio_MASTER_INDEX);
	}
	else {
		psy_List* buses;

		/* Bus Connection */
		self->state->newwire = psy_audio_wire_zero();
		buses = outputrouteparam_buses(self);
		if (buses) {
			psy_List* p;

			p = psy_list_at(buses, selidx - 2);
			if (p) {
				uintptr_t dest;

				dest = (uintptr_t)psy_list_entry(p);
				self->state->newwire = psy_audio_wire_make(src, dest);
			}
		}
		psy_list_free(buses);
	}
	self->state->rewire = TRUE;
}

double outputrouteparam_norm_value(OutputRouteParam* self)
{
	psy_audio_Wire wire;
	uintptr_t selidx;
	intptr_t minval;
	intptr_t maxval;

	outputrouteparam_range(self, &minval, &maxval);
	selidx = 0;
	if (self->state->rewire) {
		wire = self->state->newwire;
	}
	else {
		wire = outputrouteparam_wire(self);
	}
	if (wire.dst == psy_INDEX_INVALID) {
		selidx = 0;
	}
	else if (wire.dst == psy_audio_MASTER_INDEX) {
		selidx = 1;
	}
	else {
		selidx = outputrouteparam_busid(self, wire);
		if (selidx == psy_INDEX_INVALID) {
			selidx = 0;
		}
		else {
			selidx += 2;
		}
	}
	return ((maxval - minval) != 0)
		? (selidx - minval) / (double)(maxval - minval)
		: 0.0;
}

psy_audio_Wire outputrouteparam_wire(OutputRouteParam* self)
{
	MachineStackColumn* column;

	column = machinestackstate_column(self->state, self->column);
	if (column) {
		return machinestackcolumn_outputwire(column);
	}
	return psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
}

uintptr_t outputrouteparam_busid(OutputRouteParam* self, psy_audio_Wire wire)
{
	psy_List* buses;

	buses = outputrouteparam_buses(self);
	if (buses) {
		uintptr_t rv;

		rv = psy_list_entry_index(buses, (void*)wire.dst);
		psy_list_free(buses);
		return rv;
	}
	return psy_INDEX_INVALID;
}

int outputrouteparam_describe(OutputRouteParam* self, char* text)
{
	psy_audio_Wire wire;

	if (self->state->rewire) {
		wire = self->state->newwire;
	}
	else {
		wire = outputrouteparam_wire(self);
	}
	if (psy_audio_wire_valid(&wire)) {
		if (wire.dst == psy_audio_MASTER_INDEX) {
			psy_snprintf(text, 128, "%s", "Master");
		}
		else {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->state->machines,
				wire.dst);
			if (machine && psy_audio_machine_is_bus(machine)) {
				psy_snprintf(text, 128, "Bus %.2X", (int)wire.dst);
			}
			else {
				psy_snprintf(text, 128, "End at %.2X", (int)wire.dst);
			}
		}
	}
	else {
		psy_snprintf(text, 128, "%s", "None");
	}
	return 1;
}

int outputrouteparam_name(OutputRouteParam* self, char* text)
{
	psy_snprintf(text, 128, "%s", "Output");
	return 1;
}

int outputrouteparam_type(OutputRouteParam* self)
{
	return MPF_STATE | MPF_SMALL;
}

void outputrouteparam_range(OutputRouteParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	psy_List* buses;

	/*
	** 0 : Not Connected to Master / Bus
	** 1 : Connected to Master
	*/
	*minval = 0;
	*maxval = 1;
	/* 2 ..numbuses */
	buses = outputrouteparam_buses(self);
	if (buses) {
		*maxval += psy_list_size(buses);
		psy_list_free(buses);
	}
}

psy_List* outputrouteparam_buses(OutputRouteParam* self)
{
	psy_List* rv;

	rv = machinestackstate_buses(self->state);
	if (rv) {
		MachineStackColumn* column;
		uintptr_t input;
		psy_List* p;

		/* avoid connnection to same bus */
		column = machinestackstate_column(self->state, self->column);
		if (column) {
			input = machinestackcolumn_at(column, 0);
			p = psy_list_find_entry(rv, (void*)input);
			if (p) {
				psy_list_remove(&rv, p);
			}
		}
	}
	return rv;
}


/* MachineStackColumn */

/* prototypes */
static void  machinestackcolumn_level_norm_value(MachineStackColumn*,
	psy_audio_IntMachineParam* sender, double* rv);
static void machinestackcolumn_mute_tweak(MachineStackColumn*,
	psy_audio_IntMachineParam* sender, double value);
static void machinestackcolumn_mute_norm_value(MachineStackColumn*,
	psy_audio_IntMachineParam* sender, double* rv);

/* implementation */
void machinestackcolumn_init(MachineStackColumn* self, uintptr_t column,
	uintptr_t inputroute, uintptr_t input, MachineStackState* state)
{
	psy_audio_Machine* machine;

	self->state = state;
	self->column = column;
	self->inputroute = inputroute;
	machine = psy_audio_machines_at(state->machines, input);
	if (machine &&
		(psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR ||
			psy_audio_machine_is_bus(machine))) {
		self->input = input;
	}
	else {
		self->input = psy_INDEX_INVALID;
	}
	self->chain = NULL;
	self->wirevolume = NULL;
	self->offset = 0;
	outputrouteparam_init(&self->outputroute, state->machines, column, state);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_LEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		machinestackcolumn_level_norm_value);
	psy_audio_intmachineparam_init(&self->mute_param,
		"M", "M", MPF_CHECK | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->mute_param.machineparam.signal_tweak, self,
		machinestackcolumn_mute_tweak);
	psy_signal_connect(&self->mute_param.machineparam.signal_normvalue, self,
		machinestackcolumn_mute_norm_value);
}

void machinestackcolumn_dispose(MachineStackColumn* self)
{
	if (self->wirevolume) {
		psy_audio_wiremachineparam_deallocate(self->wirevolume);
		self->wirevolume = NULL;
	}
	outputrouteparam_dispose(&self->outputroute);
	psy_audio_intmachineparam_dispose(&self->level_param);
	psy_audio_intmachineparam_dispose(&self->mute_param);
}

void machinestackcolumn_setwire(MachineStackColumn* self, psy_audio_Wire wire)
{
	psy_audio_Machine* machine;

	if (self->wirevolume) {
		psy_audio_wiremachineparam_deallocate(self->wirevolume);
		self->wirevolume = NULL;
	}
	machine = psy_audio_machines_at(self->state->machines, wire.src);
	if (machine) {
		self->wirevolume = psy_audio_wiremachineparam_allocinit(
			psy_audio_machine_edit_name(machine), wire, self->state->machines);
	}
}

psy_audio_Wire machinestackcolumn_outputwire(MachineStackColumn* self)
{
	if (self->wirevolume) {
		return self->wirevolume->wire;
	}
	return psy_audio_wire_zero();
}

psy_audio_Wire machinestackcolumn_sendwire(MachineStackColumn* self)
{
	if (self->chain) {
		const psy_List* p;

		p = psy_list_last(self->chain);
		if (p && (p->prev)) {
			uintptr_t dst;
			uintptr_t src;

			dst = (uintptr_t)psy_list_entry_const(p);
			src = (uintptr_t)psy_list_entry_const(p->prev);
			return psy_audio_wire_make(src, dst);
		}
	}
	return psy_audio_wire_make(psy_INDEX_INVALID, psy_INDEX_INVALID);
}

void machinestackcolumn_append(MachineStackColumn* self, uintptr_t macid)
{
	psy_list_append(&self->chain, (void*)macid);
}

uintptr_t machinestackcolumn_lastbeforemaster(const MachineStackColumn* self)
{
	if (self->chain) {
		const psy_List* last;

		last = psy_list_last_const(self->chain);
		if (last) {
			uintptr_t macid;

			macid = (uintptr_t)psy_list_entry_const(last);
			if (macid != psy_audio_MASTER_INDEX) {
				return macid;
			}
			if (last->prev) {
				return (uintptr_t)psy_list_entry_const(last->prev);
			}
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_lastbeforeroute(const MachineStackColumn* self)
{
	if (self->chain) {
		const psy_List* last;

		last = psy_list_last_const(self->chain);
		if (last) {
			uintptr_t macid;

			macid = (uintptr_t)psy_list_entry_const(last);
			if (macid != psy_audio_MASTER_INDEX) {
				psy_audio_Machine* machine;

				machine = psy_audio_machines_at(self->state->machines, macid);
				if (self->chain == last || !psy_audio_machine_is_bus(machine)) {
					return macid;
				}
			}
			if (last->prev) {
				return (uintptr_t)psy_list_entry_const(last->prev);
			}
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_at(const MachineStackColumn* self, uintptr_t index)
{
	if (self->chain && index != psy_INDEX_INVALID) {
		const psy_List* p;

		p = psy_list_at(self->chain, index);
		if (p) {
			return (uintptr_t)psy_list_entry_const(p);
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_last(const MachineStackColumn* self)
{
	if (self->chain) {
		const psy_List* p;

		p = psy_list_last(self->chain);
		if (p) {
			return (uintptr_t)psy_list_entry_const(p);
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_lastbeforeindex(const MachineStackColumn* self,
	uintptr_t index)
{
	if (self->chain && index > 0) {
		const psy_List* last;

		last = psy_list_at(self->chain, index - 1);
		if (last) {
			return (uintptr_t)psy_list_entry_const(last);
		}
	}
	return psy_INDEX_INVALID;
}

uintptr_t machinestackcolumn_nextindex(const MachineStackColumn* self,
	uintptr_t index)
{
	if (self->chain && index > 0) {
		const psy_List* next;

		next = psy_list_at(self->chain, index + 1);
		if (next) {
			return (uintptr_t)psy_list_entry_const(next);
		}
	}
	return psy_INDEX_INVALID;
}

bool machinestackcolumn_connectedtomaster(const MachineStackColumn* self)
{
	MachineStackColumn* const_cast_self;

	const_cast_self = (MachineStackColumn*)self;
	return (psy_audio_machineparam_norm_value(
		&const_cast_self->outputroute.machineparam) > 0.f);
}

void machinestackcolumn_level_norm_value(MachineStackColumn* self,
	psy_audio_IntMachineParam* sender, double* rv)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	*rv = 0.0;
	machine = psy_audio_machines_at(self->state->machines,
		machinestackcolumn_lastbeforemaster(self));
	if (machine) {
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			double temp;
			double volume;
			MachineStackColumn* column;

			column = machinestackstate_column(self->state, self->column);
			if (column) {
				volume = psy_audio_connections_wire_volume(
					&self->state->machines->connections,
					machinestackcolumn_outputwire(column));
			}
			else {
				volume = 1.0;
			}
			temp = ((int)(50.0 * log10(volume))) / 97.0;
			*rv = psy_audio_buffer_rmsdisplay(memory) + temp;
		}
	}
}

void machinestackcolumn_mute_tweak(MachineStackColumn* self,
	psy_audio_IntMachineParam* sender, double value)
{
	psy_audio_WireSocket* socket;

	socket = psy_audio_connections_input(&self->state->machines->connections,
		machinestackcolumn_sendwire(self));
	if (socket) {
		socket->mute = value > 0.0;
	}
}

void machinestackcolumn_mute_norm_value(MachineStackColumn* self,
	psy_audio_IntMachineParam* sender, double* rv)
{
	psy_audio_WireSocket* socket;

	socket = psy_audio_connections_input(&self->state->machines->connections,
		machinestackcolumn_sendwire(self));
	if (socket) {
		*rv = (socket->mute)
			? 1.0
			: 0.0;
	}
	else {
		*rv = 0.0;
	}
}

void machinestackcolumn_trace(const MachineStackColumn* self)
{
	const psy_List* p;

	assert(self);

	printf("Stack column ");
	for (p = self->chain; p != NULL; p = p->next) {
		uintptr_t mac_id;

		mac_id = (uintptr_t)psy_list_entry_const(p);
		printf("mac %d ", (int)mac_id);
	}
	printf("\n");
}


/* MachineStackState */

/* prototypes */
static uintptr_t machinestackstate_buildcolumnoutchain(MachineStackState*,
	uintptr_t mac_id, uintptr_t prevslot, uintptr_t columnindex, bool first);

/* implementation */
void machinestackstate_init(MachineStackState* self, psy_ui_Component* view,
	ParamViews* paramviews)
{
	psy_ui_Style* effect_style;

	psy_table_init(&self->columns);
	self->machines = NULL;
	self->selected = psy_INDEX_INVALID;
	self->view = view;
	effect_style = psy_ui_style(STYLE_MV_EFFECT);
	self->inputs_size = psy_ui_size_make(
		psy_ui_value_make_px(138.0),
		psy_ui_value_make_px(52.0 + 19.0));
	self->volume_size = psy_ui_size_make(
		psy_ui_value_make_px(138.0 + 19),
		psy_ui_value_make_px(182.0));
	self->columnselected = FALSE;
	self->drawvirtualgenerators = FALSE;
	self->col_margin_right = 2.0;
	/* 20: right margin of column */
	self->update = FALSE;
	self->effectinsertpos = psy_INDEX_INVALID;
	self->effectinsertright = FALSE;
	self->preventrebuild = FALSE;
	self->rewire = FALSE;
	self->paramviews = paramviews;
	psy_audio_wire_init(&self->oldwire);
	psy_audio_wire_init(&self->newwire);
}

void machinestackstate_dispose(MachineStackState* self)
{
	psy_table_dispose_all(&self->columns, (psy_fp_disposefunc)
		machinestackcolumn_dispose);
}

void machinestackstate_clear(MachineStackState* self)
{
	psy_table_dispose_all(&self->columns, (psy_fp_disposefunc)
		machinestackcolumn_dispose);
	psy_table_init(&self->columns);
}

void machinestackstate_setmachines(MachineStackState* self,
	psy_audio_Machines* machines)
{
	self->machines = machines;
}

MachineStackColumn* machinestackstate_insertcolumn(MachineStackState* self,
	uintptr_t columnindex, uintptr_t inputroute, uintptr_t input)
{
	MachineStackColumn* column;

	column = machinestackstate_column(self, columnindex);
	if (column) {
		machinestackcolumn_dispose(column);
		free(column);
	}
	column = (MachineStackColumn*)malloc(sizeof(MachineStackColumn));
	machinestackcolumn_init(column, columnindex, inputroute, input, self);
	psy_table_insert(&self->columns, columnindex, column);
	return column;
}

MachineStackColumn* machinestackstate_column(MachineStackState* self,
	uintptr_t column)
{
	return (MachineStackColumn*)psy_table_at(&self->columns, column);
}

MachineStackColumn* machinestackstate_selectedcolumn(MachineStackState* self)
{
	return machinestackstate_column(self, self->selected);
}

psy_List* machinestackstate_inputs(MachineStackState* self)
{
	if (self->machines) {
		psy_TableIterator it;
		psy_List* p;
		psy_List* rv;
		psy_List* effectswithoutinput;

		assert(self);

		rv = NULL;
		effectswithoutinput = NULL;
		for (it = psy_audio_machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);

			if (psy_audio_machine_mode(machine) ==
				psy_audio_MACHMODE_GENERATOR ||
				psy_audio_machine_is_bus(machine)) {
				if (self->drawvirtualgenerators ||
					!psy_audio_machines_isvirtualgenerator(self->machines,
						psy_tableiterator_key(&it))) {
					psy_list_append(&rv, (void*)psy_tableiterator_key(&it));
				}
			}
			else if (psy_tableiterator_key(&it) != psy_audio_MASTER_INDEX) {
				psy_audio_MachineSockets* sockets;

				sockets = psy_audio_connections_at(&self->machines->connections,
					psy_tableiterator_key(&it));
				if (!sockets || wiresockets_size(&sockets->inputs) == 0) {
					psy_list_append(&effectswithoutinput,
						(void*)psy_tableiterator_key(&it));
				}
			}
		}
		for (p = effectswithoutinput; p != NULL; psy_list_next(&p)) {
			psy_list_append(&rv, psy_list_entry(p));
		}
		psy_list_free(effectswithoutinput);
		return rv;
	}
	return NULL;
}

psy_List* machinestackstate_buses(MachineStackState* self)
{
	if (self->machines) {
		psy_TableIterator it;
		psy_List* rv;

		assert(self);

		rv = NULL;
		for (it = psy_audio_machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			if (psy_audio_machine_is_bus(machine)) {
				psy_list_append(&rv, (void*)psy_tableiterator_key(&it));
			}
		}
		return rv;
	}
	return NULL;
}

void machinestackstate_check_wirechange(MachineStackState* self)
{
	if (!psy_audio_wire_equal(&self->newwire, &self->oldwire)) {
		if (psy_audio_wire_valid(&self->oldwire)) {
			psy_audio_machines_disconnect(self->machines, self->oldwire);
		}
		if (psy_audio_wire_valid(&self->newwire)) {
			psy_audio_machines_connect(self->machines, self->newwire);
		}
	}
}

psy_ui_Value machinestackstate_column_width(const MachineStackState* self,
	const psy_ui_TextMetric* tm)
{
	psy_ui_Style* style;
	psy_ui_Size curr_size;
	double width;
	const psy_ui_TextMetric* curr_tm;

	style = psy_ui_style(STYLE_MV_GENERATOR);
	if (style->font.imp) {
		curr_tm = psy_ui_font_textmetric(&style->font);
	}
	else {
		curr_tm = tm;
	}
	curr_size = psy_ui_style_size(style);
	width = psy_ui_value_px(&curr_size.width, curr_tm, NULL);

	style = psy_ui_style(STYLE_MV_EFFECT);
	if (style->font.imp) {
		curr_tm = psy_ui_font_textmetric(&style->font);
	}
	else {
		curr_tm = tm;
	}
	curr_size = psy_ui_style_size(style);
	width = psy_max(width, psy_ui_value_px(&curr_size.width, curr_tm, NULL));

	curr_size = psy_ui_size_make_em(30.0, 0.0);
	curr_tm = tm;
	width = psy_max(width, psy_ui_value_px(&curr_size.width, curr_tm, NULL));

	return psy_ui_value_make_px(width);
}

void machinestackstate_buildcolumns(MachineStackState* self)
{
	machinestackstate_clear(self);
	if (self->machines) {
		MachineList* inputs;
		MachineList* p;
		uintptr_t columnindex;
		uintptr_t i;

		inputs = machinestackstate_inputs(self);
		self->currlevel = 0;
		if (inputs == NULL) {
			machinestackstate_buildcolumnoutchain(self, psy_INDEX_INVALID, 0,
				psy_INDEX_INVALID, TRUE);
		}
		else {
			columnindex = 0;
			for (p = inputs; p != NULL; psy_list_next(&p)) {
				columnindex = machinestackstate_buildcolumnoutchain(self,
					(uintptr_t)(p->entry), psy_INDEX_INVALID,
					columnindex, TRUE) + 1;
			}
			psy_list_free(inputs);
			for (i = 0; i < psy_table_maxsize(&self->columns); ++i) {
				MachineStackColumn* column;

				column = (MachineStackColumn*)psy_table_at(&self->columns, i);
				if (column) {
					psy_List* lastnode;

					lastnode = psy_list_last(column->chain);
					if (lastnode) {
						if (lastnode->prev) {
							uintptr_t src;
							uintptr_t dst;

							src = (uintptr_t)psy_list_entry(lastnode->prev);
							dst = (uintptr_t)psy_list_entry(lastnode);
							machinestackcolumn_setwire(column,
								psy_audio_wire_make(src, dst));
						}
						else {
							uintptr_t src;
							uintptr_t dst;

							src = column->inputroute;
							dst = (uintptr_t)psy_list_entry(lastnode);
							machinestackcolumn_setwire(column,
								psy_audio_wire_make(src, dst));
						}
					}
				}
			}
		}
	}
}

uintptr_t machinestackstate_buildcolumnoutchain(MachineStackState* self,
	uintptr_t mac_id, uintptr_t prevslot, uintptr_t columnindex, bool first)
{
	uintptr_t rv;
	MachineStackColumn* column;
	psy_audio_MachineSockets* sockets;
	psy_audio_Machine* machine;

	if (mac_id == psy_INDEX_INVALID) {
		return columnindex;
	}
	rv = columnindex;
	column = machinestackstate_column(self, columnindex);
	if (!machinestackstate_column(self, columnindex)) {
		column = machinestackstate_insertcolumn(self, columnindex, prevslot,
			mac_id);
		column->offset = self->currlevel;
	}
	machinestackcolumn_append(column, mac_id);
	machine = psy_audio_machines_at(self->machines, mac_id);
	if (!first && psy_audio_machine_is_bus(machine)) {
		return rv;
	}
	sockets = psy_audio_connections_at(&self->machines->connections, mac_id);
	if (sockets && mac_id != psy_audio_MASTER_INDEX) {
		uintptr_t i;
		uintptr_t c;
		uintptr_t maxnumoutputs;

		maxnumoutputs = psy_table_maxsize(&sockets->outputs.sockets);
		for (c = 0, i = 0; i < maxnumoutputs; ++i) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_audio_wiresockets_at(
				&sockets->outputs, i);
			if (socket && socket->slot != psy_INDEX_INVALID) {
				if (!first) {
					++self->currlevel;
				}
				rv = machinestackstate_buildcolumnoutchain(self, socket->slot,
					mac_id, rv + c, FALSE);
				c = 1;
				if (!first) {
					--self->currlevel;
				}
			}
		}
	}
	return rv;
}

uintptr_t machinestackstate_maxnumcolumns(const MachineStackState* self)
{
	return psy_table_maxsize(&self->columns);
}


/* MachineStackLabels */

/* prototypes */
static void machinestackdesc_onalign(MachineStackDesc*);
static void machinestackdesc_ontoggleoutputs(MachineStackDesc*,
	psy_ui_Button* sender);
static void machinestackdesc_ontogglevolumes(MachineStackDesc*,
	psy_ui_Button* sender);
static void machinestackdesc_configureeffects(MachineStackDesc*,
	psy_ui_Button* sender);

/* implementation */
void machinestackdesc_init(MachineStackDesc* self, psy_ui_Component* parent,
	MachineStackView* view, MachineStackState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->view = view;
	self->state = state;
	// psy_ui_component_setcolour(&self->component,
	//	psy_ui_colour_make(0x00AABBBB));
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));
	psy_ui_label_init_text(&self->inputs, &self->component,
		"stackview.inputs");
	psy_ui_label_prevent_wrap(&self->inputs);
	psy_ui_component_set_style_type(&self->inputs.component,
		psy_ui_STYLE_BUTTON);
	psy_ui_label_set_text_alignment(&self->inputs,
		psy_ui_ALIGNMENT_TOP | psy_ui_ALIGNMENT_RIGHT);
	psy_ui_component_set_align(&self->inputs.component,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_preferred_height(&self->inputs.component,
		self->state->inputs_size.height);
	psy_ui_button_init_text_connect(&self->effects, &self->component,
		"stackview.effects", self, machinestackdesc_configureeffects);
	psy_ui_button_set_text_alignment(&self->effects,
		psy_ui_ALIGNMENT_TOP | psy_ui_ALIGNMENT_RIGHT);
	psy_ui_component_set_align(&self->effects.component,
		psy_ui_ALIGN_TOP);
	psy_ui_button_load_resource(&self->effects, IDB_SETTINGS_LIGHT,
		IDB_SETTINGS_DARK, psy_ui_colour_white());
	psy_ui_button_init_text_connect(&self->volumes, &self->component,
		"stackview.volumes", self, machinestackdesc_ontogglevolumes);
	psy_ui_button_set_icon(&self->volumes, psy_ui_ICON_LESS);
	psy_ui_component_set_align(&self->volumes.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_preferred_height(&self->volumes.component,
		self->state->volume_size.height);
	psy_ui_button_set_text_alignment(&self->volumes,
		psy_ui_ALIGNMENT_TOP | psy_ui_ALIGNMENT_RIGHT);
	psy_ui_button_init_text_connect(&self->outputs, &self->component,
		"stackview.outputs", self, machinestackdesc_ontoggleoutputs);
	psy_ui_button_set_icon(&self->outputs, psy_ui_ICON_LESS);
	psy_ui_component_set_align(&self->outputs.component,
		psy_ui_ALIGN_BOTTOM);
	psy_ui_button_set_text_alignment(&self->outputs,
		psy_ui_ALIGNMENT_TOP | psy_ui_ALIGNMENT_RIGHT);
}

void machinestackdesc_ontoggleoutputs(MachineStackDesc* self,
	psy_ui_Button* sender)
{
	psy_ui_component_toggle_visibility(&self->view->outputs.component);
	if (psy_ui_component_visible(&self->view->outputs.component)) {
		psy_ui_button_set_icon(&self->outputs, psy_ui_ICON_LESS);
	}
	else {
		psy_ui_button_set_icon(&self->outputs, psy_ui_ICON_MORE);
	}
}

void machinestackdesc_ontogglevolumes(MachineStackDesc* self,
	psy_ui_Button* sender)
{
	psy_ui_component_toggle_visibility(&self->view->volumes.component);
	if (psy_ui_component_visible(&self->view->volumes.component)) {
		psy_ui_button_set_icon(&self->volumes, psy_ui_ICON_LESS);
	}
	else {
		psy_ui_button_set_icon(&self->volumes, psy_ui_ICON_MORE);
	}
}

void machinestackdesc_configureeffects(MachineStackDesc* self,
	psy_ui_Button* sender)
{
	assert(self);

	workspace_select_view(self->view->workspace,
		viewindex_make_all(VIEW_ID_SETTINGS, 2, 1, psy_INDEX_INVALID));
}


/* MachineStackInputs */

/* prototypes */
static void machinestackinputs_onpreferredsize(MachineStackInputs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void machinestackinputs_ondraw(MachineStackInputs*, psy_ui_Graphics*);
static void machinestackinputs_onmouseenter(MachineStackInputs*);
static void machinestackinputs_onmousedoubleclick(MachineStackInputs*,
	psy_ui_MouseEvent*);
static void machinestackinputs_on_mouse_down(MachineStackInputs*,
	psy_ui_MouseEvent*);
static void machinestackinputs_on_mouse_up(MachineStackInputs*,
	psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable machinestackinputs_vtable;
static bool machinestackinputs_vtable_initialized = FALSE;

static void machinestackinputs_vtable_init(MachineStackInputs* self)
{
	if (!machinestackinputs_vtable_initialized) {
		machinestackinputs_vtable = *(self->component.vtable);
		machinestackinputs_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			machinestackinputs_onpreferredsize;
		machinestackinputs_vtable.onmouseenter =
			(psy_ui_fp_component)
			machinestackinputs_onmouseenter;
		machinestackinputs_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			machinestackinputs_onmousedoubleclick;
		machinestackinputs_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			machinestackinputs_on_mouse_down;
		machinestackinputs_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			machinestackinputs_on_mouse_up;
		machinestackinputs_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &machinestackinputs_vtable);
}

/* implementation */
void machinestackinputs_init(MachineStackInputs* self,
	psy_ui_Component* parent, MachineStackState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	machinestackinputs_vtable_init(self);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.5, state->col_margin_right, 0.5, 0.0));
	self->workspace = workspace;
	self->state = state;
}

void machinestackinputs_onpreferredsize(MachineStackInputs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->height = self->state->inputs_size.height;
	rv->width = machinestackstate_column_width(self->state,
		psy_ui_component_textmetric(&self->component));
	/* + 1: empty space to add new generator */
	psy_ui_value_mul_real(&rv->width,
		psy_max(1.0, machinestackstate_maxnumcolumns(self->state) + 1));
}

void machinestackinputs_build(MachineStackInputs* self)
{
	psy_ui_component_clear(&self->component);
	if (self->state->machines) {
		uintptr_t i;
		uintptr_t maxnumcolumns;

		maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
		for (i = 0; i < maxnumcolumns; ++i) {
			MachineStackColumn* column;
			psy_ui_Component* component;

			column = machinestackstate_column(self->state, i);
			if (column && column->input != psy_INDEX_INVALID) {
				component = machineui_create(
					psy_audio_machines_at(self->state->machines, column->input),
					&self->component, self->state->paramviews, FALSE,
					self->state->machines,
					psycleconfig_macview(workspace_cfg(self->workspace)));
			}
			else if (!column || column->offset != 0) {
				component = psy_ui_component_allocinit(&self->component, NULL);
			}
			else {
				ArrowUi* arrow;
				uintptr_t first;

				first = machinestackcolumn_at(column, 0);
				arrow = arrowui_allocinit(&self->component,
					psy_audio_wire_make(column->inputroute, first),
					self->state->machines);
				component = &arrow->component;
			}
			if (component) {
				psy_ui_component_set_preferred_width(component,
					machinestackstate_column_width(self->state,
						psy_ui_component_textmetric(&self->component)));
				psy_ui_component_set_align(component, psy_ui_ALIGN_LEFT);
			}
		}
	}
	psy_ui_component_align(&self->component);
}

void machinestackinputs_onmouseenter(MachineStackInputs* self)
{
}

void machinestackinputs_onmousedoubleclick(MachineStackInputs* self,
	psy_ui_MouseEvent* ev)
{
}

void machinestackinputs_on_mouse_down(MachineStackInputs* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_component_visible(&self->state->machine_menu->component)) {
		psy_ui_component_hide_align(&self->state->machine_menu->component);
		psy_ui_component_invalidate(&self->state->machine_menu->component);
	}
}

void machinestackinputs_on_mouse_up(MachineStackInputs* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 2 && psy_ui_mouseevent_target(ev) &&
		psy_ui_mouseevent_target(ev) != &self->component) {
		psy_ui_Component* ancestor;
		uintptr_t index;

		ancestor = psy_ui_component_intersect(&self->component,
			psy_ui_mouseevent_offset(ev), &index);
		if (ancestor && (psy_ui_component_id(ancestor) != psy_INDEX_INVALID)) {
			machinemenu_select(self->state->machine_menu,
				psy_ui_component_id(ancestor));
			psy_ui_mouseevent_stop_propagation(ev);
		}
	}
}


/* MachineStackOutputs */

/* prototypes */
static void machinestackoutputs_on_mouse_up(MachineStackOutputs*,
	psy_ui_MouseEvent*);
static void machinestackoutputs_onpreferredsize(MachineStackOutputs*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

/* vtable */
static psy_ui_ComponentVtable machinestackoutputs_vtable;
static bool machinestackoutputs_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackoutputs_vtable_init(
	MachineStackOutputs* self)
{
	if (!machinestackoutputs_vtable_initialized) {
		machinestackoutputs_vtable = *(self->component.vtable);
		machinestackoutputs_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			machinestackoutputs_on_mouse_up;
		machinestackoutputs_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			machinestackoutputs_onpreferredsize;
		machinestackoutputs_vtable_initialized = TRUE;
	}
	return &machinestackoutputs_vtable;
}

/* implementation */
void machinestackoutputs_init(MachineStackOutputs* self,
	psy_ui_Component* parent, MachineStackState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_vtable(&self->component,
		machinestackoutputs_vtable_init(self));
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_minimum_size(&self->component,
		psy_ui_size_make_em(0.0, 2.0));
	self->state = state;
}

void machinestackoutputs_build(MachineStackOutputs* self)
{
	psy_ui_component_clear(&self->component);
	if (self->state->machines) {
		uintptr_t i;
		uintptr_t maxnumcolumns;

		maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
		for (i = 0; i < maxnumcolumns; ++i) {
			MachineStackColumn* column;
			KnobUi* knobui;

			column = machinestackstate_column(self->state, i);
			knobui = knobui_allocinit(&self->component,
				NULL, psy_INDEX_INVALID,
				(column)
				? &column->outputroute.machineparam
				: NULL,
				NULL);
			if (knobui) {
				psy_ui_component_set_preferred_width(&knobui->component,
					machinestackstate_column_width(self->state,
						psy_ui_component_textmetric(&self->component)));
				psy_ui_component_set_align(&knobui->component,
					psy_ui_ALIGN_LEFT);
				psy_ui_component_set_margin(&knobui->component,
					psy_ui_margin_make_em(0.0, self->state->col_margin_right, 0.0, 0.0));
			}
		}
	}
}

void machinestackoutputs_on_mouse_up(MachineStackOutputs* self,
	psy_ui_MouseEvent* ev)
{
	if (self->state->rewire) {
		machinestackstate_check_wirechange(self->state);
		self->state->rewire = FALSE;
	}
}

void machinestackoutputs_onpreferredsize(MachineStackOutputs* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = machinestackstate_column_width(self->state,
		psy_ui_component_textmetric(&self->component));
	/* + 1: empty space to add new generator */
	psy_ui_value_mul_real(&rv->width,
		psy_max(1.0, machinestackstate_maxnumcolumns(self->state) + 1));
}


/* MachineStackPaneTrackCient */

/* prototypes */
static void machinestackpanetrackclient_ondraw(MachineStackPaneTrackClient*,
	psy_ui_Graphics*);

/* vtable */
static psy_ui_ComponentVtable machinestackpanetrackclient_vtable;
static bool machinestackpanetrackclient_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpanetrackclient_vtable_init(
	MachineStackPaneTrackClient* self)
{
	if (!machinestackpanetrackclient_vtable_initialized) {
		machinestackpanetrackclient_vtable = *(self->component.vtable);
		machinestackpanetrackclient_vtable_initialized = TRUE;
	}
	return &machinestackpanetrackclient_vtable;
}

/* implementation */
void machinestackpanetrackclient_init(MachineStackPaneTrackClient* self,
	psy_ui_Component* parent, uintptr_t column,
	MachineStackState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_vtable(&self->component,
		machinestackpanetrackclient_vtable_init(self));
	psy_ui_component_set_overflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_set_wheel_scroll(&self->component, 1);
	psy_ui_component_set_scroll_step_height(&self->component,
		psy_ui_value_make_eh(4.0));
	self->state = state;
	self->column = column;
}


/* MachineStackPaneTrack */

/* prototypes */
static void machinestackpanetrack_on_mouse_double_click(MachineStackPaneTrack*,
	psy_ui_MouseEvent*);
static void machinestackpane_prepare_add_effect(MachineStackPaneTrack*,
	uintptr_t effectinsertpos, bool effectinsertright);
static void machinestackpanetrack_on_mouse_up(MachineStackPaneTrack*,
	psy_ui_MouseEvent*);
static void machinestackpanetrack_on_mouse_down(MachineStackPaneTrack*,
	psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable machinestackpanetrack_vtable;
static bool machinestackpanetrack_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpanetrack_vtable_init(
	MachineStackPaneTrack* self)
{
	if (!machinestackpanetrack_vtable_initialized) {
		machinestackpanetrack_vtable = *(self->component.vtable);
		machinestackpanetrack_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			machinestackpanetrack_on_mouse_double_click;
		machinestackpanetrack_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			machinestackpanetrack_on_mouse_down;
		machinestackpanetrack_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			machinestackpanetrack_on_mouse_up;
		machinestackpanetrack_vtable_initialized = TRUE;
	}
	return &machinestackpanetrack_vtable;
}

/* implementation */
void machinestackpanetrack_init(MachineStackPaneTrack* self,
	psy_ui_Component* parent, uintptr_t column,
	MachineStackState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_vtable(&self->component,
		machinestackpanetrack_vtable_init(self));
	psy_ui_component_set_colour(&self->component,
		psy_ui_colour_make(0x00CACACA));
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	machinestackpanetrackclient_init(&self->client, &self->component,
		column, state);
	psy_ui_component_set_wheel_scroll(&self->client.component, 4);
	psy_ui_component_set_align(&self->client.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_default_align(&self->client.component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, &self->client.component);
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->client.component, psy_ui_ALIGN_FIXED);
	self->column = column;
	self->state = state;
	self->workspace = workspace;
}

MachineStackPaneTrack* machinestackpanetrack_alloc(void)
{
	return (MachineStackPaneTrack*)malloc(sizeof(MachineStackPaneTrack));
}

MachineStackPaneTrack* machinestackpanetrack_allocinit(
	psy_ui_Component* parent, uintptr_t column,
	MachineStackState* state, Workspace* workspace)
{
	MachineStackPaneTrack* rv;

	rv = machinestackpanetrack_alloc();
	if (rv) {
		machinestackpanetrack_init(rv, parent, column, state, workspace);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void machinestackpanetrack_on_mouse_double_click(MachineStackPaneTrack* self,
	psy_ui_MouseEvent* ev)
{
	MachineStackColumn* column;

	self->state->columnselected = TRUE;
	column = machinestackstate_column(self->state, self->column);
	if (column && column->chain) {
		psy_List* p;
		psy_List* q;
		uintptr_t effect;
		uintptr_t c;
		uintptr_t effectinsertpos;
		bool effectinsertright;

		effectinsertright = FALSE;
		q = psy_ui_component_children(&self->client.component,
			psy_ui_NONE_RECURSIVE);
		effect = psy_INDEX_INVALID;
		for (p = q, c = 0; p != NULL; psy_list_next(&p), ++c) {
			psy_ui_Component* component;
			psy_ui_RealRectangle position;

			component = (psy_ui_Component*)psy_list_entry(p);
			position = psy_ui_component_position(component);
			if (position.bottom > psy_ui_mouseevent_pt(ev).y) {
				break;
			}
			if (psy_ui_realrectangle_intersect(&position,
				psy_ui_mouseevent_pt(ev))) {
				effect = c - column->offset;
				break;
			}
		}
		psy_list_free(q);
		if (effect == psy_INDEX_INVALID) {
			psy_ui_Value colwidth;

			colwidth = machinestackstate_column_width(self->state,
				psy_ui_component_textmetric(&self->component));
			effectinsertpos = c - column->offset;
			effectinsertright = (psy_ui_mouseevent_offset(ev).x >
				psy_ui_value_px(&colwidth,
					psy_ui_component_textmetric(&self->component), NULL));
			self->state->selected = self->column;

		}
		else {
			effectinsertpos = psy_INDEX_INVALID;
			effectinsertright = FALSE;
			psy_ui_mouseevent_stop_propagation(ev);
		}
		machinestackpane_prepare_add_effect(self, effectinsertpos,
			effectinsertright);
	}
	else {
		psy_ui_mouseevent_stop_propagation(ev);
	}
}

void machinestackpane_prepare_add_effect(MachineStackPaneTrack* self,
	uintptr_t effectinsertpos, bool effectinsertright)
{
	MachineStackColumn* insertcolumn;

	insertcolumn = machinestackstate_column(self->state, self->column);
	if (insertcolumn) {
		uintptr_t firstmacid;
		bool hasinput;
		uintptr_t insertpos;
		psy_audio_Machine* input;
		uintptr_t prevmacid;
		uintptr_t nextmacid;

		firstmacid = machinestackcolumn_at(insertcolumn, 0);
		prevmacid = psy_INDEX_INVALID;
		nextmacid = psy_INDEX_INVALID;
		input = psy_audio_machines_at(self->state->machines, firstmacid);
		hasinput = (input && (psy_audio_machine_is_bus(input) ||
			psy_audio_machine_mode(input) == psy_audio_MACHMODE_GENERATOR));
		insertpos = effectinsertpos;
		if (!hasinput) {
			if (effectinsertpos > 0) {
				insertpos -= 1;
			}
			else {
				insertpos = psy_INDEX_INVALID;
			}
		}
		if (insertpos != psy_INDEX_INVALID) {
			prevmacid = machinestackcolumn_at(insertcolumn, insertpos);
		}
		else {
			prevmacid = insertcolumn->inputroute;
			nextmacid = machinestackcolumn_at(insertcolumn, 0);
		}
		if (effectinsertright) {
			nextmacid = psy_INDEX_INVALID;
		}
		else if (insertpos != psy_INDEX_INVALID) {
			nextmacid = machinestackcolumn_at(insertcolumn, insertpos + 1);
		}
		machineinsert_append(&self->workspace->insert, psy_audio_wire_make(
			prevmacid, nextmacid));
	}
}

void machinestackpanetrack_on_mouse_down(MachineStackPaneTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_component_visible(&self->state->machine_menu->component)) {
		psy_ui_component_hide_align(&self->state->machine_menu->component);
		psy_ui_component_invalidate(&self->state->machine_menu->component);
	}
}

void machinestackpanetrack_on_mouse_up(MachineStackPaneTrack* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_button(ev) == 2 && psy_ui_mouseevent_target(ev) &&
		psy_ui_mouseevent_target(ev) != &self->component) {
		psy_ui_Component* ancestor;
		uintptr_t index;

		ancestor = psy_ui_component_intersect(&self->client.component,
			psy_ui_mouseevent_offset(ev), &index);
		if (ancestor && (psy_ui_component_id(ancestor) != psy_INDEX_INVALID)) {
			machinemenu_select(self->state->machine_menu,
				psy_ui_component_id(ancestor));
			psy_ui_mouseevent_stop_propagation(ev);
		}
	}
}


/* MachineStackPane */

/* prototypes */
static void machinestackpane_onmousedoubleclick(MachineStackPane*,
	psy_ui_MouseEvent*);
static psy_ui_Component* machinestackpane_insert(MachineStackPane*,
	uintptr_t mac_id, psy_ui_Component* trackpane);

/* vtable */
static psy_ui_ComponentVtable machinestackpane_vtable;
static bool machinestackpane_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackpane_vtable_init(
	MachineStackPane* self)
{
	assert(self);

	if (!machinestackpane_vtable_initialized) {
		machinestackpane_vtable = *(self->component.vtable);
		machinestackpane_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			machinestackpane_onmousedoubleclick;
		machinestackpane_vtable_initialized = TRUE;
	}
	return &machinestackpane_vtable;
}

/* implementation */
void machinestackpane_init(MachineStackPane* self, psy_ui_Component* parent,
	MachineStackState* state, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_vtable(&self->component,
		machinestackpane_vtable_init(self));
	psy_ui_component_set_wheel_scroll(&self->component, 4);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	self->workspace = workspace;
	self->state = state;
	self->vudrawupdate = FALSE;
	self->opcount = 0;
	machinestackpane_build(self);
}

void machinestackpane_build(MachineStackPane* self)
{
	assert(self);

	psy_ui_component_clear(&self->component);
	if (self->state->machines) {
		uintptr_t i;
		uintptr_t maxnumcolumns;

		maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
		for (i = 0; i < maxnumcolumns; ++i) {
			MachineStackPaneTrack* trackpane;

			trackpane = machinestackpanetrack_allocinit(&self->component, i,
				self->state, self->workspace);
			if (trackpane) {
				MachineStackColumn* column;
				bool insert;

				insert = FALSE;
				column = machinestackstate_column(self->state, i);
				if (column && column->offset > 0) {
					ArrowUi* arrow;
					uintptr_t first;

					first = machinestackcolumn_at(column, 0);
					arrow = arrowui_allocinit(&trackpane->client.component,
						psy_audio_wire_make(column->inputroute, first),
						self->state->machines);
					if (arrow) {
						psy_ui_Margin levelmargin;

						/* !@todo height */
						psy_ui_margin_init_all(&levelmargin,
							psy_ui_value_make_px((column->offset - 1) *
								(138.0 + 20.0) + 20.0),
							psy_ui_value_zero(), psy_ui_value_zero(),
							psy_ui_value_zero());
						psy_ui_component_set_margin(&arrow->component,
							levelmargin);
						psy_ui_component_set_align(&trackpane->component,
							psy_ui_ALIGN_TOP);
					}
				}
				psy_ui_component_set_align(&trackpane->component,
					psy_ui_ALIGN_LEFT);
				if (column) {
					psy_List* p;

					for (p = column->chain; p != NULL; psy_list_next(&p)) {
						uintptr_t mac_id;
						psy_audio_Machine* machine;

						mac_id = (uintptr_t)psy_list_entry(p);
						machine = psy_audio_machines_at(self->state->machines,
							mac_id);
						if (machine && psy_audio_machine_mode(machine) ==
							psy_audio_MACHMODE_FX &&
							!psy_audio_machine_is_bus(machine)) {
							machinestackpane_insert(self, mac_id,
								&trackpane->client.component);
							insert = TRUE;
						}
					}
				}
				if (!insert) {
					psy_ui_component_set_preferred_width(&trackpane->component,
						machinestackstate_column_width(self->state,
							psy_ui_component_textmetric(&self->component)));
					psy_ui_component_set_align(&trackpane->component,
						psy_ui_ALIGN_LEFT);
				}
				psy_ui_component_align(&trackpane->component);
			}
		}
		psy_ui_component_align(&self->component);
	}
}

psy_ui_Component* machinestackpane_insert(MachineStackPane* self,
	uintptr_t mac_id, psy_ui_Component* trackpane)
{
	psy_ui_Component* rv;

	assert(self);

	if (!self->state->machines) {
		return NULL;
	}
	rv = machineui_create(
		psy_audio_machines_at(self->state->machines, mac_id),
		trackpane, self->state->paramviews, FALSE, self->state->machines,
		psycleconfig_macview(workspace_cfg(self->workspace)));
	if (rv) {
		psy_ui_component_set_align(rv, psy_ui_ALIGN_TOP);
		psy_ui_component_set_preferred_width(rv,
			machinestackstate_column_width(self->state,
				psy_ui_component_textmetric(&self->component)));
		psy_ui_component_set_margin(rv,
			psy_ui_margin_make_em(0.5, self->state->col_margin_right, 0.0, 0.0));
		return rv;
	}
	return NULL;
}

void machinestackpane_onmousedoubleclick(MachineStackPane* self,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (!self->state->columnselected) {
		psy_Logger* logger;

		logger = psy_ui_component_logger(&self->component, psy_ui_LOG_STATUS);
		if (logger) {
			psy_logger_output(logger,
				"Add first input (Double click in Inputs)");
		}
		psy_ui_mouseevent_stop_propagation(ev);
	}
	self->state->columnselected = FALSE;
}


/* MachineStackVolumes */

/* prorotypes */
static void machinestackvolumes_onpreferredsize(MachineStackVolumes*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

/* vtable */
static psy_ui_ComponentVtable machinestackvolumes_vtable;
static bool machinestackvolumes_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* machinestackvolumes_vtable_init(
	MachineStackVolumes* self)
{
	assert(self);

	if (!machinestackvolumes_vtable_initialized) {
		machinestackvolumes_vtable = *(self->component.vtable);
		machinestackvolumes_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			machinestackvolumes_onpreferredsize;
		machinestackvolumes_vtable_initialized = TRUE;
	}
	return &machinestackvolumes_vtable;
}


/* prototypes */
static void machinestackvolumes_onpreferredsize(MachineStackVolumes*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

/* implementation */
void machinestackvolumes_init(MachineStackVolumes* self,
	psy_ui_Component* parent, MachineStackState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	machinestackvolumes_vtable_init(self);
	self->state = state;
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
}

void machinestackvolumes_build(MachineStackVolumes* self)
{
	uintptr_t i;
	uintptr_t maxnumcolumns;

	assert(self);

	psy_ui_component_clear(&self->component);
	maxnumcolumns = machinestackstate_maxnumcolumns(self->state);
	for (i = 0; i < maxnumcolumns; ++i) {
		psy_ui_Component* component;
		MachineStackColumn* column;

		component = NULL;
		column = machinestackstate_column(self->state, i);
		if (column && machinestackcolumn_wire(column)) {
			SliderGroupUi* slidergroup;

			slidergroup = slidergroupui_allocinit(&self->component,
				NULL, psy_INDEX_INVALID,
				&column->wirevolume->machineparam,
				psy_INDEX_INVALID,
				&column->level_param.machineparam);
			if (slidergroup) {
				SwitchUi* mute;

				component = &slidergroup->component;
				mute = switchui_allocinit(&slidergroup->controls,
					NULL, psy_INDEX_INVALID,
					&column->mute_param.machineparam,
					STYLE_MACPARAM_CHECKOFF, STYLE_MACPARAM_CHECKON);
				psy_ui_component_set_align(&mute->component, psy_ui_ALIGN_TOP);
			}
		}
		else {
			component = psy_ui_component_allocinit(&self->component, NULL);
		}
		if (component) {
			psy_ui_component_set_align(component, psy_ui_ALIGN_LEFT);
			psy_ui_component_set_preferred_width(component,
				machinestackstate_column_width(self->state,
					psy_ui_component_textmetric(&self->component)));
			psy_ui_component_set_margin(component,
				psy_ui_margin_make_em(0.0, self->state->col_margin_right, 0.0, 0.0));

		}
	}
	psy_ui_component_align(&self->component);
}

void machinestackvolumes_onpreferredsize(MachineStackVolumes* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_Value margin_right;

	rv->width = machinestackstate_column_width(self->state,
		psy_ui_component_textmetric(&self->component));
	/* + 1: empty space to add new generator */
	psy_ui_value_mul_real(&rv->width,
		psy_max(1.0, machinestackstate_maxnumcolumns(self->state) + 1));
	margin_right = psy_ui_value_make_ew(self->state->col_margin_right);
	psy_ui_value_add(&rv->width, &margin_right,
		psy_ui_component_textmetric(&self->component), NULL);
}

/* MachineStackView */

/* prototypes */
static void machinestackview_on_destroyed(MachineStackView*);
static void machinestackview_on_song_changed(MachineStackView*,
	psy_audio_Player* sender);
static void machinestackview_onbuschanged(MachineStackView*, Workspace*,
	psy_audio_Machine*);
static void machinestackview_build(MachineStackView*);
static void machinestackview_onmachineinsert(MachineStackView*,
	psy_audio_Machines*, uintptr_t mac_id);
static void machinestackview_onmachineremoved(MachineStackView*,
	psy_audio_Machines*, uintptr_t mac_id);
static void machinestackview_onconnected(MachineStackView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackview_ondisconnected(MachineStackView*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void machinestackview_setmachines(MachineStackView*,
	psy_audio_Machines*);
static void machinestackview_on_virtual_generators(MachineStackView*,
	psy_Property* sender);
static void machinestackview_on_app_zoom(
	MachineStackView*, psy_ui_AppZoom* sender);
static void machinestackview_on_zoom(
	MachineStackView*, psy_Property* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(MachineStackView* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			machinestackview_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void machinestackview_init(MachineStackView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent,
	ParamViews* paramviews, MachineMenu* machine_menu, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->paramviews = paramviews;
	self->workspace = workspace;
	psy_ui_component_set_id(&self->component, SECTION_ID_MACHINEVIEW_STACK);
	psy_ui_component_set_style_type(&self->component, STYLE_MV_STACK);
	/* state */
	machinestackstate_init(&self->state, &self->component, self->paramviews);
	self->state.machine_menu = machine_menu;
	/* desc column */
	machinestackdesc_init(&self->desc, &self->component, self,
		&self->state);
	psy_ui_component_set_align(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_init(&self->columns, &self->component, NULL);
	psy_ui_component_set_align(&self->columns, psy_ui_ALIGN_VCLIENT);
	psy_ui_component_set_overflow(&self->columns, psy_ui_OVERFLOW_HSCROLL);
	psy_ui_scroller_init(&self->scroller_columns, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller_columns, &self->columns);
	psy_ui_component_set_align(&self->columns, psy_ui_ALIGN_VCLIENT);
	psy_ui_component_set_align(&self->scroller_columns.component,
		psy_ui_ALIGN_CLIENT);
	machinestackinputs_init(&self->inputs, &self->columns,
		&self->state, workspace);
	psy_ui_component_set_align(&self->inputs.component, psy_ui_ALIGN_TOP);
	machinestackpane_init(&self->pane, &self->columns, &self->state,
		workspace);
	psy_ui_component_set_align(&self->pane.component, psy_ui_ALIGN_CLIENT);
	machinestackvolumes_init(&self->volumes, &self->columns, &self->state);
	psy_ui_component_set_align(&self->volumes.component, psy_ui_ALIGN_BOTTOM);
	machinestackoutputs_init(&self->outputs, &self->columns, &self->state);
	psy_ui_component_set_align(&self->outputs.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_init(&self->spacer, &self->columns, NULL);
	psy_ui_component_set_preferred_size(&self->spacer,
		psy_ui_size_make_em(0.0, 0.5));
	psy_ui_component_set_aligner(&self->spacer, NULL);
	psy_ui_component_set_align(&self->spacer, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&workspace->player_.signal_song_changed, self,
		machinestackview_on_song_changed);
	psy_signal_connect(&workspace->signal_bus_changed, self,
		machinestackview_onbuschanged);
	machinestackview_setmachines(self, workspace_song(workspace)
		? &workspace_song(workspace)->machines_ : NULL);
	psy_configuration_connect(psycleconfig_macview(workspace_cfg(self->workspace)),
		"drawvirtualgenerators", self, machinestackview_on_virtual_generators);
	psy_signal_connect(&psy_ui_app_zoom(psy_ui_app())->signal_zoom, self,
		machinestackview_on_app_zoom);
	psy_configuration_connect(psycleconfig_macview(workspace_cfg(self->workspace)), "zoom",
		self, machinestackview_on_zoom);
}

void machinestackview_on_destroyed(MachineStackView* self)
{
	machinestackstate_dispose(&self->state);
}

void machinestackview_on_song_changed(MachineStackView* self,
	psy_audio_Player* sender)
{
	machinestackview_setmachines(self,
		(psy_audio_player_song(sender))
		? &psy_audio_player_song(sender)->machines_
		: NULL);
}

void machinestackview_onbuschanged(MachineStackView* self, Workspace* sender,
	psy_audio_Machine* machine)
{
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_setmachines(MachineStackView* self,
	psy_audio_Machines* machines)
{
	if (machines) {
		psy_signal_connect(&machines->signal_insert, self,
			machinestackview_onmachineinsert);
		psy_signal_connect(&machines->signal_removed, self,
			machinestackview_onmachineremoved);
		psy_signal_connect(&machines->connections.signal_connected, self,
			machinestackview_onconnected);
		psy_signal_connect(&machines->connections.signal_disconnected, self,
			machinestackview_ondisconnected);
	}
	machinestackstate_setmachines(&self->state, machines);
	machinestackview_build(self);
}

void machinestackview_onmachineinsert(MachineStackView* self,
	psy_audio_Machines* sender, uintptr_t mac_id)
{
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_onmachineremoved(MachineStackView* self,
	psy_audio_Machines* sender, uintptr_t mac_id)
{
	machinestackview_build(self);
	// machinestackstate_rebuildview(&self->state);
}

void machinestackview_onconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_ondisconnected(MachineStackView* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_idle(MachineStackView* self)
{
	if (machinestackstate_rebuildingview(&self->state)) {
		machinestackview_build(self);
	}
}

void machinestackview_build(MachineStackView* self)
{
	assert(self);

	if (!self->state.preventrebuild) {
		psy_ui_Value col_width;

		psy_audio_exclusivelock_enter();
		machinestackstate_buildcolumns(&self->state);
		machinestackpane_build(&self->pane);
		machinestackinputs_build(&self->inputs);
		machinestackoutputs_build(&self->outputs);
		machinestackvolumes_build(&self->volumes);
		col_width = machinestackstate_column_width(&self->state,
			psy_ui_component_textmetric(&self->component));
		psy_ui_component_set_scroll_step_width(&self->columns, col_width);
		psy_ui_app()->alignvalid = FALSE;
		psy_ui_component_align(&self->component);
		psy_ui_app()->alignvalid = TRUE;
		psy_ui_component_invalidate(&self->component);
		psy_audio_exclusivelock_leave();
	}
	machinestackstate_endviewbuild(&self->state);
}

void machinestackview_showvirtualgenerators(MachineStackView* self)
{
	self->state.drawvirtualgenerators = TRUE;
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_hidevirtualgenerators(MachineStackView* self)
{
	self->state.drawvirtualgenerators = FALSE;
	machinestackstate_rebuildview(&self->state);
}

void machinestackview_on_virtual_generators(MachineStackView* self,
	psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		machinestackview_showvirtualgenerators(self);
	}
	else {
		machinestackview_hidevirtualgenerators(self);
	}
}

void machinestackview_on_app_zoom(MachineStackView* self,
	psy_ui_AppZoom* sender)
{
	assert(self);

	machinestackview_build(self);
}

void machinestackview_on_zoom(MachineStackView* self,
	psy_Property* sender)
{
	assert(self);

	machinestackview_build(self);
}
