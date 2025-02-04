/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_APP_H
#define psy_ui_APP_H

/* local */
#include "timers.h"
#include "uibitmaps.h"
#include "uiclipboard.h"
#include "uieventdispatch.h"
#include "uievents.h"
#include "uidefaults.h"
#include "uiicons.h"
/* container */
#include <signal.h>
/* file */
#include <translator.h>

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_AppImp;
struct psy_ui_Component;
struct psy_ui_ImpFactory;

/* psy_ui_AppZoom */
typedef struct psy_ui_AppZoom {
	/* signals */
	psy_Signal signal_zoom;
	/* internal data */
	double rate;
	double basefontsize;	
} psy_ui_AppZoom;

void psy_ui_appzoom_init(psy_ui_AppZoom*);
void psy_ui_appzoom_dispose(psy_ui_AppZoom*);

void psy_ui_appzoom_set_rate(psy_ui_AppZoom*, double rate);
double psy_ui_appzoom_rate(const psy_ui_AppZoom*);

void psy_ui_appzoom_update_base_font_size(psy_ui_AppZoom*,
	const psy_ui_Font*);

struct psy_ui_Component;
struct psy_ui_ComponentImp;

/* psy_ui_App */
typedef struct psy_ui_App {
	/* signals */
	psy_Signal signal_dispose;
	psy_Signal signal_mousehook;
	/* internal data */
	struct psy_ui_AppImp* imp;
	struct psy_ui_ImpFactory* impfactory;
	psy_ui_AppZoom zoom;
	psy_ui_Defaults defaults;
	psy_Translator translator;	
	bool alignvalid;
	bool mousetracking;
	bool setpositioncacheonly;	
	psy_Timers wintimers;
	/* references */
	struct psy_ui_Component* main;	
	struct psy_ui_Component* hover;
	struct psy_ui_Component* focus;
	psy_ui_DragEvent dragevent;	
	psy_ui_EventDispatch eventdispatch;
	psy_Table components;
	psy_ui_Bitmaps bitmaps;
	psy_ui_Icons icons;
	psy_List* captures_;
	psy_ui_Clipboard clipboard;	
} psy_ui_App;

psy_ui_App* psy_ui_app(void);

void psy_ui_app_init(psy_ui_App*, bool dark, uintptr_t instance);
void psy_ui_app_dispose(psy_ui_App*);

void psy_ui_app_set_main(psy_ui_App*, struct psy_ui_Component* main);
struct psy_ui_Component* psy_ui_app_main(psy_ui_App*);
int psy_ui_app_run(psy_ui_App*);
int psy_ui_app_wait(psy_ui_App*);
int psy_ui_app_continue(psy_ui_App*);
void psy_ui_app_stop(psy_ui_App*);
void psy_ui_app_close(psy_ui_App*);
void psy_ui_app_start_mouse_hook(psy_ui_App*);
void psy_ui_app_stop_mouse_hook(psy_ui_App*);

void psy_ui_app_set_zoom_rate(psy_ui_App*, double rate);
double psy_ui_app_zoom_rate(const psy_ui_App*);
psy_ui_AppZoom* psy_ui_app_zoom(psy_ui_App*);
void psy_ui_app_set_default_font(psy_ui_App*, const psy_ui_Font*);
void psy_ui_app_light_theme(psy_ui_App*);
void psy_ui_app_dark_theme(psy_ui_App*);
bool psy_ui_app_has_dark_theme(const psy_ui_App*);
psy_ui_Style* psy_ui_app_style(psy_ui_App*, uintptr_t styletype);
const psy_ui_Style* psy_ui_app_style_const(const psy_ui_App*, uintptr_t styletype);
void psy_ui_app_set_hover(psy_ui_App*, struct psy_ui_Component* hover);
void psy_ui_app_start_drag(psy_ui_App*);
void psy_ui_app_stop_drag(psy_ui_App*);
void psy_ui_app_start_timer(psy_ui_App*, struct psy_ui_Component*, uintptr_t id,
	uintptr_t interval);
void psy_ui_app_stop_timer(psy_ui_App*, struct psy_ui_Component*, uintptr_t id);
void psy_ui_app_register_native(psy_ui_App*,
	uintptr_t handle, struct psy_ui_ComponentImp*, bool top_level);
void psy_ui_app_unregister_native(psy_ui_App*,
	uintptr_t handle);
void psy_ui_app_set_bmp_path(psy_ui_App*, const char* path);
void psy_ui_app_add_app_bmp(psy_ui_App*, uintptr_t id,
	const char* filename);
void psy_ui_app_notify_theme_change(psy_ui_App*);

INLINE struct psy_ui_Component* psy_ui_app_capture(psy_ui_App* self)
{	
	psy_List* p;
	
	p = psy_list_last(self->captures_);
	if (p) {
		return (struct psy_ui_Component*)p->entry;
	}
	return NULL;	
}

void psy_ui_app_push_capture(psy_ui_App*, struct psy_ui_Component*);
void psy_ui_app_pop_capture(psy_ui_App*);
void psy_ui_app_remove_capture(psy_ui_App*, struct psy_ui_Component*);

INLINE struct psy_ui_Component* psy_ui_app_hover(psy_ui_App* self)
{
	return self->hover;
}

INLINE struct psy_ui_ImpFactory* psy_ui_app_impfactory(psy_ui_App* self)
{
	assert(self);

	return self->impfactory;
}

INLINE struct psy_ui_Component* psy_ui_app_focus(psy_ui_App* self)
{
	return self->focus;
}

void psy_ui_app_set_focus(psy_ui_App*, struct psy_ui_Component*);

psy_ui_Style* psy_ui_style(uintptr_t styletype);
const psy_ui_Style* psy_ui_style_const(uintptr_t styletype);
psy_List* psy_ui_app_top_level(psy_ui_App* self);
const psy_List* psy_ui_app_fonts(const psy_ui_App*);

/* psy_ui_AppImp */
typedef void (*psy_ui_fp_appimp_dispose)(struct psy_ui_AppImp*);
typedef int (*psy_ui_fp_appimp_run)(struct psy_ui_AppImp*);
typedef int (*psy_ui_fp_appimp_wait)(struct psy_ui_AppImp*);
typedef int (*psy_ui_fp_appimp_continue)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_stop)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_close)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_sendevent)(struct psy_ui_AppImp*,
	struct psy_ui_Component*, psy_ui_Event*);
typedef struct psy_ui_Component* (*psy_ui_fp_appimp_component)(struct psy_ui_AppImp*,
	uintptr_t platformhandle);
typedef psy_List* (*psy_ui_fp_appimp_toplevel)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_onappdefaultschange)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_startmousehook)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_stopmousehook)(struct psy_ui_AppImp*);
typedef void (*psy_ui_fp_appimp_register_native)(struct psy_ui_AppImp*, uintptr_t handle, struct psy_ui_ComponentImp*, bool);
typedef void (*psy_ui_fp_appimp_unregister_native)(struct psy_ui_AppImp*, uintptr_t handle);
typedef const psy_List* (*psy_ui_fp_appimp_fonts)(const struct psy_ui_AppImp*);

typedef struct psy_ui_AppImpVTable {
	psy_ui_fp_appimp_dispose dev_dispose;
	psy_ui_fp_appimp_run dev_run;
	psy_ui_fp_appimp_wait dev_wait;
	psy_ui_fp_appimp_continue dev_continue;
	psy_ui_fp_appimp_stop dev_stop;
	psy_ui_fp_appimp_close dev_close;	
	psy_ui_fp_appimp_onappdefaultschange dev_onappdefaultschange;
	psy_ui_fp_appimp_startmousehook dev_startmousehook;
	psy_ui_fp_appimp_stopmousehook dev_stopmousehook;
	psy_ui_fp_appimp_sendevent dev_sendevent;
	psy_ui_fp_appimp_component dev_component;
	psy_ui_fp_appimp_toplevel dev_toplevel;
	psy_ui_fp_appimp_register_native dev_register_native;
	psy_ui_fp_appimp_unregister_native dev_unregister_native;
	psy_ui_fp_appimp_fonts dev_fonts;
} psy_ui_AppImpVTable;

typedef struct psy_ui_AppImp {
	psy_ui_AppImpVTable* vtable;
} psy_ui_AppImp;

void psy_ui_appimp_init(psy_ui_AppImp*);

const struct psy_ui_Defaults* psy_ui_defaults(void);
struct psy_ui_Defaults* psy_ui_app_defaults(void);
const struct psy_ui_Defaults* psy_ui_app_defaults_const(void);

int psy_ui_log_pixel_sx(void);
int psy_ui_log_pixel_sy(void);



#ifdef __cplusplus
}
#endif

#endif /* psy_ui_APP_H */
