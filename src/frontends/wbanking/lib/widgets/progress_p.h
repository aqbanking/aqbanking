

#include "progress_l.h"


#define WB_GUI_PROGRESS_CHILD_STATIC   1
#define WB_GUI_PROGRESS_CHILD_EDIT     2
#define WB_GUI_PROGRESS_CHILD_PROGRESS 3
#define WB_GUI_PROGRESS_CHILD_CLOSE    4
#define WB_GUI_PROGRESS_CHILD_ABORT    5


struct WB_GUI_PROGRESS {
  HWND hStatic;
  HWND hEdit;
  HWND hProgress;
  HWND hClose;
  HWND hAbort;
};

