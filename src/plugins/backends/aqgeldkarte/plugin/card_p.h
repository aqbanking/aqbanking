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

#ifndef AG_CARD_P_H
#define AG_CARD_P_H

#include <aqgeldkarte/card.h>


struct AG_CARD {
  GWEN_LIST_ELEMENT(AG_CARD);
  AB_ACCOUNT *account;
  AB_JOB_LIST2 *bankingJobs;
};



#endif
