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

#ifndef AO_QUEUES_P_H
#define AO_QUEUES_P_H


#include "queues_l.h"


struct AO_QUEUE {
  AO_USERQUEUE_LIST *userQueues;
};


struct AO_USERQUEUE {
  GWEN_LIST_ELEMENT(AO_USERQUEUE)
  AB_USER *user;
  AB_JOB_LIST2 *jobs;
};



#endif
