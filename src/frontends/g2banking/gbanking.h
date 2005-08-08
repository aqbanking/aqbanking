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



#ifndef GBANKING_H
#define GBANKING_H

#include <gtk/gtk.h>
#include <aqbanking/banking.h>
#include <aqbanking/imexporter.h>


#define GBANKING_LOGDOMAIN "gbanking"


typedef int (*GBANKING_IMPORTCONTEXT_FN)(AB_BANKING *ab,
                                         AB_IMEXPORTER_CONTEXT *ctx);


AB_BANKING *GBanking_new(const char *appName,
                         const char *fname);


GWEN_TYPE_UINT32 GBanking_GetLastAccountUpdate(const AB_BANKING *ab);
GWEN_TYPE_UINT32 GBanking_GetLastQueueUpdate(const AB_BANKING *ab);

void GBanking_AccountsUpdated(AB_BANKING *ab);
void GBanking_QueueUpdated(AB_BANKING *ab);

int GBanking_ImportContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx);

void GBanking_SetImportContextFn(AB_BANKING *ab,
                                 GBANKING_IMPORTCONTEXT_FN cb);


#endif /* GBANKING_H */









