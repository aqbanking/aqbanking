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


#ifndef AQBANKING_IMEXPORTER_BE_H
#define AQBANKING_IMEXPORTER_BE_H


#include <aqbanking/imexporter.h>
#include <gwenhywfar/misc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef AB_IMEXPORTER* (*AB_IMEXPORTER_FACTORY_FN)(AB_BANKING *ab,
                                                   GWEN_DB_NODE *db);



/**
 * Takes over ownership of the given transaction.
 */
void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t);


/**
 * Takes over ownership of the given account.
 */
void AB_ImExporterContext_AddAccount(AB_IMEXPORTER_CONTEXT *iec,
                                     AB_ACCOUNT *a);

#ifdef __cplusplus
}
#endif



#endif /* AQBANKING_IMEXPORTER_BE_H */


