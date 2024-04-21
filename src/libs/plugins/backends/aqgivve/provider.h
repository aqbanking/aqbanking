/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_PROVIDER_H
#define AG_PROVIDER_H


#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/provider_be.h>

#define AQGIVVE_BACKENDNAME "aqgivve"
#define AQGIVVE_LOGDOMAIN "aqgivve"

typedef struct AG_PROVIDER AG_PROVIDER;
    
AB_PROVIDER *AG_Provider_new(AB_BANKING *ab);




#endif /* AH_PROVIDER_H */




