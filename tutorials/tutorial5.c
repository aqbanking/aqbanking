/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/***************************************************************************
 * This tutorial shows the list of accounts currently known to AqBanking.  *
 *                                                                         *
 * It also gives an introduction into the usage of XXX_List_ForEach        *
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


static AB_ACCOUNT_SPEC *printAccountList_cb(AB_ACCOUNT_SPEC *as, void *user_data);



int main(int argc, char **argv)
{
  AB_BANKING *ab;
  AB_ACCOUNT_SPEC_LIST *accs=NULL;
  int rv;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("tutorial5", 0, 0);

  /* Initialize AqBanking */
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d: %s)\n", rv, GWEN_Error_SimpleToString(rv));;
    return 2;
  }

  fprintf(stderr, "AqBanking successfully initialized.\n");

  /* Get a list of accounts which are known to AqBanking.
   * We own the list returned, so in order to avoid memory
   * leaks we need to free it afterwards.
   *
   * The rest of this tutorial shows how lists are generally used by
   * AqBanking.
   */
  rv=AB_Banking_GetAccountSpecList(ab, &accs);
  if (rv<0) {
    fprintf(stderr, "Unable to get the list of accounts (%d: %s)\n", rv, GWEN_Error_SimpleToString(rv));
    return 3;
  }
  else {
    AB_AccountSpec_List_ForEach(accs, printAccountList_cb, NULL);
    /* free the list to avoid memory leaks */
    AB_AccountSpec_List_free(accs);
  }

  /* deinitialize AqBanking */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }

  /* free AqBanking object */
  AB_Banking_free(ab);
  return 0;
}



AB_ACCOUNT_SPEC *printAccountList_cb(AB_ACCOUNT_SPEC *as, void *user_data)
{
  fprintf(stderr,
          "Account: %s %s (%s) [%s]\n",
          AB_AccountSpec_GetBankCode(as),
          AB_AccountSpec_GetAccountNumber(as),
          AB_AccountSpec_GetAccountName(as),
          /* every account is assigned to a backend (sometimes called provider)
           * which actually performs online banking tasks. We get a pointer
           * to the name of that provider/backend with this call.*/
          AB_AccountSpec_GetBackendName(as));
  return NULL;
}

