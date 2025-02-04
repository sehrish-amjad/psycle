/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SAMPLEEDITOR_H)
#define SAMPLEEDITOR_H

#include <uibutton.h>
#include <uicheckbox.h>
#include <uicombobox.h>
#include <uitext.h>
#include <uieditor.h>
#include <uilabel.h>
#include <uislider.h>
#include <uisplitbar.h>
#include <uinotebook.h>
#include "scrollzoom.h"
#include "wavebox.h"
#include "workspace.h"
/* ui */
#include <uilistbox.h>
/* audio */
#include <sample.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
** SampleEditor
**
** Wave editor and loop point definer.
*/

struct SampleEditor;

typedef struct {
	psy_ui_Component component;
	psy_ui_CheckBox selecttogether;
	psy_ui_CheckBox doublecontloop;
	psy_ui_CheckBox doublesustainloop;
	psy_ui_CheckBox drawlines;
	psy_ui_Label visualrepresentationdesc;
	psy_ui_ComboBox visualrepresentation;
	psy_ui_Label selstartlabel;
	psy_ui_Text selstartedit;
	psy_ui_Label selendlabel;
	psy_ui_Text selendedit;
	struct SampleEditor* editor;
	Workspace* workspace;
} SampleEditorBar;

void  sampleeditorbar_init(SampleEditorBar*, psy_ui_Component* parent,
	struct SampleEditor* editor,
	Workspace*);
void sampleeditorbar_setselection(SampleEditorBar* self,
	uintptr_t selectionstart, uintptr_t selectionend);
void sampleeditorbar_clearselection(SampleEditorBar* self);

typedef struct {
	psy_ui_Component component;
	psy_ui_Button cut;
	psy_ui_Button crop;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button del;
	Workspace* workspace;
} SampleEditorOperations;

void sampleeditoroperations_init(SampleEditorOperations*, psy_ui_Component* parent, Workspace*);

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	psy_ui_Slider gain;
	double gainvalue;
	psy_ui_Label dbdisplay;
	Workspace* workspace;
} SampleEditorAmplify;

void sampleeditoramplify_init(SampleEditorAmplify*, psy_ui_Component* parent,
	Workspace* workspace);

typedef struct SampleEditLuaProcessor {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Label header;
	psy_ui_Editor editor;	
} SampleEditLuaProcessor;

void sampleeditluaprocessor_init(SampleEditLuaProcessor*, psy_ui_Component* parent);

typedef struct SampleEditorProcessView {
	/*! @extends  */
	psy_ui_Component component;
	/*! @internal */
	psy_ui_Component client;
	SampleEditorOperations copypaste;
	psy_ui_Button process;
	psy_ui_ListBox processors;
	Workspace* workspace;
	psy_ui_Notebook notebook;
	psy_ui_Component emptypage1;
	psy_ui_Component emptypage2;
	psy_ui_Component emptypage3;
	psy_ui_Component emptypage4;
	psy_ui_Component emptypage5;
	psy_ui_Component emptypage6;
	SampleEditorAmplify amplify;
	SampleEditLuaProcessor luaprocessor;
} SampleEditorProcessView;

void sampleprocessview_init(SampleEditorProcessView*, psy_ui_Component* parent,
	Workspace* workspace);

typedef struct {
	psy_ui_Component component;
	psy_ui_Button loop;
	psy_ui_Button play;
	psy_ui_Button stop;
	psy_ui_Button pause;
	Workspace* workspace;
} SampleEditorPlayBar;

void sampleeditorplaybar_init(SampleEditorPlayBar*, psy_ui_Component* parent,
	Workspace*);

typedef struct {
	psy_ui_Component component;
	WaveBoxContext* metric;
} SampleEditorHeader;

void sampleeditorheader_init(SampleEditorHeader*, psy_ui_Component* parent);


typedef struct SampleBox {
	psy_ui_Component component;
	psy_Table waveboxes;
	psy_Signal signal_selectionchanged;
	Workspace* workspace;
} SampleBox;

void samplebox_init(SampleBox*, psy_ui_Component* parent, Workspace*);
void samplebox_setzoom(SampleBox*, double zoomleft, double zoomright);
void samplebox_setloopviewmode(SampleBox*, WaveBoxLoopViewMode);
void samplebox_drawlines(SampleBox*);
void samplebox_drawbars(SampleBox*);
void samplebox_setquality(SampleBox*, psy_dsp_ResamplerQuality);

/* SampleEditor */
typedef struct SampleEditor {	
	psy_ui_Component component;
	SampleEditorPlayBar playbar;
	SampleEditorHeader header;
	psy_ui_Splitter splitbar;
	SampleEditorProcessView processview;
	SampleBox samplebox;	
	ScrollZoom zoom;
	psy_audio_Sample* sample;	
	Workspace* workspace;
	SampleEditorBar sampleeditortbar;
	psy_Signal signal_samplemodified;
	WaveBoxLoopViewMode loopviewmode;	
} SampleEditor;

void sampleeditor_init(SampleEditor*, psy_ui_Component* parent, Workspace*);

void sampleeditor_setsample(SampleEditor*, psy_audio_Sample*);
void sampleeditor_showdoublecontloop(SampleEditor*);
void sampleeditor_showsinglecontloop(SampleEditor*);
void sampleeditor_showdoublesustainloop(SampleEditor*);
void sampleeditor_showsinglesustainloop(SampleEditor*);
void sampleeditor_drawlines(SampleEditor*);
void sampleeditor_drawbars(SampleEditor*);
void sampleeditor_setquality(SampleEditor*, psy_dsp_ResamplerQuality);

#ifdef __cplusplus
}
#endif

#endif /* SAMPLEEDITOR_H */
