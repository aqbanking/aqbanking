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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "fbanking.h"




FBanking::FBanking(const char *appName,
		   const char *dirName)
:AB_Banking(appName, dirName)
{
}



FBanking::~FBanking() {
}



int FBanking::init() {
  return AB_Banking::init();
}



int FBanking::fini() {
  return AB_Banking::fini();
}







