/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "provider_p.h"
#include "aqdtaus_l.h"
#include <aqbanking/account_be.h>
#include <aqbanking/job_be.h>
#include <aqbanking/jobsingletransfer_be.h>
#include <aqbanking/jobsingledebitnote_be.h>
#include <aqbanking/value.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/bio_buffer.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/gwentime.h>

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_I18N
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
# ifdef HAVE_LIBINTL_H
#  include <libintl.h>
# endif

# define I18N(msg) dgettext(PACKAGE, msg)
#else
# define I18N(msg) msg
#endif

#define I18N_NOOP(msg) msg



GWEN_INHERIT(AB_PROVIDER, AD_PROVIDER)



AB_PROVIDER *AD_Provider_new(AB_BANKING *ab){
  AB_PROVIDER *pro;
  AD_PROVIDER *dp;

  pro=AB_Provider_new(ab, AD_PROVIDER_NAME);
  GWEN_NEW_OBJECT(AD_PROVIDER, dp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AD_PROVIDER, pro, dp,
                       AD_Provider_FreeData);
  dp->myJobs=AD_Job_List_new();
  dp->bankingJobs=AB_Job_List2_new();

  AB_Provider_SetInitFn(pro, AD_Provider_Init);
  AB_Provider_SetFiniFn(pro, AD_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, AD_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, AD_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, AD_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, AD_Provider_ResetQueue);
  AB_Provider_SetExtendAccountFn(pro, AD_Provider_ExtendAccount);

  return pro;
}



void AD_Provider_FreeData(void *bp, void *p) {
  AD_PROVIDER *dp;

  dp=(AD_PROVIDER*)p;
  assert(dp);

  AB_Job_List2_free(dp->bankingJobs);
  AD_Job_List_free(dp->myJobs);

  GWEN_FREE_OBJECT(dp);
}



int AD_Provider_AddTransfer(AB_PROVIDER *pro,
                            AB_ACCOUNT *acc,
                            const AB_TRANSACTION *t,
                            GWEN_TYPE_UINT32 *jobId) {
  AD_PROVIDER *dp;
  AD_JOB *dj;
  int maxXfers;

  assert(t);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  maxXfers=AD_Account_GetMaxTransfersPerJob(acc);

  /* find job which can take this transaction */
  dj=AD_Job_List_First(dp->myJobs);
  while(dj) {
    if (!AD_Job_GetIsDebitNote(dj) &&
        AD_Job_GetAccount(dj)==acc &&
        (maxXfers==0 || AD_Job_GetTransferCount(dj)<maxXfers)
       ) {
      break;
    }
  }

  if (!dj) {
    /* no job matches, create new one */
    dj=AD_Job_new(acc, 0, ++(dp->lastJobId));
    AD_Job_List_Add(dj, dp->myJobs);
  }

  AD_Job_AddTransfer(dj, AB_Transaction_dup(t));
  *jobId=AD_Job_GetJobId(dj);
  return 0;
}



int AD_Provider_AddDebitNote(AB_PROVIDER *pro,
                             AB_ACCOUNT *acc,
                             const AB_TRANSACTION *t,
                             GWEN_TYPE_UINT32 *jobId) {
  AD_PROVIDER *dp;
  AD_JOB *dj;
  int maxXfers;

  assert(t);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  maxXfers=AD_Account_GetMaxTransfersPerJob(acc);

  /* find job which can take this transaction */
  dj=AD_Job_List_First(dp->myJobs);
  while(dj) {
    if (AD_Job_GetIsDebitNote(dj) &&
        AD_Job_GetAccount(dj)==acc &&
        (maxXfers==0 || AD_Job_GetTransferCount(dj)<maxXfers)
       ) {
      break;
    }
  }

  if (!dj) {
    /* no job matches, create new one */
    dj=AD_Job_new(acc, 1, ++(dp->lastJobId));
    AD_Job_List_Add(dj, dp->myJobs);
  }

  AD_Job_AddTransfer(dj, AB_Transaction_dup(t));
  *jobId=AD_Job_GetJobId(dj);

  return 0;
}



int AD_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  AD_PROVIDER *dp;
  const char *logLevelName;

  if (!GWEN_Logger_IsOpen(AQDTAUS_LOGDOMAIN)) {
    GWEN_Logger_Open(AQDTAUS_LOGDOMAIN,
                     "aqdtaus", 0,
		     GWEN_LoggerTypeConsole,
		     GWEN_LoggerFacilityUser);
  }

  logLevelName=getenv("AQDTAUS_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevelUnknown) {
      GWEN_Logger_SetLevel(AQDTAUS_LOGDOMAIN, ll);
      DBG_WARN(AQDTAUS_LOGDOMAIN,
               "Overriding loglevel for AqDTAUS with \"%s\"",
               logLevelName);
    }
    else {
      DBG_ERROR(AQDTAUS_LOGDOMAIN, "Unknown loglevel \"%s\"",
                logLevelName);
    }
  }


  DBG_NOTICE(AQDTAUS_LOGDOMAIN, "Initializing AqDTAUS backend");
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  dp->dbConfig=dbData;
  dp->lastJobId=GWEN_DB_GetIntValue(dp->dbConfig, "lastJobId", 0, 0);

  return 0;
}



int AD_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData){
  AD_PROVIDER *dp;
  int errors=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  DBG_NOTICE(AQDTAUS_LOGDOMAIN, "Deinitializing AqDTAUS backend");

  GWEN_DB_SetIntValue(dp->dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastJobId", dp->lastJobId);

  GWEN_DB_ClearGroup(dp->dbConfig, "accounts");

  dp->dbConfig=0;
  AB_Job_List2_Clear(dp->bankingJobs);
  AD_Job_List_Clear(dp->myJobs);

  if (errors)
    return AB_ERROR_GENERIC;

  return 0;
}



AD_JOB *AD_Provider_FindMyJob(AB_PROVIDER *pro, GWEN_TYPE_UINT32 jid) {
  AD_PROVIDER *dp;
  AD_JOB *dj;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  dj=AD_Job_List_First(dp->myJobs);
  while(dj) {
    if (AD_Job_GetJobId(dj)==jid)
      break;
    dj=AD_Job_List_Next(dj);
  } /* while */

  return dj;
}



int AD_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  AD_PROVIDER *dp;
  AB_ACCOUNT *da;
  AB_TRANSACTION_LIMITS *lim;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  da=AB_Job_GetAccount(j);
  assert(da);

  lim=AB_TransactionLimits_new();
  AB_TransactionLimits_SetMaxLinesPurpose(lim,
                                          AD_Account_GetMaxPurposeLines(da));
  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeTransfer:
    AB_JobSingleTransfer_SetFieldLimits(j, lim);
    return 0;

  case AB_Job_TypeDebitNote:
    AB_JobSingleDebitNote_SetFieldLimits(j, lim);
    return 0;

  default:
    DBG_INFO(AQDTAUS_LOGDOMAIN, "Job not supported (%d)", AB_Job_GetType(j));
    AB_TransactionLimits_free(lim);
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */
}



int AD_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  AD_PROVIDER *dp;
  AB_ACCOUNT *da;
  const AB_TRANSACTION *t;
  int rv;
  GWEN_TYPE_UINT32 jobId;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  da=AB_Job_GetAccount(j);
  assert(da);

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeTransfer:
    t=AB_JobSingleTransfer_GetTransaction(j);
    assert(t);
    rv=AD_Provider_AddTransfer(pro, da, t, &jobId);
    if (rv) {
      DBG_INFO(AQDTAUS_LOGDOMAIN, "here");
    }
    else {
      GWEN_DB_NODE *dbJobData;

      dbJobData=AB_Job_GetProviderData(j, pro);
      assert(dbJobData);
      GWEN_DB_SetIntValue(dbJobData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "jobId", jobId);
      AB_Job_SetIdForProvider(j, jobId);
      AB_Job_List2_PushBack(dp->bankingJobs, j);
    }
    return rv;

  case AB_Job_TypeDebitNote:
    t=AB_JobSingleDebitNote_GetTransaction(j);
    assert(t);
    rv=AD_Provider_AddDebitNote(pro, da, t, &jobId);
    if (rv) {
      DBG_INFO(AQDTAUS_LOGDOMAIN, "here");
    }
    else {
      GWEN_DB_NODE *dbJobData;

      dbJobData=AB_Job_GetProviderData(j, pro);
      assert(dbJobData);
      GWEN_DB_SetIntValue(dbJobData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "jobId", jobId);
      AB_Job_SetIdForProvider(j, jobId);
      AB_Job_List2_PushBack(dp->bankingJobs, j);
    }
    return rv;

  default:
    DBG_INFO(AQDTAUS_LOGDOMAIN, "Job not supported (%d)", AB_Job_GetType(j));
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */
}



int AD_Provider_ResetQueue(AB_PROVIDER *pro){
  AD_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  AB_Job_List2_Clear(dp->bankingJobs);
  AD_Job_List_Clear(dp->myJobs);

  return 0;
}



int AD_Provider_ExecCommand(AB_PROVIDER *pro, const char *cmd){
  GWEN_PROCESS *pr;
  char cmdBuf[128];
  const char *prg;
  const char *p;
  GWEN_PROCESS_STATE pst;
  int result;

  pr=GWEN_Process_new();
  prg=GWEN_Text_GetWord(cmd, " ",
                        cmdBuf, sizeof(cmdBuf),
                        GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
                        GWEN_TEXT_FLAGS_DEL_QUOTES |
                        GWEN_TEXT_FLAGS_CHECK_BACKSLASH,
                        &p);
  if (!prg || !p) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Bad command");
    GWEN_Process_free(pr);
    return AB_ERROR_BAD_CONFIG_FILE;
  }
  while(isspace(*p)) p++;
  pst=GWEN_Process_Start(pr, prg, p);
  if (pst!=GWEN_ProcessStateRunning) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Error executing command");
    GWEN_Process_free(pr);
    return AB_ERROR_GENERIC;
  }

  /* wait for process to terminate */
  while((pst=GWEN_Process_CheckState(pr))==GWEN_ProcessStateRunning) {
    if (AB_Banking_ProgressAdvance(AB_Provider_GetBanking(pro),
                                   0, AB_BANKING_PROGRESS_NONE)) {
      DBG_ERROR(AQDTAUS_LOGDOMAIN, "User aborted via waitcallback");
      GWEN_Process_Terminate(pr);
      GWEN_Process_free(pr);
      return AB_ERROR_USER_ABORT;
    }
    GWEN_Socket_Select(0, 0, 0, 500);
  } /* while */
  if (pst!=GWEN_ProcessStateExited) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Abnormal command termination.");
    GWEN_Process_free(pr);
    return AB_ERROR_GENERIC;
  }

  result=GWEN_Process_GetResult(pr);
  GWEN_Process_free(pr);

  if (result!=0) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Command returned an error (%d)", result);
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AD_Provider_CheckEmptyDir(const char *path) {
  GWEN_DIRECTORYDATA *dir;

  dir=GWEN_Directory_new();
  if (GWEN_Directory_Open(dir, path)==0) {
    char buffer[256];

    while(0==GWEN_Directory_Read(dir, buffer, sizeof(buffer))) {
      if (strcmp(buffer, "..")!=0 &&
          strcmp(buffer, ".")!=0) {
        DBG_INFO(AQDTAUS_LOGDOMAIN,
                 "Folder \"%s\" is not empty", path);
        return AB_ERROR_FOUND;
      }
    }
    GWEN_Directory_Close(dir);
  }
  else {
    DBG_INFO(AQDTAUS_LOGDOMAIN, "Could not open folder \"%s\"", path);
    GWEN_Directory_free(dir);
    return AB_ERROR_NOT_FOUND;
  }

  GWEN_Directory_free(dir);
  return 0;
}




int AD_Provider__WriteDTAUS(AB_PROVIDER *pro,
                            const char *path,
                            GWEN_BUFFER *buf){
  AD_PROVIDER *dp;
  FILE *f;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  f=fopen(path, "w+b");
  if (f==0) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN,
              "Could not create file \"%s\": %s",
              path,
              strerror(errno));
    return AB_ERROR_GENERIC;
  }

  if (1!=fwrite(GWEN_Buffer_GetStart(buf),
                GWEN_Buffer_GetUsedBytes(buf),
                1,
                f)) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN,
              "Could not write to file \"%s\": %s",
              path,
              strerror(errno));
    fclose(f);
    return AB_ERROR_GENERIC;
  }

  if (fclose(f)) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN,
              "Could not close file \"%s\": %s",
              path,
              strerror(errno));
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AD_Provider_WriteDTAUS(AB_PROVIDER *pro,
                           AB_ACCOUNT *da,
                           GWEN_BUFFER *buf){
  AD_PROVIDER *dp;
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *tbuf;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, AD_Account_GetFolder(da));
  GWEN_Buffer_AppendByte(tbuf, '/');
  GWEN_Buffer_AppendString(tbuf, "DTAUS0.TXT");
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Directory_OsifyPath(GWEN_Buffer_GetStart(tbuf), pbuf, 1);
  GWEN_Buffer_free(tbuf);
  rv=AD_Provider__WriteDTAUS(pro, GWEN_Buffer_GetStart(pbuf), buf);
  GWEN_Buffer_free(pbuf);
  return rv;
}



int AD_Provider_MountDisc(AB_PROVIDER *pro, AB_ACCOUNT *da) {
  const char *cmd;
  int rv=0;

  cmd=AD_Account_GetMountCommand(da);
  if (cmd) {
    int rv;
    GWEN_TYPE_UINT32 bid;

    bid=AB_Banking_ShowBox(AB_Provider_GetBanking(pro),
                           0,
                           I18N("Mounting disc"),
                           I18N("Mounting floppy disc, please wait..."));
    rv=AD_Provider_ExecCommand(pro, cmd);
    AB_Banking_HideBox(AB_Provider_GetBanking(pro), bid);
    if (rv) {
      DBG_WARN(AQDTAUS_LOGDOMAIN, "Could not mount disc");
    }
  }
  return rv;
}



int AD_Provider_UnmountDisc(AB_PROVIDER *pro, AB_ACCOUNT *da) {
  const char *cmd;
  int rv=0;

  cmd=AD_Account_GetUnmountCommand(da);
  if (cmd) {
    int rv;
    GWEN_TYPE_UINT32 bid;

    bid=AB_Banking_ShowBox(AB_Provider_GetBanking(pro),
                           0,
                           I18N("Unmounting disc"),
                           I18N("Unmounting floppy disc, please wait..."));
    rv=AD_Provider_ExecCommand(pro, cmd);
    AB_Banking_HideBox(AB_Provider_GetBanking(pro), bid);
    if (rv) {
      DBG_WARN(AQDTAUS_LOGDOMAIN, "Could not unmount disc");
    }
  }
  return rv;
}



int AD_Provider_PrintBegleitZettel(AB_PROVIDER *pro,
                                   AD_JOB *dj,
                                   GWEN_DB_NODE *dbTransfers) {
  AD_PROVIDER *dp;
  GWEN_BUFFER *abuf;
  GWEN_BUFFER *hbuf;
  GWEN_DB_NODE *dbT;
  AB_ACCOUNT *da;
  char tbuf[512];
  GWEN_BUFFER *dateBuf;
  GWEN_TIME *ti;
  int i;
  const char *s;
  double v1=0, v2=0, v3=0;
  int rv;
  const char *msgHeadTxt=I18N_NOOP
    (
     "Begleitzettel fuer beleglosen Datentraegeraustausch\n"
     "\n"
     "Name           : %s\n"
     "BLZ            : %s\n"
     "Kontonummer    : %s\n"
     "Datum          : %s\n"
     "Diskettennummer: %d\n"
     "Anzahl C-Saetze : %d\n"
    );
  const char *msgHeadHtml=I18N_NOOP
    (
     "<h2>Begleitzettel fuer beleglosen Datentraegeraustausch</h2>\n"
     "<table>\n"
     "<tr><td>Name</td><td>%s</td></tr>\n"
     "<tr><td>BLZ</td><td>%s</td></tr>\n"
     "<tr><td>Kontonummer</td><td>%s</td></tr>\n"
     "<tr><td>Datum</td><td>%s</td></tr>\n"
     "<tr><td>Disketten-Nummer</td><td>%d</td></tr>\n"
     "<tr><td>Anzahl C-Saetze</td><td>%d</td></tr>\n"
     "</table>\n"
    );
  const char *msgHeadHtmlTable=I18N_NOOP
    (
     "<h2>Transaktionen</h2>"
     "<table>\n"
     "<tr><th>Name</th><th>BLZ</th><th>Konto</th><th>Betrag</th></tr>\n"
    );
  const char *msgLineTxt=I18N_NOOP
    (
     "Name: %s BLZ: %s Kto: %s Betrag: %8.2lf Euro\n"
    );
  const char *msgLineHtml=I18N_NOOP
    (
     "<tr><td>%s</td><td>%s</td><td>%s</td><td>%8.2lf EUR</td></tr>\n"
    );
  const char *msgTailTxt=I18N_NOOP
    (
     "\n"
     "Pruefsummen\n"
     "BLZ-Summe: %.0lf Kto-Summe: %.0lf Euro-Summe: %.2lf\n"
    );

  const char *msgTailTable=I18N_NOOP
    (
     "</table>\n"
    );
  const char *msgTailHtml=I18N_NOOP
    (
     "<h2>Pruefsummen</h2>\n"
     "<table>\n"
     "<tr><th>BLZ-Summe</th><th>Kto-Summe</th><th>Euro-Summe</th></tr>\n"
     "<tr><td>%.0lf</td><td>%.0lf</td><td>%.2lf</td></tr>\n"
     "</table>\n"
    );

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  da=AD_Job_GetAccount(dj);
  assert(da);
  abuf=GWEN_Buffer_new(0, 4096, 0, 1);
  hbuf=GWEN_Buffer_new(0, 4096, 0, 1);

  /* add head */
  i=AB_Transaction_List2_GetSize(AD_Job_GetTransfers(dj));
  ti=GWEN_CurrentTime();
  assert(ti);
  dateBuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), dateBuf);
  snprintf(tbuf, sizeof(tbuf),
           I18N(msgHeadTxt),
           AB_Account_GetOwnerName(da),
           AB_Account_GetBankCode(da),
           AB_Account_GetAccountNumber(da),
           GWEN_Buffer_GetStart(dateBuf),
           AD_Job_GetJobId(dj),
           i);
  GWEN_Buffer_AppendString(abuf, tbuf);

  snprintf(tbuf, sizeof(tbuf),
           I18N(msgHeadHtml),
           AB_Account_GetOwnerName(da),
           AB_Account_GetBankCode(da),
           AB_Account_GetAccountNumber(da),
           GWEN_Buffer_GetStart(dateBuf),
           AD_Job_GetJobId(dj),
           i);
  GWEN_Buffer_AppendString(hbuf, tbuf);
  GWEN_Buffer_free(dateBuf);
  GWEN_Time_free(ti);

  if (AD_Account_GetPrintAllTransactions(da)) {
    GWEN_Buffer_AppendString(hbuf, I18N(msgHeadHtmlTable));
    dbT=GWEN_DB_GetFirstGroup(dbTransfers);
    while(dbT) {
      if (strcasecmp(GWEN_DB_GroupName(dbT), "transfer")==0 ||
          strcasecmp(GWEN_DB_GroupName(dbT), "debitNote")==0) {
        GWEN_DB_NODE *dbValue;
        double ev=0;

        dbValue=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                 "value");
        if (dbValue) {
          AB_VALUE *v;

          v=AB_Value_fromDb(dbValue);
          assert(v);
          ev=AB_Value_GetValue(v);
          AB_Value_free(v);
        }

        snprintf(tbuf, sizeof(tbuf),
                 I18N(msgLineTxt),
                 GWEN_DB_GetCharValue(dbT, "remoteName", 0,
                                      I18N("unknown")),
                 GWEN_DB_GetCharValue(dbT, "remoteBankCode", 0,
                                      I18N("unknown")),
                 GWEN_DB_GetCharValue(dbT, "remoteAccountNumber", 0,
                                      I18N("unknown")),
                 ev);
        GWEN_Buffer_AppendString(abuf, tbuf);

        snprintf(tbuf, sizeof(tbuf),
                 I18N(msgLineHtml),
                 GWEN_DB_GetCharValue(dbT, "remoteName", 0,
                                      I18N("unknown")),
                 GWEN_DB_GetCharValue(dbT, "remoteBankCode", 0,
                                      I18N("unknown")),
                 GWEN_DB_GetCharValue(dbT, "remoteAccountNumber", 0,
                                      I18N("unknown")),
                 ev);
        GWEN_Buffer_AppendString(hbuf, tbuf);
      }

      dbT=GWEN_DB_GetNextGroup(dbT);
    } /* while transactions */
    GWEN_Buffer_AppendString(hbuf, I18N(msgTailTable));
  } /* if printAllTransactions */

  /* add tail */
  dbT=GWEN_DB_GetGroup(dbTransfers, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "summary");
  assert(dbT);
  GWEN_DB_Dump(dbT, stderr, 2);

  s=GWEN_DB_GetCharValue(dbT, "sumBankCodes", 0, 0);
  if (s)
    GWEN_Text_StringToDouble(s, &v1);
  s=GWEN_DB_GetCharValue(dbT, "sumAccountIds", 0, 0);
  if (s)
    GWEN_Text_StringToDouble(s, &v2);
  s=GWEN_DB_GetCharValue(dbT, "sumEur", 0, 0);
  if (s)
    GWEN_Text_StringToDouble(s, &v3);

  snprintf(tbuf, sizeof(tbuf),
           I18N(msgTailTxt),
           v1, v2, v3);
  GWEN_Buffer_AppendString(abuf, tbuf);

  snprintf(tbuf, sizeof(tbuf),
           I18N(msgTailHtml),
           v1, v2, v3);
  GWEN_Buffer_AppendString(hbuf, tbuf);

  /* concatenate ASCII and HTML buffer */
  GWEN_Buffer_AppendString(abuf, "<html>");
  GWEN_Buffer_AppendBuffer(abuf, hbuf);
  GWEN_Buffer_AppendString(abuf, "</html>");

  GWEN_Buffer_Dump(abuf, stderr, 2);

  rv=AB_Banking_Print(AB_Provider_GetBanking(pro),
                      I18N("Begleitzettel fuer DTAUS Disketten"),
                      "AQDTAUS:BEGLEITZETTEL",
                      I18N("Diesen Zettel muessen Sie der Diskette "
                           "beilegen."),
                      GWEN_Buffer_GetStart(abuf));
  GWEN_Buffer_free(hbuf);
  GWEN_Buffer_free(abuf);

  return rv;
}



int AD_Provider_SaveJob(AB_PROVIDER *pro, AD_JOB *dj, GWEN_BUFFER *data){
  int rv;
  AB_ACCOUNT *da;
  GWEN_BUFFER *pbuf;
  char numbuf[32];

  da=AD_Job_GetAccount(dj);
  assert(da);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Provider_GetUserDataDir(pro, pbuf);
  if (rv) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_AppendByte(pbuf, '/');
  GWEN_Buffer_AppendString(pbuf, AB_Account_GetBankCode(da));
  GWEN_Buffer_AppendByte(pbuf, '/');
  GWEN_Buffer_AppendString(pbuf, AB_Account_GetAccountNumber(da));
  GWEN_Buffer_AppendByte(pbuf, '/');
  snprintf(numbuf, sizeof(numbuf), "%08x", AD_Job_GetJobId(dj));
  GWEN_Buffer_AppendString(pbuf, numbuf);
  GWEN_Buffer_AppendString(pbuf, ".dtaus");
  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf),
                             GWEN_PATH_FLAGS_CHECKROOT |
                             GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN,
              "Could not create file \"%s\"",
              GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }

  rv=AD_Provider__WriteDTAUS(pro, GWEN_Buffer_GetStart(pbuf), data);
  GWEN_Buffer_free(pbuf);
  return rv;
}



int AD_Provider_ProcessJob(AB_PROVIDER *pro, AD_JOB *dj){
  AD_PROVIDER *dp;
  AB_ACCOUNT *da;
  GWEN_DB_NODE *dbCfg;
  GWEN_DB_NODE *dbTransfers;
  const char *s;
  AB_TRANSACTION_LIST2_ITERATOR *ait;
  GWEN_DBIO *dbio;
  GWEN_BUFFER *buf;
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int mounted=0;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  dbio=GWEN_DBIO_GetPlugin("dtaus");
  if (!dbio) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "DTAUS parser plugin not available");
    AD_Job_SetResult(dj, AB_ERROR_NOT_AVAILABLE,
                     I18N("DTAUS parser not available"));
    return AB_ERROR_NOT_AVAILABLE;
  }

  da=AD_Job_GetAccount(dj);
  assert(da);
  dbCfg=GWEN_DB_Group_new("config");
  GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "bankCode",
                       AB_Account_GetBankCode(da));
  GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "accountId",
                       AB_Account_GetAccountNumber(da));
  s=AB_Account_GetCurrency(da);
  if (!s)
    s="EUR";
  GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "currency", s);

  GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "type",
                       (AD_Job_GetIsDebitNote(dj))?"debitNote":"transfer");

  /* create DB with all transfers */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                         AB_Banking_LogLevelNotice,
                         I18N("Creating DTAUS object from job"));
  dbTransfers=GWEN_DB_Group_new("transfers");
  ait=AB_Transaction_List2_First(AD_Job_GetTransfers(dj));
  if (ait==0) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "No transactions");
    GWEN_DB_Group_free(dbTransfers);
    GWEN_DB_Group_free(dbCfg);
    AD_Job_SetResult(dj, AB_ERROR_NO_DATA,
                     I18N("No transactions"));
    return AB_ERROR_NO_DATA;
  }
  else {
    AB_TRANSACTION *t;

    t=AB_Transaction_List2Iterator_Data(ait);
    assert(t);
    while(t) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_GetGroup(dbTransfers, GWEN_PATH_FLAGS_CREATE_GROUP,
                           (AD_Job_GetIsDebitNote(dj))?
                           "debitNote":"transfer");
      assert(dbT);
      if (AB_Transaction_toDb(t, dbT)) {
        DBG_ERROR(AQDTAUS_LOGDOMAIN, "Bad transaction");
        GWEN_DB_Group_free(dbTransfers);
        GWEN_DB_Group_free(dbCfg);
        AD_Job_SetResult(dj, AB_ERROR_BAD_DATA,
                         I18N("Malformed transaction"));
        return AB_ERROR_BAD_DATA;
      }
      t=AB_Transaction_List2Iterator_Next(ait);
    }
    AB_Transaction_List2Iterator_free(ait);
  }

  buf=GWEN_Buffer_new(0, 4096, 0, 1);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);

  if (GWEN_DBIO_Export(dbio, bio, GWEN_DB_FLAGS_DEFAULT,
                       dbTransfers, dbCfg)) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Error creating DTAUS object");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    GWEN_DB_Group_free(dbTransfers);
    GWEN_DB_Group_free(dbCfg);
    AD_Job_SetResult(dj, AB_ERROR_BAD_DATA,
                     I18N("Could not create DTAUS data"));
    return AB_ERROR_BAD_DATA;
  }

  GWEN_Buffer_Dump(buf, stderr, 2);

  /* now we have the DTAUS object in our buffer */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                         AB_Banking_LogLevelNotice,
                         I18N("Verifying DTAUS object"));
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Error flushing DTAUS object");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    GWEN_DB_Group_free(dbTransfers);
    GWEN_DB_Group_free(dbCfg);
    AD_Job_SetResult(dj, AB_ERROR_BAD_DATA,
                     I18N("Could not flush DTAUS data"));
    return AB_ERROR_BAD_DATA;
  }
  GWEN_BufferedIO_free(bio);

  /* try to read back the DTAUS object */
  GWEN_Buffer_Rewind(buf);
  GWEN_DB_ClearGroup(dbTransfers, 0);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  if (GWEN_DBIO_Import(dbio, bio, GWEN_PATH_FLAGS_CREATE_GROUP,
                       dbTransfers, dbCfg)) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Error re-importing DTAUS object");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    GWEN_DB_Group_free(dbTransfers);
    GWEN_DB_Group_free(dbCfg);
    AD_Job_SetResult(dj, AB_ERROR_GENERIC,
                     I18N("Could not re-import DTAUS data"));
    return AB_ERROR_GENERIC;
  }
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  GWEN_DB_Group_free(dbCfg);

  /* ok, now we have the DTAUS object still in the buffer and dbTransfers
   * contains a group with the E-set data needed for the "Begleitzettel" */

  /* mount if needed */
  if (AD_Account_GetUseDisc(da)) {
    int rv;
    const char *discMsg=I18N_NOOP
      (
       "Please label a disc with \"Vol %d\"\n"
       "and insert it into the disc drive"
       "<html>"
       "Please label a disc with <b>Vol %d</b>"
       " and insert it into the disc drive"
       "</html>"
      );
    char msgBuf[512];

    snprintf(msgBuf, sizeof(msgBuf),
             I18N(discMsg),
             AD_Job_GetJobId(dj),
             AD_Job_GetJobId(dj));
    rv=AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                             AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                             AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                             AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                             I18N("Insert Disc"),
                             msgBuf,
                             I18N("Ok"), I18N("Abort"), 0);
    if (rv!=1) {
      DBG_ERROR(AQDTAUS_LOGDOMAIN, "User aborted");
      GWEN_Buffer_free(buf);
      GWEN_DB_Group_free(dbTransfers);
      if (mounted)
        AD_Provider_UnmountDisc(pro, da);
      AD_Job_SetResult(dj, AB_ERROR_USER_ABORT, I18N("User aborted"));
      return AB_ERROR_USER_ABORT;
    }

    if (AD_Account_GetMountAllowed(da)) {
      AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                             AB_Banking_LogLevelNotice,
                             I18N("Mounting disc"));
      rv=AD_Provider_MountDisc(pro, da);
      if (rv) {
        DBG_INFO(AQDTAUS_LOGDOMAIN, "here");
        GWEN_Buffer_free(buf);
        GWEN_DB_Group_free(dbTransfers);
        AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                              AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                              AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                              AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                              I18N("Error"),
                              I18N("Could not mount disc.\n"
                                   "Please see console logs for details."
                                   "<html>"
                                   "<p>"
                                   "Could not mount disc."
                                   "Please see console logs for details."
                                   "</p>"
                                   "</html>"),
                              I18N("Dismiss"), 0, 0);
        AD_Job_SetResult(dj, rv, I18N("Could not mount disc"));
        return rv;
      }
      mounted=1;
    } /* if mounting allowed */

    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           I18N("Checking whether disc is empty"));
    for (;;) {
      rv=AD_Provider_CheckEmptyDir(AD_Account_GetFolder(da));
      if (rv==AB_ERROR_FOUND) {
        /* disc not empty */
        rv=AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                                 AB_BANKING_MSG_FLAGS_TYPE_WARN |
                                 AB_BANKING_MSG_FLAGS_CONFIRM_B2 |
                                 AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                                 I18N("Warning"),
                                 I18N("The disc is not empty.\n"
                                      "Please insert an empty disc and click "
                                      " \"OK\""
                                      "<html>"
                                      "<p>"
                                      "The disc is not empty."
                                      "</p>"
                                      "<p>"
                                      "Please insert an <b>empty</b> disc and"
                                      " click <i>OK</i>"
                                      "</p>"
                                      "</html>"),
                                 I18N("Ok"),
                                 I18N("Use This Disc"),
                                 I18N("Abort"));
        if (rv==2)
          break;
        else if (rv!=1) {
          DBG_ERROR(AQDTAUS_LOGDOMAIN, "User aborted");
          GWEN_Buffer_free(buf);
          GWEN_DB_Group_free(dbTransfers);
          if (mounted)
            AD_Provider_UnmountDisc(pro, da);
          AD_Job_SetResult(dj, AB_ERROR_USER_ABORT, I18N("User aborted"));
          return AB_ERROR_USER_ABORT;
        }
      }
      else if (rv) {
        DBG_INFO(AQDTAUS_LOGDOMAIN, "here");
        AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                              AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                              AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                              AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                              I18N("Error"),
                              I18N("Could not open disc folder.\n"
                                   "Please see console logs for details."
                                   "<html>"
                                   "<p>"
                                   "Could not open disc folder."
                                   "Please see console logs for details."
                                   "</p>"
                                   "</html>"),
                              I18N("Dismiss"), 0, 0);
        GWEN_Buffer_free(buf);
        GWEN_DB_Group_free(dbTransfers);
        if (mounted)
          AD_Provider_UnmountDisc(pro, da);
        AD_Job_SetResult(dj, rv, I18N("Could not open disc folder"));
        return rv;
      }
      else
        break;
    }

  } /* if useDisc */

  /* write file */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                         AB_Banking_LogLevelNotice,
                         I18N("Saving DTAUS object"));
  rv=AD_Provider_WriteDTAUS(pro, da, buf);
  if (rv==0)
    rv=AD_Provider_SaveJob(pro, dj, buf);

  /* unmount if mounted */
  if (mounted) {
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           I18N("Unmounting disc"));
    AD_Provider_UnmountDisc(pro, da);
  }

  /* check result */
  if (rv) {
    AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                          AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                          AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                          AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                          I18N("Error"),
                          I18N("Could not create DTAUS file.\n"
                               "Please see console logs for details."
                               "<html>"
                               "<p>"
                               "Could not create DTAUS file."
                               "Please see console logs for details."
                               "</p>"
                               "</html>"),
                          I18N("Dismiss"), 0, 0);
    AD_Job_SetResult(dj, rv, I18N("Could not write DTAUS file"));
    GWEN_Buffer_free(buf);
    GWEN_DB_Group_free(dbTransfers);
    return rv;
  }
  else
    AD_Job_SetResult(dj, 0, I18N("Ok."));

  /* print "Begleitzettel" */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                         AB_Banking_LogLevelNotice,
                         I18N("Printing Begleitzettel"));
  rv=AD_Provider_PrintBegleitZettel(pro, dj, dbTransfers);
  if (rv) {
    if (rv==AB_ERROR_NOT_SUPPORTED) {
      AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                            AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                            AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                            AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                            I18N("Warning"),
                            I18N("Your application does not support "
                                 "printing.\n"
                                 "Therefore the Begleitzettel can not be "
                                 "printed."
                                 "<html>"
                                 "<p>"
                                 "Your application does not support "
                                 "printing."
                                 "</p>"
                                 "<p>"
                                 "Therefore the Begleitzettel can not be "
                                 "printed."
                                 "</p>"
                                 "</html>"),
                            I18N("Dismiss"), 0, 0);
    }
    else {
      AB_Banking_MessageBox(AB_Provider_GetBanking(pro),
                            AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                            AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                            AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                            I18N("Error"),
                            I18N("Could not print Begleitzettel.\n"
                                 "Please see console logs for details."
                                 "<html>"
                                 "<p>"
                                 "Could not print Begleitzettel"
                                 "Please see console logs for details."
                                 "</p>"
                               "</html>"),
                            I18N("Dismiss"), 0, 0);
    }

  }

  GWEN_Buffer_free(buf);
  GWEN_DB_Group_free(dbTransfers);
  return 0;
}



int AD_Provider__Execute(AB_PROVIDER *pro){
  AD_PROVIDER *dp;
  AD_JOB *dj;
  int succeeded=0;
  int done;
  AB_JOB_LIST2_ITERATOR *ait;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  dj=AD_Job_List_First(dp->myJobs);
  if (!dj) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "No jobs");
  }
  done=0;
  while(dj) {
    int rv;

    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Handling job");
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           I18N("Handling job"));
    rv=AD_Provider_ProcessJob(pro, dj);
    if (rv) {
      DBG_INFO(AQDTAUS_LOGDOMAIN, "Error processing job (%d)", rv);
    }
    else
      succeeded++;
    done++;
    if (AB_Banking_ProgressAdvance(AB_Provider_GetBanking(pro),
                                   0, done)) {
      DBG_ERROR(AQDTAUS_LOGDOMAIN, "User aborted via waitcallback");
      return AB_ERROR_USER_ABORT;
    }
    dj=AD_Job_List_Next(dj);
  }

  /* exchange job results */
  ait=AB_Job_List2_First(dp->bankingJobs);
  if (ait) {
    AB_JOB *bj;

    bj=AB_Job_List2Iterator_Data(ait);
    assert(bj);
    while(bj) {
      dj=AD_Provider_FindMyJob(pro, AB_Job_GetIdForProvider(bj));
      if (dj) {
        int res;

        res=AD_Job_GetResultCode(dj);
        if (res)
          AB_Job_SetStatus(bj, AB_Job_StatusError);
        else
          AB_Job_SetStatus(bj, AB_Job_StatusFinished);
        AB_Job_SetResultText(bj, AD_Job_GetResultText(dj));
      }
      else {
        AB_Job_SetStatus(bj, AB_Job_StatusError);
        AB_Job_SetResultText(bj, I18N("Internal error: Job not found"));
      }

      bj=AB_Job_List2Iterator_Next(ait);
    } /* while */
    AB_Job_List2Iterator_free(ait);
  }

  if (succeeded==0) {
    DBG_ERROR(AQDTAUS_LOGDOMAIN, "Not a single job succeeded");
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AD_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx){
  AD_PROVIDER *dp;
  GWEN_TYPE_UINT32 pid;
  int cnt;
  int rv;
  const char *msg=I18N_NOOP
    (
     "Please prepare %d disc(s).\n"
     "These discs will be filled with DTAUS data sets.\n"
     "<html>"
     "<p>"
     "Please prepare <b>%d</b> disc(s)."
     "</p>"
     "<p>"
     "These discs will be filled with DTAUS data sets."
     "</p>"
     "</html>"
    );
  char msgBuf[512];

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AD_PROVIDER, pro);
  assert(dp);

  cnt=AD_Job_List_GetCount(dp->myJobs);
  snprintf(msgBuf, sizeof(msgBuf),
           I18N(msg), cnt, cnt);

  pid=AB_Banking_ProgressStart(AB_Provider_GetBanking(pro),
                               I18N("Creating DTAUS Disc(s)"), msgBuf, cnt);
  rv=AD_Provider__Execute(pro);
  AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);

  return rv;
}



int AD_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                              AB_PROVIDER_EXTEND_MODE em){
  AD_Account_Extend(a, pro, em);
  return 0;
}









