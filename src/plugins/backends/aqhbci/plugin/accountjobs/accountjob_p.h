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


#ifndef AH_ACCOUNTJOBS_P_H
#define AH_ACCOUNTJOBS_P_H


#include "accountjob_l.h"


typedef struct AH_ACCOUNTJOB AH_ACCOUNTJOB;
struct AH_ACCOUNTJOB {
  AB_ACCOUNT *account;
};


static void GWENHYWFAR_CB AH_AccountJob_FreeData(void *bp, void *p);




#endif /* AH_ACCOUNTJOBS_P_H */


