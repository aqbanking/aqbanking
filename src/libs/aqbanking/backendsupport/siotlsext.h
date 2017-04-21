/***************************************************************************
    begin       : Fri Apr 21 2017
    copyright   : (C) 2017 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_SIOTLSEXT_H
#define AQBANKING_SIOTLSEXT_H

#include <aqbanking/error.h>
#include <aqbanking/user.h>

#include <gwenhywfar/syncio.h>



#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API void AB_SioTlsExt_Extend(GWEN_SYNCIO *sio, AB_USER *u);
AQBANKING_API void AB_SioTlsExt_Unextend(GWEN_SYNCIO *sio);


#ifdef __cplusplus
}
#endif



#endif /* AQBANKING_SIOTLSEXT_H */


