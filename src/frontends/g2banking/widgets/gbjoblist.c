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

#include "gbjoblist_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GtkWidget *GB_JobList_new(AB_BANKING *ab,
                          GtkWidget *parent,
                          GtkWidget *tree) {
  GB_JOBLIST *jl;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *sel;

  GWEN_NEW_OBJECT(GB_JOBLIST, jl);
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

  g_object_set_data(G_OBJECT(jl->store), GB_JOBLIST_ID, jl);
  gtk_object_set_data_full(GTK_OBJECT(jl->tree),
                           GB_JOBLIST_ID,
                           jl,
                           GB_JobList_freeData);

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
		    G_CALLBACK(GB_JobList__selection_changed_cb),
		    NULL);

  /* populate list */
  GB_JobList__populate(jl);
  gtk_widget_show(jl->tree);
  return jl->tree;
}



void GB_JobList__populate(GB_JOBLIST *jl) {
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

    jobtype=AB_Job_Type2LocalChar(AB_Job_GetType(j));

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



void GB_JobList_free(GB_JOBLIST *jl) {
  if (jl->jobs)
    AB_Job_List2_free(jl->jobs);
  GWEN_FREE_OBJECT(jl);
}



void GB_JobList_freeData(gpointer data) {
  GB_JOBLIST *jl;

  jl=(GB_JOBLIST*)data;
  GB_JobList_free(jl);
}



void GB_JobList__selection_changed_cb(GtkTreeSelection *selection,
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



AB_JOB *GB_JobList__FindJob(AB_JOB_LIST2 *sjl, GWEN_TYPE_UINT32 id) {
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



void GB_JobList__selected_foreach_func(GtkTreeModel  *model,
                                       GtkTreePath   *path,
                                       GtkTreeIter   *iter,
                                       gpointer       userdata) {
  GB_JOBLIST *jl;
  AB_JOB_LIST2 *sjl;
  gint int_data;
  AB_JOB *j;

  jl=gtk_object_get_data(GTK_OBJECT(model), GB_JOBLIST_ID);
  g_assert(jl);

  sjl=(AB_JOB_LIST2*)userdata;

  /* Make sure you terminate calls to gtk_tree_model_get()
   * with a '-1' value
   */
  gtk_tree_model_get(GTK_TREE_MODEL(jl->store), iter,
                     GB_JOBLISTCOLUMN_JOBID, &int_data,
                     -1);

  /* get job from list, add here */
  j=GB_JobList__FindJob(jl->jobs, int_data);
  if (j)
    AB_Job_List2_PushBack(sjl, j);
}



AB_JOB_LIST2 *GB_JobList_GetSelectedJobs(GtkWidget *w) {
  GB_JOBLIST *jl;
  AB_JOB_LIST2 *sjl;
  GtkTreeSelection *selection;

  jl=gtk_object_get_data(GTK_OBJECT(w), GB_JOBLIST_ID);
  g_assert(jl);

  sjl=AB_Job_List2_new();

  selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(jl->tree));
  gtk_tree_selection_selected_foreach(selection,
                                      GB_JobList__selected_foreach_func,
                                      sjl);

  if (AB_Job_List2_GetSize(sjl)==0) {
    AB_Job_List2_free(sjl);
    return 0;
  }

  return sjl;
}



void GB_JobList_Update(GtkWidget *w) {
  GB_JOBLIST *jl;

  jl=gtk_object_get_data(GTK_OBJECT(w), GB_JOBLIST_ID);
  g_assert(jl);

  GB_JobList__populate(jl);
}











