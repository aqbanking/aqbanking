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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "provider_p.h"
#include <assert.h>

#include <gwenhywfar/inherit.h>


namespace AB {

  GWEN_INHERIT(AB_PROVIDER_WIZARD, ProviderWizard)


  int ProviderWizard_Linker::setup(AB_PROVIDER_WIZARD *pw){
    ProviderWizard *kw;

    assert(pw);
    kw=GWEN_INHERIT_GETDATA(AB_PROVIDER_WIZARD, ProviderWizard, pw);
    assert(kw);

    return kw->setup();
  }




  ProviderWizard::ProviderWizard(AB_PROVIDER *pro,
                                 const char *name){
    assert(pro);
    _providerWizard=AB_ProviderWizard_new(pro, name);
    GWEN_INHERIT_SETDATA(AB_PROVIDER_WIZARD, ProviderWizard,
                         _providerWizard, this, 0);
    AB_ProviderWizard_SetSetupFn(_providerWizard,
                                 ProviderWizard_Linker::setup);
  }

  ProviderWizard::~ProviderWizard(){
  }



  const char *ProviderWizard::getName(){
    return AB_ProviderWizard_GetName(_providerWizard);
  }



  AB_PROVIDER *ProviderWizard::getProvider(){
    return AB_ProviderWizard_GetProvider(_providerWizard);
  }



  GWEN_DB_NODE *ProviderWizard::getData(){
    return AB_ProviderWizard_GetData(_providerWizard);
  }



  AB_PROVIDER_WIZARD *ProviderWizard::getProviderWizardPtr(){
    return _providerWizard;
  }



} /* namespace */










