/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQFINTS_PROVIDER_H
#define AQFINTS_PROVIDER_H


#define AF_PROVIDER_NAME "aqfints"


#include <aqbanking/backendsupport/provider.h>




AB_PROVIDER *AF_Provider_new(AB_BANKING *ab);




#endif

