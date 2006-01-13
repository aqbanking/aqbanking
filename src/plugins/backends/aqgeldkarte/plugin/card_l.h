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

#ifndef AG_CARD_L_H
#define AG_CARD_L_H

#include <aqbanking/account_be.h>
#include <aqbanking/transaction.h>
#include <aqbanking/job.h>
#include <aqgeldkarte/provider.h>
#include <aqgeldkarte/account.h>
#include <gwenhywfar/misc.h>


typedef struct AG_CARD AG_CARD;
GWEN_LIST_FUNCTION_DEFS(AG_CARD, AG_Card)

AG_CARD *AG_Card_new(AB_ACCOUNT *acc);
void AG_Card_free(AG_CARD *dj);

AB_JOB_LIST2 *AG_Card_GetBankingJobs(const AG_CARD *dj);
AB_ACCOUNT *AG_Card_GetAccount(const AG_CARD *dj);

void AG_Card_AddJob(AG_CARD *dj, AB_JOB *bj);


#endif
