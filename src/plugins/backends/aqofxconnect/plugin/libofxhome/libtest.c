



#include "ofxhome.h"

#include <aqbanking/banking.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/cgui.h>



int test1(int argc, char **argv) {
  OFXHOME *oh;
  const OH_INSTITUTE_SPEC_LIST *sl;
  GWEN_GUI *gui;
  int rv;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(AQOFXCONNECT_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_CGui_SetCharSet(gui, "ISO-8859-15");
  GWEN_Gui_SetGui(gui);

  oh=OfxHome_new("/tmp/ofx");
  sl=OfxHome_GetSpecs(oh);
  if (sl==NULL) {
    fprintf(stderr, "No specs...\n");
    return 2;
  }
  OfxHome_free(oh);

  return 0;
}



int test2(int argc, char **argv) {
  OFXHOME *oh;
  const OH_INSTITUTE_DATA *d;
  GWEN_GUI *gui;
  int rv;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(AQOFXCONNECT_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_CGui_SetCharSet(gui, "ISO-8859-15");
  GWEN_Gui_SetGui(gui);

  oh=OfxHome_new("/tmp/ofx");
  d=OfxHome_GetData(oh, 542);
  if (d==NULL) {
    fprintf(stderr, "No data...\n");
    return 2;
  }
  OfxHome_free(oh);

  return 0;
}



int main(int argc, char **argv) {
  //return test1(argc, argv);
  return test2(argc, argv);
}


