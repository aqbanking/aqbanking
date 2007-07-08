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

#ifndef QBANKING_BANKING_P_H
#define QBANKING_BANKING_P_H


#include <aqbanking/banking.h>
#include "qbanking.h"

/* Note: We use the key "AqBanking" because from the windows registry
 * point of view, these plugins all belong to the large AqBanking
 * package. */
#define QBANKING_REGKEY_PATHS        "Software\\AqBanking\\Paths"
#define QBANKING_REGKEY_CFGMODULEDIR "cfgmoduledir"
#define QBANKING_CFGMODULEDIR        "cfgmodules"

#endif /* QBANKING_BANKING_P_H */


