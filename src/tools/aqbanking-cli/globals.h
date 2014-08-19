/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQB_TOOL_GLOBALS_H
#define AQB_TOOL_GLOBALS_H


#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqhbci/provider.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/cgui.h>

#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg



int readContext(const char *ctxFile,
		AB_IMEXPORTER_CONTEXT **pCtx,
		int mustExist);
int writeContext(const char *ctxFile, const AB_IMEXPORTER_CONTEXT *ctx);
AB_TRANSACTION *mkTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db, AB_JOB_TYPE *jobType);
AB_TRANSACTION *mkSepaTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db, int expTransferType);
AB_TRANSACTION *mkSepaDebitNote(AB_ACCOUNT *a, GWEN_DB_NODE *db);




#endif




