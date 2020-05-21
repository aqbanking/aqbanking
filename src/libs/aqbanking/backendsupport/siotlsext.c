/***************************************************************************
    begin       : Fri Apr 21 2017
    copyright   : (C) 2019 by Martin Preuss
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




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

/** @return 0 if undecided, 1 if acceptable, negative on error */
static int _checkCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert);
static int _checkStoredUserCerts(AB_USER *u, const GWEN_SSLCERTDESCR *cert);
static int _checkAgainstStoredCert(const GWEN_SSLCERTDESCR *cert, GWEN_SSLCERTDESCR *storedCert, GWEN_DB_NODE *dbC);
static int _checkStoredUserResponse(GWEN_DB_NODE *dbC, const char *sFingerprint);
static int _checkAutoDecision(const GWEN_SSLCERTDESCR *cert);
static int _askUserAboutCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert);
static void _storeAccessDate(GWEN_DB_NODE *dbCert);
static void _storeCertAndUserResponseInUser(AB_USER *u, const GWEN_SSLCERTDESCR *cert, int response);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


GWEN_INHERIT(GWEN_SYNCIO, AB_SIOTLS_EXT)



void AB_SioTlsExt_Extend(GWEN_SYNCIO *sio, AB_USER *u)
{
  AB_SIOTLS_EXT *xsio;

  GWEN_NEW_OBJECT(AB_SIOTLS_EXT, xsio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio, xsio, AB_SioTlsExt_FreeData);

  /* set data */
  xsio->user=u;

  /* set callbacks */
  xsio->oldCheckCertFn=GWEN_SyncIo_Tls_SetCheckCertFn(sio, AB_SioTlsExt_CheckCert);
}



void AB_SioTlsExt_Unextend(GWEN_SYNCIO *sio)
{
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



void GWENHYWFAR_CB AB_SioTlsExt_FreeData(void *bp, void *p)
{
  AB_SIOTLS_EXT *xsio;

  xsio=(AB_SIOTLS_EXT *) p;
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





int GWENHYWFAR_CB AB_SioTlsExt_CheckCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert)
{
  int rv;

  rv=_checkCert(sio, cert);
  if (rv==1) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Cert accepted.");
    return 0;
  }
  else if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* undecided, abort */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Undecided, assuming abort");
  return GWEN_ERROR_USER_ABORTED;
}



int _checkCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert)
{
  AB_SIOTLS_EXT *xsio;
  int rv;

  assert(sio);
  xsio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio);
  assert(xsio);

  assert(xsio->user);

  rv=_checkStoredUserCerts(xsio->user, cert);
  if (rv!=0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_checkAutoDecision(cert);
  if (rv!=0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_askUserAboutCert(sio, cert);
  if (rv!=0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    _storeCertAndUserResponseInUser(xsio->user, cert, rv);
    return rv;
  }

  /* undecided */
  return 0;
}



int _checkStoredUserCerts(AB_USER *u, const GWEN_SSLCERTDESCR *cert)
{
  GWEN_DB_NODE *dbCerts;
  GWEN_DB_NODE *dbC;
  const char *sFingerprint;

  assert(u);

  sFingerprint=GWEN_SslCertDescr_GetFingerPrint(cert);

  /* get or create user-based certificate store */
  dbCerts=AB_User_GetCertDb(u);
  if (dbCerts==NULL) {
    dbCerts=GWEN_DB_Group_new("certs");
    AB_User_SetCertDb(u, dbCerts);
  }

  /* find group which contains the certificate with the given fingerprint */
  dbC=GWEN_DB_GetGroup(dbCerts, GWEN_PATH_FLAGS_PATHMUSTEXIST, sFingerprint);
  if (dbC) {
    GWEN_SSLCERTDESCR *storedCert;
    int rv;

    /* there is such a group, read stored certificate */
    storedCert=GWEN_SslCertDescr_fromDb(dbC);
    if (storedCert==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to load stored certificate \"%s\"", sFingerprint);
      return GWEN_ERROR_INTERNAL;
    }

    _storeAccessDate(dbC);

    rv=_checkAgainstStoredCert(cert, storedCert, dbC);
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_SslCertDescr_free(storedCert);
    return rv;
  } /* if dbC */

  /* undecided */
  return 0;
}



int _checkAgainstStoredCert(const GWEN_SSLCERTDESCR *cert, GWEN_SSLCERTDESCR *storedCert, GWEN_DB_NODE *dbC)
{
  const char *sFingerprint;
  const char *sStatus;
  uint32_t iStatus;
  uint32_t iStoredStatus;

  sFingerprint=GWEN_SslCertDescr_GetFingerPrint(cert);
  sStatus=GWEN_SslCertDescr_GetStatusText(cert);
  iStatus=GWEN_SslCertDescr_GetStatusFlags(cert);

  /* get status of stored certificate */
  iStoredStatus=GWEN_SslCertDescr_GetStatusFlags(storedCert);

  /* compare status texts */
  if (iStatus==iStoredStatus) {
    /* found matching cert, return user's previous answer */
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Found matching certificate \"%s\" with same status", sFingerprint);
    return _checkStoredUserResponse(dbC, sFingerprint);
  } /* if same status */
  else {
    DBG_NOTICE(AQBANKING_LOGDOMAIN,
               "Status for certificate \%s\" has changed to \"%s\" (%08x->%08x), need to present",
               sFingerprint, sStatus,
               iStoredStatus, iStatus);
  }
  return 0;
}



int _checkStoredUserResponse(GWEN_DB_NODE *dbC, const char *sFingerprint)
{
  int rv;

  rv=GWEN_DB_GetIntValue(dbC, "userResponse", 0, -1);
  if (rv==0) {
    /* last user response was to accept the certificate so we're done */
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Automatically accepting certificate [%s]", sFingerprint);
    return 1;
  }
  else {
    /* last user response was to reject the certificate so we're done */
    /* DBG_NOTICE(AQBANKING_LOGDOMAIN, "Automatically rejecting certificate [%s] (%d)", sFingerprint, rv);
     return rv; */
    /* undecided (ask user again) */
    return 0;
  }
}



int _checkAutoDecision(const GWEN_SSLCERTDESCR *cert)
{
  GWEN_GUI *gui;
  const char *sFingerprint;

  sFingerprint=GWEN_SslCertDescr_GetFingerPrint(cert);

  /* at this point the certificate was either not found or its status has changed,
   * possibly ask the user how to preceed */
  gui=GWEN_Gui_GetGui();
  assert(gui);

  if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_NONINTERACTIVE) {
    uint32_t fl;

    fl=GWEN_SslCertDescr_GetStatusFlags(cert);
    if (fl==GWEN_SSL_CERT_FLAGS_OK) {
      if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_ACCEPTVALIDCERTS) {
        DBG_NOTICE(AQBANKING_LOGDOMAIN, "Automatically accepting valid new certificate [%s]", sFingerprint);
        return 1;
      }
      else {
        DBG_NOTICE(AQBANKING_LOGDOMAIN, "Automatically rejecting certificate [%s] (noninteractive)", sFingerprint);
        GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning,
                              "Automatically rejecting certificate [%s] (noninteractive)",
                              sFingerprint);
        return GWEN_ERROR_USER_ABORTED;
      }
    } /* if cert is valid */
    else {
      if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_REJECTINVALIDCERTS) {
        DBG_NOTICE(AQBANKING_LOGDOMAIN, "Automatically rejecting invalid certificate [%s] (noninteractive)", sFingerprint);
        GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning,
                              "Automatically rejecting invalid certificate [%s] (noninteractive)",
                              sFingerprint);
        return GWEN_ERROR_USER_ABORTED;
      }
    }
  } /* if non-interactive */

  /* undecided */
  return 0;
}



int _askUserAboutCert(GWEN_SYNCIO *sio, const GWEN_SSLCERTDESCR *cert)
{
  AB_SIOTLS_EXT *xsio;

  assert(sio);
  xsio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, AB_SIOTLS_EXT, sio);
  assert(xsio);

  /* use previous checkCert function, which normally presents the certificate
   * to the user and asks for a response */
  if (xsio->oldCheckCertFn) {
    int rv;

    /* get user response */
    rv=xsio->oldCheckCertFn(sio, cert);
    if (rv==0)
      return 1;
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Internal error: No previous checkCert function");
    return GWEN_ERROR_INTERNAL;
  }
}



void _storeCertAndUserResponseInUser(AB_USER *u, const GWEN_SSLCERTDESCR *cert, int response)
{
  GWEN_DB_NODE *dbCerts;
  GWEN_DB_NODE *dbC;
  const char *sFingerprint;
  const char *sStatus;

  sFingerprint=GWEN_SslCertDescr_GetFingerPrint(cert);
  sStatus=GWEN_SslCertDescr_GetStatusText(cert);

  dbCerts=AB_User_GetCertDb(u);
  assert(dbCerts);

  /* store certificate in database */
  dbC=GWEN_DB_GetGroup(dbCerts, GWEN_DB_FLAGS_OVERWRITE_GROUPS, sFingerprint);
  assert(dbC);
  GWEN_SslCertDescr_toDb(cert, dbC);

  /* store user response */
  GWEN_DB_SetIntValue(dbC, GWEN_DB_FLAGS_OVERWRITE_VARS, "userResponse", (response==1)?0:response);
  DBG_NOTICE(AQBANKING_LOGDOMAIN,
             "User response to presentation of cert \"%s\" (%s): %d",
             sFingerprint, sStatus, (response==1)?0:response);

  _storeAccessDate(dbC);
}



void _storeAccessDate(GWEN_DB_NODE *dbCert)
{
  GWEN_DATE *dt;

  dt=GWEN_Date_CurrentDate();
  GWEN_DB_SetCharValue(dbCert, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastAccessDate", GWEN_Date_GetString(dt));
  GWEN_Date_free(dt);
}

