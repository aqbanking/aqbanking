
#include "qgui.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwenhywfar.h>

#include <qapplication.h>


#define COUNT_LEVEL_1_1 30

#define COUNT_LEVEL_2_1 4
#define COUNT_LEVEL_2_2 10

#define COUNT_LEVEL_3_1 3
#define COUNT_LEVEL_3_2 3
#define COUNT_LEVEL_3_3 10


int test1(int argc, char **argv) {
  QApplication a(argc, argv);
  QGui *gui;
  int i;
  uint32_t progressId;
  int err;

  gui=new QGui(NULL);
  GWEN_Gui_SetGui(gui->getCInterface());

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
				    GWEN_GUI_PROGRESS_SHOW_LOG |
				    GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    "Doing something nasty...",
				    "Here should be a description",
				    COUNT_LEVEL_1_1,
				    0);

  for (i=1; i<=COUNT_LEVEL_1_1; i++) {
    fprintf(stderr, "Advance to %d...\n", i);
    err=GWEN_Gui_ProgressAdvance(progressId, i);
    if (err==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user\n");
      return 2;
    }
    sleep(1);
    //GWEN_Gui_ProgressLog(progressId, GWEN_LoggerLevel_Notice, "Advancing...");
  }

  GWEN_Gui_ProgressEnd(progressId);

  return 0;
}



int test2(int argc, char **argv) {
  QApplication a(argc, argv);
  QGui *gui;
  int i;
  uint32_t progressId;
  int err;

  gui=new QGui(NULL);
  GWEN_Gui_SetGui(gui->getCInterface());

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
				    GWEN_GUI_PROGRESS_SHOW_LOG |
				    GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    "Doing something nasty...",
				    "Here should be a description",
				    COUNT_LEVEL_2_1,
				    0);

  for (i=1; i<=COUNT_LEVEL_2_1; i++) {
    uint32_t subProgressId;
    int j;

    fprintf(stderr, "Advance 1 to %d...\n", i);
    err=GWEN_Gui_ProgressAdvance(progressId, i);
    if (err==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user\n");
      GWEN_Gui_ProgressEnd(progressId);
      return 2;
    }

    subProgressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS |
					 GWEN_GUI_PROGRESS_ALLOW_EMBED |
					 GWEN_GUI_PROGRESS_SHOW_LOG |
                                         GWEN_GUI_PROGRESS_DELAY |
					 GWEN_GUI_PROGRESS_SHOW_ABORT,
					 "This is a sub progress.",
					 "With another description",
					 COUNT_LEVEL_2_2,
					 0);
    for (j=1; j<=COUNT_LEVEL_2_2; j++) {
      fprintf(stderr, "Advance 2 to %d...\n", j);
      err=GWEN_Gui_ProgressAdvance(subProgressId, j);
      if (err==GWEN_ERROR_USER_ABORTED) {
	fprintf(stderr, "Aborted by user\n");
	GWEN_Gui_ProgressEnd(subProgressId);
	GWEN_Gui_ProgressEnd(progressId);
	return 2;
      }
      sleep(1);
    }
    GWEN_Gui_ProgressEnd(subProgressId);

    //GWEN_Gui_ProgressLog(progressId, GWEN_LoggerLevel_Notice, "Advancing...");
  }

  GWEN_Gui_ProgressEnd(progressId);

  return 0;
}



int test3(int argc, char **argv) {
  QApplication a(argc, argv);
  QGui *gui;
  int i;
  uint32_t level1;
  int err;

  gui=new QGui(NULL);
  GWEN_Gui_SetGui(gui->getCInterface());

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  level1=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
				GWEN_GUI_PROGRESS_SHOW_LOG |
				GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
				GWEN_GUI_PROGRESS_KEEP_OPEN |
				GWEN_GUI_PROGRESS_SHOW_ABORT,
				"Doing something nasty...",
				"Here should be a description",
				COUNT_LEVEL_3_1,
				0);

  GWEN_Gui_ProgressLog(level1, GWEN_LoggerLevel_Notice,
		       "Starting level 1");
  for (i=1; i<COUNT_LEVEL_3_1; i++) {
    uint32_t level2;
    int j;

    fprintf(stderr, "Advance 1 to %d...\n", i);
    GWEN_Gui_ProgressLog(level1, GWEN_LoggerLevel_Notice,
			 "1: Advancing");
    err=GWEN_Gui_ProgressAdvance(level1, i);
    if (err==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user\n");
      GWEN_Gui_ProgressEnd(level1);
      return 2;
    }

    level2=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				  GWEN_GUI_PROGRESS_ALLOW_EMBED |
				  GWEN_GUI_PROGRESS_SHOW_LOG |
				  GWEN_GUI_PROGRESS_DELAY |
				  GWEN_GUI_PROGRESS_SHOW_ABORT,
				  "This is level2.",
				  "With another description",
				  COUNT_LEVEL_3_2,
				  0);
    GWEN_Gui_ProgressLog(level2, GWEN_LoggerLevel_Notice,
			 "22: Starting");
    for (j=1; j<COUNT_LEVEL_3_2; j++) {
      uint32_t level3;
      int k;

      fprintf(stderr, "Advance 2 to %d...\n", j);
      GWEN_Gui_ProgressLog(level2, GWEN_LoggerLevel_Notice,
			   "22: Advancing");
      err=GWEN_Gui_ProgressAdvance(level2, j);
      if (err==GWEN_ERROR_USER_ABORTED) {
	fprintf(stderr, "Aborted by user\n");
	GWEN_Gui_ProgressEnd(level2);
	GWEN_Gui_ProgressEnd(level1);
	return 2;
      }

      level3=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_LOG |
				    GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    "This is level 3.",
				    "With another description",
				    COUNT_LEVEL_3_3,
				    0);
      GWEN_Gui_ProgressLog(level3, GWEN_LoggerLevel_Notice,
			   "333: Starting");
      for (k=1; k<COUNT_LEVEL_3_3; k++) {
	fprintf(stderr, "Advance 3 to %d...\n", k);
	GWEN_Gui_ProgressLog(level3, GWEN_LoggerLevel_Notice,
			     "333: Advancing");
	err=GWEN_Gui_ProgressAdvance(level3, k);
	if (err==GWEN_ERROR_USER_ABORTED) {
	  fprintf(stderr, "Aborted by user\n");
	  GWEN_Gui_ProgressEnd(level3);
	  GWEN_Gui_ProgressEnd(level2);
	  GWEN_Gui_ProgressEnd(level1);
	  return 2;
	}
	sleep(1);
      }
      GWEN_Gui_ProgressLog(level3, GWEN_LoggerLevel_Notice,
			   "333: Finished");
      GWEN_Gui_ProgressEnd(level3);
    }
    GWEN_Gui_ProgressLog(level2, GWEN_LoggerLevel_Notice,
			 "22: Finished");
    GWEN_Gui_ProgressEnd(level2);
  }

  GWEN_Gui_ProgressLog(level1, GWEN_LoggerLevel_Notice,
		       "1: Finished");
  GWEN_Gui_ProgressEnd(level1);

  return 0;
}




int test4(int argc, char **argv) {
  QApplication a(argc, argv);
  QGui *gui;
  int err;
  const char *x;
  QString qs;

  if (argc<2) {
    fprintf(stderr, "Missing argument.\n");
    return 1;
  }

  x=argv[1];

  gui=new QGui(NULL);
  GWEN_Gui_SetGui(gui->getCInterface());

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  qs=gui->extractHtml(x);
  fprintf(stderr, "Result: [%s]\n",
	  gui->qstringToUtf8String(qs).c_str());

  return 0;
}




int main(int argc, char **argv) {
  return test4(argc, argv);
}


