/***************************************************************************
    begin       : Thu Nov 29 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included by provider.c */

#include "aqofxconnect/v2/r_accounts.h"




int AO_Provider__AddAccountInfoReq(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf)
{
  int rv;

  GWEN_Buffer_AppendString(buf, "<ACCTINFORQ>\r\n");
  GWEN_Buffer_AppendString(buf, "<DTACCTUP>19691231\r\n");
  GWEN_Buffer_AppendString(buf, "</ACCTINFORQ>\r\n");

  /* wrap into request */
  rv=AO_Provider__WrapRequest(pro, u, "SIGNUP", "ACCTINFO", buf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider_RequestAccounts(AB_PROVIDER *pro, AB_USER *u, int keepOpen)
{
  AO_PROVIDER *dp;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ictx;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
                             (keepOpen?GWEN_GUI_PROGRESS_KEEP_OPEN:0) |
                             GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Requesting account list"),
                             I18N("We are now requesting a list of "
                                  "accounts\n"
                                  "which can be managed via OFX.\n"
                                  "<html>"
                                  "We are now requesting a list of "
                                  "accounts "
                                  "which can be managed via <i>OFX</i>.\n"
                                  "</html>"),
                             1,
                             0);

  ictx=AB_ImExporterContext_new();
  rv=AO_V2_RequestAccounts(pro, u, ictx);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Gui_ProgressEnd(pid);
    AB_ImExporterContext_free(ictx);
    return rv;
  }

  /* create accounts */
  rv=AO_Provider__ProcessImporterContext(pro, u, ictx);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Error importing accounts (%d)", rv);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Error,
                         I18N("Error importing accounts"));
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }


  AB_ImExporterContext_free(ictx);

  GWEN_Gui_ProgressEnd(pid);
  return 0;

#if 0
  AO_PROVIDER *dp;
  GWEN_BUFFER *reqbuf;
  GWEN_BUFFER *rbuf=NULL;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ictx;

  assert(u);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
                             (keepOpen?GWEN_GUI_PROGRESS_KEEP_OPEN:0) |
                             GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Requesting account list"),
                             I18N("We are now requesting a list of "
                                  "accounts\n"
                                  "which can be managed via OFX.\n"
                                  "<html>"
                                  "We are now requesting a list of "
                                  "accounts "
                                  "which can be managed via <i>OFX</i>.\n"
                                  "</html>"),
                             1,
                             0);
  ictx=AB_ImExporterContext_new();

  reqbuf=GWEN_Buffer_new(0, 2048, 0, 1);
  GWEN_Buffer_ReserveBytes(reqbuf, 1024);

  /* add actual request */
  rv=AO_Provider__AddAccountInfoReq(pro, u, reqbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }

  /* wrap message (adds headers etc) */
  rv=AO_Provider__WrapMessage(pro, u, reqbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }

  /* exchange mesages (could also return HTTP code!) */
  rv=AO_Provider_SendAndReceive(pro, u,
                                (const uint8_t *)GWEN_Buffer_GetStart(reqbuf),
                                GWEN_Buffer_GetUsedBytes(reqbuf),
                                &rbuf);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error exchanging getAccounts-request (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }
  else {
    GWEN_DB_NODE *dbProfile;

    /* parse response */
    GWEN_Buffer_free(reqbuf);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Info,
                         I18N("Parsing response"));

    /* prepare import */
    dbProfile=GWEN_DB_Group_new("profile");
    /* actually import */
    rv=AB_Banking_ImportFromBuffer(AB_Provider_GetBanking(pro),
                                   "ofx", ictx,
                                   (const uint8_t *) GWEN_Buffer_GetStart(rbuf),
                                   GWEN_Buffer_GetUsedBytes(rbuf),
                                   dbProfile);
    GWEN_DB_Group_free(dbProfile);
    GWEN_Buffer_free(rbuf);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "Error importing server response (%d)", rv);
      GWEN_Gui_ProgressLog(pid,
                           GWEN_LoggerLevel_Error,
                           I18N("Error parsing response"));
      AB_ImExporterContext_free(ictx);
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }

    /* create accounts */
    rv=AO_Provider__ProcessImporterContext(pro, u, ictx);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "Error importing accounts (%d)", rv);
      GWEN_Gui_ProgressLog(pid,
                           GWEN_LoggerLevel_Error,
                           I18N("Error importing accounts"));
      AB_ImExporterContext_free(ictx);
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }
  }

  AB_ImExporterContext_free(ictx);
  GWEN_Gui_ProgressEnd(pid);
  return rv;
#endif
}



