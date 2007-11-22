/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_IMEXPORTER_L_H
#define AQBANKING_IMEXPORTER_L_H

#define AB_IMEXPORTER_FOLDER "imexporters"


#include <aqbanking/imexporter.h>
#include <aqbanking/imexporter_be.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/libloader.h>


GWEN_LIST_FUNCTION_LIB_DEFS(AB_IMEXPORTER, AB_ImExporter, AQBANKING_API)


void AB_ImExporter_SetLibLoader(AB_IMEXPORTER *ie, GWEN_LIBLOADER *ll);

int AB_ImExporterAccountInfo_toDb(const AB_IMEXPORTER_ACCOUNTINFO *iea,
                                  GWEN_DB_NODE *db);
AB_IMEXPORTER_ACCOUNTINFO*
  AB_ImExporterAccountInfo_fromDb(GWEN_DB_NODE *db);


#endif /* AQBANKING_IMEXPORTER_L_H */


