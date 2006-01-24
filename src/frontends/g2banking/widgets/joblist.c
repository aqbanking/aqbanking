/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "joblist_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GtkWidget *GBanking_JobList_new(AB_BANKING *ab,
                                GtkWidget *parent,
                                GtkWidget *tree) {
  GBANKING_JOBLIST *jl;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *sel;

  GWEN_NEW_OBJECT(GBANKING_JOBLIST, jl);
  jl->banking=ab;
  jl->store=gtk_list_store_new(7,
			       G_TYPE_INT,    /* job id */
                               G_TYPE_STRING, /* job type */
                               G_TYPE_STRING, /* bank */
                               G_TYPE_STRING, /* account */
                               G_TYPE_STRING, /* status */
                               G_TYPE_STRING, /* backend */
                               G_TYPE_STRING  /* application */
			      );

  if (tree) {
    jl->tree=tree;
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree),
                            GTK_TREE_MODEL(jl->store));
  }
  else {
    jl->tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(jl->store));
    if (parent)
      gtk_widget_reparent(jl->tree, parent);
  }

  g_object_set_data(G_OBJECT(jl->store), GBANKING_JOBLIST_ID, jl);
  gtk_object_set_data_full(GTK_OBJECT(jl->tree),
                           GBANKING_JOBLIST_ID,
                           jl,
                           GBanking_JobList_freeData);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Job Id"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_JOBID,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Job Type"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_JOBTYPE,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Bank"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_BANK,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Account"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_ACCOUNT,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Status"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_STATUS,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Backend"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_BACKEND,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Application"),
                                                  renderer,
						  "text",
						  GB_JOBLISTCOLUMN_APP,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(jl->tree), column);


  /* Setup the selection handler */
  sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(jl->tree));
  gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (sel), "changed",
		    G_CALLBACK(GBanking_JobList__selection_changed_cb),
		    NULL);

  /* populate list */
  GBanking_JobList__populate(jl);
  gtk_widget_show(jl->tree);
  return jl->tree;
}



void GBanking_JobList__populate(GBANKING_JOBLIST *jl) {
  AB_JOB_LIST2_ITERATOR *jit;
  AB_JOB *j;
  GtkTreeIter iter;

  gtk_list_store_clear(jl->store);
  if (jl->jobs)
    AB_Job_List2_free(jl->jobs);
  jl->jobs=AB_Banking_GetEnqueuedJobs(jl->banking);

  if (jl->jobs==0) {
    return;
  }

  jit=AB_Job_List2_First(jl->jobs);
  assert(jit);
  j=AB_Job_List2Iterator_Data(jit);
  assert(j);
  while(j) {
    AB_ACCOUNT *a;
    const char *jobtype;
    const char *bankName;
    const char *accountName;
    const char *backend;
    const char *status;
    const char *app;

    a=AB_Job_GetAccount(j);
    assert(a);
    bankName=AB_Account_GetBankName(a);
    if (!bankName)
      bankName=AB_Account_GetBankCode(a);
    accountName=AB_Account_GetAccountName(a);
    if (!accountName)
      accountName=AB_Account_GetAccountNumber(a);
    backend=AB_Provider_GetName(AB_Account_GetProvider(a));

    switch(AB_Job_GetType(j)) {
    case AB_Job_TypeGetBalance:
      jobtype=I18N("Get Balance");
      break;
    case AB_Job_TypeGetTransactions:
      jobtype=I18N("Get Transactions");
      break;
    case AB_Job_TypeTransfer:
      jobtype=I18N("Transfer");
      break;
    case AB_Job_TypeDebitNote:
      jobtype=I18N("Debit Note");
      break;
    default:
      jobtype=I18N("Unknown");
      break;
    }


    switch(AB_Job_GetStatus(j)) {
    case AB_Job_StatusNew:
      status=I18N("new");
      break;
    case AB_Job_StatusUpdated:
      status=I18N("updated");
      break;
    case AB_Job_StatusEnqueued:
      status=I18N("enqueued");
      break;
    case AB_Job_StatusSent:
      status=I18N("sent");
      break;
    case AB_Job_StatusPending:
      status=I18N("pending");
      break;
    case AB_Job_StatusFinished:
      status=I18N("finished");
      break;
    case AB_Job_StatusError:
      status=I18N("error");
      break;
    default:
      status=I18N("(unknown)");
      break;
    }

    app=AB_Job_GetCreatedBy(j);
    if (!app)
      app=I18N("(unknown)");

    /* really set data */
    gtk_list_store_append(jl->store, &iter);
    gtk_list_store_set(jl->store, &iter,
		       GB_JOBLISTCOLUMN_JOBID,
		       AB_Job_GetJobId(j),
                       GB_JOBLISTCOLUMN_JOBTYPE,
                       jobtype,
		       GB_JOBLISTCOLUMN_BANK,
                       bankName,
		       GB_JOBLISTCOLUMN_ACCOUNT,
                       accountName,
		       GB_JOBLISTCOLUMN_STATUS,
		       status,
		       GB_JOBLISTCOLUMN_BACKEND,
                       backend,
                       GB_JOBLISTCOLUMN_APP,
                       app,
		       -1);
    j=AB_Job_List2Iterator_Next(jit);
  } /* while */
  AB_Job_List2Iterator_free(jit);
}



void GBanking_JobList_free(GBANKING_JOBLIST *jl) {
  if (jl->jobs)
    AB_Job_List2_free(jl->jobs);
  GWEN_FREE_OBJECT(jl);
}



void GBanking_JobList_freeData(gpointer data) {
  GBANKING_JOBLIST *jl;

  jl=(GBANKING_JOBLIST*)data;
  GBanking_JobList_free(jl);
}



void GBanking_JobList__selection_changed_cb(GtkTreeSelection *selection,
                                            gpointer data) {
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *jobType;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)){
    gtk_tree_model_get(model, &iter,
                       1, /* column */
                       &jobType,
                       -1);

    g_free(jobType);
  }
}



AB_JOB *GBanking_JobList__FindJob(AB_JOB_LIST2 *sjl, GWEN_TYPE_UINT32 id) {
  AB_JOB_LIST2_ITERATOR *jit;
  AB_JOB *j;

  jit=AB_Job_List2_First(sjl);
  assert(jit);
  j=AB_Job_List2Iterator_Data(jit);
  assert(j);
  while(j) {
    if (AB_Job_GetJobId(j)==id)
      return j;
    j=AB_Job_List2Iterator_Next(jit);
  } /* while */
  AB_Job_List2Iterator_free(jit);
  return 0;
}



AB_JOB_LIST2 *GBanking_JobList_GetSelectedJobs(GtkWidget *w) {
  GBANKING_JOBLIST *jl;
  AB_JOB_LIST2 *sjl;
  GtkTreeIter iter;
  gboolean valid;

  jl=gtk_object_get_data(GTK_OBJECT(w), GBANKING_JOBLIST_ID);
  g_assert(jl);

  sjl=AB_Job_List2_new();

  /* Get the first iter in the list */
  valid=gtk_tree_model_get_iter_first(GTK_TREE_MODEL(jl->store), &iter);

  while (valid){
    /* Walk through the list, reading each row */
    gint int_data;
    AB_JOB *j;

    /* Make sure you terminate calls to gtk_tree_model_get()
     * with a '-1' value
     */
    gtk_tree_model_get(GTK_TREE_MODEL(jl->store), &iter,
		       GB_JOBLISTCOLUMN_JOBID, &int_data,
		       -1);

    // get job from list, add here
    j=GBanking_JobList__FindJob(jl->jobs, int_data);
    if (j)
      AB_Job_List2_PushBack(sjl, j);

    valid=gtk_tree_model_iter_next(GTK_TREE_MODEL(jl->store), &iter);
  }

  if (AB_Job_List2_GetSize(sjl)==0) {
    AB_Job_List2_free(sjl);
    return 0;
  }

  return sjl;
}



void GBanking_JobList_Update(GtkWidget *w) {
  GBANKING_JOBLIST *jl;

  jl=gtk_object_get_data(GTK_OBJECT(w), GBANKING_JOBLIST_ID);
  g_assert(jl);

  GBanking_JobList__populate(jl);
}











