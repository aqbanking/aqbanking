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

#ifndef AQ_PROVIDER_CPP_H
#define AQ_PROVIDER_CPP_H


#include <aqbanking/provider.h>


namespace AB {


  class ProviderWizard {
    friend class ProviderWizard_Linker;
  private:
    AB_PROVIDER_WIZARD *_providerWizard;
  protected:
    virtual int setup();

  public:
    ProviderWizard(AB_PROVIDER *pro,
                   const char *name);
    virtual ~ProviderWizard();

    const char *getName();
    AB_PROVIDER *getProvider();
    GWEN_DB_NODE *getData();

    /**
     * Returns a pointer to the underlying C object. This is needed
     * for plugins which actually implement a wizard
     */
    AB_PROVIDER_WIZARD *getProviderWizardPtr();


  };



} /* namespace */


#endif /* AQ_PROVIDER_CPP_H */


