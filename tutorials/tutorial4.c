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

#include <gwenhywfar/cgui.h>
#include <aqbanking/jobloadcellphone.h>




int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  AB_ACCOUNT *a;
  AB_VALUE *v;
  GWEN_GUI *gui;
  const char *accNr;
  const char *number;
  const char *vstring;
  AB_JOB_LIST2 *jl;

  if (argc<4) {
    fprintf(stderr,
	    "Usage:\n"
	    "%s ACCOUNT_ID PHONE_NUMBER VALUE\n",
            argv[0]);
    return 1;
  }

  accNr=argv[1];
  number=argv[2];
  vstring=argv[3];

  gui=GWEN_Gui_CGui_new();
  assert(gui);
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("tutorial4", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d)\n", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    fprintf(stderr, "Error on OnlineInit (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "AqBanking successfully initialized.\n");

  /* Any type of job needs an account to operate on. The following function
   * allows wildcards (*) and jokers (?) in any of the arguments. */
  a=AB_Banking_FindAccount(ab,
                           "aqhbci", /* backend name */
                           "de",     /* two-char ISO country code */
			   "*",      /* bank code (with wildcard) */
                           accNr,    /* account number (wildcard) */
                           "*");     /* sub account id (Unterkontomerkmal) */
  if (a) {
    AB_JOB *j;
    AB_IMEXPORTER_CONTEXT *ctx;
    const AB_CELLPHONE_PRODUCT_LIST *pl;

    /* create a job which loads a prepaid card for cell phones. */
    j=AB_JobLoadCellPhone_new(a);

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

    /* select correct product */
    pl=AB_JobLoadCellPhone_GetCellPhoneProductList(j);
    if (pl==NULL) {
      fprintf(stderr, "No products supported\n");
      return 2;
    }
    else {
      AB_CELLPHONE_PRODUCT *cp;

      cp=AB_CellPhoneProduct_List_First(pl);
      while(cp) {
        const char *s;

	s=AB_CellPhoneProduct_GetProviderName(cp);
	fprintf(stderr, "Provider: [%s]\n", s);
	if (s && strcasecmp(s, "T-Mobile")==0)
          break;
	cp=AB_CellPhoneProduct_List_Next(cp);
      }

      if (cp==NULL) {
	fprintf(stderr, "Provider not found\n");
        return 2;
      }
      AB_JobLoadCellPhone_SetCellPhoneProduct(j, cp);
    }

    /* set number */
    AB_JobLoadCellPhone_SetPhoneNumber(j, number);

    /* set value */
    v=AB_Value_fromString(vstring);
    if (v==NULL) {
      fprintf(stderr, "Bad value\n");
      return 2;
    }
    AB_JobLoadCellPhone_SetValue(j, v);
    AB_Value_free(v);

    /* enqueue this job so that AqBanking knows we want it executed. */
    jl=AB_Job_List2_new();
    AB_Job_List2_PushBack(jl, j);

    /* When executing a list of enqueued jobs (as we will do below) all the
     * data returned by the server will be stored within an ImExporter
     * context.
     */
    ctx=AB_ImExporterContext_new();

    /* execute the queue. This effectivly sends all jobs which have been
     * enqueued to the respective backends/banks.
     * It only returns an error code (!=0) if not a single job could be
     * executed successfully. */
    rv=AB_Banking_ExecuteJobs(ab, jl, ctx);
    if (rv) {
      fprintf(stderr, "Error on executeQueue (%d)\n", rv);
      return 2;
    }
    else {

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



