/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/***************************************************************************
 * This tutorial simply creates an instance of AqBanking, initializes and  *
 * deinitializes it.                                                       *
 *                                                                         *
 * You must either choose a frontend to be used with AqBanking or create   *
 * one yourself by implementing the user interface callbacks of AqBanking. *
 *                                                                         *
 * However, for simplicity reasons we use the console frontend CBanking    *
 * which implements these callbacks for you.                               *
 *                                                                         *
 * There are other frontends, e.g. G2Banking for GTK2/Gnome, QBanking for  *
 * QT3 or KDE3 or KBanking for KDE3.                                       *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cbanking/cbanking.h>



int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;

  /* The first argument is the name of the application. This is needed for
   * AqBanking to internally store some application-specific settings.
   * This name may contain whatever characters you like, it is escaped
   * internally before creating file paths or DB groups from it.
   *
   * The second argument is the folder in which the AqBanking settings are
   * stored. You should in most cases provide NULL here which makes AqBanking
   * choose the default path ($HOME/.banking).
   * If this folder doesn't exist it will be created as soon as AqBanking has
   * something to store (in most cases when closing the application).
   */
  ab=CBanking_new("tutorial1", 0);

  /* This function initializes AqBanking. It is only after successfull return
   * from this function that any other AqBanking function may be used.
   */
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "AqBanking successfully initialized.\n");

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


