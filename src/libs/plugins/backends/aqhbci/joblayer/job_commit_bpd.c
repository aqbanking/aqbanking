/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "job_commit_bpd.h"
#include "aqhbci/banking/user_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _getJobGroup(GWEN_DB_NODE *dbJob, const char *groupName, GWEN_DB_NODE **pResult);
static void _readLanguages(GWEN_DB_NODE *dbRd, AH_BPD *bpd);
static void _readVersions(GWEN_DB_NODE *dbRd, AH_BPD *bpd);
static void _readCommParams(GWEN_DB_NODE *dbJob, AH_BPD *bpd);
static void _readPinTanBpd(GWEN_DB_NODE *dbJob, AH_BPD *bpd, int protocolVersion);
static void _readBpdJobs(GWEN_DB_NODE *dbJob, AH_BPD *bpd, GWEN_MSGENGINE *msgEngine);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Job_Commit_Bpd(AH_JOB *j)
{
  GWEN_DB_NODE *dbJob    ;
  GWEN_DB_NODE *dbRd=NULL;
  AH_BPD *bpd;
  const char *p;
  int rv;
  AB_USER *user;
  GWEN_MSGENGINE *msgEngine;

  user=AH_Job_GetUser(j);
  msgEngine=AH_User_GetMsgEngine(user);
  assert(msgEngine);

  //dbJob=GWEN_DB_GetFirstGroup(j->jobResponses);
  dbJob=AH_Job_GetResponses(j);

  rv=_getJobGroup(dbJob, "bpd", &dbRd);
  if (rv<0) {
    if (rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_INFO(AQHBCI_LOGDOMAIN, "No BPD in response for job %s", AH_Job_GetName(j));
    /*GWEN_DB_Dump(j->jobResponses, 2);*/
    return 0;
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD, replacing existing");

  /* create new BPD */
  bpd=AH_Bpd_new();

  /* read version */
  AH_Bpd_SetBpdVersion(bpd, GWEN_DB_GetIntValue(dbRd, "version", 0, 0));

  /* read bank name */
  p=GWEN_DB_GetCharValue(dbRd, "name", 0, 0);
  if (p)
    AH_Bpd_SetBankName(bpd, p);

  /* read message and job limits */
  AH_Bpd_SetJobTypesPerMsg(bpd, GWEN_DB_GetIntValue(dbRd, "jobtypespermsg", 0, 0));
  AH_Bpd_SetMaxMsgSize(bpd, GWEN_DB_GetIntValue(dbRd, "maxmsgsize", 0, 0));

  _readLanguages(dbRd, bpd);
  _readVersions(dbRd, bpd);
  _readCommParams(dbJob, bpd);
  _readPinTanBpd(dbJob, bpd, GWEN_MsgEngine_GetProtocolVersion(msgEngine));
  _readBpdJobs(dbJob, bpd, msgEngine);

  /* set BPD */
  AH_User_SetBpd(user, bpd);
  return 0;
}



int _getJobGroup(GWEN_DB_NODE *dbJob, const char *groupName, GWEN_DB_NODE **pResult)
{
  GWEN_DB_NODE *dbRd;

  dbRd=GWEN_DB_GetGroup(dbJob, GWEN_PATH_FLAGS_NAMEMUSTEXIST, groupName);
  if (dbRd==NULL) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Group \"%s\" not found in response", groupName);
    return GWEN_ERROR_NOT_FOUND;
  }

  dbRd=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
  if (dbRd==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing \"data\" group inside group \"%s\"", groupName);
    return GWEN_ERROR_INVALID;
  }

  dbRd=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, groupName);
  if (dbRd==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing effective group \"%s\" inside response", groupName);
    return GWEN_ERROR_INVALID;
  }

  *pResult=dbRd;
  return 0;
}



void _readLanguages(GWEN_DB_NODE *dbRd, AH_BPD *bpd)
{
  GWEN_DB_NODE *n;

  /* read languages */
  n=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "languages");
  if (n) {
    int i;

    for (i=0;; i++) {
      int k;

      k=GWEN_DB_GetIntValue(n, "language", i, 0);
      if (k) {
        if (AH_Bpd_AddLanguage(bpd, k)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many languages (%d)", i);
          break;
        }
      }
      else
        break;
    } /* for */
  } /* if languages */

}



void _readVersions(GWEN_DB_NODE *dbRd, AH_BPD *bpd)
{
  GWEN_DB_NODE *n;

  /* read supported version */
  n=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "versions");
  if (n) {
    int i;

    for (i=0;; i++) {
      int k;

      k=GWEN_DB_GetIntValue(n, "version", i, 0);
      if (k) {
        if (AH_Bpd_AddHbciVersion(bpd, k)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many versions (%d)", i);
          break;
        }
      }
      else
        break;
    } /* for */
  } /* if versions */
}



void _readCommParams(GWEN_DB_NODE *dbJob, AH_BPD *bpd)
{
  int rv;
  GWEN_DB_NODE *dbRd=NULL;

  /* communication parameters */
  rv=_getJobGroup(dbJob, "ComData", &dbRd);
  if (rv==0) {
    GWEN_DB_NODE *currService;

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found communication infos");

    currService=GWEN_DB_FindFirstGroup(dbRd, "service");
    while (currService) {
      AH_BPD_ADDR *ba;

      ba=AH_BpdAddr_FromDb(currService);
      if (ba) {
        if (1) { /* dump info */
          GWEN_BUFFER *tbuf;
          const char *s;

          tbuf=GWEN_Buffer_new(0, 256, 0, 1);

          switch (AH_BpdAddr_GetType(ba)) {
          case AH_BPD_AddrTypeTCP:
            GWEN_Buffer_AppendString(tbuf, "TCP: ");
            break;
          case AH_BPD_AddrTypeBTX:
            GWEN_Buffer_AppendString(tbuf, "BTX: ");
            break;
          case AH_BPD_AddrTypeSSL:
            GWEN_Buffer_AppendString(tbuf, "SSL: ");
            break;
          default:
            GWEN_Buffer_AppendString(tbuf, "<UNK>: ");
            break;
          }

          s=AH_BpdAddr_GetAddr(ba);
          if (s && *s)
            GWEN_Buffer_AppendString(tbuf, s);
          else
            GWEN_Buffer_AppendString(tbuf, "<empty>");

          s=AH_BpdAddr_GetSuffix(ba);
          if (s && *s) {
            GWEN_Buffer_AppendString(tbuf, ", ");
            GWEN_Buffer_AppendString(tbuf, s);
          }

          GWEN_Buffer_AppendString(tbuf, ", ");
          switch (AH_BpdAddr_GetFType(ba)) {
          case AH_BPD_FilterTypeNone:
            GWEN_Buffer_AppendString(tbuf, "none");
            break;
          case AH_BPD_FilterTypeBase64:
            GWEN_Buffer_AppendString(tbuf, "base64");
            break;
          case AH_BPD_FilterTypeUUE:
            GWEN_Buffer_AppendString(tbuf, "uue");
            break;
          default:
            GWEN_Buffer_AppendString(tbuf, "<unk>");
            break;
          }

          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Server address found: %s", GWEN_Buffer_GetStart(tbuf));
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Info,
                                I18N("Server address found: %s"),
                                GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }

        /* add service */
        AH_Bpd_AddAddr(bpd, ba);
      }
      currService=GWEN_DB_FindNextGroup(currService, "service");
    }
  } /* if ComData found */
}



void _readPinTanBpd(GWEN_DB_NODE *dbJob, AH_BPD *bpd, int protocolVersion)
{
  int rv;
  GWEN_DB_NODE *dbRd=NULL;

  /* special extension of BPD for PIN/TAN mode */
  rv=_getJobGroup(dbJob, "PinTanBPD", &dbRd);
  if (rv==0) {
    GWEN_DB_NODE *bn;
    GWEN_DB_NODE *currJob;

    bn=AH_Bpd_GetBpdJobs(bpd, protocolVersion);
    assert(bn);

    currJob=GWEN_DB_FindFirstGroup(dbRd, "job");
    while (currJob) {
      const char *jobName;
      int needTAN;
      GWEN_DB_NODE *dbJob;

      jobName=GWEN_DB_GetCharValue(currJob, "job", 0, 0);
      assert(jobName);
      dbJob=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT, jobName);
      assert(dbJob);
      needTAN=strcasecmp(GWEN_DB_GetCharValue(currJob, "needTan", 0, "N"), "J")==0;
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS, "needTan", needTAN);
      currJob=GWEN_DB_FindNextGroup(currJob, "job");
    } /* while */
  } /* if PIN/TAN extension found */
}



void _readBpdJobs(GWEN_DB_NODE *dbJob, AH_BPD *bpd, GWEN_MSGENGINE *msgEngine)
{
  GWEN_DB_NODE *n;

  /* check for BPD jobs */
  n=GWEN_DB_GetFirstGroup(dbJob);
  while (n) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(n, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd)
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    if (dbRd) {
      GWEN_XMLNODE *bpdn;
      int segver;
      /* check for BPD job */

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking whether \"%s\" is a BPD job", GWEN_DB_GroupName(dbRd));
      segver=GWEN_DB_GetIntValue(dbRd, "head/version", 0, 0);
      /* get segment description (first try id, then code) */
      bpdn=GWEN_MsgEngine_FindNodeByProperty(msgEngine, "SEG", "id", segver, GWEN_DB_GroupName(dbRd));
      if (!bpdn)
        bpdn=GWEN_MsgEngine_FindNodeByProperty(msgEngine, "SEG", "code", segver, GWEN_DB_GroupName(dbRd));
      if (bpdn) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a candidate");
        if (atoi(GWEN_XMLNode_GetProperty(bpdn, "isbpdjob", "0"))) {
          /* segment contains a BPD job, move contents */
          GWEN_DB_NODE *bn;
          char numbuffer[32];

          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD job \"%s\"", GWEN_DB_GroupName(dbRd));
          bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(msgEngine));
          assert(bn);
          bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT,
                              GWEN_DB_GroupName(dbRd));
          assert(bn);

          if (GWEN_Text_NumToString(segver, numbuffer, sizeof(numbuffer)-1, 0)<1) {
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "Buffer too small");
            abort();
          }
          bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_OVERWRITE_GROUPS, numbuffer);
          assert(bn);

          GWEN_DB_AddGroupChildren(bn, dbRd);
          /* remove "head" and "segment" group */
          GWEN_DB_DeleteGroup(bn, "head");
          GWEN_DB_DeleteGroup(bn, "segment");
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Added BPD Job %s:%d", GWEN_DB_GroupName(dbRd), segver);
        } /* if isbpdjob */
        else {
          DBG_DEBUG(AQHBCI_LOGDOMAIN,
                    "Segment \"%s\" is known but not as a BPD job",
                    GWEN_DB_GroupName(dbRd));
        }
      } /* if segment found */
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Did not find segment \"%s\" (%d) ignoring",
                 GWEN_DB_GroupName(dbRd), segver);
      }
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */
}





