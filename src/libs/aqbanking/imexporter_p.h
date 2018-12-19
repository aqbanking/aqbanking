/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_IMEXPORTER_P_H
#define AQBANKING_IMEXPORTER_P_H


#define AH_IMEXPORTER_TRANSFORM_MAXLEVEL 16

#include "imexporter_l.h"

#include <aqbanking/transaction.h>

#include <gwenhywfar/misc.h>


struct AB_IMEXPORTER {
  GWEN_LIST_ELEMENT(AB_IMEXPORTER);
  GWEN_INHERIT_ELEMENT(AB_IMEXPORTER);

  AB_BANKING *banking;
  char *name;
  uint32_t flags;

  GWEN_LIBLOADER *libLoader;
  AB_IMEXPORTER_IMPORT_FN importFn;
  AB_IMEXPORTER_EXPORT_FN exportFn;
  AB_IMEXPORTER_CHECKFILE_FN checkFileFn;
  AB_IMEXPORTER_GET_EDITPROFILE_DIALOG_FN getEditProfileDialogFn;
};


static int AB_ImExporter__Transform_Var(GWEN_DB_NODE *db, int level);
static int AB_ImExporter__Transform_Group(GWEN_DB_NODE *db, int level);


typedef struct AB_PLUGIN_IMEXPORTER AB_PLUGIN_IMEXPORTER;
struct AB_PLUGIN_IMEXPORTER {
  AB_PLUGIN_IMEXPORTER_FACTORY_FN pluginFactoryFn;
};

static void GWENHYWFAR_CB AB_Plugin_ImExporter_FreeData(void *bp, void *p);


#endif /* AQBANKING_IMEXPORTER_P_H */




