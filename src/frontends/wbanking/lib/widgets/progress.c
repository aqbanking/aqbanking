

#include "progress_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

/* needed for commctrl.h with MINGW */
#define _WIN32_IE  0x0300

#include <windows.h>
#include <commctrl.h>


#define WB_GUI_PROGRESS_CLASSNAME "WB_GUI_PROGRESS"

#define I18N(msg) msg




WB_GUI_PROGRESS *WB_GuiProgress_new() {
  WB_GUI_PROGRESS *pro;

  GWEN_NEW_OBJECT(WB_GUI_PROGRESS, pro);

  return pro;
}



LRESULT CALLBACK WB_GuiProgress_WndProc(HWND hWnd,
					UINT msg,
					WPARAM wParam,
					LPARAM lParam) {
  return DefWindowProc(hWnd, msg, wParam, lParam);
}



ATOM WB_GuiProgress_RegisterClass(WB_GUI_PROGRESS *pro,
				  HINSTANCE hInstance) {
  WNDCLASSEX wcex;

  wcex.cbSize=sizeof(WNDCLASSEX);

  wcex.style=CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc=WB_GuiProgress_WndProc;
  wcex.cbClsExtra=0;
  wcex.cbWndExtra=4;
  wcex.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszClassName=WB_GUI_PROGRESS_CLASSNAME;

  return RegisterClassEx(&wcex);
}



HWND WB_GuiProgress_Create(const char *title,
			   HWND hParent) {
  HWND hWnd;
  WB_GUI_PROGRESS *pro=NULL;
  HINSTANCE hInstance;
  INITCOMMONCONTROLSEX initCtrlEx;

  hInstance=GetModuleHandle(NULL);

  /* init comctrl32 */
  initCtrlEx.dwSize=sizeof(INITCOMMONCONTROLSEX);
  initCtrlEx.dwICC=ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&initCtrlEx);

  /* create dialog window */
  hWnd=CreateWindowEx(WS_EX_CONTROLPARENT,
		      WB_GUI_PROGRESS_CLASSNAME,
		      title,
		      WS_OVERLAPPEDWINDOW,
		      CW_USEDEFAULT,
		      CW_USEDEFAULT,
		      CW_USEDEFAULT,
		      CW_USEDEFAULT,
		      hParent,
		      NULL,
		      hInstance,
		      NULL);
  if (hWnd==NULL) {
    DBG_ERROR(WBANKING_LOGDOMAIN, "Unable to create progress widget");
    return hWnd;
  }

  pro=WB_GuiProgress_new();
  SetWindowLong(hWnd, 0, (LONG) pro);

  pro->hStatic=CreateWindow("STATIC",
			    NULL,
			    WS_CHILD | WS_VISIBLE |
			    SS_LEFT | SS_NOPREFIX,
			    CW_USEDEFAULT,
			    CW_USEDEFAULT,
			    CW_USEDEFAULT,
			    CW_USEDEFAULT,
			    hWnd,
			    (HMENU) WB_GUI_PROGRESS_CHILD_STATIC,
			    hInstance,
			    NULL);
  if (pro->hStatic==NULL) {
    DBG_ERROR(WBANKING_LOGDOMAIN, "Unable to create STATIC widget");
    return NULL;
  }

  pro->hEdit=CreateWindow("EDIT",
			  NULL,
			  WS_CHILD | WS_VISIBLE |
			  ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL |
			  ES_LEFT | ES_READONLY,
			  CW_USEDEFAULT,
			  CW_USEDEFAULT,
			  CW_USEDEFAULT,
			  CW_USEDEFAULT,
			  hWnd,
			  (HMENU) WB_GUI_PROGRESS_CHILD_EDIT,
			  hInstance,
			  NULL);
  if (pro->hEdit==NULL) {
    DBG_ERROR(WBANKING_LOGDOMAIN, "Unable to create EDIT widget");
    return NULL;
  }

  pro->hProgress=CreateWindow(PROGRESS_CLASS,
			      NULL,
			      WS_CHILD | WS_VISIBLE |
			      PBS_SMOOTH,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      hWnd,
			      (HMENU) WB_GUI_PROGRESS_CHILD_PROGRESS,
			      hInstance,
			      NULL);
  if (pro->hProgress==NULL) {
    DBG_ERROR(WBANKING_LOGDOMAIN, "Unable to create ProgressBar widget");
    return NULL;
  }

  pro->hClose=CreateWindow("BUTTON",
			   I18N("&Close"),
			   WS_CHILD | WS_VISIBLE |
			   BS_PUSHBUTTON | BS_TEXT |
			   BS_VCENTER | BS_CENTER,
			   CW_USEDEFAULT,
			   CW_USEDEFAULT,
			   CW_USEDEFAULT,
			   CW_USEDEFAULT,
			   hWnd,
			   (HMENU) WB_GUI_PROGRESS_CHILD_CLOSE,
			   hInstance,
			   NULL);
  if (pro->hClose==NULL) {
    DBG_ERROR(WBANKING_LOGDOMAIN, "Unable to create CLOSE button");
    return NULL;
  }

  pro->hAbort=CreateWindow("BUTTON",
			   I18N("&Abort"),
			   WS_CHILD | WS_VISIBLE |
			   BS_PUSHBUTTON | BS_TEXT |
			   BS_VCENTER | BS_CENTER,
			   CW_USEDEFAULT,
			   CW_USEDEFAULT,
			   CW_USEDEFAULT,
			   CW_USEDEFAULT,
			   hWnd,
			   (HMENU) WB_GUI_PROGRESS_CHILD_ABORT,
			   hInstance,
			   NULL);
  if (pro->hAbort==NULL) {
    DBG_ERROR(WBANKING_LOGDOMAIN, "Unable to create CLOSE button");
    return NULL;
  }


  return hWnd;
}





