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

#ifndef AQ_PROVIDER_CPP_P_H
#define AQ_PROVIDER_CPP_P_H


#include <gwenhywfar/inherit.h>
#include <aqbanking/provider.h>
#include "provider.h"



namespace AB {

  class ProviderWizard_Linker {
    friend class ProviderWizard;
  private:
    static int setup(AB_PROVIDER_WIZARD *pw);
  };

} /* namespace */



#endif /* AQ_PROVIDER_CPP_P_H */
