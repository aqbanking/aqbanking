/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQB_TOOL_GLOBALS_H
#define AQB_TOOL_GLOBALS_H


#include <aqbanking/banking.h>
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
AB_TRANSACTION *mkSepaTransfer(GWEN_DB_NODE *db, int cmd);

AB_TRANSACTION *mkSepaDebitNote(GWEN_DB_NODE *db);


/**
 * Get selected AqBanking account sepcs matching the user given parameters in command line db.
 */
int getSelectedAccounts(AB_BANKING *ab, GWEN_DB_NODE *db, AB_ACCOUNT_SPEC_LIST **pAccountSpecList);


/**
 * Replace variables in the given text.
 * Variables are used like this: "$(VAR1)".
 */
int replaceVars(const char *s, GWEN_DB_NODE *db, GWEN_BUFFER *dbuf);




#endif




