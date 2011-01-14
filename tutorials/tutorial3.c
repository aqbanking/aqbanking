/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
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
#include <aqbanking/jobgettransactions.h>




int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  AB_ACCOUNT *a;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("tutorial3", 0, 0);

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

  /* This function loads the settings file of AqBanking so the users and
   * accounts become available after this function successfully returns.
   */
  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    fprintf(stderr, "Error on init of online modules (%d)\n", rv);
    return 2;
  }

  /* Any type of job needs an account to operate on. The following function
   * allows wildcards (*) and jokers (?) in any of the arguments. */
  a=AB_Banking_FindAccount(ab,
                           "aqhbci", /* backend name */
                           "de",     /* two-char ISO country code */
                           "200*",   /* bank code (with wildcard) */
                           "*",      /* account number (wildcard) */
                           "*");     /* sub account id (Unterkontomerkmal) */
  if (a) {
    AB_JOB_LIST2 *jl;
    AB_JOB *j;
    AB_IMEXPORTER_CONTEXT *ctx;

    /* create a job which retrieves transaction statements. */
    j=AB_JobGetTransactions_new(a);

    /* This function checks whether the given job is available with the
     * backend/provider to which the account involved is assigned.
     * The corresponding provider/backend might also check whether this job
     * is available with the given account.
     * If the job is available then 0 is returned, otherwise the error code
     * might give you a hint why the job is not supported. */
    rv=AB_Job_CheckAvailability(j);
    if (rv) {
      fprintf(stderr, "Job is not available (%d)\n", rv);
      return 2;
    }

    /* create a job list to which the jobs to be executed are added.
     * This list is later given as an argument to the queue execution
     * function.
     */
    jl=AB_Job_List2_new();

    /* add job to this list */
    AB_Job_List2_PushBack(jl, j);

    /* When executing a list of enqueued jobs (as we will do below) all the
     * data returned by the server will be stored within an ImExporter
     * context.
     */
    ctx=AB_ImExporterContext_new();

    /* execute the jobs which are in the given list (well, for this tutorial
     * there is only one job in the list, but the number is not limited).
     * This effectivly sends all jobs to the respective backends/banks.
     * It only returns an error code (!=0) if there has been a problem
     * sending the jobs.
     */
    rv=AB_Banking_ExecuteJobs(ab, jl, ctx);
    if (rv) {
      fprintf(stderr, "Error on executeQueue (%d)\n", rv);
      return 2;
    }
    else {
      AB_IMEXPORTER_ACCOUNTINFO *ai;

      ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
      while(ai) {
        const AB_TRANSACTION *t;

        t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
        while(t) {
          const AB_VALUE *v;

          v=AB_Transaction_GetValue(t);
          if (v) {
            const GWEN_STRINGLIST *sl;
            const char *purpose;

            /* The purpose (memo field) might contain multiple lines.
             * Therefore AqBanking stores the purpose in a string list
             * of which the first entry is used in this tutorial */
            sl=AB_Transaction_GetPurpose(t);
            if (sl)
              purpose=GWEN_StringList_FirstString(sl);
            else
              purpose="";

            fprintf(stderr, " %-32s (%.2f %s)\n",
                    purpose,
		    AB_Value_GetValueAsDouble(v),
                    AB_Value_GetCurrency(v));
          }
          t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
        } /* while transactions */
        ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
      } /* while ai */
    } /* if executeQueue successfull */
    /* free the job to avoid memory leaks */
    AB_Job_free(j);
  } /* if account found */
  else {
    fprintf(stderr, "No account found.\n");
  }

  /* This function MUST be called in order to let AqBanking save the changes
   * to the users and accounts (like they occur after executing jobs).
   */
  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit online modules (%d)\n", rv);
    return 3;
  }

  /* This function deinitializes AqBanking. It undoes the effects of
   * AB_Banking_Init() and should be called before destroying an AB_BANKING
   * object.
   */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }
  AB_Banking_free(ab);

  return 0;
}



