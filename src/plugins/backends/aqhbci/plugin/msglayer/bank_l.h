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


#ifndef AH_BANK_L_H
#define AH_BANK_L_H

#include <aqhbci/bank.h>


AH_BANK *AH_Bank_fromDb(AH_HBCI *hbci, GWEN_DB_NODE *db);
int AH_Bank_toDb(const AH_BANK *b, GWEN_DB_NODE *db);





#endif /* AH_BANK_L_H */


