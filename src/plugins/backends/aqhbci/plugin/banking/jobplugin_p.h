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


#ifndef AH_JOBPLUGIN_P_H
#define AH_JOBPLUGIN_P_H

#include <aqhbci/jobplugin.h>


struct AH_JOBPLUGIN {
  GWEN_LIST_ELEMENT(AH_JOBPLUGIN);
  GWEN_INHERIT_ELEMENT(AH_JOBPLUGIN);

  AH_PROVIDER *provider;

  GWEN_LIBLOADER *libLoader;

  char *name;
  char *description;

  AH_JOBPLUGIN_FACTORYFN factoryFn;
  AH_JOBPLUGIN_CHECKFN checkFn;
};


#endif
