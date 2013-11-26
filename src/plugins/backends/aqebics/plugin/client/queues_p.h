/***************************************************************************
    begin       : Wed May 07 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_QUEUES_P_H
#define AO_QUEUES_P_H


#include "queues_l.h"
#include "context_l.h"


struct EBC_QUEUE {
  EBC_USERQUEUE_LIST *userQueues;
};


struct EBC_USERQUEUE {
  GWEN_LIST_ELEMENT(EBC_USERQUEUE)
  AB_USER *user;

  EBC_ACCOUNTQUEUE_LIST *accountQueues;
};


struct EBC_ACCOUNTQUEUE {
  GWEN_LIST_ELEMENT(EBC_ACCOUNTQUEUE)
  AB_ACCOUNT *account;
  EBC_CONTEXT_LIST *contexts;
};


#endif
