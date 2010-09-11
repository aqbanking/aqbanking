/***************************************************************************
 begin       : Thu Jun 18 2009
 copyright   : (C) 2009 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "abgui_p.h"

#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_GUI, AB_GUI)




GWEN_GUI *AB_Gui_new(AB_BANKING *ab) {
  GWEN_GUI *gui;
  AB_GUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(AB_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, AB_GUI, gui, xgui, AB_Gui_FreeData);

  xgui->banking=ab;
  xgui->checkCertFn=GWEN_Gui_SetCheckCertFn(gui, AB_Gui_CheckCert);
  xgui->readDialogPrefsFn=GWEN_Gui_SetReadDialogPrefsFn(gui, AB_Gui_ReadDialogPrefs);
  xgui->writeDialogPrefsFn=GWEN_Gui_SetWriteDialogPrefsFn(gui, AB_Gui_WriteDialogPrefs);

  return gui;
}



void AB_Gui_Extend(GWEN_GUI *gui, AB_BANKING *ab) {
  AB_GUI *xgui;

  assert(gui);
  GWEN_NEW_OBJECT(AB_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, AB_GUI, gui, xgui, AB_Gui_FreeData);

  xgui->banking=ab;
  xgui->checkCertFn=GWEN_Gui_SetCheckCertFn(gui, AB_Gui_CheckCert);
  GWEN_Gui_SetReadDialogPrefsFn(gui, AB_Gui_ReadDialogPrefs);
  GWEN_Gui_SetWriteDialogPrefsFn(gui, AB_Gui_WriteDialogPrefs);
}



void AB_Gui_Unextend(GWEN_GUI *gui) {
  AB_GUI *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_GUI, gui);
  assert(xgui);

  /* reset callbacks which point into AB_GUI */
  GWEN_Gui_SetCheckCertFn(gui, xgui->checkCertFn);

  /* unlink from GWEN_GUI object */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Unlinking GUI from banking object");
  GWEN_Gui_SetReadDialogPrefsFn(gui, xgui->readDialogPrefsFn);
  GWEN_Gui_SetWriteDialogPrefsFn(gui, xgui->writeDialogPrefsFn);
  GWEN_INHERIT_UNLINK(GWEN_GUI, AB_GUI, gui);
}



void GWENHYWFAR_CB AB_Gui_FreeData(void *bp, void *p) {
  AB_GUI *xgui;

  xgui=(AB_GUI*) p;
  assert(xgui);
  GWEN_FREE_OBJECT(xgui);
}



int AB_Gui__HashPair(const char *token,
		     const char *pin,
		     GWEN_BUFFER *buf) {
  GWEN_MDIGEST *md;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Md5_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)token, strlen(token));
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)pin, strlen(pin));
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  GWEN_Text_ToHexBuffer((const char*)GWEN_MDigest_GetDigestPtr(md),
			GWEN_MDigest_GetDigestSize(md),
			buf,
			0, 0, 0);
  GWEN_MDigest_free(md);
  return 0;
}




int AB_Gui_CheckCert(GWEN_GUI *gui,
		     const GWEN_SSLCERTDESCR *cd,
		     GWEN_SYNCIO *sio, uint32_t guiid) {
  AB_GUI *xgui;
  const char *hash;
  const char *status;
  GWEN_BUFFER *hbuf;
  GWEN_DB_NODE *dbCerts=NULL;
  int rv;
  int result=GWEN_ERROR_USER_ABORTED;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_GUI, gui);
  assert(xgui);

  hash=GWEN_SslCertDescr_GetFingerPrint(cd);
  status=GWEN_SslCertDescr_GetStatusText(cd);

  hbuf=GWEN_Buffer_new(0, 64, 0, 1);
  AB_Gui__HashPair(hash, status, hbuf);

  /* lock certificate data */
  rv=AB_Banking_LockSharedConfig(xgui->banking, "certs");
  if (rv<0) {
    /* fallback */
    DBG_WARN(AQBANKING_LOGDOMAIN, "Could not lock certs db, asking user (%d)", rv);
    result=xgui->checkCertFn(gui, cd, sio, guiid);
  }
  else {
    int i;

    /* load certificate data */
    rv=AB_Banking_LoadSharedConfig(xgui->banking, "certs", &dbCerts);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Could not load certs (%d)", rv);
      dbCerts=GWEN_DB_Group_new("certs");
    }

    /* lookup cert or ask */
    i=GWEN_DB_GetIntValue(dbCerts, GWEN_Buffer_GetStart(hbuf), 0, 1);
    if (i==0) {
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
		 "Automatically accepting certificate [%s]",
		 hash);
      result=0;
    }
    else {
      /* no info in the cert cache.
       * If in non-interactive mode we check whether the certificate is valid. If so
       * and the GUI flags allow us to accept valid certs we do so. Otherwise we ask the
       * user (only if not in non-interactive mode)
       */
      if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_NONINTERACTIVE) {
	uint32_t fl;

	fl=GWEN_SslCertDescr_GetStatusFlags(cd);
	if (fl==GWEN_SSL_CERT_FLAGS_OK) {
	  if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_ACCEPTVALIDCERTS) {
	    DBG_NOTICE(AQBANKING_LOGDOMAIN,
		       "Automatically accepting valid new certificate [%s]",
		       hash);
	    GWEN_Buffer_free(hbuf);
	    AB_Banking_UnlockSharedConfig(xgui->banking, "certs");
	    return 0;
	  }
	  else {
	    DBG_NOTICE(AQBANKING_LOGDOMAIN,
		       "Automatically rejecting certificate [%s] (noninteractive)",
		       hash);
	    GWEN_Buffer_free(hbuf);
	    AB_Banking_UnlockSharedConfig(xgui->banking, "certs");
	    return GWEN_ERROR_USER_ABORTED;
	  }
	} /* if cert is valid */
	else {
	  if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_REJECTINVALIDCERTS) {
	    DBG_NOTICE(AQBANKING_LOGDOMAIN,
		       "Automatically rejecting invalid certificate [%s] (noninteractive)",
		       hash);
	    GWEN_Buffer_free(hbuf);
	    AB_Banking_UnlockSharedConfig(xgui->banking, "certs");
	    return GWEN_ERROR_USER_ABORTED;
	  }
	}
      } /* if non-interactive */

      if (xgui->checkCertFn) {
	result=xgui->checkCertFn(gui, cd, sio, guiid);
	if (result==0) {
	  GWEN_DB_SetIntValue(dbCerts, GWEN_DB_FLAGS_OVERWRITE_VARS,
			      GWEN_Buffer_GetStart(hbuf), result);
	}
      }
    }

    /* write new certs */
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Saving certs");
    rv=AB_Banking_SaveSharedConfig(xgui->banking, "certs", dbCerts);
    if (rv<0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Could not unlock certs db (%d)", rv);
    }

    /* unlock certs */
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Unlocking certs");
    rv=AB_Banking_UnlockSharedConfig(xgui->banking, "certs");
    if (rv<0) {
      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Could not unlock certs db (%d)", rv);
    }
  }

  GWEN_Buffer_free(hbuf);

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Returning %d", result);

  return result;
}



int AB_Gui_ReadDialogPrefs(GWEN_GUI *gui,
			   const char *groupName,
			   const char *altName,
			   GWEN_DB_NODE **pDb) {
  AB_GUI *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_GUI, gui);
  assert(xgui);

  if (groupName && *groupName) {
    int rv;
    const char *s;
    GWEN_DB_NODE *db;
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    s=GWEN_Gui_GetName();
    if (s && *s) {
      GWEN_Buffer_AppendString(nbuf, s);
      GWEN_Buffer_AppendString(nbuf, "_");
    }
    GWEN_Buffer_AppendString(nbuf, groupName);

    rv=AB_Banking_LoadSharedConfig(xgui->banking,
                                   GWEN_Buffer_GetStart(nbuf),
				   &db);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(nbuf);
      return rv;
    }
    *pDb=db;
    GWEN_Buffer_free(nbuf);
    return 0;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No groupName");
    return GWEN_ERROR_NO_DATA;
  }
}



int AB_Gui_WriteDialogPrefs(GWEN_GUI *gui,
			    const char *groupName,
			    GWEN_DB_NODE *db) {
  AB_GUI *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_GUI, gui);
  assert(xgui);

  if (groupName && *groupName && db) {
    int rv;
    const char *s;
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    s=GWEN_Gui_GetName();
    if (s && *s) {
      GWEN_Buffer_AppendString(nbuf, s);
      GWEN_Buffer_AppendString(nbuf, "_");
    }
    GWEN_Buffer_AppendString(nbuf, groupName);

    /* lock configuration */
    rv=AB_Banking_LockSharedConfig(xgui->banking,
                                   GWEN_Buffer_GetStart(nbuf));
    if (rv==0) {
      /* save configuration */
      rv=AB_Banking_SaveSharedConfig(xgui->banking,
				     GWEN_Buffer_GetStart(nbuf),
				     db);
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      }

      /* unlock configuration */
      rv=AB_Banking_UnlockSharedConfig(xgui->banking,
				       GWEN_Buffer_GetStart(nbuf));
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      }
    }
    GWEN_Buffer_free(nbuf);
  }

  return 0;
}




