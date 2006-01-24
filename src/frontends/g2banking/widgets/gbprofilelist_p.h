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

#ifndef GB_PROFILELIST_P_H
#define GB_PROFILELIST_P_H

#define GB_PROFILELIST_ID "GB_PROFILELIST"

#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <aqbanking/job.h>

#include "gbprofilelist.h"





typedef struct GB_PROFILELIST GB_PROFILELIST;
struct GB_PROFILELIST {
  AB_BANKING *banking;
  GtkWidget *tree;
  GtkListStore *store;
  GWEN_DB_NODE *dbProfiles;
};


void GB_ProfileList_freeData(gpointer data);


void GB_ProfileList__addProfile(GB_PROFILELIST *pl,
                                GWEN_DB_NODE *dbT);
void GB_ProfileList__populate(GB_PROFILELIST *jl);

static void
GB_ProfileList__selection_changed_cb(GtkTreeSelection *selection,
                                           gpointer data);

void GB_ProfileList__selected_foreach_func(GtkTreeModel  *model,
                                           GtkTreePath   *path,
                                           GtkTreeIter   *iter,
                                           gpointer       userdata);

GWEN_DB_NODE *GB_ProfileList__FindProfile(GB_PROFILELIST *pl,
                                          const char *name);






#endif



