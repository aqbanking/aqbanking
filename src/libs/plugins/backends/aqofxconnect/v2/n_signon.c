/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "n_signon.h"
#include "n_utils.h"
#include "aqofxconnect/user.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>




/*
 * <SIGNONMSGSRQV1>
 *   <SONRQ>
 *     <DTCLIENT>YYYYMMDDhhmmss.000</DTCLIENT>
 *     <USERID>user</USERID>
 *     <USERPASS>secret</USERPASS>
 *     <LANGUAGE>ENG</LANGUAGE>
 *     <FI>
 *       <ORG>org</ORG>
 *       <FID>1234</FID>
 *     </FI>
 *     <APPID>MyApp</APPID>
 *     <APPVER>0500</APPVER>
 *   </SONRQ>
 * </SIGNONMSGSRQV1>
 */




GWEN_XMLNODE *AO_V2_MkSignOnNode(AB_USER *u)
{
  GWEN_XMLNODE *xmlSignonMsg;
  GWEN_XMLNODE *xmlSignonRq;
  const char *s;

  xmlSignonMsg=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SIGNONMSGSRQV1");
  xmlSignonRq=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SONRQ");
  GWEN_XMLNode_AddChild(xmlSignonMsg, xmlSignonRq);

  AO_V2_Util_SetCurrentTimeValue(xmlSignonRq, AO_User_GetFlags(u), "DTCLIENT");

  s=AB_User_GetUserId(u);
  if (s && *s)
    GWEN_XMLNode_SetCharValue(xmlSignonRq, "USERID", s);

  if (1) {
    char userpass[64];

    /* get password */
    userpass[0]=0;
    while (strlen(userpass)<4) {
      GWEN_BUFFER *nbuf;
      int rv;
      char msg[]=I18S("Please enter the password for user %s"
                      "<html>"
                      "Please enter the password for user <b>%s</b>"
                      "</html>");
      char msgbuf[512];

      nbuf=GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_Buffer_AppendString(nbuf, "OFX::userpass::");
      GWEN_Buffer_AppendString(nbuf, s);
      snprintf(msgbuf, sizeof(msgbuf), I18N(msg), s, s);
      rv=GWEN_Gui_GetPassword(0,
                              GWEN_Buffer_GetStart(nbuf),
                              I18N("Enter Password"),
                              msgbuf,
                              userpass,
                              4,
                              sizeof(userpass),
                              GWEN_Gui_PasswordMethod_Text, NULL,
                              0);
      GWEN_Buffer_free(nbuf);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        memset(userpass, 0, sizeof(userpass));
        GWEN_XMLNode_free(xmlSignonMsg);
        return NULL;
      }
    } /* while */

    GWEN_XMLNode_SetCharValue(xmlSignonRq, "USERPASS", userpass);
    memset(userpass, 0, sizeof(userpass));
  }

  GWEN_XMLNode_SetCharValue(xmlSignonRq, "LANG", "ENG");

  if (!(AO_User_GetFlags(u) & AO_USER_FLAGS_EMPTY_FID) && AO_User_GetFid(u)) {
    GWEN_XMLNODE *xmlFi;

    xmlFi=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FI");
    GWEN_XMLNode_AddChild(xmlSignonRq, xmlFi);
    s=AO_User_GetOrg(u);
    if (s)
      GWEN_XMLNode_SetCharValue(xmlFi, "ORG", s);
    s=AO_User_GetFid(u);
    if (s)
      GWEN_XMLNode_SetCharValue(xmlFi, "FID", s);
  }

  s=AO_User_GetAppId(u);
  GWEN_XMLNode_SetCharValue(xmlSignonRq, "APPID", s?s:"QWIN");

  s=AO_User_GetAppVer(u);
  GWEN_XMLNode_SetCharValue(xmlSignonRq, "APPVER", s?s:"1200");

  s=AO_User_GetClientUid(u);
  if (s)
    GWEN_XMLNode_SetCharValue(xmlSignonRq, "CLIENTUID", s);

  return xmlSignonMsg;
}






