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
  GWEN_GUI *gui;
  AB_BANKING *ab;
  int rv;
  AB_ACCOUNT_SPEC_LIST *accs=NULL;
  AB_ACCOUNT_SPEC *as;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("tutorial3", 0, 0);
  AB_Banking_Init(ab);
  fprintf(stderr, "AqBanking successfully initialized.\n");


  /* get the list of known accounts */
  AB_Banking6_GetAccountSpecList(ab, &accs);

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
  if (as) {
    AB_TRANSACTION_LIST *cmdList;
    AB_TRANSACTION *t;
    AB_IMEXPORTER_CONTEXT *ctx;

    cmdList=AB_Transaction_List_new();

    t=AB_Transaction_new();
    AB_Transaction_SetCommand(t, AB_Transaction_CommandGetTransactions);
    AB_Transaction_SetUniqueAccountId(t, AB_AccountSpec_GetUniqueId(as));

    AB_Transaction_List_Add(t, cmdList);

    ctx=AB_ImExporterContext_new();
    AB_Banking6_SendCommands(ab, cmdList, ctx);

    ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
    while(ai) {
      const AB_TRANSACTION *t;

      t=AB_ImExporterAccountInfo_GetFirstTransaction(ai, 0, 0);
      while(t) {
	const AB_VALUE *v;

	v=AB_Transaction_GetValue(t);
	if (v) {
	  const char *purpose;

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
    AB_ImExporterContext_free(ctx);
  } /* if (as) */

  AB_Banking_Fini(ab);
  AB_Banking_free(ab);

  return 0;
}



