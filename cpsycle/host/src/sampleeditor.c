/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sampleeditor.h"
/* host */
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <operations.h>
#include <psyclescript.h>
#include <songio.h>
/* std */
#include <assert.h>
#include <math.h>
/* platform */
#include "../../detail/portable.h"

#define LUAPROCESSOR_DEFAULT \
"local frames = {}""\n"\
"\n"\
"function work()""\n"\
"\n"\
"  for i = 0, 1000 do""\n"\
"    frames[i] = 0.5""\n"\
"  end""\n"\
"  return frames""\n"\
"end""\n"\
"\n"

static void sampleeditorbar_ondoublecontloop(SampleEditorBar*,
	psy_ui_Component* sender);
static void sampleeditorbar_ondoublesustainloop(SampleEditorBar*,
	psy_ui_Component* sender);
static void sampleeditorbar_ondrawlines(SampleEditorBar*,
	psy_ui_Component* sender);

void sampleeditorbar_init(SampleEditorBar* self, psy_ui_Component* parent,
	SampleEditor* editor,
	Workspace* workspace)
{
	self->workspace = workspace;
	self->editor = editor;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 2.0, 0.0, 0.0));	
	psy_ui_checkbox_init(&self->selecttogether, &self->component);
	psy_ui_checkbox_set_text(&self->selecttogether,
		"samplesview.select-together");
	psy_ui_checkbox_check(&self->selecttogether);
	psy_ui_label_init_text(&self->selstartlabel, &self->component,
		"samplesview.selstart");
	psy_ui_text_init(&self->selstartedit, &self->component);	
	psy_ui_text_set_char_number(&self->selstartedit, 10);			
	psy_ui_label_init_text(&self->selendlabel, &self->component,
		"samplesview.selend");
	psy_ui_text_init(&self->selendedit, &self->component);	
	psy_ui_text_set_char_number(&self->selendedit, 10);
	psy_ui_label_init_text(&self->visualrepresentationdesc, &self->component,
		"samplesview.visual");
	psy_ui_checkbox_init(&self->doublecontloop, &self->component);
	psy_ui_checkbox_set_text(&self->doublecontloop, "samplesview.doublecont");		
	psy_signal_connect(&self->doublecontloop.signal_clicked, self,
		sampleeditorbar_ondoublecontloop);
	psy_ui_checkbox_init(&self->doublesustainloop, &self->component);
	psy_ui_checkbox_set_text(&self->doublesustainloop, "samplesview.doublesus");
	psy_signal_connect(&self->doublesustainloop.signal_clicked, self,
		sampleeditorbar_ondoublesustainloop);	
	psy_ui_checkbox_init(&self->drawlines, &self->component);
	psy_ui_checkbox_set_text(&self->drawlines, "samplesview.lines");
	psy_signal_connect(&self->drawlines.signal_clicked, self,
		sampleeditorbar_ondrawlines);
	{	// resampling methods
		int quality;
		
		psy_ui_combobox_init(&self->visualrepresentation, &self->component);
		psy_ui_combobox_set_char_number(&self->visualrepresentation, 12);
		for (quality = 0; quality < psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS;
				++quality) {
			psy_ui_combobox_add_text(&self->visualrepresentation,
				psy_dsp_multiresampler_name(
					(psy_dsp_ResamplerQuality)quality));
		}
	}
	// set to default in wavebox, change it there, too
	psy_ui_combobox_select(&self->visualrepresentation,
		psy_dsp_RESAMPLERQUALITY_SPLINE);	
	sampleeditorbar_clearselection(self);	
}

void sampleeditorbar_setselection(SampleEditorBar* self,
	uintptr_t selectionstart, uintptr_t selectionend)
{
	char text[128];

	psy_ui_text_enable(&self->selstartedit);
	psy_ui_text_enable(&self->selendedit);
	psy_snprintf(text, 128, "%u", (unsigned int)selectionstart);
	psy_ui_text_set_text(&self->selstartedit, text);
	psy_snprintf(text, 128, "%u", (unsigned int)selectionend);
	psy_ui_text_set_text(&self->selendedit, text);
}

void sampleeditorbar_clearselection(SampleEditorBar* self)
{
	psy_ui_text_prevent(&self->selstartedit);
	psy_ui_text_prevent(&self->selendedit);
	psy_ui_text_set_text(&self->selstartedit, "");
	psy_ui_text_set_text(&self->selendedit, "");
}

void sampleeditorbar_ondoublecontloop(SampleEditorBar* self,
	psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->doublecontloop)) {
		sampleeditor_showdoublecontloop(self->editor);
	} else {
		sampleeditor_showsinglecontloop(self->editor);
	}
	psy_ui_checkbox_disable_check(&self->doublesustainloop);
}

void sampleeditorbar_ondoublesustainloop(SampleEditorBar* self,
	psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->doublesustainloop)) {
		sampleeditor_showdoublesustainloop(self->editor);
	} else {
		sampleeditor_showsinglesustainloop(self->editor);		
	}
	psy_ui_checkbox_disable_check(&self->doublecontloop);
}

void sampleeditorbar_ondrawlines(SampleEditorBar* self,
	psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->drawlines)) {
		sampleeditor_drawlines(self->editor);
	} else {
		sampleeditor_drawbars(self->editor);
	}	
}

static void sampleeditoroperations_initalign(SampleEditorOperations*);

void sampleeditoroperations_init(SampleEditorOperations* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_button_init(&self->cut, &self->component);
	psy_ui_button_set_text(&self->cut, "edit.cut");
	psy_ui_button_init(&self->crop, &self->component);
	psy_ui_button_set_text(&self->crop, "edit.crop");
	psy_ui_button_init(&self->copy, &self->component);
	psy_ui_button_set_text(&self->copy, "edit.copy");
	psy_ui_button_init(&self->paste, &self->component);
	psy_ui_button_set_text(&self->paste, "edit.paste");
	psy_ui_button_init(&self->del, &self->component);
	psy_ui_button_set_text(&self->del, "edit.delete");
	sampleeditoroperations_initalign(self);
}

void sampleeditoroperations_initalign(SampleEditorOperations* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_em(&margin, 0.0, 0.5, 0.0, 0.5);		
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONE_RECURSIVE),
		psy_ui_ALIGN_LEFT, margin));
}

static void sampleeditoramplify_ontweak(SampleEditorAmplify*, psy_ui_Slider*,
	double value);
static void sampleeditoramplify_onvalue(SampleEditorAmplify*, psy_ui_Slider*,
	double* value);
static void sampleeditoramplify_ondescribe(SampleEditorAmplify*, psy_ui_Slider*,
	char* text);

void sampleeditoramplify_init(SampleEditorAmplify* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	char text[128];

	self->workspace = workspace;
	self->gainvalue = 2.0 / 3.0;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->header, &self->component, "Adjust Volume");
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_init(&self->gain, &self->component);
	psy_ui_slider_show_vertical(&self->gain);
	psy_ui_slider_set_char_number(&self->gain, 4);
	psy_ui_slider_connect(&self->gain, self,
		(ui_slider_fpdescribe)sampleeditoramplify_ondescribe,
		(ui_slider_fptweak)sampleeditoramplify_ontweak,
		(ui_slider_fpvalue)sampleeditoramplify_onvalue);
	psy_ui_component_set_align(&self->gain.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->dbdisplay, &self->component);
	psy_ui_label_prevent_translation(&self->dbdisplay);
	sampleeditoramplify_ondescribe(self, 0, text);
	psy_ui_label_set_text(&self->dbdisplay, text);
	psy_ui_component_set_align(&self->dbdisplay.component, psy_ui_ALIGN_BOTTOM);
}

void sampleeditoramplify_ontweak(SampleEditorAmplify* self,
	psy_ui_Slider* slider, double value)
{
	char text[128];

	self->gainvalue = (intptr_t)(value * 288) / 288.f;
	sampleeditoramplify_ondescribe(self, 0, text);
	psy_ui_label_set_text(&self->dbdisplay, text);
}

void sampleeditoramplify_onvalue(SampleEditorAmplify* self,
	psy_ui_Slider* slider, double* value)
{
		*value = self->gainvalue;
}

void sampleeditoramplify_ondescribe(SampleEditorAmplify* self,
	psy_ui_Slider* slider, char* text)
{			
	if (self->gainvalue == 0.f) {
		psy_snprintf(text, 10, "-inf. dB");
	} else {
		double db = (self->gainvalue - 2/3.0) * 144.f;
		psy_snprintf(text, 10, "%.2f dB", db);
	}	
}

void sampleeditluaprocessor_init(SampleEditLuaProcessor* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->header, &self->component, "Lua Processor");
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_editor_init(&self->editor, &self->component);
	psy_ui_editor_set_text(&self->editor, LUAPROCESSOR_DEFAULT);
	psy_ui_component_set_align(&self->editor.component, psy_ui_ALIGN_CLIENT);	
}

/* SampleEditorProcessView */
/* prototypes */
static void sampleprocessview_buildprocessorlist(SampleEditorProcessView*);
static void sampleeditorprocessview_onprocessorselected(
	SampleEditorProcessView*, psy_ui_Component* sender, int index);


/* implementation */
void sampleprocessview_init(SampleEditorProcessView* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;
	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_init(&self->client, &self->component, NULL);
	psy_ui_component_set_align(&self->client, psy_ui_ALIGN_CLIENT);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 1.0);		
	psy_ui_component_set_margin(&self->client, margin);
	sampleeditoroperations_init(&self->copypaste, &self->client, workspace);
	psy_ui_component_set_align(&self->copypaste.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->process, &self->client);
	psy_ui_button_set_text(&self->process, "samplesview.process");
	psy_ui_component_set_align(&self->process.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_em(&margin, 1.5, 0.0, 0.5, 0.0);		
	psy_ui_component_set_margin(&self->process.component, margin);
	psy_ui_listbox_init(&self->processors, &self->client);
	psy_ui_component_set_align(&self->processors.component, psy_ui_ALIGN_TOP);
	psy_ui_component_set_margin(&self->processors.component, margin);	
	psy_ui_notebook_init(&self->notebook, &self->client);
	psy_ui_component_set_align(&self->notebook.component, psy_ui_ALIGN_CLIENT);
	sampleeditoramplify_init(&self->amplify, &self->notebook.component,
		workspace);	
	sampleprocessview_buildprocessorlist(self);
	psy_ui_component_init(&self->emptypage1,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage2,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage3,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage4,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage5,
		psy_ui_notebook_base(&self->notebook), NULL);
	psy_ui_component_init(&self->emptypage6,
		psy_ui_notebook_base(&self->notebook), NULL);
	sampleeditluaprocessor_init(&self->luaprocessor,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_listbox_set_cur_sel(&self->processors, 0);
	psy_ui_notebook_select(&self->notebook, 0);
	psy_signal_connect(&self->processors.signal_selchanged, self,
		sampleeditorprocessview_onprocessorselected);	
}

void sampleprocessview_buildprocessorlist(SampleEditorProcessView* self)
{
	psy_ui_listbox_clear(&self->processors);
	psy_ui_listbox_add_text(&self->processors, "Amplify");
	psy_ui_listbox_add_text(&self->processors, "Fade In");
	psy_ui_listbox_add_text(&self->processors, "Fade Out");
	psy_ui_listbox_add_text(&self->processors, "Insert Silence");
	psy_ui_listbox_add_text(&self->processors, "Normalize");
	psy_ui_listbox_add_text(&self->processors, "Remove DC");
	psy_ui_listbox_add_text(&self->processors, "Reverse");
	psy_ui_listbox_add_text(&self->processors, "Lua");
}

void sampleeditorprocessview_onprocessorselected(SampleEditorProcessView* self,
	psy_ui_Component* sender, int index)
{
	psy_ui_notebook_select(&self->notebook, index);
}

static void sampleeditorplaybar_initalign(SampleEditorPlayBar*);

enum {
	SAMPLEEDITOR_DRAG_NONE,
	SAMPLEEDITOR_DRAG_LEFT,
	SAMPLEEDITOR_DRAG_RIGHT,
	SAMPLEEDITOR_DRAG_MOVE
};

void sampleeditorplaybar_init(SampleEditorPlayBar* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_padding(&self->component,
		psy_ui_margin_make_em(0.5, 0.0, 0.5, 0.0));
	// psy_ui_button_init(&self->loop, &self->component);
	// psy_ui_button_set_text(&self->loop, "Loop");	
	// psy_signal_connect(&self->loop.signal_clicked, self, onloopclicked);	
	psy_ui_button_init(&self->play, &self->component);
	psy_ui_button_set_text(&self->play, "samplesview.play");	
	psy_ui_button_init(&self->stop, &self->component);
	psy_ui_button_set_text(&self->stop, "samplesview.stop");
	// psy_signal_connect(&self->stop.signal_clicked, self, onstopclicked);	
	sampleeditorplaybar_initalign(self);	
}

void sampleeditorplaybar_initalign(SampleEditorPlayBar* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_em(&margin, 0.0, 0.5, 0.0, 0.0);		
	psy_ui_component_set_align_expand(&self->component,
		psy_ui_HEXPAND);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONE_RECURSIVE),
		psy_ui_ALIGN_LEFT, margin));
}

// Header
static void sampleeditorheader_ondraw(SampleEditorHeader*, psy_ui_Graphics*);
static void sampleeditorheader_onpreferredsize(SampleEditorHeader*,
	psy_ui_Size* limit, psy_ui_Size* size);
static void sampleeditorheader_drawruler(SampleEditorHeader*, psy_ui_Graphics*);
static void sampleeditorheader_setwaveboxmetric(SampleEditorHeader*,
	WaveBoxContext* metric);
/* vtable */
static psy_ui_ComponentVtable sampleeditorheader_vtable;
static bool sampleeditorheader_vtable_initialized = FALSE;

static void sampleeditorheader_vtable_init(SampleEditorHeader* self)
{
	if (!sampleeditorheader_vtable_initialized) {
		sampleeditorheader_vtable = *(self->component.vtable);
		sampleeditorheader_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			sampleeditorheader_ondraw;
		sampleeditorheader_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			sampleeditorheader_onpreferredsize;
		sampleeditorheader_vtable_initialized = TRUE;
	}
	self->component.vtable = &sampleeditorheader_vtable;
}

/* implementation */
void sampleeditorheader_init(SampleEditorHeader* self,
	psy_ui_Component* parent)
{
	self->metric = 0;
	psy_ui_component_init(&self->component, parent, NULL);
	sampleeditorheader_vtable_init(self);			
}

void sampleeditorheader_setwaveboxmetric(SampleEditorHeader* self,
	WaveBoxContext* metric)
{
	self->metric = metric;
}

void sampleeditorheader_ondraw(SampleEditorHeader* self, psy_ui_Graphics* g)
{		
	sampleeditorheader_drawruler(self, g);	
}

void sampleeditorheader_onpreferredsize(SampleEditorHeader* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {
		rv->width = limit->width;
		rv->height = psy_ui_value_make_eh(1.5);
	}
}

void sampleeditorheader_drawruler(SampleEditorHeader* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	double baseline;
	const psy_ui_TextMetric* tm;
	uintptr_t frame;
	uintptr_t step;

	size = psy_ui_component_size_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = size.height - 1;
	psy_ui_graphics_set_colour(g, psy_ui_colour_make(0x00666666));
	psy_ui_drawline(g, psy_ui_realpoint_make(0, baseline),
		psy_ui_realpoint_make(size.width, baseline));
	psy_ui_graphics_set_background_mode(g, psy_ui_TRANSPARENT);
	psy_ui_graphics_set_text_colour(g, psy_ui_colour_make(0x00999999));
	if (self->metric) {
		step = waveboxcontext_numframes(self->metric) / 10;
		step = (uintptr_t)((double)step * (self->metric->zoomright -
			self->metric->zoomleft));
		if (step == 0) {
			step = 1;
		}
		for (frame = 0; frame < waveboxcontext_numframes(self->metric);
				frame += step) {
			double cpx;

			cpx = waveboxcontext_frametoscreen(self->metric, frame);
			if (cpx >= 0 && cpx < size.width) {
				char txt[40];

				psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
					psy_ui_realpoint_make(cpx, baseline - tm->tmHeight / 3));
				psy_snprintf(txt, 40, "%d", (int)waveboxcontext_realframe(
					self->metric, frame));
				psy_ui_graphics_textout(g, psy_ui_realpoint_make((int)cpx + (int)(tm->tmAveCharWidth * 0.75),
					baseline - tm->tmHeight - tm->tmHeight / 6), txt,
					psy_strlen(txt));
			}
		}
	}
}


/* prototypes */
static void samplebox_on_destroyed(SampleBox*);
static void samplebox_clearwaveboxes(SampleBox*);
static void samplebox_buildwaveboxes(SampleBox*, psy_audio_Sample*,
	WaveBoxLoopViewMode);
static void samplebox_onselectionchanged(SampleBox*, WaveBox* sender);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(SampleBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			samplebox_on_destroyed;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void samplebox_init(SampleBox* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->workspace = workspace;	
	psy_table_init(&self->waveboxes);	
	psy_signal_init(&self->signal_selectionchanged);
}

void samplebox_on_destroyed(SampleBox* self)
{		
	psy_signal_dispose(&self->signal_selectionchanged);
	psy_table_dispose(&self->waveboxes);
}

void samplebox_clearwaveboxes(SampleBox* self)
{
	psy_table_clear(&self->waveboxes);	
	psy_ui_component_clear(&self->component);	
}

void samplebox_buildwaveboxes(SampleBox* self, psy_audio_Sample* sample,
	WaveBoxLoopViewMode loopviewmode)
{
	samplebox_clearwaveboxes(self);
	if (sample) {
		uintptr_t channel;

		for (channel = 0; channel < psy_audio_buffer_num_channels(
			&sample->channels); ++channel) {
			WaveBox* wavebox;

			wavebox = wavebox_allocinit(&self->component, self->workspace);			
			wavebox_setloopviewmode(wavebox, loopviewmode);
			wavebox->preventdrawonselect = TRUE;
			wavebox_setsample(wavebox, sample, channel);
			psy_ui_component_set_align(&wavebox->component, psy_ui_ALIGN_CLIENT);
			psy_signal_connect(&wavebox->selectionchanged, self,
				samplebox_onselectionchanged);
			psy_table_insert(&self->waveboxes, channel, (void*)wavebox);
		}
	}
	psy_ui_component_align_invalidate(&self->component);	
}

void samplebox_setzoom(SampleBox* self, double zoomleft, double zoomright)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setzoom(wavebox, zoomleft, zoomright);
	}	
}

void samplebox_setloopviewmode(SampleBox* self, WaveBoxLoopViewMode mode)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setloopviewmode(wavebox, mode);
	}
	psy_ui_component_invalidate(&self->component);
}

void samplebox_drawlines(SampleBox* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_drawlines(wavebox);
	}
	psy_ui_component_invalidate(&self->component);
}

void samplebox_setquality(SampleBox* self, psy_dsp_ResamplerQuality quality)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setquality(wavebox, quality);
	}	
}

void samplebox_drawbars(SampleBox* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_drawbars(wavebox);
	}
	psy_ui_component_invalidate(&self->component);
}


void samplebox_onselectionchanged(SampleBox* self, WaveBox* sender)
{
	psy_signal_emit(&self->signal_selectionchanged, self, 1, sender);
}


/* SampleEditor*/

/* prototypes */
static void sampleeditor_on_destroyed(SampleEditor*);
static void sampleeditor_on_zoom(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_on_song_changed(SampleEditor*,
	psy_audio_Player* sender);
static void sampleeditor_on_play(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_on_stop(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_on_process(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_on_crop(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_on_delete(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_on_selection_changed(SampleEditor*, SampleBox*,
	WaveBox* wavebox);
static void sampleeditor_onscrollzoom_customdraw(SampleEditor*,
	ScrollZoom* sender, psy_ui_Graphics*);
static void sampleeditor_set_loopview_mode(SampleEditor*,
	WaveBoxLoopViewMode mode);
static void sampleeditor_amplify(SampleEditor*, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend,
	double gain);
static void sampleeditor_remove_dc(SampleEditor*, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend);
static void sampleeditor_processlua(SampleEditor*, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend);
static void sampleeditor_selectionbound(SampleEditor*, uintptr_t* framestart,
	uintptr_t* frameend, bool* hasselection);

/* vtable */
static psy_ui_ComponentVtable sampleeditor_vtable;
static bool sampleeditor_vtable_initialized = FALSE;

static void sampleeditor_vtable_init(SampleEditor* self)
{
	if (!vtable_initialized) {
		sampleeditor_vtable = *(self->component.vtable);
		sampleeditor_vtable.on_destroyed =
			(psy_ui_fp_component)
			sampleeditor_on_destroyed;
		sampleeditor_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &sampleeditor_vtable);
}

/* implementation */
void sampleeditor_init(SampleEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	assert(self);
	assert(workspace);
	
	self->sample = NULL;	
	self->workspace = workspace;
	self->loopviewmode = WAVEBOX_LOOPVIEW_CONT_SINGLE;
	psy_ui_component_init(&self->component, parent, NULL);	
	sampleeditor_vtable_init(self);
	sampleprocessview_init(&self->processview, &self->component, workspace);
	psy_ui_component_set_align(&self->processview.component, psy_ui_ALIGN_RIGHT);
	//psy_ui_component_set_margin(&self->processview.component, &margin);
	psy_signal_connect(&self->processview.process.signal_clicked, self,
		sampleeditor_on_process);
	psy_signal_connect(&self->processview.copypaste.crop.signal_clicked, self,
		sampleeditor_on_crop);
	psy_signal_connect(&self->processview.copypaste.del.signal_clicked, self,
		sampleeditor_on_delete);
	psy_ui_splitter_init(&self->splitbar, &self->component);
	psy_ui_component_set_align(&self->splitbar.component, psy_ui_ALIGN_RIGHT);
	sampleeditorplaybar_init(&self->playbar, &self->component, workspace);
	psy_signal_connect(&self->playbar.play.signal_clicked, self,
		sampleeditor_on_play);
	psy_signal_connect(&self->playbar.stop.signal_clicked, self,
		sampleeditor_on_stop);
	psy_ui_component_set_align(&self->playbar.component, psy_ui_ALIGN_TOP);	
	sampleeditorheader_init(&self->header, &self->component);
	psy_ui_component_set_align(&self->header.component, psy_ui_ALIGN_TOP);		
	samplebox_init(&self->samplebox, &self->component, workspace);	
	psy_ui_component_set_align(&self->samplebox.component, psy_ui_ALIGN_CLIENT);	
	psy_signal_connect(&self->samplebox.signal_selectionchanged, self,
		sampleeditor_on_selection_changed);
	scrollzoom_init(&self->zoom, &self->component);
	psy_signal_connect(&self->zoom.signal_customdraw, self,
		sampleeditor_onscrollzoom_customdraw);
	psy_ui_component_set_align(&self->zoom.component, psy_ui_ALIGN_BOTTOM);
	psy_ui_component_set_preferred_size(&self->zoom.component,
		psy_ui_size_make_em(0.0, 2.0));	
	psy_signal_connect(&self->zoom.signal_zoom, self, sampleeditor_on_zoom);
	psy_signal_connect(&workspace->player_.signal_song_changed, self,
		sampleeditor_on_song_changed);	
	psy_signal_init(&self->signal_samplemodified);
}

void sampleeditor_on_destroyed(SampleEditor* self)
{			
	assert(self);
	
	psy_signal_disconnect(&self->workspace->player_.signal_song_changed, self,
		sampleeditor_on_song_changed);
	psy_signal_dispose(&self->signal_samplemodified);
}

void sampleeditor_setsample(SampleEditor* self, psy_audio_Sample* sample)
{
	if (self->sample != NULL || sample != NULL) {
		self->sample = sample;
		sampleeditorheader_setwaveboxmetric(&self->header, 0);
		samplebox_buildwaveboxes(&self->samplebox, sample, self->loopviewmode);
		if (!sample) {
			psy_ui_component_invalidate(&self->component);
		}
		if (psy_table_size(&self->samplebox.waveboxes) > 0) {
			WaveBox* wavebox;
			
			wavebox = (WaveBox*)psy_table_at(&self->samplebox.waveboxes, 0);
			if (wavebox) {
				sampleeditorheader_setwaveboxmetric(&self->header,
					wavebox_metric(wavebox));
			}
		}
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_invalidate(&self->samplebox.component);
		psy_ui_component_invalidate(&self->zoom.component);
	}
}

void sampleeditor_on_zoom(SampleEditor* self, psy_ui_Component* sender)
{
	samplebox_setzoom(&self->samplebox, scrollzoom_start(&self->zoom),
		scrollzoom_end(&self->zoom));
	psy_ui_component_invalidate(&self->header.component);
}

void sampleeditor_on_song_changed(SampleEditor* self, psy_audio_Player* sender)
{	
	sampleeditorbar_clearselection(&self->sampleeditortbar);
}

void sampleeditor_on_play(SampleEditor* self, psy_ui_Component* sender)
{	
	if (self->sample) {
		psy_audio_player_wave_prev_play_sample(&self->workspace->player_,
			self->sample);	
	}
}

void sampleeditor_on_stop(SampleEditor* self, psy_ui_Component* sender)
{	
	psy_audio_player_wave_prev_stop(&self->workspace->player_);
}

void sampleeditor_on_process(SampleEditor* self, psy_ui_Component* sender)
{
	psy_TableIterator it;

	intptr_t selected;

	selected = psy_ui_listbox_cur_sel(&self->processview.processors);
	for (it = psy_table_begin(&self->samplebox.waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		if (wavebox_hasselection(wavebox)) {
			uintptr_t channel;

			channel = psy_tableiterator_key(&it);
			switch (selected) {
				case 0: { // Amplify			
					double ratio;

					ratio = pow(10.0,
						(self->processview.amplify.gainvalue - 2.0 / 3.0) /
						(1 / 7.0));
					sampleeditor_amplify(self, channel,
						wavebox->context.selection.start,
						wavebox->context.selection.end,
						ratio);
					break;
				}				
				case 1:
					// FadeIn
					psy_dsp_fade(&dsp, self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection),
							0.f, 1.f);				
					break;
				case 2:
					// FadeOut
					psy_dsp_fade(&dsp, self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection),
						1.f, 0.f);
					break;
				case 4:
					psy_dsp_normalize(&dsp,
						self->sample->channels.samples[channel] +
							wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection));
					break;
				case 5:
					sampleeditor_remove_dc(self, channel,
						wavebox->context.selection.start,
						wavebox->context.selection.end);
					break;
				case 6:
					dsp.reverse(self->sample->channels.samples[channel] +
						wavebox->context.selection.start,
						waveboxselection_numframes(&wavebox->context.selection));				
					break;
				case 7:
					sampleeditor_processlua(self, channel,
						wavebox->context.selection.start,
						wavebox->context.selection.end);
					break;
				default:
					break;
			}
		} else			
		if (selected == 7) {
			if (self->sample) {
				/*psy_ui_editor_set_text(&self->processview.luaprocessor.console,
					"No Selection, first mark samples to process"); */
			} 			
		}		
	}
	if (psy_table_size(&self->samplebox.waveboxes) == 0) {
		if (selected == 7) {
			if (!self->sample) {
			/*	psy_ui_editor_set_text(&self->processview.luaprocessor.console,
					"No wave data, first load sample"); */
			}
		}
	}
	psy_ui_component_invalidate(&self->samplebox.component);
}

void sampleeditor_on_crop(SampleEditor* self, psy_ui_Component* sender)
{
	if (self->sample && self->sample->numframes > 0) {
		psy_TableIterator it;
		uintptr_t framestart;
		uintptr_t frameend;
		bool hasselection;

		sampleeditor_selectionbound(self, &framestart, &frameend,
			&hasselection);
		if (hasselection && frameend <= self->sample->numframes) {
			self->sample->numframes = frameend - framestart + 1;			
			for (it = psy_table_begin(&self->samplebox.waveboxes);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				WaveBox* wavebox;

				wavebox = (WaveBox*)psy_tableiterator_value(&it);
				self->sample->channels.samples[psy_tableiterator_key(&it)] = 
					dsp.crop(self->sample->channels.samples[psy_tableiterator_key(&it)]
						, framestart, self->sample->numframes);
				wavebox_clearselection(wavebox);
				wavebox_refresh(wavebox);
			}			
		}
	}
	psy_ui_component_invalidate(&self->samplebox.component);
	psy_ui_component_invalidate(&self->header.component);
	psy_signal_emit(&self->signal_samplemodified, self, 1, self->sample);
}

void sampleeditor_on_delete(SampleEditor* self, psy_ui_Component* sender)
{
	if (self->sample && self->sample->numframes > 0) {
		psy_TableIterator it;
		uintptr_t framestart;
		uintptr_t frameend;		
		bool hasselection = FALSE;		
		sampleeditor_selectionbound(self, &framestart, &frameend,
			&hasselection);
		psy_audio_exclusivelock_enter();		
		if (hasselection && frameend <= self->sample->numframes) {
			uintptr_t range;
			uintptr_t newnumframes;

			range = frameend - framestart + 1;
			newnumframes = self->sample->numframes - range;
			for (it = psy_table_begin(&self->samplebox.waveboxes);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				WaveBox* wavebox;
				float* oldwave;
				float* newwave;				

				wavebox = (WaveBox*)psy_tableiterator_value(&it);
				oldwave = self->sample->channels.samples[psy_tableiterator_key(&it)];	
				if (newnumframes > 0) {
					newwave = (float*)dsp.memory_alloc(newnumframes,
						sizeof(float));
					dsp.clear(newwave, newnumframes);
					if (framestart > 0) {
						dsp.add(oldwave, newwave, framestart, 1.f);
					}
					if (newnumframes - range > 0) {
						dsp.add(oldwave + frameend + 1, newwave + framestart,
							newnumframes - framestart - 1, 1.f);
					}
				} else {
					newwave = NULL;
				}
				self->sample->channels.samples[psy_tableiterator_key(&it)] =
					newwave;
				dsp.memory_dealloc(oldwave);
				wavebox_clearselection(wavebox);
				wavebox_refresh(wavebox);
			}
			self->sample->numframes = newnumframes;
		}		
		psy_audio_exclusivelock_leave();
	}
	psy_ui_component_invalidate(&self->samplebox.component);
	psy_ui_component_invalidate(&self->header.component);
	psy_signal_emit(&self->signal_samplemodified, self, 1, self->sample);
}

void sampleeditor_selectionbound(SampleEditor* self, uintptr_t* framestart,
	uintptr_t* frameend, bool* hasselection)
{
	psy_TableIterator it;	

	*hasselection = FALSE;
	*framestart = self->sample->numframes;
	*frameend = 0;
	for (it = psy_table_begin(&self->samplebox.waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		if (wavebox->context.selection.start < *framestart) {
			*framestart = wavebox->context.selection.start;
		}
		if (wavebox->context.selection.end > *frameend) {
			*frameend = wavebox->context.selection.end;
		}
		if (wavebox->context.selection.hasselection) {
			*hasselection = TRUE;
		}
	}
}

void sampleeditor_amplify(SampleEditor* self, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend, double gain)
{	
	dsp.mul(self->sample->channels.samples[channel] + framestart,
		frameend - framestart + 1, (float)gain);
}

void sampleeditor_processlua(SampleEditor* self, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend)
{
	/* psy_PsycleScript processor;
	char text[65535];

	psy_ui_editor_clear(&self->processview.luaprocessor.console);
	psyclescript_init(&processor);
	psy_ui_editor_text(&self->processview.luaprocessor.editor, 65534, text);

	if (psyclescript_loadstring(&processor, text) != LUA_OK) {
		const char* msg = lua_tostring(processor.L, -1);
		psy_ui_editor_set_text(&self->processview.luaprocessor.console,
			msg);
		psyclescript_dispose(&processor);
		return;
	}
	if (psyclescript_run(&processor) != LUA_OK) {
		const char* msg = lua_tostring(processor.L, -1);
		psy_ui_editor_set_text(&self->processview.luaprocessor.console,
			msg);
		psyclescript_dispose(&processor);
		return;
	}
	lua_getglobal(processor.L, "work");
	if (!lua_isnil(processor.L, -1)) { 
		lua_Unsigned len;
		lua_Unsigned i;
		int status = lua_pcall(processor.L, 0, LUA_MULTRET, 0);

		if (status) {
			const char* msg = lua_tostring(processor.L, -1);
			psy_ui_editor_set_text(&self->processview.luaprocessor.console,
				msg);
			psyclescript_dispose(&processor);
			return;
		}
		len = lua_rawlen(processor.L, -1);		
		for (i = 1; i <= len; ++i) {
			lua_Unsigned dst;
			lua_rawgeti(processor.L, -1, i);

			dst = i + framestart - 1;
			if (dst <= frameend && dst < self->sample->numframes) {
				self->sample->channels.samples[channel][dst] =
					(float)luaL_checknumber(processor.L, -1) * 32768.f;
				lua_pop(processor.L, 1);
			} else {
				break;
			}
		}
		psyclescript_dispose(&processor);
		return;
	}
	psyclescript_dispose(&processor); */
}

void sampleeditor_remove_dc(SampleEditor* self, uintptr_t channel,
	uintptr_t framestart, uintptr_t frameend)
{
	uintptr_t c;
	uintptr_t numframes;
	double meanL = 0.0;
	double meanR = 0.0;
	float* wdLeft;
	float* wdRight;
	float buf;

	wdLeft = self->sample->channels.samples[channel];
	wdRight = NULL;

	numframes = frameend - framestart + 1;
	for (c = framestart; c < framestart + numframes; c++) {
		meanL = meanL + ((double)*(wdLeft + c) / numframes);

		//if (wdStereo) meanR = (double)meanR + ((double)*(wdRight + c) /
		//	length);
	}
	for (c = framestart; c < framestart + numframes; c++) {
		buf = *(wdLeft + c);
		if (meanL > 0) {
			if ((double)(buf - meanL) < (-32768)) {
				*(wdLeft + c) = (float) -32768;
			} else {
				*(wdLeft + c) = (float)(buf - meanL);
			}
		} else if (meanL < 0) {
			if ((double)(buf - meanL) > 32767) {
				*(wdLeft + c) = (float)32767;
			} else {
				*(wdLeft + c) = (float)(buf - meanL);
			}
		}
	}
}

void sampleeditor_on_selection_changed(SampleEditor* self, SampleBox* sender,
	WaveBox* wavebox)
{
	if (wavebox_hasselection(wavebox)) {
		sampleeditorbar_setselection(&self->sampleeditortbar,
			wavebox->context.selection.start,
			wavebox->context.selection.end);
	} else {
		sampleeditorbar_clearselection(&self->sampleeditortbar);
	}
	if (psy_ui_checkbox_checked(&self->sampleeditortbar.selecttogether)) {
		psy_TableIterator it;
		for (it = psy_table_begin(&self->samplebox.waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			WaveBox* currwavebox;

			currwavebox = (WaveBox*)psy_tableiterator_value(&it);
			if (currwavebox != wavebox) {
				if (wavebox_hasselection(wavebox)) {
					wavebox_setselection(currwavebox,
						wavebox->context.selection.start,
						wavebox->context.selection.end);
				} else {
					wavebox_clearselection(currwavebox);
				}				
			}
		}
		psy_ui_component_invalidate(&self->samplebox.component);		
	} else {		
		psy_ui_component_invalidate(&sender->component);		
	}	
}

void sampleeditor_showdoublecontloop(SampleEditor* self)
{
	sampleeditor_set_loopview_mode(self, WAVEBOX_LOOPVIEW_CONT_DOUBLE);
}

void sampleeditor_showsinglecontloop(SampleEditor* self)
{
	sampleeditor_set_loopview_mode(self, WAVEBOX_LOOPVIEW_CONT_SINGLE);
}

void sampleeditor_showdoublesustainloop(SampleEditor* self)
{
	sampleeditor_set_loopview_mode(self, WAVEBOX_LOOPVIEW_SUSTAIN_DOUBLE);
}

void sampleeditor_showsinglesustainloop(SampleEditor* self)
{
	sampleeditor_set_loopview_mode(self, WAVEBOX_LOOPVIEW_SUSTAIN_SINGLE);
}

void sampleeditor_set_loopview_mode(SampleEditor* self, WaveBoxLoopViewMode mode)
{
	self->loopviewmode = mode;
	samplebox_setloopviewmode(&self->samplebox, mode);		
	psy_ui_component_invalidate(&self->header.component);
}

void sampleeditor_drawlines(SampleEditor* self)
{
	samplebox_drawlines(&self->samplebox);
}

void sampleeditor_drawbars(SampleEditor* self)
{
	samplebox_drawbars(&self->samplebox);
}

void sampleeditor_setquality(SampleEditor* self,
	psy_dsp_ResamplerQuality quality)
{
	samplebox_setquality(&self->samplebox, quality);
}

void sampleeditor_onscrollzoom_customdraw(SampleEditor* self,
	ScrollZoom* sender, psy_ui_Graphics* g)
{
	if (self->sample) {
		psy_ui_RealRectangle r;
		const psy_ui_TextMetric* tm;
		psy_ui_RealSize size;
		
		size = psy_ui_component_size_px(&sender->component);
		tm = psy_ui_component_textmetric(&sender->component);
		r = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0, 0),
			psy_ui_realsize_make(size.width, size.height));
		psy_ui_graphics_set_colour(g, psy_ui_colour_make(0x00B1C8B0));
		if (!self->sample) {			
			static const char* txt = "No wave loaded";
			
			psy_ui_graphics_set_background_mode(g, psy_ui_TRANSPARENT);
			psy_ui_graphics_set_text_colour(g, psy_ui_colour_make(0x00D1C5B6));
			psy_ui_graphics_textout(g, psy_ui_realpoint_make(
				(size.width - tm->tmAveCharWidth * psy_strlen(txt)) / 2,
				(size.height - tm->tmHeight) / 2),
				txt, psy_strlen(txt));
		} else {
			double x;
			double centery;
			double offsetstep;
			double scaley;

			centery = size.height / 2;
			scaley = size.height / 2 / 32768.0;
			offsetstep = (double)self->sample->numframes / size.width;
			psy_ui_graphics_set_colour(g, psy_ui_colour_make(0x00B1C8B0));
			for (x = 0; x < size.width; ++x) {
				uintptr_t frame;
				double framevalue;
				
				frame = (uintptr_t)(offsetstep * x);
				if (frame >= self->sample->numframes) {
					break;
				}
				framevalue = self->sample->channels.samples[0][frame];							
				psy_ui_drawline(g, psy_ui_realpoint_make(x, centery),
					psy_ui_realpoint_make(x, centery + framevalue * scaley));
			}
		}
	}
}
