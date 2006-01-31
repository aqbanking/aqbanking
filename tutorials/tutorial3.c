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
 * This tutorial shows how to use jobs in AqBanking.                       *
 * In this example we retrieve transaction statements for a given account. *
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
#include <aqbanking/jobgettransactions.h>



int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  AB_ACCOUNT *a;

  ab=CBanking_new("tutorial3", 0);
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "AqBanking successfully initialized.\n");

  /* Any type of job needs an account to operate on. The following function
   * allows wildcards (*) and jokers (?) in any of the arguments. */
  a=AB_Banking_FindAccount(ab,
                           "aqhbci", /* backend name */
                           "de",     /* two-char ISO country code */
                           "200*",   /* bank code (with wildcard) */
                           "*");     /* account number (wildcard) */
  if (a) {
    AB_JOB *j;

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

    /* enqueue this job so that AqBanking knows we want it executed. */
    rv=AB_Banking_EnqueueJob(ab, j);
    if (rv) {
      fprintf(stderr, "Error on enqueueJob (%d)\n", rv);
      return 2;
    }

    /* execute the queue. This effectivly sends all jobs which have been
     * enqueued to the respective backends/banks.
     * It only returns an error code (!=0) if not a single job could be
     * executed successfully. */
    rv=AB_Banking_ExecuteQueue(ab);
    if (rv) {
      fprintf(stderr, "Error on executeQueue (%d)\n", rv);
      return 2;
    }
    else {
      AB_TRANSACTION_LIST2 *tl;

      /* upon success the job might have some transactions for us.
       * Please refer to tutorial2 for an introduction into the usage of
       * List2's and List2 iterators in AqBanking. */
      tl=AB_JobGetTransactions_GetTransactions(j);
      if (tl) {
        AB_TRANSACTION_LIST2_ITERATOR *it;

        it=AB_Transaction_List2_First(tl);
        if (it) {
          AB_TRANSACTION *t;

          t=AB_Transaction_List2Iterator_Data(it);
          while(t) {
            const AB_VALUE *v;

            v=AB_Transaction_GetValue(t);
            if (v) {
              const GWEN_STRINGLIST *sl;
              const char *purpose;

              /* The purpose (memo field) might contain multiple lines.
               * Therefore AqBanking stores the purpose in a string list
               * of which the first is used in this tutorial */
              sl=AB_Transaction_GetPurpose(t);
              if (sl)
                purpose=GWEN_StringList_FirstString(sl);
              else
                purpose="";

              fprintf(stderr, "Transaction: %s (%.2lf %s)\n",
                      purpose,
                      AB_Value_GetValue(v),
                      AB_Value_GetCurrency(v));
            }
            t=AB_Transaction_List2Iterator_Next(it);
          }

          AB_Transaction_List2Iterator_free(it);
        } /* if iterator */
        /* At this point we do NOT free the list because it is owned by
         * the job */
      } /* if transaction list */
    } /* if executeQueue successfull */
    /* free the job to avoid memory leaks */
    AB_Job_free(j);
  } /* if account found */
  else {
    fprintf(stderr, "No account found.\n");
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }
  AB_Banking_free(ab);

  return 0;
}


