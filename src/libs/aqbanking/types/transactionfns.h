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



#ifndef AQBANKING_TRANSACTIONFNS_H
#define AQBANKING_TRANSACTIONFNS_H

#include <aqbanking/transaction.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @return 0 if both transactions are equal, 1 otherwise (and -1 on error)
 */
AQBANKING_API 
int AB_Transaction_Compare(const AB_TRANSACTION *t1,
                           const AB_TRANSACTION *t0);



#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* AQBANKING_TRANSACTIONFNS_H */
