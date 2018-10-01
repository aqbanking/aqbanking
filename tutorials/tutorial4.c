/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/***************************************************************************
 * This tutorial shows how to use jobs in AqBanking.                       *
 * In this example we retrieve transaction statements for a given account. *
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
#include <aqbanking/transaction.h>




int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  AB_ACCOUNT_SPEC_LIST *accs=NULL;
  AB_ACCOUNT_SPEC *as;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("tutorial4", 0, 0);

  /* This is the basic init function. It only initializes the minimum (like
   * setting up plugin and data paths). After this function successfully
   * returns you may freely use any non-online function. To use online
   * banking functions (like getting the list of managed accounts, users
   * etc) you will have to call AB_Banking_OnlineInit().
   */
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "AqBanking successfully initialized.\n");


  /* get the list of known accounts */
  rv=AB_Banking6_GetAccountSpecList(ab, &accs);
  if (rv<0) {
    fprintf(stderr, "Unable to get the list of accounts (%d: %s)\n", rv, GWEN_Error_SimpleToString(rv));
    return 3;
  }

  /* find a matching account within the given list */
  as=AB_AccountSpec_List_FindFirst(accs,
                                   "aqhbci",                /* backendName */
                                   "de",                    /* country */
                                   "28*",                   /* bankId bank */
                                   "*",                     /* accountNumber */
                                   "*",                     /* subAccountId */
                                   "*",                     /* iban */
                                   "*",                     /* currency */
                                   AB_AccountType_Unknown); /* ty */
  if (as==NULL) {
    fprintf(stderr, "No matching account found.\n");
    return 3;
  } /* if (as==NULL) */

  if (as) {
    AB_TRANSACTION_LIST *cmdList;
    AB_TRANSACTION *t;
    AB_IMEXPORTER_CONTEXT *ctx;

    /* create a list to which banking commands are added */
    cmdList=AB_Transaction_List_new();

    /* create an online banking command */
    t=AB_Transaction_new();
    AB_Transaction_SetCommand(t, AB_Transaction_CommandGetTransactions);
    AB_Transaction_SetUniqueAccountId(t, AB_AccountSpec_GetUniqueId(as));

    /* add command to the list */
    AB_Transaction_List_Add(t, cmdList);

    /* we could now add any number of commands here */

    /* When sending a list of commands (as we will do below) all the
     * data returned by the server will be stored within an ImExporter
     * context.
     */
    ctx=AB_ImExporterContext_new();

    /* execute the jobs which are in the given list (well, for this tutorial
     * there is only one job in the list, but the number is not limited).
     * This effectivly sends all jobs to the respective backends/banks.
     * It only returns an error code (!=0) if there has been a problem
     * sending the jobs. */
    rv=AB_Banking6_SendCommands(ab, cmdList, ctx);
    if (rv<0) {
      fprintf(stderr, "Error on executeQueue (%d)\n", rv);
      /* clean up */
      AB_ImExporterContext_free(ctx);
      AB_Banking_Fini(ab);
      AB_Banking_free(ab);
      return 2;
    }
    else {
      AB_IMEXPORTER_ACCOUNTINFO *ai;

      ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
      while(ai) {
        const AB_TRANSACTION *t;

        t=AB_ImExporterAccountInfo_GetFirstTransaction(ai, 0, 0);
        while(t) {
          const AB_VALUE *v;

          v=AB_Transaction_GetValue(t);
          if (v) {
            const char *purpose;

            /* The purpose (memo field) might contain multiple lines. */
            purpose=AB_Transaction_GetPurpose(t);

            fprintf(stderr, " %-32s (%.2f %s)\n",
                    purpose,
		    AB_Value_GetValueAsDouble(v),
                    AB_Value_GetCurrency(v));
          }
          t=AB_Transaction_List_Next(t);
        } /* while transactions */
        ai=AB_ImExporterAccountInfo_List_Next(ai);
      } /* while ai */
    } /* if executeQueue successfull */

    /* free im-/exporter context */
    AB_ImExporterContext_free(ctx);
  } /* if (as) */

  /* This function deinitializes AqBanking. It undoes the effects of
   * AB_Banking_Init() and should be called before destroying an AB_BANKING
   * object.
   */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }

  /* free AqBanking object */
  AB_Banking_free(ab);

  return 0;
}



