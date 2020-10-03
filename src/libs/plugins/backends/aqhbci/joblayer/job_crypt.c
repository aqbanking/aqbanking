/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "job_swift.h"

#include "aqhbci/banking/user_l.h"

#include <aqbanking/i18n_l.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>




int AH_Job_CheckEncryption(AH_JOB *j, GWEN_DB_NODE *dbRsp)
{
  AB_USER *user;
  const char *sExpectedCrypter;

  assert(j);

  user=AH_Job_GetUser(j);
  assert(user);

  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Not checking security in PIN/TAN mode");
  }
  else {
    GWEN_DB_NODE *dbSecurity;
    const char *s;

    assert(dbRsp);
    dbSecurity=GWEN_DB_GetGroup(dbRsp, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "security");
    if (!dbSecurity) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No security settings, should not happen");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Response without security info (internal)"));
      return AB_ERROR_SECURITY;
    }

    s=GWEN_DB_GetCharValue(dbSecurity, "crypter", 0, 0);
    if (s) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Response encrypted with key [%s]", s);

      if (*s=='!' || *s=='?') {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Encrypted with invalid key (%s)", s);
        GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Error,
                             I18N("Response encrypted with invalid key"));
        return AB_ERROR_SECURITY;
      }
    }

    sExpectedCrypter=AH_Job_GetExpectedCrypter(j);
    if (sExpectedCrypter) {
      /* check crypter */
      if (!s) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Response is not encrypted (but expected to be)");
        GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Error,
                             I18N("Response is not encrypted as expected"));
        return AB_ERROR_SECURITY;

      }

      if (strcasecmp(s, sExpectedCrypter)!=0) {
        DBG_WARN(AQHBCI_LOGDOMAIN,
                 "Not encrypted with the expected key "
                 "(exp: \"%s\", is: \"%s\"",
                 sExpectedCrypter, s);
        /*
         GWEN_Gui_ProgressLog(
                              0,
                              GWEN_LoggerLevel_Error,
                              I18N("Response not encrypted with expected key"));
         return AB_ERROR_SECURITY;
         */
      }
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No specific encrypter expected");
    }
  }

  return 0;
}



int AH_Job_CheckSignature(AH_JOB *j, GWEN_DB_NODE *dbRsp)
{
  AB_USER *user;
  const char *sExpectedSigner;

  assert(j);

  user=AH_Job_GetUser(j);
  assert(user);

  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Not checking signature in PIN/TAN mode");
  }
  else {
    GWEN_DB_NODE *dbSecurity;
    int i;
    uint32_t uFlags;

    assert(j);

    uFlags=AH_User_GetFlags(user);

    assert(dbRsp);
    dbSecurity=GWEN_DB_GetGroup(dbRsp, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                "security");
    if (!dbSecurity) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"No security settings, should not happen");
      GWEN_Gui_ProgressLog(
			   0,
			   GWEN_LoggerLevel_Error,
			   I18N("Response without security info (internal)"));
      return GWEN_ERROR_GENERIC;
    }

    /* check for invalid signers */
    for (i=0; ; i++) {
      const char *s;

      s=GWEN_DB_GetCharValue(dbSecurity, "signer", i, 0);
      if (!s)
        break;
      if (*s=='!') {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Invalid signature found, will not tolerate it");
        GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Error,
                             I18N("Invalid bank signature"));
        return AB_ERROR_SECURITY;
      }
    } /* for */

    sExpectedSigner=AH_Job_GetExpectedSigner(j);
    if (sExpectedSigner && !(uFlags & AH_USER_FLAGS_BANK_DOESNT_SIGN)) {
      /* check signer */
      for (i=0; ; i++) {
        const char *s;

        s=GWEN_DB_GetCharValue(dbSecurity, "signer", i, 0);
        if (!s) {
          DBG_ERROR(AQHBCI_LOGDOMAIN,
                    "Not signed by expected signer (%d)", i);
          GWEN_Gui_ProgressLog(0,
                               GWEN_LoggerLevel_Error,
                               I18N("Response not signed by the bank"));
          if (i==0) {
            int button;

            /* check whether the user want's to accept the unsigned message */
            button=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
				       GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
				       GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
				       I18N("Security Warning"),
				       I18N("The HBCI response of the bank has not been signed by the bank, \n"
					    "contrary to what has been expected. This can be the case because the \n"
					    "bank just stopped signing their HBCI responses. This error message \n"
					    "would also occur if there were a replay attack against your computer \n"
					    "in progress right now, which is probably quite unlikely. \n"
					    " \n"
					    "Please contact your bank and ask them whether their HBCI server \n"
					    "stopped signing the HBCI responses. If the bank is concerned about \n"
					    "your security, it should not stop signing the HBCI responses. \n"
					    " \n"
					    "Do you nevertheless want to accept this response this time or always?"
					    "<html><p>"
					    "The HBCI response of the bank has not been signed by the bank, \n"
					    "contrary to what has been expected. This can be the case because the \n"
					    "bank just stopped signing their HBCI responses. This error message \n"
					    "would also occur if there were a replay attack against your computer \n"
					    "in progress right now, which is probably quite unlikely. \n"
					    "</p><p>"
					    "Please contact your bank and ask them whether their HBCI server \n"
					    "stopped signing the HBCI responses. If the bank is concerned about \n"
					    "your security, it should not stop signing the HBCI responses. \n"
					    "</p><p>"
					    "Do you nevertheless want to accept this response this time or always?"
					    "</p></html>"
					   ),
				       I18N("Accept this time"),
				       I18N("Accept always"),
				       I18N("Abort"), 0);
	    if (button==1) {
	      GWEN_Gui_ProgressLog(0,
                                   GWEN_LoggerLevel_Notice,
                                   I18N("User accepts this unsigned "
                                        "response"));
              AH_Job_SetExpectedSigner(j, 0);
              break;
            }
            else if (button==2) {
              GWEN_Gui_ProgressLog(0,
                                   GWEN_LoggerLevel_Notice,
                                   I18N("User accepts all further unsigned "
                                        "responses"));
              AH_User_AddFlags(user, AH_USER_FLAGS_BANK_DOESNT_SIGN);
              AH_Job_SetExpectedSigner(j, NULL);
              break;
            }
            else {
              GWEN_Gui_ProgressLog(0,
                                   GWEN_LoggerLevel_Error,
                                   I18N("Aborted"));
              return AB_ERROR_SECURITY;
            }
          }
          else {
            int ii;

            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Job signed with unexpected key(s)"
                      "(was expecting \"%s\"):",
                      sExpectedSigner);
            for (ii=0; ; ii++) {
              s=GWEN_DB_GetCharValue(dbSecurity, "signer", ii, 0);
              if (!s)
                break;
              DBG_ERROR(AQHBCI_LOGDOMAIN,
                        "Signed unexpectedly with key \"%s\"", s);
            }
            return AB_ERROR_SECURITY;
          }
        }
        else {
          if (strcasecmp(s, sExpectedSigner)==0) {
            DBG_DEBUG(AQHBCI_LOGDOMAIN,
                      "Jobs signed as expected with \"%s\"",
                      sExpectedSigner);
            break;
          }
          else if (*s!='!' && *s!='?') {
            DBG_INFO(AQHBCI_LOGDOMAIN,
                     "Signer name does not match expected name (%s!=%s), "
		     "but we accept it anyway",
		     s, sExpectedSigner);
            break;
          }
        }
      } /* for */
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Signature check ok");
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signature expected");
    }
  }
  return 0;
}

