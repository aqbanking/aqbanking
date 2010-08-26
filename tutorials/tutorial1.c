/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/***************************************************************************
 * This tutorial simply creates an instance of AqBanking, initializes and  *
 * deinitializes it.                                                       *
 *                                                                         *
 * You must either choose a GUI implementation to be used with AqBanking   *
 * or create one yourself by implementing the user interface callbacks of  *
 * LibGwenhywfar.                                                          *
 *                                                                         *
 * However, for simplicity reasons we use the console GUI implementation   *
 * which implements these callbacks for you.                               *
 *                                                                         *
 * There are other GUI implementations, e.g. for GTK2, QT3, QT4 and FOX16. *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <aqbanking/banking.h>
#include <gwenhywfar/cgui.h>



int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  /* The first argument is the name of the application. This is needed for
   * AqBanking to internally store some application-specific settings.
   * This name may contain whatever characters you like, it is escaped
   * internally before creating file paths or DB groups from it.
   *
   * The second argument is the folder in which the AqBanking settings are
   * stored. You should in most cases provide NULL here which makes AqBanking
   * choose the default path ($HOME/.aqbanking).
   * If this folder doesn't exist it will be created as soon as AqBanking has
   * something to store (in most cases when closing the application).
   */
  ab=AB_Banking_new("tutorial1", 0, 0);

  /* This function initializes AqBanking. It is only after successfull return
   * from this function that any other AqBanking function may be used.
   */
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d)\n", rv);
    return 2;
  }

  /* Initialize the only banking part of AqBanking. This is needed to
   * actually perform online banking actions (like retrieving account
   * statements etc).
   */
  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    fprintf(stderr, "Error on onlineinit (%d)\n", rv);
    return 2;
  }


  fprintf(stderr, "AqBanking successfully initialized.\n");


  /* deinit the online banking part of AqBanking */
  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }

  /* You must always call this function before exiting, because only then
   * AqBanking's settings are written.
   * After this function has been called no other function except
   * AB_Banking_free() or AB_Banking_Init() may be called.
   */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }

  /* The AqBanking instance you created at the beginning must always be
   * destroyed using this function to avoid memory leaks.
   */
  AB_Banking_free(ab);

  return 0;
}


