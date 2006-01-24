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

#include "gbprofilelist_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GtkWidget *GB_ProfileList_new(GtkWidget *parent,
                              GtkWidget *tree) {
  GB_PROFILELIST *pl;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *sel;

  GWEN_NEW_OBJECT(GB_PROFILELIST, pl);
  pl->store=gtk_list_store_new(3,
                               G_TYPE_STRING, /* name */
                               G_TYPE_STRING, /* version */
                               G_TYPE_STRING  /* short description */
                              );

  if (tree) {
    pl->tree=tree;
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree),
                            GTK_TREE_MODEL(pl->store));
  }
  else {
    pl->tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(pl->store));
    if (parent)
      gtk_widget_reparent(pl->tree, parent);
  }

  g_object_set_data(G_OBJECT(pl->store), GB_PROFILELIST_ID, pl);
  gtk_object_set_data_full(GTK_OBJECT(pl->tree),
                           GB_PROFILELIST_ID,
                           pl,
                           GB_ProfileList_freeData);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Name"),
                                                  renderer,
						  "text",
						  GB_PROFILELIST_COL_NAME,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Version"),
                                                  renderer,
						  "text",
						  GB_PROFILELIST_COL_VERSION,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pl->tree), column);

  renderer=gtk_cell_renderer_text_new();
  column=gtk_tree_view_column_new_with_attributes(I18N("Short Description"),
                                                  renderer,
						  "text",
                                                  GB_PROFILELIST_COL_SHORT,
                                                  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pl->tree), column);

  /* Setup the selection handler */
  sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(pl->tree));
  gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (sel), "changed",
		    G_CALLBACK(GB_ProfileList__selection_changed_cb),
		    NULL);
  pl->dbProfiles=GWEN_DB_Group_new("profiles");
  return pl->tree;
}



void GB_ProfileList__addProfile(GB_PROFILELIST *pl,
                                GWEN_DB_NODE *dbT) {
  GtkTreeIter iter;
  const char *name;
  const char *version;
  const char *shortDescr;

  name=GWEN_DB_GetCharValue(dbT, "name", 0, "");
  version=GWEN_DB_GetCharValue(dbT, "version", 0, "");
  shortDescr=GWEN_DB_GetCharValue(dbT, "shortDescr", 0, "");

  /* really set data */
  gtk_list_store_append(pl->store, &iter);
  gtk_list_store_set(pl->store, &iter,
                     GB_PROFILELIST_COL_NAME,
                     name,
                     GB_PROFILELIST_COL_VERSION,
                     version,
                     GB_PROFILELIST_COL_SHORT,
                     shortDescr,
                     -1);
}



void GB_ProfileList__populate(GB_PROFILELIST *pl) {
  GWEN_DB_NODE *dbT;

  gtk_list_store_clear(pl->store);

  dbT=GWEN_DB_GetFirstGroup(pl->dbProfiles);
  while(dbT) {
    GB_ProfileList__addProfile(pl, dbT);
    dbT=GWEN_DB_GetNextGroup(dbT);
  } /* while */
}



void GB_ProfileList_freeData(gpointer data) {
  GB_PROFILELIST *pl;

  pl=(GB_PROFILELIST*)data;

  GWEN_DB_Group_free(pl->dbProfiles);
  GWEN_FREE_OBJECT(pl);
}



void GB_ProfileList__selection_changed_cb(GtkTreeSelection *selection,
                                          gpointer data) {
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *name;

  if (gtk_tree_selection_get_selected(selection, &model, &iter)){
    gtk_tree_model_get(model, &iter,
                       GB_PROFILELIST_COL_NAME, /* column */
                       &name,
                       -1);
    DBG_ERROR(0, "Profile selected: %s", name);
    g_free(name);
  }
}



GWEN_DB_NODE *GB_ProfileList__FindProfile(GB_PROFILELIST *pl,
                                          const char *name) {
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_GetFirstGroup(pl->dbProfiles);
  while(dbT) {
    const char *ename;

    ename=GWEN_DB_GetCharValue(dbT, "name", 0, "");
    if (name && ename && strcasecmp(name, ename)==0)
      return dbT;
    dbT=GWEN_DB_GetNextGroup(pl->dbProfiles);
  } /* while */
  return 0;
}



void GB_ProfileList__selected_foreach_func(GtkTreeModel  *model,
                                           GtkTreePath   *path,
                                           GtkTreeIter   *iter,
                                           gpointer       userdata){
  gchar *name;
  GWEN_DB_NODE *dbResult;
  GB_PROFILELIST *pl;

  pl=gtk_object_get_data(GTK_OBJECT(model), GB_PROFILELIST_ID);
  g_assert(pl);

  dbResult=(GWEN_DB_NODE *)userdata;

  gtk_tree_model_get(model, iter,
                     GB_PROFILELIST_COL_NAME, &name,
                     -1);
  if (name) {
    GWEN_DB_NODE *dbT;

    dbT=GB_ProfileList__FindProfile(pl, name);
    if (dbT)
      GWEN_DB_AddGroup(dbResult, GWEN_DB_Group_dup(dbT));
    g_free(name);
  }
}



GWEN_DB_NODE *GB_ProfileList_GetSelectedProfiles(GtkWidget *w) {
  GB_PROFILELIST *pl;
  GWEN_DB_NODE *dbResult;
  GtkTreeSelection *selection;

  pl=gtk_object_get_data(GTK_OBJECT(w), GB_PROFILELIST_ID);
  g_assert(pl);

  dbResult=GWEN_DB_Group_new("profiles");

  selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(pl->tree));
  gtk_tree_selection_selected_foreach(selection,
                                      GB_ProfileList__selected_foreach_func,
                                      dbResult);

  if (GWEN_DB_Groups_Count(dbResult)==0) {
    GWEN_DB_Group_free(dbResult);
    DBG_ERROR(0, "Nothing selected");
    return 0;
  }

  return dbResult;
}



void GB_ProfileList_Update(GtkWidget *w) {
  GB_PROFILELIST *pl;

  pl=gtk_object_get_data(GTK_OBJECT(w), GB_PROFILELIST_ID);
  g_assert(pl);

  DBG_ERROR(0, "Updating...");
  GB_ProfileList__populate(pl);
}



void GB_ProfileList_AddProfile(GtkWidget *w, GWEN_DB_NODE *db) {
  GB_PROFILELIST *pl;
  GWEN_DB_NODE *dbNew;

  pl=gtk_object_get_data(GTK_OBJECT(w), GB_PROFILELIST_ID);
  g_assert(pl);

  dbNew=GWEN_DB_Group_dup(db);
  GWEN_DB_AddGroup(pl->dbProfiles, dbNew);
  GB_ProfileList__addProfile(pl, dbNew);
}



void GB_ProfileList_Clear(GtkWidget *w, GWEN_DB_NODE *db) {
  GB_PROFILELIST *pl;

  pl=gtk_object_get_data(GTK_OBJECT(w), GB_PROFILELIST_ID);
  g_assert(pl);
  gtk_list_store_clear(pl->store);
  GWEN_DB_ClearGroup(pl->dbProfiles, 0);
}



void GB_ProfileList_AddProfiles(GtkWidget *w, GWEN_DB_NODE *db) {
  GB_PROFILELIST *pl;
  GWEN_DB_NODE *dbT;

  pl=gtk_object_get_data(GTK_OBJECT(w), GB_PROFILELIST_ID);
  g_assert(pl);

  dbT=GWEN_DB_GetFirstGroup(db);
  while(dbT) {
    GB_ProfileList_AddProfile(w, dbT);
    dbT=GWEN_DB_GetNextGroup(dbT);
  }
}











