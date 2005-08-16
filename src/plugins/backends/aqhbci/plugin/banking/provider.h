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

#ifndef AH_PROVIDER_H
#define AH_PROVIDER_H

#include <aqhbci/hbci.h>
#include <aqbanking/banking.h>
#include <aqbanking/provider.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct AH_PROVIDER AH_PROVIDER;


AQHBCI_API
AB_PROVIDER *AH_Provider_new(AB_BANKING *ab, const char *name);



AQHBCI_API
AH_HBCI *AH_Provider_GetHbci(AB_PROVIDER *pro);


#ifdef __cplusplus
}
#endif






#endif /* AH_PROVIDER_H */




