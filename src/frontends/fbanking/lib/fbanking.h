/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbanking.h 935 2006-02-14 02:11:55Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FBANKING_BANKING_H
#define FBANKING_BANKING_H



#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <fx.h>

class FBanking;

#include "banking.h"


class FBANKING_API FBanking: public Banking {
  FXDECLARE(FBanking)

protected:
  FBanking(): Banking("none", NULL) {};

public:
  FBanking(const char *appName,
           const char *dirName=NULL);
  ~FBanking();

  int init();
  int fini();

};



#endif


