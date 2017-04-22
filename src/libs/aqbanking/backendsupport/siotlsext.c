/***************************************************************************
    begin       : Fri Apr 21 2017
    copyright   : (C) 2017 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "siotlsext_p.h"

#include <gwenhywfar/gui.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(GWEN_SYNCIO, AB_SIOTLS_EXT)




void AB_SioTlsExt_Extend(GWEN_SYNCIO *sio, AB_USER *u) {
  AB_SIOTLS_EXT *xsio;

  GWEN_NEW_OBJECT(AB_SIOTLS_EXT, xsio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio, xsio, AB_SioTlsExt_FreeData);

  /* set data */
  xsio->user=u;

  /* set callbacks */
  xsio->oldCheckCertFn=GWEN_SyncIo_Tls_SetCheckCertFn(sio, AB_SioTlsExt_CheckCert);
}



void AB_SioTlsExt_Unextend(GWEN_SYNCIO *sio) {
  AB_SIOTLS_EXT *xsio;

  assert(sio);
  xsio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio);
  assert(xsio);

  /* reset callbacks which point into AB_SioTlsExt */
  GWEN_SyncIo_Tls_SetCheckCertFn(sio, xsio->oldCheckCertFn);

  /* unlink from GWEN_SYNCIO object */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Unlinking SIO from banking object");
  GWEN_INHERIT_UNLINK(GWEN_SYNCIO, AB_SIOTLS_EXT, sio);
}



void GWENHYWFAR_CB AB_SioTlsExt_FreeData(void *bp, void *p) {
  AB_SIOTLS_EXT *xsio;

  xsio=(AB_SIOTLS_EXT*) p;
  assert(xsio);
  GWEN_FREE_OBJECT(xsio);
}



/*
AB_USER *AB_SioTlsExt_GetUser(const GWEN_SYNCIO *sio) {
  AB_SIOTLS_EXT *xsio;

  assert(sio);
  xsio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio);
  assert(xsio);

  return xsio->user;
}
*/





int GWENHYWFAR_CB AB_SioTlsExt_CheckCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert) {
  AB_SIOTLS_EXT *xsio;
  GWEN_DB_NODE *dbCerts;
  GWEN_DB_NODE *dbC;
  const char *sFingerprint;
  const char *sStatus;
  uint32_t iStatus;

  assert(sio);
  xsio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio);
  assert(xsio);

  assert(xsio->user);

  sFingerprint=GWEN_SslCertDescr_GetFingerPrint(cert);
  if (!(sFingerprint && *sFingerprint)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No fingerpint in certificate");
    return GWEN_ERROR_BAD_DATA;
  }

  sStatus=GWEN_SslCertDescr_GetStatusText(cert);
  if (!(sStatus && *sStatus)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No status text in certificate");
    return GWEN_ERROR_BAD_DATA;
  }

  iStatus=GWEN_SslCertDescr_GetStatusFlags(cert);
  if (iStatus==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Empty status flags in certificate \"%s\"", sFingerprint);
    return GWEN_ERROR_BAD_DATA;
  }

  /* get or create user-based certificate store */
  dbCerts=AB_User_GetCertDb(xsio->user);
  if (dbCerts==NULL) {
    dbCerts=GWEN_DB_Group_new("certs");
    AB_User_SetCertDb(xsio->user, dbCerts);
  }

  /* find group which contains the certificate with the given fingerprint */
  dbC=GWEN_DB_GetGroup(dbCerts, GWEN_PATH_FLAGS_PATHMUSTEXIST, sFingerprint);
  if (dbC) {
    GWEN_SSLCERTDESCR *storedCert;
    uint32_t iStoredStatus;
    GWEN_DATE *dt;

    /* there is such a group, read stored certificate */
    storedCert=GWEN_SslCertDescr_fromDb(dbC);
    if (storedCert==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to load stored certificate \"%s\"", sFingerprint);
      return GWEN_ERROR_INTERNAL;
    }

    /* store last access date */
    dt=GWEN_Date_CurrentDate();
    GWEN_DB_SetCharValue(dbC, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastAccessDate",
                         GWEN_Date_GetString(dt));
    GWEN_Date_free(dt);

    /* get status of stored certificate */
    iStoredStatus=GWEN_SslCertDescr_GetStatusFlags(storedCert);

    /* compare status texts */
    if (iStatus==iStoredStatus) {
      int rv;

      /* found matching cert, return user's previous answer */
      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Found matching certificate \"%s\" with same status", sFingerprint);
      rv=GWEN_DB_GetIntValue(dbC, "userResponse", 0, -1);
      if (rv==0) {
        /* last user response was to accept the certificate so we're done */
        DBG_NOTICE(AQBANKING_LOGDOMAIN,
                   "Automatically accepting certificate [%s]",
                   sFingerprint);
        GWEN_SslCertDescr_free(storedCert);
        return 0;
      }
      else {
        /* last user response was to reject the certificate so we're done */
        DBG_NOTICE(AQBANKING_LOGDOMAIN,
                   "Automatically rejecting certificate [%s] (%d)",
                   sFingerprint, rv);
        GWEN_SslCertDescr_free(storedCert);
        return rv;
      }
    } /* if same status */
    else {
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
                 "Status for certificate \%s\" has changed to \"%s\" (%08x->%08x), need to present",
                 sFingerprint, sStatus,
                 iStoredStatus, iStatus);
    }
    GWEN_SslCertDescr_free(storedCert);
  } /* if dbC */

  {
    GWEN_GUI *gui;

    /* at this point the certificate was either not found or its status has changed,
     * possibly ask the user how to preceed */
    gui=GWEN_Gui_GetGui();
    assert(gui);

    if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_NONINTERACTIVE) {
      uint32_t fl;

      fl=GWEN_SslCertDescr_GetStatusFlags(cert);
      if (fl==GWEN_SSL_CERT_FLAGS_OK) {
        if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_ACCEPTVALIDCERTS) {
          DBG_NOTICE(AQBANKING_LOGDOMAIN,
                     "Automatically accepting valid new certificate [%s]",
                     sFingerprint);
          return 0;
        }
        else {
          DBG_NOTICE(AQBANKING_LOGDOMAIN,
                     "Automatically rejecting certificate [%s] (noninteractive)",
                     sFingerprint);
          return GWEN_ERROR_USER_ABORTED;
        }
      } /* if cert is valid */
      else {
        if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_REJECTINVALIDCERTS) {
          DBG_NOTICE(AQBANKING_LOGDOMAIN,
                     "Automatically rejecting invalid certificate [%s] (noninteractive)",
                     sFingerprint);
          return GWEN_ERROR_USER_ABORTED;
        }
      }
    } /* if non-interactive */

    /* use previous checkCert function, which normally presents the certificate
     * to the user and asks for a response */
    if (xsio->oldCheckCertFn) {
      int rv;
      GWEN_DATE *dt;

      /* get user response */
      rv=xsio->oldCheckCertFn(sio, cert);

      /* store certificate in database */
      dbC=GWEN_DB_GetGroup(dbCerts,
                           GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                           sFingerprint);
      assert(dbC);
      GWEN_SslCertDescr_toDb(cert, dbC);

      /* store user response */
      GWEN_DB_SetIntValue(dbC, GWEN_DB_FLAGS_OVERWRITE_VARS, "userResponse", rv);
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
                 "User response to presentation of cert \"%s\" (%s): %d",
                 sFingerprint, sStatus, rv);

      /* store last access date */
      dt=GWEN_Date_CurrentDate();
      GWEN_DB_SetCharValue(dbC, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastAccessDate",
                           GWEN_Date_GetString(dt));
      GWEN_Date_free(dt);

      return rv;
    } /* if oldCheckCertFn */
    else {
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
                 "Internal error: No previous checkCert function while checking cert \"%s\"",
                 sFingerprint);
      return GWEN_ERROR_INTERNAL;
    }
  }
}






