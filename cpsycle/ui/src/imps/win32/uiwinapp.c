/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiwinapp.h"
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
/* local */
#include "../../uicomponent.h"
/* windows */
#include "uiwincomponentimp.h"
#include <excpt.h>
/* common control header */
#include <commctrl.h>
/* platform */
#include "../../detail/trace.h"

static psy_ui_WinApp* winapp = NULL;

/* prototypes */
static psy_ui_KeyboardEvent keyboardevent(psy_ui_EventType, WPARAM, LPARAM);
static psy_ui_MouseEvent mouseevent(int msg, WPARAM, LPARAM);
static bool handle_ctlcolor(psy_ui_WinApp*, int msg, HWND, WPARAM, LPARAM,
	LRESULT* rv);
static void psy_ui_winapp_onappdefaultschange(psy_ui_WinApp*);
static psy_ui_EventType translate_win_event_type(int message);
static int translate_win_button(int message);
static void CALLBACK psy_ui_winapp_timerproc(HWND, UINT, UINT_PTR, DWORD);

static LPARAM psy_ui_winapp_pack_pt(psy_ui_RealPoint pt)
{
	return (int16_t)pt.x | ((int16_t)pt.y << 16);
}

LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void psy_ui_winapp_registerclasses(psy_ui_WinApp*);

static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component*
	self)
{
	return (psy_ui_win_ComponentImp*)self->imp->vtable->dev_platform(self->imp);
}

static void psy_ui_winapp_handle_destroy_window(psy_ui_WinApp*,
	psy_ui_Component*);

/* virtual */
static void psy_ui_winapp_dispose(psy_ui_WinApp*);
static int psy_ui_winapp_run(psy_ui_WinApp*);
static int psy_ui_winapp_wait(psy_ui_WinApp*);
static int psy_ui_winapp_continue(psy_ui_WinApp*);
static void psy_ui_winapp_stop(psy_ui_WinApp*);
static void psy_ui_winapp_close(psy_ui_WinApp*);
static void psy_ui_winapp_startmousehook(psy_ui_WinApp*);
static void psy_ui_winapp_stopmousehook(psy_ui_WinApp*);
static void psy_ui_winapp_sendevent(psy_ui_WinApp*, psy_ui_Component*,
	psy_ui_Event*);
static psy_ui_Component* psy_ui_winapp_component(psy_ui_WinApp*,
	uintptr_t handle);
psy_ui_win_ComponentImp* psy_ui_winapp_componentimp(psy_ui_WinApp*,
	uintptr_t handle);
static psy_List* psy_ui_winapp_toplevel(psy_ui_WinApp*);
static void psy_ui_winimp_register_native(psy_ui_WinApp*,
	uintptr_t handle, psy_ui_ComponentImp*, bool top_level);
static void psy_ui_winimp_unregister_native(psy_ui_WinApp*,
	uintptr_t handle);
static const psy_List* psy_ui_winimp_fonts(const psy_ui_WinApp*);
static void psy_ui_winimp_read_fonts(psy_ui_WinApp*);

/* vtable */
static psy_ui_AppImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_WinApp* self)
{
	assert(self);

	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_fp_appimp_dispose)
			psy_ui_winapp_dispose;
		imp_vtable.dev_run =
			(psy_ui_fp_appimp_run)
			psy_ui_winapp_run;
		imp_vtable.dev_wait =
			(psy_ui_fp_appimp_wait)
			psy_ui_winapp_wait;
		imp_vtable.dev_continue =
			(psy_ui_fp_appimp_continue)
			psy_ui_winapp_continue;
		imp_vtable.dev_stop =
			(psy_ui_fp_appimp_stop)
			psy_ui_winapp_stop;
		imp_vtable.dev_close =
			(psy_ui_fp_appimp_close)
			psy_ui_winapp_close;
		imp_vtable.dev_onappdefaultschange =
			(psy_ui_fp_appimp_onappdefaultschange)
			psy_ui_winapp_onappdefaultschange;		
		imp_vtable.dev_startmousehook =
			(psy_ui_fp_appimp_startmousehook)
			psy_ui_winapp_startmousehook;
		imp_vtable.dev_stopmousehook =
			(psy_ui_fp_appimp_stopmousehook)
			psy_ui_winapp_stopmousehook;
		imp_vtable.dev_sendevent =
			(psy_ui_fp_appimp_sendevent)
			psy_ui_winapp_sendevent;
		imp_vtable.dev_component =
			(psy_ui_fp_appimp_component)
			psy_ui_winapp_component;		
		imp_vtable.dev_toplevel =
			(psy_ui_fp_appimp_toplevel)
			psy_ui_winapp_toplevel;
		imp_vtable.dev_register_native =
			(psy_ui_fp_appimp_register_native)
			psy_ui_winimp_register_native;
		imp_vtable.dev_unregister_native =
			(psy_ui_fp_appimp_unregister_native)
			psy_ui_winimp_unregister_native;
		imp_vtable.dev_fonts = 
			(psy_ui_fp_appimp_fonts)
			psy_ui_winimp_fonts;
		imp_vtable_initialized = TRUE;
	}
	self->imp.vtable = &imp_vtable;
}

void psy_ui_winapp_init(psy_ui_WinApp* self, psy_ui_App* app, HINSTANCE instance)
{
	static TCHAR szAppClass[] = TEXT("PsycleApp");	
	static TCHAR szComponentClass[] = TEXT("PsycleComponent");	
	HRESULT hr;

	assert(self);

	psy_ui_appimp_init(&self->imp);
	imp_vtable_init(self);	
	winapp = self; /* init static winapp reference */
	self->app = app;
	self->instance = instance;
	self->appclass = szAppClass;
	self->componentclass = szComponentClass;
	self->winproc = ui_winproc;
	self->comwinproc = ui_com_winproc;
	self->winid = 20000;
	self->mousehook = 0;
	psy_ui_winapp_registerclasses(self);
	hr = CoInitialize(NULL);	
	if (hr == S_FALSE) {
		psy_ui_error(
			"The COM library is already initialized on this thread. ",
			"Warning! psy_ui_winapp_init: CoInitialize already initialized");
	} else if (hr == RPC_E_CHANGED_MODE) {
		psy_ui_error(
			"A previous call to CoInitializeEx specified the concurrency model "
			"for this thread as multithread apartment (MTA). This could also "
			"indicate that a change from neutral-threaded apartment to "
			"single-threaded apartment has occurred. ",
			"Warning! psy_ui_winapp_init: CoInitialize RPC_E_CHANGED_MODE");
	}
	psy_table_init(&self->selfmap);
	psy_table_init(&self->winidmap);
	psy_table_init(&self->toplevelmap);
	self->defaultbackgroundbrush = CreateSolidBrush(0x00232323);
	self->fonts = NULL;
	psy_ui_winimp_read_fonts(self);
}

void psy_ui_winapp_dispose(psy_ui_WinApp* self)
{
	psy_ui_winapp_stopmousehook(self);
	psy_table_dispose(&self->selfmap);
	psy_table_dispose(&self->winidmap);
	psy_table_dispose(&self->toplevelmap);
	DeleteObject(self->defaultbackgroundbrush);
	psy_list_deallocate(&self->fonts, NULL);
	CoUninitialize();	
}

void psy_ui_winapp_registerclasses(psy_ui_WinApp* self)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	int succ;
		
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wndclass.lpfnWndProc   = self->winproc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = (HINSTANCE) self->instance;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH)GetStockObject (NULL_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = self->appclass;
	if (!RegisterClass (&wndclass))
    {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      self->appclass, MB_ICONERROR) ;		
    }
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = self->winproc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (long); 
	wndclass.hInstance     = self->instance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = self->componentclass;
     
	RegisterClass (&wndclass) ;	
	/* Ensure that the common control DLL is loaded. */
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);	
}

LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	psy_ui_win_ComponentImp* imp;
	psy_ui_Component* component;
	psy_ui_fp_winproc winproc;
	bool preventdefault;

	preventdefault = 0;	
	imp = psy_ui_winapp_componentimp(winapp, (uintptr_t)hwnd);		
	if (imp) {		
		winproc = imp->wndproc;
		component = imp->component;
		switch (message) {		
		case WM_NCDESTROY:
			/* restore default winproc */			
#if defined(_WIN64)		
			SetWindowLongPtr(imp->hwnd, GWLP_WNDPROC, (LONG_PTR)
				imp->wndproc);
#else	
			SetWindowLong(imp->hwnd, GWL_WNDPROC, (LONG)imp->wndproc);
#endif				
			psy_ui_winapp_handle_destroy_window(winapp, component);
			break;
		case WM_DESTROY:
			/* deallocation handled in WM_NCDESTROY */			
			break;		
		case WM_CHAR:
			if (imp->preventwmchar) {
				imp->preventwmchar = 0;
				preventdefault = 1;
			}
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (imp->component) {
				psy_ui_KeyboardEvent keyevent;

				keyevent = keyboardevent(translate_win_event_type(message),
					wParam, lParam);
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					imp->component, psy_ui_keyboardevent_base(&keyevent));
				if (psy_ui_event_default_prevented(&keyevent.event)) {
					imp->preventwmchar = 1;
				}
				preventdefault = psy_ui_event_default_prevented(&keyevent.event);
			}
			break;
		case WM_SETFOCUS: {
			psy_ui_Event ev;

			psy_ui_event_init(&ev, psy_ui_FOCUS);
			psy_ui_event_stop_propagation(&ev);
			psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
				component, &ev);
			psy_ui_event_init(&ev, psy_ui_FOCUSIN);			
			psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
				component, &ev);
			break; }
		case WM_KILLFOCUS:
			if (component) {
				psy_ui_Event ev;

				psy_ui_event_init(&ev, psy_ui_FOCUSOUT);
				psy_ui_event_stop_propagation(&ev);				
				psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
					component, &ev);
			}
			break;	
		case WM_NCPAINT:
			if (component && component->ncpaint) {					
#ifndef DCX_USESTYLE
#define DCX_USESTYLE 0x00010000
#endif
				psy_ui_Colour bgcolour;

				bgcolour = psy_ui_component_background_colour(imp->component);
				if (!bgcolour.mode.transparent) {
					HDC hdc = GetDCEx(hwnd, 0, DCX_WINDOW | DCX_USESTYLE);
					if (hdc) {
						RECT rcclient;
						RECT rcwin;
						POINT ptupleft;
						HRGN rgntemp;
						HBRUSH hbrush;

						GetClientRect(hwnd, &rcclient);							
						GetWindowRect(hwnd, &rcwin);							
						ptupleft.x = rcwin.left;
						ptupleft.y = rcwin.top;
						MapWindowPoints(0, hwnd, (LPPOINT)&rcwin, (sizeof(RECT) / sizeof(POINT)));
						OffsetRect(&rcclient, -rcwin.left, -rcwin.top);
						OffsetRect(&rcwin, -rcwin.left, -rcwin.top);
						rgntemp = NULL;
						if (wParam == NULLREGION || wParam == ERROR) {
							ExcludeClipRect(hdc, rcclient.left, rcclient.top, rcclient.right, rcclient.bottom);
						} else {
							rgntemp = CreateRectRgn(rcclient.left + ptupleft.x, rcclient.top + ptupleft.y,
								rcclient.right + ptupleft.x, rcclient.bottom + ptupleft.y);
							if (CombineRgn(rgntemp, (HRGN)wParam, rgntemp, RGN_DIFF) == NULLREGION) {
								/* nothing to paint */
							}
							OffsetRgn(rgntemp, -ptupleft.x, -ptupleft.y);
							ExtSelectClipRgn(hdc, rgntemp, RGN_AND);
						}
						hbrush = CreateSolidBrush(RGB(bgcolour.r, bgcolour.g, bgcolour.b));
						FillRect(hdc, &rcwin, hbrush);
						DeleteObject(hbrush);
						ReleaseDC(hwnd, hdc);
						if (rgntemp != 0) {
							DeleteObject(rgntemp);
						}
					}
					return 0;
				}
			}
			break;
		case WM_NCCALCSIZE:
			if (component && component->ncpaint && wParam != 0) {
				NCCALCSIZE_PARAMS* nc;					

				nc = (NCCALCSIZE_PARAMS*)lParam;					
				if (nc) {	
					const psy_ui_TextMetric* tm;
					RECT* rc;
					int dy;

					tm = psy_ui_component_textmetric(component);
					rc = &nc->rgrc[0];
					dy = -((((nc->rgrc[0].bottom - nc->rgrc[0].top) -
						(tm->tmHeight - 1)) / 2));
					rc->top -= dy + 1;
					rc->bottom += dy;
				}
				return 0;
			}
			break;				
		default:
			break;
		}
		if (preventdefault) {
			return 0;
		}
		return CallWindowProc(winproc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{    
	psy_ui_WinApp* self;
	psy_ui_win_ComponentImp* imp;
	psy_ui_Component* component;	
	LRESULT lres;

	self = winapp;
	if (handle_ctlcolor(self, message, hwnd, wParam, lParam, &lres)) {
		return lres;
	}	
	imp = psy_ui_winapp_componentimp(self, (uintptr_t)hwnd);	
	if (imp) {
		component = imp->component;
		switch (message) {
		case WM_SHOWWINDOW:
			if (component) {
				if (wParam != FALSE) {					
					psy_signal_emit(&component->signal_show, component, 0);
				} else {
					psy_signal_emit(&component->signal_hide, component, 0);
				}
			}
			return 0;
		case WM_SIZE:
			if (component) {
				psy_ui_Event ev;

				imp->sizecachevalid = FALSE;
				psy_ui_event_init(&ev, psy_ui_RESIZE);
				psy_ui_event_stop_propagation(&ev);				
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					component, &ev);
			}
			return 0;		
		case WM_ERASEBKGND:
			return 1;
		case WM_SETTINGCHANGE: {
			int scrolllines;

			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
				&scrolllines, 0);
			/*
			** scrolllines usually equals 3 or 0 (for no scrolling)
			** WHEEL_DELTA equals 120, so delta_per_line_ will be 40
			*/
			psy_ui_eventdispatch_set_wheel_delta_per_line(
				&self->app->eventdispatch, (scrolllines)
				? WHEEL_DELTA / scrolllines
				: 0);
			return 0; }
		case WM_COMMAND:
			imp = psy_table_at(&self->winidmap, (uintptr_t)LOWORD(wParam));
			if (imp && imp->component &&
				imp->component->signal_command.slots) {
				psy_signal_emit(&imp->component->signal_command,
					imp->component, 2, wParam, lParam);				
			} else if (imp && imp->imp.signal_command.slots) {
				psy_signal_emit(&imp->imp.signal_command, imp->component, 2,
					wParam, lParam);				
			}
			return 0;			
		case WM_CREATE:
			psy_signal_emit(&imp->component->signal_create, component, 0);
			return 0;			
		case WM_PAINT: {			
			HDC hdc;
			PAINTSTRUCT ps;
			psy_ui_RealSize clipsize;
			HRGN hrgn;
			DWORD rectcount;
			DWORD r;
			const RECT* pRect;
			RGNDATA* pRegion;

			hrgn = CreateRectRgn(0, 0, 0, 0);
			rectcount = 0;
			pRegion = 0;
			if (GetUpdateRgn(hwnd, hrgn, FALSE) != NULLREGION) {
				DWORD size;

				size = GetRegionData(
					hrgn, /* handle to region */
					0,    /* size of region data buffering a region. */
					NULL  /* region data buffer */
				);
				if (size) {
					pRegion = (RGNDATA*)malloc(size);
					GetRegionData(hrgn, size, pRegion);
					pRect = (const RECT*)&pRegion->Buffer;						
					rectcount = pRegion->rdh.nCount;						
				}
			}
			hdc = BeginPaint(hwnd, &ps);				
			for (r = 0; r < rectcount; ++r) {
				RECT rcPaint;
				rcPaint = pRect[r];
				/* store clip / repaint size of paint request */
				clipsize = psy_ui_realsize_make(
					(double)rcPaint.right - (double)rcPaint.left,
					(double)rcPaint.bottom - (double)rcPaint.top);
				/* anything to paint ? */
				if (!psy_ui_realsize_equals(&clipsize, psy_ui_realsize_zero())) {
					psy_ui_Graphics	g;
					psy_ui_Bitmap dblbuffer;

					if (psy_ui_component_doublebuffered(component)) {
						/*
						** create a graphics context with back buffer bitmap
						** with origin (0; 0) and size of the paint request
						*/
						psy_ui_bitmap_init_size(&dblbuffer, clipsize);
						psy_ui_graphics_init_bitmap(&g, &dblbuffer);
						/* translate paint request to buffer 0, 0 origin */
						psy_ui_setorigin(&g, psy_ui_realpoint_make(
							rcPaint.left, rcPaint.top));
					} else {
						/* create graphics handle with the paint hdc */
						psy_ui_graphics_init(&g, hdc);
					}						
					/* set clip */						
					psy_ui_graphics_set_clip_rect(&g,
						psy_ui_realrectangle_make(
							psy_ui_realpoint_make(rcPaint.left,
								rcPaint.top),
							clipsize));
					/* draw */					
					psy_ui_component_draw(component, &g);						
					if (psy_ui_component_doublebuffered(component)) {
						/* copy the double buffer bitmap to the paint hdc */
						BitBlt(hdc, rcPaint.left, rcPaint.top,
							(int)clipsize.width, (int)clipsize.height,
							(HDC)psy_ui_graphics_dev_gc(&g), 0, 0, SRCCOPY);
						psy_ui_bitmap_dispose(&dblbuffer);
					}
					psy_ui_graphics_dispose(&g);
				}					
			}				
			free(pRegion);
			pRegion = NULL;
			if (hrgn) {
				DeleteObject((HGDIOBJ)hrgn);
			}
			EndPaint(hwnd, &ps);				
			return 0;			
			break; }
		case WM_NCDESTROY: {
			psy_ui_winapp_handle_destroy_window(winapp, component);
			break; }
		case WM_DESTROY:
			if (component) {
				/* deallocation handled in WM_NCDESTROY */			
				return 0;
			}
			break;
		case WM_CLOSE: {
			if (component) {
				bool close;

				close = component->vtable->onclose(component);
				psy_signal_emit(&component->signal_close,
					component, 1, (void*)&close);
				if (!close) {
					return 0;
				}
			}
			break; }
		case WM_SYSKEYDOWN:
			if (!(component && (wParam >= VK_F10 && wParam <= VK_F12 ||
					wParam >= 0x41 && wParam <= psy_ui_KEY_Z ||
					wParam >= psy_ui_KEY_DIGIT0 &&
					wParam <= psy_ui_KEY_DIGIT9))) {
				return 0;
			}
			/* fallthrough */
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (component) {
				psy_ui_KeyboardEvent keyevent;

				keyevent = keyboardevent(translate_win_event_type(message),
					wParam, lParam);
				if (psy_ui_app_focus(psy_ui_app())) {
					component = psy_ui_app_focus(psy_ui_app());
				}
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					component, psy_ui_keyboardevent_base(&keyevent));
				if (psy_ui_event_default_prevented(&keyevent.event)) {
					imp->preventwmchar = 1;
				}		
				return 0;
			}
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONDOWN: 
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:	
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEMOVE:			
			if (component) {
				psy_ui_MouseEvent ev;
				
				ev = mouseevent(message, wParam, lParam);				
				if (message == WM_MOUSEMOVE && !self->app->mousetracking) {
					TRACKMOUSEEVENT tme;

					// component->imp->vtable->dev_mouseenter(component->imp);
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 200;
					tme.hwndTrack = hwnd;
					if (_TrackMouseEvent(&tme)) {
						psy_ui_app()->mousetracking = TRUE;
					}
				}				
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					component, psy_ui_mouseevent_base(&ev));
				return 0;
			}
			break;		
		case WM_MOUSEWHEEL:	{
			if (component) {
				psy_ui_MouseEvent ev;
				POINT pt_client;

				pt_client.x = (SHORT)LOWORD(lParam);
				pt_client.y = (SHORT)HIWORD(lParam);
				ScreenToClient(imp->hwnd, &pt_client);
				psy_ui_mouseevent_init_all(&ev,
					psy_ui_realpoint_make(pt_client.x, pt_client.y),
					(short)LOWORD(wParam), (short)HIWORD(wParam),
					GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);				
				psy_ui_mouseevent_settype(&ev, psy_ui_WHEEL);
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					component, psy_ui_mouseevent_base(&ev));				
			}
			break; }
		case WM_MOUSEHOVER:
			if (component) {
				psy_signal_emit(&component->signal_mousehover, component, 0);
				return 0;
			}
			break;
		case WM_MOUSELEAVE:
			if (component) {
				psy_ui_Event ev;

				psy_ui_event_init(&ev, psy_ui_MOUSELEAVE);
				psy_ui_event_stop_propagation(&ev);
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					component, &ev);				
			}			
			return 0;	
		case WM_SETFOCUS: {
			psy_ui_Event ev;
		
			psy_ui_event_init(&ev, psy_ui_FOCUS);
			psy_ui_event_stop_propagation(&ev);
			psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
				component, &ev);
			psy_ui_event_init(&ev, psy_ui_FOCUSIN);
			psy_ui_eventdispatch_send(&winapp->app->eventdispatch,
				component, &ev);			
			break; }
		case WM_KILLFOCUS:
			if (component) {
				psy_ui_Event ev;
				
				psy_ui_event_init(&ev, psy_ui_FOCUSOUT);
				psy_ui_event_stop_propagation(&ev);
				psy_ui_eventdispatch_send(&self->app->eventdispatch,
					component, &ev);
				return 0;
			}							
			break;
		case WM_NCACTIVATE:
			if (component && component->dropdown && (wParam == 0)) {
				EnableWindow(GetParent(imp->hwnd), TRUE);						
				psy_ui_component_hide(component);				
			}
			break;
		default:			
			break;
		}	
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

psy_ui_EventType translate_win_event_type(int message)
{	
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		return psy_ui_MOUSEDOWN;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		return psy_ui_MOUSEUP;
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
		return psy_ui_DBLCLICK;
	case WM_MOUSEMOVE:
		return psy_ui_MOUSEMOVE;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return psy_ui_KEYDOWN;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		return psy_ui_KEYUP;
	default:
		return psy_ui_UNKNOWNEVENT;
	}
}

bool handle_ctlcolor(psy_ui_WinApp* self, int msg, HWND hwnd, WPARAM wparam,
	LPARAM lparam, LRESULT* rv)
{
	psy_ui_win_ComponentImp* imp;	

	*rv = 0;
	switch (msg) {
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT: {
		uint32_t colorref;
		uint32_t bgcolorref;
		HBRUSH brush;

		imp = psy_ui_winapp_componentimp(self, (uintptr_t)lparam);
		if (!imp) {
			imp = psy_ui_winapp_componentimp(self, (uintptr_t)hwnd);
		}
		if (imp && imp->component) {
			psy_ui_Colour colour;

			colour = psy_ui_component_colour(imp->component);
			colorref = psy_ui_colour_colorref(&colour);
			colour = psy_ui_component_background_colour(imp->component);
			bgcolorref = psy_ui_colour_colorref(&colour);
			if (!colour.mode.transparent) {
				DeleteObject(psy_ui_win_component_details(imp->component)->background);
				psy_ui_win_component_details(imp->component)->background =
					CreateSolidBrush(RGB(colour.r, colour.g, colour.b));
			} else {
				brush = psy_ui_win_component_details(imp->component)->background;
			}			
		} else {
			colorref = psy_ui_colour_colorref(
				&psy_ui_style_const(psy_ui_STYLE_ROOT)->colour);
			bgcolorref = psy_ui_colour_colorref(
				&psy_ui_style_const(psy_ui_STYLE_ROOT)->background.colour);
			brush = winapp->defaultbackgroundbrush;
		}
		SetTextColor((HDC)wparam, colorref);
		SetBkColor((HDC)wparam, bgcolorref);
		*rv = (LRESULT)brush;
		return TRUE;
		break; }
	default:
		break;
	}
	return FALSE;
}

void psy_ui_winapp_handle_destroy_window(psy_ui_WinApp* self,
	psy_ui_Component* component)
{
	assert(self);

	if (component) {
		bool deallocate;		
			
		deallocate = component->deallocate;
		psy_ui_component_dispose(component);		
		if (deallocate) {
			free(component);
			component = NULL;
		}
	}
}

int translate_win_button(int message)
{
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		return 1;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		return 2;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		return 3;
	default:
		return 1;
	}
}

psy_ui_KeyboardEvent keyboardevent(psy_ui_EventType type, WPARAM wparam, LPARAM lparam)
{
	psy_ui_KeyboardEvent rv;

	psy_ui_keyboardevent_init_all(&rv, (int)wparam, lparam,
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
		GetKeyState(VK_MENU) < 0, (lparam & 0x40000000) == 0x40000000);
	psy_ui_keyboardevent_settype(&rv, type);
	return rv;
}

psy_ui_MouseEvent mouseevent(int msg, WPARAM wparam, LPARAM lparam)
{
	psy_ui_MouseEvent rv;
	uintptr_t button;

	if (msg == WM_MOUSEMOVE) {
		if (wparam & MK_LBUTTON) {
			button = 1;
		} else if (wparam & MK_RBUTTON) {
			button = 2;
		} else if (wparam & MK_MBUTTON) {
			button = 3;
		} else {
			button = 0;
		}
	} else {
		button = translate_win_button(msg);
	}
	psy_ui_mouseevent_init_all(&rv,
		psy_ui_realpoint_make((SHORT)LOWORD(lparam), (SHORT)HIWORD(lparam)),	
		button, 0,
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);	
	psy_ui_mouseevent_settype(&rv, translate_win_event_type(msg));
	return rv;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0 && winapp && winapp->app && 
			(wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)) {
		psy_ui_MouseEvent ev;
		MOUSEHOOKSTRUCT* pMouseStruct = (MOUSEHOOKSTRUCT*)lParam;

		psy_ui_mouseevent_init_all(&ev,
			psy_ui_realpoint_make(pMouseStruct->pt.x, pMouseStruct->pt.y),			
			(wParam == WM_LBUTTONDOWN) ? 0 : 1,
			0, GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0);
		psy_signal_emit(&winapp->app->signal_mousehook, winapp->app,
			1, &ev);		
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

int psy_ui_winapp_run(psy_ui_WinApp* self) 
{
	MSG msg;

	SetTimer(NULL, 1, 10, (TIMERPROC)psy_ui_winapp_timerproc);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}	
    return (int)msg.wParam ;
}

int psy_ui_winapp_wait(psy_ui_WinApp* self) 
{
	return 0;
}

int psy_ui_winapp_continue(psy_ui_WinApp* self) 
{
	return 0;
}

void CALLBACK psy_ui_winapp_timerproc(
	HWND hwnd,        /* handle to window for timer messages */
	UINT message,     /* WM_TIMER message */
	UINT_PTR idTimer, /* timer identifier */
	DWORD dwTime)     /* current system time */
{
	psy_timers_tick(&winapp->app->wintimers);
}

void psy_ui_winapp_startmousehook(psy_ui_WinApp* self)
{
	if (self->mousehook == 0) {
		self->mousehook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
	}
}

void psy_ui_winapp_stopmousehook(psy_ui_WinApp* self)
{
	if (self->mousehook) {
		UnhookWindowsHookEx(self->mousehook);
		self->mousehook = NULL;
	}
}

void psy_ui_winapp_stop(psy_ui_WinApp* self)
{
	PostQuitMessage(0);
}

void psy_ui_winapp_close(psy_ui_WinApp* self)
{		
	assert(self);

	if (psy_ui_app_main(winapp->app)) {
		assert(psy_ui_app_main(winapp->app)->imp);

		PostMessage(((psy_ui_win_ComponentImp*)
			(psy_ui_app_main(winapp->app)->imp))->hwnd,
			WM_CLOSE, 0, 0);
	}
}

void psy_ui_winapp_onappdefaultschange(psy_ui_WinApp* self)
{
	DeleteObject(self->defaultbackgroundbrush);
	self->defaultbackgroundbrush = CreateSolidBrush(
		psy_ui_colour_colorref(
			&psy_ui_style_const(psy_ui_STYLE_ROOT)->background.colour));
}

void psy_ui_winapp_sendevent(psy_ui_WinApp* self, psy_ui_Component* component,
	psy_ui_Event* ev)
{
	psy_ui_win_ComponentImp* imp;

	assert(self);
	assert(component);
	assert(ev);

	imp = (psy_ui_win_ComponentImp*)component->imp;
	if (!imp) {
		return;
	}
	switch (psy_ui_event_type(ev)) {
	case psy_ui_KEYDOWN: {
		psy_ui_KeyboardEvent* keyevent;

		keyevent = (psy_ui_KeyboardEvent*)ev;
		SendMessage(imp->hwnd, (UINT)WM_KEYDOWN,
			(WPARAM)psy_ui_keyboardevent_keycode(keyevent),
			(LPARAM)psy_ui_keyboardevent_keydata(keyevent));
		break; }
	case psy_ui_KEYUP: {
		psy_ui_KeyboardEvent* keyevent;

		keyevent = (psy_ui_KeyboardEvent*)ev;
		SendMessage(imp->hwnd, (UINT)WM_KEYUP,
			(WPARAM)psy_ui_keyboardevent_keycode(keyevent),
			(LPARAM)psy_ui_keyboardevent_keydata(keyevent));
		break; }
	case psy_ui_MOUSEDOWN: {
		psy_ui_MouseEvent* mouseevent;
		UINT msg;

		mouseevent = (psy_ui_MouseEvent*)ev;		
		switch (psy_ui_mouseevent_button(mouseevent)) {
		case 1: msg = (UINT)WM_LBUTTONDOWN; break;
		case 2: msg = (UINT)WM_RBUTTONDOWN; break;
		case 3: msg = (UINT)WM_MBUTTONDOWN; break;
		default: msg = 0; break;
		}
		if (msg != 0) {		
			SendMessage(imp->hwnd, msg, (WPARAM)psy_ui_mouseevent_button(mouseevent),
				psy_ui_winapp_pack_pt(psy_ui_mouseevent_offset(mouseevent)));
		}
		break; }
	case psy_ui_MOUSEUP: {
		psy_ui_MouseEvent* mouseevent;
		UINT msg;

		mouseevent = (psy_ui_MouseEvent*)ev;		
		switch (psy_ui_mouseevent_button(mouseevent)) {
		case 1: msg = (UINT)WM_LBUTTONUP; break;
		case 2: msg = (UINT)WM_RBUTTONUP; break;
		case 3: msg = (UINT)WM_MBUTTONUP; break;
		default: msg = 0; break;
		}		
		if (msg != 0) {
			SendMessage(imp->hwnd, msg, (WPARAM)psy_ui_mouseevent_button(mouseevent),
				psy_ui_winapp_pack_pt(psy_ui_mouseevent_offset(mouseevent)));
		}		
		break; }
	case psy_ui_DBLCLICK : {
		psy_ui_MouseEvent* mouseevent;
		UINT msg;

		mouseevent = (psy_ui_MouseEvent*)ev;
		switch (psy_ui_mouseevent_button(mouseevent)) {
		case 1: msg = (UINT)WM_LBUTTONDBLCLK; break;
		case 2: msg = (UINT)WM_RBUTTONDBLCLK; break;
		case 3: msg = (UINT)WM_MBUTTONDBLCLK; break;
		default: msg = 0; break;
		}
		if (msg != 0) {			
			SendMessage(imp->hwnd, msg, (WPARAM)psy_ui_mouseevent_button(mouseevent),
				psy_ui_winapp_pack_pt(psy_ui_mouseevent_offset(mouseevent)));
		}
		break; }
	case psy_ui_MOUSEMOVE: {
		psy_ui_MouseEvent* mouseevent;		

		mouseevent = (psy_ui_MouseEvent*)ev;		
		SendMessage(imp->hwnd, (UINT)WM_MOUSEMOVE,
			(WPARAM)psy_ui_mouseevent_button(mouseevent),
			psy_ui_winapp_pack_pt(psy_ui_mouseevent_offset(mouseevent)));
		break; }
	default:
		break;
	}
}

psy_ui_Component* psy_ui_winapp_component(psy_ui_WinApp* self,
	uintptr_t handle)
{	
	psy_ui_win_ComponentImp* imp;
	
	imp = psy_ui_winapp_componentimp(self, handle);
	if (imp) {
		return imp->component;
	}
	return NULL;
}

psy_ui_win_ComponentImp* psy_ui_winapp_componentimp(psy_ui_WinApp* self,
	uintptr_t handle)
{
	return (psy_ui_win_ComponentImp*)psy_table_at(&winapp->selfmap, handle);
}

psy_List* psy_ui_winapp_toplevel(psy_ui_WinApp* self)
{
	psy_List* rv;
	psy_TableIterator it;	
	
	rv = NULL;
	for (it = psy_table_begin(&self->toplevelmap);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_win_ComponentImp* imp;

		imp = (psy_ui_win_ComponentImp*)psy_tableiterator_value(&it);
		if (imp->component) {
			psy_list_append(&rv, imp->component);
		}
	}
	return rv;
}

void psy_ui_winimp_register_native(psy_ui_WinApp* self, uintptr_t handle,
	psy_ui_ComponentImp* imp, bool top_level)
{
	psy_table_insert(&winapp->selfmap, handle, imp);
	if (top_level) {
		psy_table_insert(&winapp->toplevelmap, handle, imp);
	}
}

void psy_ui_winimp_unregister_native(psy_ui_WinApp* self, uintptr_t handle)
{	
	psy_table_remove(&winapp->selfmap, handle);
	psy_table_remove(&winapp->toplevelmap, handle);
}

BOOL CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID lParam);

const psy_List* psy_ui_winimp_fonts(const psy_ui_WinApp* self)
{
	return self->fonts;
}

void psy_ui_winimp_read_fonts(psy_ui_WinApp* self)
{
	HDC hdc;	

	hdc = GetDC(NULL);
	psy_list_deallocate(&self->fonts, NULL);	
	EnumFontFamilies(hdc, (LPCTSTR)NULL,
		(FONTENUMPROC)EnumFamCallBack, (LPARAM)&self->fonts);
	ReleaseDC(NULL, hdc);	
}

BOOL CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID lParam)
{
	psy_List** rv;
	psy_ui_FontInfo* fontinfo;

	rv = ((psy_List**)lParam);	
	fontinfo = (psy_ui_FontInfo*)malloc(sizeof(psy_ui_FontInfo));
	psy_ui_fontinfo_init(fontinfo, lplf->lfFaceName, lplf->lfHeight);
	psy_list_append(rv, fontinfo);

	return TRUE;
	UNREFERENCED_PARAMETER(lplf);
	UNREFERENCED_PARAMETER(lpntm);
}

#endif /* PSYCLE_TK_WIN32 */
