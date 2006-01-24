#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#undef BUILDING_GBANKING

#include <gtk/gtk.h>
#include <unistd.h>

#include <gwenhywfar/logger.h>

#include "gbanking.h"
#include "jobview.h"



int test4(AB_BANKING *ab) {
  int result;

  result=AB_Banking_MessageBox(ab, 0, "Test-Title",
                               "This is a test message"
                               "<html>"
                               "<b>This</b> is a <i>test</i> message"
                               "</html>",
                               "1st Button",
                               "2nd Button",
                               "3rd Button");
  fprintf(stderr, "Your choice: %d\n", result);
  return 0;
}



int test5(AB_BANKING *ab){
  GWEN_TYPE_UINT32 id;
  int i;

  id=AB_Banking_ProgressStart(ab, "Test-Progress",
                              "This is just a simple test",
                              15);
  if (id==0) {
    fprintf(stderr, "Error (1)\n");
    return 1;
  }

  for (i=0; i<3; i++) {
    char numbuf[32];

    if (AB_Banking_ProgressAdvance(ab, id, i)) {
      fprintf(stderr, "User aborted\n");
      break;
    }
    snprintf(numbuf, sizeof(numbuf), "Log line %d", i);
    if (AB_Banking_ProgressLog(ab, id, 1, numbuf)) {
      fprintf(stderr, "User aborted\n");
      break;
    }
    sleep(1);
  }

  fprintf(stderr, "Will end soon\n");
  sleep(1);
  if (AB_Banking_ProgressEnd(ab, id)) {
    fprintf(stderr, "User aborted\n");
  }

  while (g_main_iteration (FALSE));

  fprintf(stderr, "Finished.\n");
  sleep(1);

  return 0;

}


int test6() {
  AB_BANKING *ab;
  int rv;

  ab=AB_Banking_new("gbanking-test", 0);
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error initializing AqBanking (%d)\n", rv);
    return 2;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Error deinitializing AqBanking (%d)\n", rv);
    return 3;
  }

  AB_Banking_free(ab);

  return 0;
}



gboolean _slotDelete(GtkWidget *w,
                     GdkEvent *event,
                     gpointer user_data) {
  gtk_main_quit();
  return FALSE;
}



int test7(AB_BANKING *ab) {
  GtkWidget *w;

  w=GBanking_JobView_new(ab, 0);
  gtk_widget_set_size_request(GTK_WIDGET(w), 500, 400);
  gtk_signal_connect(GTK_OBJECT(w), "delete-event",
                     GTK_SIGNAL_FUNC(_slotDelete),
                     w);

  gtk_widget_show(w);
  while (g_main_iteration (FALSE));
  gtk_main();

  fprintf(stderr, "Finished.\n");

  gtk_widget_destroy(w);

  //gtk_main ();
  return 0;
}




int main (int argc, char *argv[]){
  AB_BANKING *ab;
  int rv;
  const char *cmd;

  GWEN_Logger_SetLevel("aqhbci", GWEN_LoggerLevelInfo);
  GWEN_Logger_SetLevel("aqbanking", GWEN_LoggerLevelInfo);
  GWEN_Logger_SetLevel("gbanking", GWEN_LoggerLevelInfo);

  //return test6(); // use this for memory leak checks

  if (argc<2) {
    fprintf(stderr, "Command needed.\n");
    return 1;
  }
  cmd=argv[1];

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  //add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
  //add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");

  ab=GBanking_new("gbanking-test", 0);
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error initializing AqBanking (%d)\n", rv);
    return 2;
  }

  if (strcasecmp(cmd, "test4")==0)
    rv=test4(ab);
  else if (strcasecmp(cmd, "test5")==0)
    rv=test5(ab);
  else if (strcasecmp(cmd, "test6")==0)
    rv=test6(ab);
  else if (strcasecmp(cmd, "test7")==0)
    rv=test7(ab);

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Error deinitializing AqBanking (%d)\n", rv);
    return 3;
  }

  AB_Banking_free(ab);

  gtk_exit(0);
  return 0;
}


