/***************************************************************************
 begin       : Mon Oct 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "./session.h"

#include "parser/parser.h"

#include "aqbanking/version.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio_file.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _anonHnsha(const uint8_t *psegment, unsigned int slen, GWEN_SYNCIO *sio);
static int _anonHkpae(const uint8_t *psegment, unsigned int slen, GWEN_SYNCIO *sio);
static int _setCharValue(GWEN_DB_NODE *n, uint32_t flags, const char *path, const char *val);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



void AQFINTS_Session_LogMessage(AQFINTS_SESSION *sess,
                                const uint8_t *ptrLogData,
                                uint32_t lenLogData,
                                int rec,
                                int crypt)
{
  GWEN_DB_NODE *db;
  GWEN_SYNCIO *sio;
  unsigned int bsize;
  const char *logFile;
  int vmajor, vminor, vpatchlevel, vbuild;
  char vbuf[32];
  int rv;

  assert(sess);

  logFile=AQFINTS_Session_GetLogFile(sess);
  if (!logFile) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No log file, logging disabled for this dialog");
    return;
  }
  DBG_INFO(AQFINTS_LOGDOMAIN, "Logging to file [%s]", logFile);

  db=GWEN_DB_Group_new("header");

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "hbciVersion", AQFINTS_Session_GetHbciVersion(sess));

  _setCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "crypt", crypt?"yes":"no");
  _setCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "sender", rec?"bank":"user");
  GWEN_Version(&vmajor, &vminor, &vpatchlevel, &vbuild);
  snprintf(vbuf, sizeof(vbuf)-1, "%d.%d.%d.%d", vmajor, vminor, vpatchlevel, vbuild);
  vbuf[sizeof(vbuf)-1]=0;
  _setCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "gwenhywfar", vbuf);
  _setCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "aqhbci", AQBANKING_VERSION_FULL_STRING);
  _setCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "appname", AQFINTS_Session_GetAppRegKey(sess));
  _setCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "appversion", AQFINTS_Session_GetAppVersion(sess));
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "size", lenLogData);
  if (GWEN_Directory_GetPath(logFile, GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Path \"%s\" is not available, cannot log", logFile);
    GWEN_DB_Group_free(db);
    return;
  }

  sio=GWEN_SyncIo_File_new(logFile, GWEN_SyncIo_File_CreationMode_OpenAlways);
  GWEN_SyncIo_AddFlags(sio,
                       GWEN_SYNCIO_FILE_FLAGS_READ |
                       GWEN_SYNCIO_FILE_FLAGS_WRITE |
                       GWEN_SYNCIO_FILE_FLAGS_UREAD |
                       GWEN_SYNCIO_FILE_FLAGS_UWRITE |
                       GWEN_SYNCIO_FILE_FLAGS_APPEND);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* write header */
  rv=GWEN_DB_WriteToIo(db, sio,
                       GWEN_DB_FLAGS_WRITE_SUBGROUPS |
                       GWEN_DB_FLAGS_DETAILED_GROUPS |
                       GWEN_DB_FLAGS_USE_COLON|
                       GWEN_DB_FLAGS_OMIT_TYPES);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* append empty line to separate header from data */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) "\n", 1);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* write data */
  bsize=lenLogData;
  if (bsize) {
    const uint8_t *p;
    unsigned int bleft;

    bleft=bsize;
    p=ptrLogData;
    while (bleft) {
      const uint8_t *segEnd;
      unsigned int slen;

      if (*p=='\'') {
        rv=GWEN_SyncIo_WriteForced(sio, p, 1);
        if (rv<0) {
          DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
          GWEN_SyncIo_Disconnect(sio);
          GWEN_SyncIo_free(sio);
          GWEN_DB_Group_free(db);
          return;
        }

        p++;
        bleft--;
      }
      else {
        segEnd=(const uint8_t *) strchr((const char *) p, '\'');
        if (segEnd==NULL) {
          /* no segment end found, write rest of the buffer */
          rv=GWEN_SyncIo_WriteForced(sio, p, bleft);
          if (rv<0) {
            DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
            GWEN_SyncIo_Disconnect(sio);
            GWEN_SyncIo_free(sio);
            GWEN_DB_Group_free(db);
            return;
          }
          break;
        }

        assert(segEnd);
        slen=segEnd-p+1;
        assert(slen);

        if (strncasecmp((const char *) p, "HNSHA:", 6)==0)
          rv=_anonHnsha(p, slen, sio);
        else if (strncasecmp((const char *) p, "HKPAE:", 6)==0 || strncasecmp((const char *) p, "DKPAE:", 6)==0)
          rv=_anonHkpae(p, slen, sio);
        /* add more segments with confidential data here */
        else {
          unsigned int l;

          l=slen;
          rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *)p, l);
        }
        if (rv<0) {
          DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
          GWEN_SyncIo_Disconnect(sio);
          GWEN_SyncIo_free(sio);
          GWEN_DB_Group_free(db);
          return;
        }

        bleft-=slen;
        p=segEnd+1;
      }
    } /* while bleft */
  }

  /* add LF for better readability */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) "\n", 1);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* close layer */
  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return;
  }

  GWEN_SyncIo_free(sio);

  GWEN_DB_Group_free(db);
  DBG_DEBUG(AQFINTS_LOGDOMAIN, "Message logged");
}



int _anonHnsha(const uint8_t *psegment, unsigned int slen, GWEN_SYNCIO *sio)
{
  int plusCount=0;
  int lastWasEscape=0;
  int segDone=0;
  const uint8_t *p;
  unsigned int count;

  p=psegment;
  count=slen;
  while (*p && !segDone && count--) {
    int normalChar=1;
    int err;

    err=0;
    if (lastWasEscape) {
      lastWasEscape=0;
      normalChar=0;
    }
    else {
      if (*p=='?') {
        lastWasEscape=1;
      }
      else {
        if (*p=='\'')
          segDone=1;
        else if (*p=='+')
          plusCount++;
        lastWasEscape=0;
      }
    }
    if (plusCount>=3 && normalChar && *p!='+' && *p!='\'' && *p!=':')
      err=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) "*", 1);
    else
      err=GWEN_SyncIo_WriteForced(sio, p, 1);
    if (err<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", err);
      return err;
    }

    p++;
  } /* while */

  return 0;
}



int _anonHkpae(const uint8_t *psegment, unsigned int slen, GWEN_SYNCIO *sio)
{
  int plusCount=0;
  int lastWasEscape=0;
  int segDone=0;
  const uint8_t *p;
  unsigned int count;

  p=psegment;
  count=slen;
  while (*p && !segDone && count--) {
    int normalChar=1;
    int err;

    err=0;
    if (lastWasEscape) {
      lastWasEscape=0;
      normalChar=0;
    }
    else {
      if (*p=='?') {
        lastWasEscape=1;
      }
      else {
        if (*p=='\'')
          segDone=1;
        else if (*p=='+')
          plusCount++;
        lastWasEscape=0;
      }
    }
    if (plusCount>=1 && normalChar && *p!='+' && *p!='\'' && *p!=':')
      err=GWEN_SyncIo_WriteForced(sio, (const uint8_t *) "*", 1);
    else
      err=GWEN_SyncIo_WriteForced(sio, p, 1);
    if (err<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", err);
      return err;
    }

    p++;
  } /* while */

  return 0;
}




int _setCharValue(GWEN_DB_NODE *n, uint32_t flags, const char *path, const char *val)
{
  return GWEN_DB_SetCharValue(n, flags, path, val?val:"<empty>");
}





