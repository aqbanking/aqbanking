/***************************************************************************
    begin       : Fri Apr 21 2017
    copyright   : (C) 2017 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_SIOTLSEXT_P_H
#define AQBANKING_SIOTLSEXT_P_H

#include <aqbanking/backendsupport/siotlsext.h>

#include <gwenhywfar/syncio_tls.h>


typedef struct AB_SIOTLS_EXT AB_SIOTLS_EXT;
struct AB_SIOTLS_EXT {
  AB_USER *user;
  GWEN_SIO_TLS_CHECKCERT_FN oldCheckCertFn;
};


static void GWENHYWFAR_CB AB_SioTlsExt_FreeData(void *bp, void *p);
static int  GWENHYWFAR_CB AB_SioTlsExt_CheckCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert);


/* static AB_USER *AB_SioTlsExt_GetUser(const GWEN_SYNCIO *sio); */



#endif /* AQBANKING_SIOTLSEXT_P_H */


