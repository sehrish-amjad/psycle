/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinimpfactory.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "../../detail/psyconf.h"

#include "../../uiapp.h"
#include "uiwinapp.h"
#include "uiwinclipboardimp.h"
#include "uiwincomponentimp.h"
#include "uiwinbitmapimp.h"
#include "uiwingraphicsimp.h"
#include "uiwinfontimp.h"
#include "uiwinclipboardimp.h"
#include "uiwincolordialogimp.h"
#include "uiwinopendialogimp.h"
#include "uiwinsavedialogimp.h"
#include "uiwinscreenimp.h"
#include "uiwinfolderdialogimp.h"
#include "uiwinfontdialogimp.h"

#include <stdlib.h>

/* psy_ui_win_ImpFactory */
/* prototypes */
static struct psy_ui_AppImp* allocinit_appimp(psy_ui_win_ImpFactory*, psy_ui_App*, uintptr_t instance);
static struct psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_win_ImpFactory*, psy_ui_RealSize size);
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_win_ImpFactory*, uintptr_t* platformdc);
static struct psy_ui_GraphicsImp* allocinit_graphicsimp_bitmap(psy_ui_win_ImpFactory*, struct psy_ui_Bitmap*);
static struct psy_ui_FontImp* allocinit_fontimp(psy_ui_win_ImpFactory*, const psy_ui_FontInfo*);
static struct psy_ui_ScreenImp* allocinit_screenimp(psy_ui_win_ImpFactory*);
static struct psy_ui_ClipboardImp* allocinit_clipboardimp(psy_ui_win_ImpFactory*);
static struct psy_ui_ComponentImp* allocinit_componentimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_frameimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_toolframeimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_popupimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_listboximp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_listboximp_multiselect(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ColourDialogImp* allocinit_colourdialogimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* parent);
static struct psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_win_ImpFactory*, struct psy_ui_OpenDialog*, struct psy_ui_Component* parent);
static psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_win_ImpFactory*, struct psy_ui_OpenDialog*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
static struct psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
static struct psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir);
static struct psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* parent);
/* vtable */
static psy_ui_ImpFactoryVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_win_ImpFactory* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.allocinit_appimp =
			(psy_ui_fp_impfactory_allocinit_appimp)
			allocinit_appimp;
		vtable.allocinit_bitmapimp =
			(psy_ui_fp_impfactory_allocinit_bitmapimp)
			allocinit_bitmapimp;
		vtable.allocinit_graphicsimp =
			(psy_ui_fp_impfactory_allocinit_graphicsimp)
			allocinit_graphicsimp;
		vtable.allocinit_graphicsimp_bitmap =
			(psy_ui_fp_impfactory_allocinit_graphicsimp_bitmap)
			allocinit_graphicsimp_bitmap;
		vtable.allocinit_fontimp =
			(psy_ui_fp_impfactory_allocinit_fontimp)
			allocinit_fontimp;
		vtable.allocinit_screenimp =
			(psy_ui_fp_impfactory_allocinit_screenimp)
			allocinit_screenimp;
		vtable.allocinit_clipboardimp =
			(psy_ui_fp_impfactory_allocinit_clipboardimp)
			allocinit_clipboardimp;
		vtable.allocinit_componentimp =
			(psy_ui_fp_impfactory_allocinit_componentimp)
			allocinit_componentimp;
		vtable.allocinit_frameimp =
			(psy_ui_fp_impfactory_allocinit_frameimp)
			allocinit_frameimp;
		vtable.allocinit_toolframeimp =
			(psy_ui_fp_impfactory_allocinit_frameimp)
			allocinit_toolframeimp;
		vtable.allocinit_popupimp =
			(psy_ui_fp_impfactory_allocinit_frameimp)
			allocinit_popupimp;		
		vtable.allocinit_colourdialogimp =
			(psy_ui_fp_impfactory_allocinit_colourdialogimp)
			allocinit_colourdialogimp;
		vtable.allocinit_opendialogimp =
			(psy_ui_fp_impfactory_allocinit_opendialogimp)
			allocinit_opendialogimp;
		vtable.allocinit_all_opendialogimp =
			(psy_ui_fp_impfactory_allocinit_all_opendialogimp)
			allocinit_all_opendialogimp;
		vtable.allocinit_savedialogimp =
			(psy_ui_fp_impfactory_allocinit_savedialogimp)
			allocinit_savedialogimp;
		vtable.allocinit_all_savedialogimp =
			(psy_ui_fp_impfactory_allocinit_all_savedialogimp)
			allocinit_all_savedialogimp;
		vtable.allocinit_folderdialogimp =
			(psy_ui_fp_impfactory_allocinit_folderdialogimp)
			allocinit_folderdialogimp;
		vtable.allocinit_all_folderdialogimp =
			(psy_ui_fp_impfactory_allocinit_all_folderdialogimp)
			allocinit_all_folderdialogimp;
		vtable.allocinit_fontdialogimp =
			(psy_ui_fp_impfactory_allocinit_fontdialogimp)
			allocinit_fontdialogimp;
		vtable_initialized = TRUE;
	}
	self->imp.vtable = &vtable;
}
/* implementation */
void psy_ui_win_impfactory_init(psy_ui_win_ImpFactory* self)
{
	psy_ui_impfactory_init(&self->imp);
	vtable_init(self);	
}

psy_ui_win_ImpFactory* psy_ui_win_impfactory_alloc(void)
{
	return (psy_ui_win_ImpFactory*)malloc(sizeof(psy_ui_win_ImpFactory));
}

psy_ui_win_ImpFactory* psy_ui_win_impfactory_alloc_init(void)
{
	psy_ui_win_ImpFactory* rv;

	rv = psy_ui_win_impfactory_alloc();
	if (rv) {
		psy_ui_win_impfactory_init(rv);
	}
	return rv;
}

psy_ui_AppImp* allocinit_appimp(psy_ui_win_ImpFactory* self, psy_ui_App* app, uintptr_t instance)
{
	psy_ui_AppImp* rv;

	rv = (psy_ui_AppImp*)malloc(sizeof(psy_ui_WinApp));
	if (rv) {
		psy_ui_winapp_init((psy_ui_WinApp*)rv, app, (HINSTANCE)instance);
	}
	return rv;
}

psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_win_ImpFactory* self, psy_ui_RealSize size)
{
	psy_ui_BitmapImp* rv;

	rv = (psy_ui_BitmapImp*)malloc(sizeof(psy_ui_win_BitmapImp));
	if (rv) {
		psy_ui_win_bitmapimp_init((psy_ui_win_BitmapImp*)rv, size);
	}
	return rv;
}

psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_win_ImpFactory* self, uintptr_t* platformdc)
{
	psy_ui_GraphicsImp* rv;

	rv = (psy_ui_GraphicsImp*)malloc(sizeof(psy_ui_win_GraphicsImp));
	if (rv) {
		psy_ui_win_graphicsimp_init((psy_ui_win_GraphicsImp*)rv, (HDC)platformdc);
	}
	return rv;
}

psy_ui_GraphicsImp* allocinit_graphicsimp_bitmap(psy_ui_win_ImpFactory* self, struct psy_ui_Bitmap* bitmap)
{
	psy_ui_GraphicsImp* rv;

	rv = (psy_ui_GraphicsImp*)malloc(sizeof(psy_ui_win_GraphicsImp));
	if (rv) {
		psy_ui_win_graphicsimp_init_bitmap((psy_ui_win_GraphicsImp*)rv, bitmap);
	}
	return rv;
}

psy_ui_FontImp* allocinit_fontimp(psy_ui_win_ImpFactory* self, const psy_ui_FontInfo* fontinfo)
{
	psy_ui_FontImp* rv;

	rv = (psy_ui_FontImp*) malloc(sizeof(psy_ui_win_FontImp));
	if (rv) {
		psy_ui_win_fontimp_init((psy_ui_win_FontImp*)rv, fontinfo);
	}
	return rv;
}

psy_ui_ScreenImp* allocinit_screenimp(psy_ui_win_ImpFactory* self)
{
	psy_ui_ScreenImp* rv;

	rv = (psy_ui_ScreenImp*)malloc(sizeof(psy_ui_win_ScreenImp));
	if (rv) {
		psy_ui_win_screenimp_init((psy_ui_win_ScreenImp*)rv);
	}
	return rv;
}

psy_ui_ClipboardImp* allocinit_clipboardimp(psy_ui_win_ImpFactory* self)
{
	psy_ui_ClipboardImp* rv;

	rv = (psy_ui_ClipboardImp*)malloc(sizeof(psy_ui_win_ClipboardImp));
	if (rv) {
		psy_ui_win_clipboardimp_init((psy_ui_win_ClipboardImp*)rv);
	}
	return rv;
}

psy_ui_ComponentImp* allocinit_componentimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	rv = psy_ui_win_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->componentclass,
		0, 0, 90, 90,
		WS_CHILDWINDOW | WS_VISIBLE,
		0);
	if (rv && rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_ComponentImp* allocinit_frameimp(psy_ui_win_ImpFactory* self,
	struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;
	psy_ui_WinApp* winapp;

/*
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
							 WS_CAPTION        | \
							 WS_SYSMENU        | \
							 WS_THICKFRAME     | \
							 WS_MINIMIZEBOX    | \
							 WS_MAXIMIZEBOX)

*/
	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	rv = psy_ui_win_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->appclass,		
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,	
		WS_OVERLAPPEDWINDOW,
		0);	
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_ComponentImp* allocinit_toolframeimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;
	psy_ui_WinApp* winapp;
	
	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	rv = psy_ui_win_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->appclass,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WS_POPUP |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		WS_CAPTION | WS_SYSMENU,
		0);
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}	
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_ComponentImp* allocinit_popupimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;	
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	rv = psy_ui_win_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->appclass,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WS_POPUP,
		0);
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_ColourDialogImp* allocinit_colourdialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* parent)
{	
	psy_ui_win_ColourDialogImp* imp;
	
	imp = (psy_ui_win_ColourDialogImp*)malloc(sizeof(psy_ui_win_ColourDialogImp));
	if (imp) {
		psy_ui_win_colourdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_OpenDialog* dlg, struct psy_ui_Component* parent)
{
	psy_ui_win_OpenDialogImp* imp;
	
	imp = (psy_ui_win_OpenDialogImp*)malloc(sizeof(psy_ui_win_OpenDialogImp));
	if (imp) {
		psy_ui_win_opendialogimp_init(imp, dlg, parent);
		return &imp->imp;
	}
	return 0;
}

psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_OpenDialog* dlg, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_win_OpenDialogImp* imp;
	
	imp = (psy_ui_win_OpenDialogImp*)malloc(sizeof(psy_ui_win_OpenDialogImp));
	if (imp) {
		psy_ui_win_opendialogimp_init_all(imp, dlg, parent,
			title, filter, defaultextension, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_win_SaveDialogImp* imp;	

	imp = (psy_ui_win_SaveDialogImp*)malloc(sizeof(psy_ui_win_SaveDialogImp));
	if (imp) {
		psy_ui_win_savedialogimp_init(imp, parent);
		return &imp->imp;
	}
	return 0;
}

psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_win_SaveDialogImp* imp;	
	
	imp = (psy_ui_win_SaveDialogImp*)malloc(sizeof(psy_ui_win_SaveDialogImp));
	if (imp) {
		psy_ui_win_savedialogimp_init_all(imp, parent,
			title, filter, defaultextension, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_win_FolderDialogImp* imp;	
	
	imp = (psy_ui_win_FolderDialogImp*)malloc(sizeof(psy_ui_win_FolderDialogImp));
	if (imp) {
		psy_ui_win_folderdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir)
{
	psy_ui_win_FolderDialogImp* imp;	
	
	imp = (psy_ui_win_FolderDialogImp*)malloc(sizeof(psy_ui_win_FolderDialogImp));
	if (imp) {
		psy_ui_win_folderdialogimp_init_all(imp, parent,
			title, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_win_FontDialogImp* imp;
	
	imp = (psy_ui_win_FontDialogImp*)malloc(sizeof(psy_ui_win_FontDialogImp));
	if (imp) {
		psy_ui_win_fontdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

#endif /* PSYCLE_TK_WIN32 */
