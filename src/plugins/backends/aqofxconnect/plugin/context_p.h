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

#ifndef AO_CONTEXT_P_H
#define AO_CONTEXT_P_H


#include "context_l.h"
#include "queues_l.h"


struct AO_CONTEXT {
  AO_BANK *bank;
  AO_USER *user;
  AB_IMEXPORTER_CONTEXT *ictx;
  AB_JOB *job;

  LibofxContextPtr ofxctx;
  struct OfxFiLogin *fi;
  struct OfxAccountInfo *ai;

  AB_IMEXPORTER_ACCOUNTINFO *lastAccountInfo;
  int lastErrorCode;
  int abort;
};



int AO_Context_StatusCallback(const struct OfxStatusData data,
                              void *user_data);
int AO_Context_AccountCallback(const struct OfxAccountData data,
                               void *user_data);
int AO_Context_SecurityCallback(const struct OfxSecurityData data,
                                void *user_data);
int AO_Context_TransactionCallback(const struct OfxTransactionData data,
                                   void *user_data);
int AO_Context_StatementCallback(const struct OfxStatementData data,
                                 void *user_data);




#endif
