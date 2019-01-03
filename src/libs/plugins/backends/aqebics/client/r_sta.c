/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <aqbanking/banking_imex.h>




int EBC_Provider_XchgStaRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				const GWEN_DATE *fromDate,
				const GWEN_DATE *toDate,
				AB_IMEXPORTER_CONTEXT *ctx) {
  AB_BANKING *ab;
  int rv;
  GWEN_BUFFER *buf;
  AB_USER *u;

  ab=AB_Provider_GetBanking(pro);
  assert(ab);
  u=AB_HttpSession_GetUser(sess);
  assert(u);
  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_SetHardLimit(buf, EBICS_BUFFER_MAX_HARD_LIMIT);

  /* TODO: get RECEIPT flag from account settings */
  rv=EBC_Provider_XchgDownloadRequest(pro, sess, u, "STA", buf, 1, fromDate, toDate);
  if (rv<0 || rv>=300) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Importing transactions");
    rv=AB_Banking_ImportFromBufferLoadProfile(ab, "swift", ctx,
                                              "SWIFT-MT940", NULL,
                                              (const uint8_t*) GWEN_Buffer_GetStart(buf),
                                              GWEN_Buffer_GetUsedBytes(buf));
    GWEN_Buffer_free(buf);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_INFO(AQEBICS_LOGDOMAIN, "Importing transactions: done");
    return 0;
  }
}



