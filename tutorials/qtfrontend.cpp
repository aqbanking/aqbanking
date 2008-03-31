/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: tutorial1.c 1121 2007-01-13 17:30:54Z martin $
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/***************************************************************************
 * This tutorial simply retrieves the balance of an account and stores the *
 * result in a file.                                                       *
 *                                                                         *
 * For this tutorial we use the QT GUI implementation.                     *                                                                        *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <aqbanking/banking.h>
#include <aqbanking/jobgetbalance.h>

#ifdef WITH_QBANKING
# include <qbanking/qbgui.h>
# include <qbanking/qbanking.h>
# include <qapplication.h>
#endif


int main(int argc, char *argv[]){
#ifdef WITH_QBANKING
  QApplication a(argc, argv);
  QBanking *qb;
  AB_BANKING *ab;
  AB_ACCOUNT *acc;
  AB_JOB *j;
  AB_JOB_LIST2 *jl;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;
  QBGui *gui;
  const char *blz;
  const char *acn;
  GWEN_DB_NODE *db;

  if (argc<3) {
    fprintf(stderr, "Missing arguments: BLZ ACCTID\n");
    return 1;
  }

  blz=argv[1];
  acn=argv[2];

  qb=new QBanking("testlib", NULL);
  gui=new QBGui(qb);
  GWEN_Gui_SetGui(gui->getCInterface());

  rv=qb->init();
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  rv=qb->onlineInit();
  if (rv) {
    fprintf(stderr, "Could not init online banking (%d)\n", rv);
    return 2;
  }

  ab=qb->getCInterface();
  acc=AB_Banking_GetAccountByCodeAndNumber(ab, blz, acn);
  if (!acc) {
    fprintf(stderr, "Account [%s/%s] not found\n", blz, acn);
    return 2;
  }

  j=AB_JobGetBalance_new(acc);
  if (!j) {
    fprintf(stderr, "Job not available\n");
    return 2;
  }

  rv=AB_Job_CheckAvailability(j, 0);
  if (rv) {
    fprintf(stderr, "Job not available with this account (%d)\n", rv);
    return 2;
  }

  ctx=AB_ImExporterContext_new();
  jl=AB_Job_List2_new();
  AB_Job_List2_PushBack(jl, j);
  rv=AB_Banking_ExecuteJobs(ab, jl, ctx, 0);
  if (rv) {
    fprintf(stderr, "Error executing job (%d)\n", rv);
    return 2;
  }

  db=GWEN_DB_Group_new("Context");
  rv=AB_ImExporterContext_toDb(ctx, db);
  if (rv) {
    fprintf(stderr, "Error writing context (%d)\n", rv);
    return 2;
  }

  rv=GWEN_DB_WriteFile(db, "test.ctx", GWEN_DB_FLAGS_DEFAULT, 0, 2000);
  if (rv) {
    fprintf(stderr, "Error writing db file (%d)\n", rv);
    return 2;
  }

  getchar();

#if 0
  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit online banking (%d)\n", rv);
    return 2;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }
#endif

  AB_Banking_free(ab);

#endif
  return 0;
}
