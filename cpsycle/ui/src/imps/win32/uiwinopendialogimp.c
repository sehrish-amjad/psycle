/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinopendialogimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "../../uiapp.h"
#include <stdlib.h>
#include "../../uiapp.h"
#include <shlobj.h>
#include "uiwincomponentimp.h"
#include <commdlg.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void dev_dispose(psy_ui_win_OpenDialogImp*);
static int dev_execute(psy_ui_win_OpenDialogImp*);
static const psy_Path* dev_path(const psy_ui_win_OpenDialogImp*);
static UINT OfnHookProc(HWND hDlg, UINT uiMsg, WPARAM wParam,
	LPARAM lParam);

/* vtable */
static psy_ui_OpenDialogImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_win_OpenDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_fp_opendialogimp_dev_dispose)
			dev_dispose;
		imp_vtable.dev_execute =
			(psy_ui_fp_opendialogimp_dev_execute)
			dev_execute;
		imp_vtable.dev_path =
			(psy_ui_fp_opendialogimp_dev_path)
			dev_path;
		imp_vtable_initialized = TRUE;
	}
}
/* implementation */
void psy_ui_win_opendialogimp_init(psy_ui_win_OpenDialogImp* self,
	psy_ui_OpenDialog* dlg, psy_ui_Component* parent)
{
	psy_ui_opendialogimp_init(&self->imp);
	imp_vtable_init(self);	
	self->imp.vtable = &imp_vtable;
	self->dlg = dlg;
	self->parent = parent;
	self->title = strdup("");
	self->filter = strdup("");
	self->defaultextension = strdup("");
	self->initialdir = strdup("");	
	psy_path_init(&self->path, NULL);
}

void psy_ui_win_opendialogimp_init_all(psy_ui_win_OpenDialogImp* self,
	psy_ui_OpenDialog* dlg,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_opendialogimp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->dlg = dlg;
	self->parent = parent;
	self->title = strdup(title);
	self->filter = strdup(filter);
	self->defaultextension = strdup(defaultextension);
	self->initialdir = strdup(initialdir);
	psy_path_init(&self->path, NULL);
}

void dev_dispose(psy_ui_win_OpenDialogImp* self)
{
	free(self->title);
	self->title = NULL;
	free(self->filter);
	self->filter = NULL;
	free(self->defaultextension);
	self->defaultextension = NULL;
	free(self->initialdir);
	self->initialdir = NULL;
	psy_path_dispose(&self->path);
}

int dev_execute(psy_ui_win_OpenDialogImp* self)
{
	int rv;
	OPENFILENAME ofn;
	char filename[MAX_PATH];
	char filter[1024];
	char title[MAX_PATH];
	char initialdir[MAX_PATH];
	char defextension[MAX_PATH];
	char* filtertoken;
	HWND hwndOwner;

	*filename = '\0';
	psy_snprintf(title, MAX_PATH, "%s", self->title);
	psy_snprintf(initialdir, MAX_PATH, "%s", self->initialdir);
	psy_snprintf(defextension, MAX_PATH, "%s", self->defaultextension);
	psy_snprintf(filter, sizeof(filter) - 1, "%s", self->filter);
	filter[strlen(filter) + 1] = '\0';
	filtertoken = strtok(filter, "|");
	while (filtertoken != 0) {
		filtertoken = strtok(0, "|");
	}
	if (psy_ui_app_main(psy_ui_app())) {
		hwndOwner = (HWND)psy_ui_component_platform(psy_ui_app_main(psy_ui_app()));
	} else {
		hwndOwner = (HWND)0;
	}
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrFilter = (LPSTR)filter;
	ofn.lpstrCustomFilter = (LPSTR)NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1L;
	ofn.lpstrFile = (LPSTR)filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = (LPSTR)NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrTitle = strlen(title) ? (LPSTR)title : (LPSTR)NULL;
	ofn.lpstrInitialDir = (LPSTR)initialdir;
	if (psy_signal_empty(&self->dlg->signal_filename_changed)) {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	} else {
		ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING;
	}
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = (LPSTR)defextension;
	ofn.lCustData = (uintptr_t)self;
	ofn.lpfnHook = (LPOFNHOOKPROC)OfnHookProc;
	rv = GetOpenFileName(&ofn);	
	if (rv) {
		psy_path_set_path(&self->path, filename);		
	}
	return rv;
}

const psy_Path* dev_path(const psy_ui_win_OpenDialogImp* self)
{
	return &self->path;
}


UINT OfnHookProc(HWND hDlg, UINT uiMsg, WPARAM wParam,
	LPARAM lParam)
{

	if (uiMsg == WM_INITDIALOG) {

	} else
	if (uiMsg == WM_NOTIFY) {
		OFNOTIFYW* notify = (OFNOTIFYW*)lParam;

		if (notify->hdr.code == CDN_SELCHANGE) {
			/* your code here */		
			psy_ui_win_OpenDialogImp* imp;
			char fname[MAX_PATH];			
						
			imp = (psy_ui_win_OpenDialogImp*)notify->lpOFN->lCustData;
			if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
				fname, sizeof(fname)) <= sizeof(fname)) {
				psy_path_set_path(&imp->path, fname);
			}
			psy_signal_emit(&imp->dlg->signal_filename_changed, imp->dlg, 0);
			printf("pressed");
		} if (notify->hdr.code == CDN_INITDONE) {			
			RECT rc;
			POINT pt;
			int cw, ch;
			int dlg_w, dlg_h;

			cw = GetSystemMetrics(SM_CXSCREEN);
			ch = GetSystemMetrics(SM_CYSCREEN);			
			GetWindowRect(GetParent(hDlg), &rc);
			pt.x = rc.left;
			pt.y = rc.top;
			dlg_w = rc.right - rc.left;
			dlg_h = rc.bottom - rc.top;
			ScreenToClient(GetParent(GetParent(hDlg)), &pt);
			GetClientRect(GetParent(hDlg), &rc);			
			SetWindowPos(GetParent(hDlg), HWND_TOPMOST,
				(cw - dlg_w) / 2, (ch - dlg_w) / 2, /* topleft */
				dlg_w, dlg_h, /* size */
				SWP_NOZORDER | SWP_NOSIZE);			
		}
	}	
	return 0;
}

#endif /* PSYCLE_TK_WIN32 */
