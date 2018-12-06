
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "ofxhome.h"

#include <aqbanking/banking.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/cgui.h>

#ifdef USE_GWENGUI_GTK2
# include <gwen-gui-gtk2/gtk2_gui.h>
# include "dlg_getinst.h"
# include <aqbanking/abgui.h>
#endif


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



int test3(int argc, char **argv) {
#ifdef USE_GWENGUI_GTK2
  GWEN_GUI *gui;
  GWEN_DIALOG *dlg;
  int rv;
  AB_BANKING *ab;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "ERROR: Unable to init Gwen.\n");
    exit(2);
  }

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(AQOFXCONNECT_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  gtk_init(&argc, &argv);

  gui=Gtk2_Gui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("test-ofxhome", NULL, 0);
  rv=AB_Banking_Init(ab);
  if (rv<0){
    fprintf(stderr, "Error on banking init: %d\n", rv);
    exit(2);
  }
  AB_Gui_Extend(gui, ab);

  dlg=OH_GetInstituteDialog_new("/tmp/ofx", NULL);
  if (dlg==NULL) {
    fprintf(stderr, "Could not create dialog\n");
    exit(2);
  }
  rv=GWEN_Gui_ExecDialog(dlg, 0);
  if (rv<=0){
    fprintf(stderr, "Dialog was aborted/rejected\n");
  }
  else {
    const OH_INSTITUTE_DATA *od;

    fprintf(stderr, "Dialog accepted, all fine\n");
    od=OH_GetInstituteDialog_GetSelectedInstitute(dlg);
    if (od) {
      fprintf(stderr, "- Id  : %d\n", OH_InstituteData_GetId(od));
      fprintf(stderr, "- Name: %s\n", OH_InstituteData_GetName(od));
      fprintf(stderr, "- FID : %s\n", OH_InstituteData_GetFid(od));
      fprintf(stderr, "- ORG : %s\n", OH_InstituteData_GetOrg(od));
      fprintf(stderr, "- URL : %s\n", OH_InstituteData_GetUrl(od));
    }

  }
  GWEN_Dialog_free(dlg);

  AB_Gui_Unextend(gui);

  rv=AB_Banking_Fini(ab);
  if (rv<0){
    fprintf(stderr, "Error on banking fini: %d\n", rv);
    exit(2);
  }
  AB_Banking_free(ab);
  GWEN_Gui_free(gui);
#endif
  return 0;
}



int main(int argc, char **argv) {
  //return test1(argc, argv);
  //return test2(argc, argv);
  return test3(argc, argv);
}


