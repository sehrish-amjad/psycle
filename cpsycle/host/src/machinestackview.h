/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINESTACKVIEW_H)
#define MACHINESTACKVIEW_H

/* host */
#include "machineui.h"
#include "machineviewmenu.h"
#include "headerui.h"
#include "paramviews.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif


struct MachineStackState;

/* OutputRouteParam */
typedef struct OutputRouteParam {
	/*! @extends  */
	psy_audio_MachineParam machineparam;
	/*! @internal */
	uintptr_t column;	
	/* references */
	psy_audio_Machines* machines;
	struct MachineStackState* state;
} OutputRouteParam;

void outputrouteparam_init(OutputRouteParam*, psy_audio_Machines*,
	uintptr_t column, struct MachineStackState*);
void outputrouteparam_dispose(OutputRouteParam*);

INLINE psy_audio_MachineParam* outputrouteparam_base(
	OutputRouteParam* self)
{
	return &(self->machineparam);
}

/* MachineStackColumn */
struct psy_audio_MachineParam;
struct psy_audio_Machines;

typedef struct MachineStackColumn {
	uintptr_t column;	
	uintptr_t inputroute;
	uintptr_t input;
	uintptr_t offset;
	psy_List* chain;	
	psy_audio_WireMachineParam* wirevolume;
	psy_audio_IntMachineParam level_param;
	psy_audio_IntMachineParam mute_param;
	OutputRouteParam outputroute;	
	struct MachineStackState* state;
} MachineStackColumn;

void machinestackcolumn_init(MachineStackColumn*, uintptr_t column,
	uintptr_t inputroute, uintptr_t input, struct MachineStackState*);
void machinestackcolumn_dispose(MachineStackColumn*);

void machinestackcolumn_setwire(MachineStackColumn*, psy_audio_Wire);

psy_audio_Wire machinestackcolumn_outputwire(MachineStackColumn*);
psy_audio_Wire machinestackcolumn_sendwire(MachineStackColumn*);
INLINE psy_audio_WireMachineParam* machinestackcolumn_wire(MachineStackColumn* self)
{
	return self->wirevolume;
}

void machinestackcolumn_append(MachineStackColumn*, uintptr_t macid);
uintptr_t machinestackcolumn_lastbeforemaster(const MachineStackColumn*);
uintptr_t machinestackcolumn_lastbeforeroute(const MachineStackColumn*);
uintptr_t machinestackcolumn_lastbeforeindex(const MachineStackColumn*,
	uintptr_t index);
uintptr_t machinestackcolumn_last(const MachineStackColumn*);
uintptr_t machinestackcolumn_at(const MachineStackColumn*,
	uintptr_t index);
uintptr_t machinestackcolumn_nextindex(const MachineStackColumn*,
	uintptr_t index);
bool machinestackcolumn_connectedtomaster(const MachineStackColumn*);
void machinestackcolumn_trace(const MachineStackColumn*);

/* MachineStackState */
typedef struct MachineStackState {
	psy_Table columns;
	psy_audio_Machines* machines;
	uintptr_t selected;
	uintptr_t effectinsertpos;
	bool effectinsertright;
	psy_ui_Size inputs_size;
	psy_ui_Size volume_size;
	double col_margin_right;
	bool update;
	bool columnselected;
	bool preventrebuild;
	bool rewire;
	psy_audio_Wire oldwire;
	psy_audio_Wire newwire;
	uintptr_t currlevel;
	bool drawvirtualgenerators;
	uintptr_t insertmachinemode;	
	ParamViews* paramviews;
	MachineMenu* machine_menu;
	psy_ui_Component* view;
} MachineStackState;

void machinestackstate_init(MachineStackState*, psy_ui_Component* view, ParamViews*);
void machinestackstate_dispose(MachineStackState*);

void machinestackstate_buildcolumns(MachineStackState*);
void machinestackstate_setmachines(MachineStackState*, psy_audio_Machines*);
MachineStackColumn* machinestackstate_insertcolumn(MachineStackState*,
	uintptr_t column, uintptr_t inputroute, uintptr_t input);
MachineStackColumn* machinestackstate_column(MachineStackState*,
	uintptr_t column);
MachineStackColumn* machinestackstate_selectedcolumn(MachineStackState*);
uintptr_t machinestackstate_maxnumcolumns(const MachineStackState*);
void machinestackstate_clear(MachineStackState*);
psy_List* machinestackstate_inputs(MachineStackState*);
psy_List* machinestackstate_buses(MachineStackState*);
void machinestackstate_check_wirechange(MachineStackState*);
psy_ui_Value machinestackstate_column_width(const MachineStackState*,
	const psy_ui_TextMetric*);

INLINE void machinestackstate_rebuildview(MachineStackState* self)
{
	self->update = TRUE;
}

INLINE bool machinestackstate_rebuildingview(const MachineStackState* self)
{
	return self->update;
}

INLINE void machinestackstate_endviewbuild(MachineStackState* self)
{
	self->update = FALSE;
}

struct MachineStackView;


/* MachineStackDesc */
typedef struct MachineStackDesc {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Label inputs;	
	psy_ui_Button effects;
	psy_ui_Button outputs;
	psy_ui_Button volumes;
	/* references */
	struct MachineStackView* view;
	MachineStackState* state;
} MachineStackDesc;

void machinestackdesc_init(MachineStackDesc*, psy_ui_Component* parent,
	struct MachineStackView*, MachineStackState*);

/* MachineStackInputs */
typedef struct MachineStackInputs {
	/*! @extends  */
	psy_ui_Component component;	
	/* references */
	Workspace* workspace;	
	MachineStackState* state;
} MachineStackInputs;

void machinestackinputs_init(MachineStackInputs*, psy_ui_Component* parent,
	MachineStackState*, Workspace*);

void machinestackinputs_build(MachineStackInputs*);

/* MachineStackOutputs */

typedef struct MachineStackOutputs {
	/*! @extends  */
	psy_ui_Component component;
	/* references */
	Workspace* workspace;	
	MachineStackState* state;
} MachineStackOutputs;

void machinestackoutputs_init(MachineStackOutputs*, psy_ui_Component* parent,
	MachineStackState* state);

void machinestackoutputs_build(MachineStackOutputs*);


/* MachineStackPane */
typedef struct MachineStackPane {
	/*! @extends  */
	psy_ui_Component component;	
	/* internal data */
	bool vudrawupdate;
	uintptr_t opcount;
	/* references */
	Workspace* workspace;	
	MachineStackState* state;
} MachineStackPane;

void machinestackpane_init(MachineStackPane*, psy_ui_Component* parent,
	MachineStackState*, Workspace*);

void machinestackpane_build(MachineStackPane*);


/* MachineStackVolumes */
typedef struct MachineStackVolumes {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	/* references */
	MachineStackState* state;		
} MachineStackVolumes;

void machinestackvolumes_init(MachineStackVolumes*, psy_ui_Component* parent,
	MachineStackState*);

void machinestackvolumes_build(MachineStackVolumes*);


/* MachineStackPaneTrackClient */
typedef struct MachineStackPaneTrackClient {
	psy_ui_Component component;
	/*! @internal */
	uintptr_t column;
	/* references */
	MachineStackState* state;
} MachineStackPaneTrackClient;

void machinestackpanetrackclient_init(MachineStackPaneTrackClient*,
	psy_ui_Component* parent, uintptr_t column,
	MachineStackState*);

/* MachineStackPaneTrack */
typedef struct MachineStackPaneTrack {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	MachineStackPaneTrackClient client;
	psy_ui_Scroller scroller;	
	uintptr_t column;	
	/* references */
	MachineStackState* state;	
	Workspace* workspace;	
} MachineStackPaneTrack;

void machinestackpanetrack_init(MachineStackPaneTrack*,
	psy_ui_Component* parent, uintptr_t column,
	MachineStackState*, Workspace*);

MachineStackPaneTrack* machinestackpanetrack_alloc(void);
MachineStackPaneTrack* machinestackpanetrack_allocinit(
	psy_ui_Component* parent, uintptr_t column,
	MachineStackState*, Workspace*);

/*!
** @struct MachineStackView
** @brief Stacks of the machines starting with their leafs.
*/
typedef struct MachineStackView {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	MachineStackDesc desc;
	psy_ui_Component columns;
	MachineStackInputs inputs;	
	MachineStackPane pane;
	psy_ui_Component spacer;	
	MachineStackOutputs outputs;
	MachineStackVolumes volumes;	
	psy_ui_Scroller scroller_columns;
	MachineStackState state;
	/* references */
	Workspace* workspace;	
	ParamViews* paramviews;	
} MachineStackView;

void machinestackview_init(MachineStackView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, ParamViews*, MachineMenu*, Workspace*);

void machinestackview_updateskin(MachineStackView*);
void machinestackview_showvirtualgenerators(MachineStackView*);
void machinestackview_hidevirtualgenerators(MachineStackView*);
void machinestackview_idle(MachineStackView*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINESTACKVIEW_H */
