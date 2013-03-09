/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_status_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_STATUS)


typedef struct AIO_OFX_GROUP_STATUS_ERROR AIO_OFX_GROUP_STATUS_ERROR;
struct AIO_OFX_GROUP_STATUS_ERROR {
  int code;           /**< The error's code */
  const char *name;        /**< The error's name */
  const char *description; /**< The long description of the error */
};



/* this list has been copied from LibOFX */
static AIO_OFX_GROUP_STATUS_ERROR error_msg_list[] = {
{0,    I18S("Success"), I18S("The server successfully processed the request.")},
{1,    I18S("Client is up-to-date"), I18S("Based on the client timestamp, the client has the latest information. The response does not supply any additional information.")},
{2000, I18S("General error"), I18S("Error other than those specified by the remaining error codes. (Note: Servers should provide a more specific error whenever possible. Error code 2000 should be reserved for cases in which a more specific code is not available.)")},
{2001, I18S("Invalid account"), I18S("The account was invalid (whatever that means)")},
{2002, I18S("General account error"), I18S("Account error not specified by the remaining error codes.")},
{2003, I18S("Account not found"), I18S("The specified account number does not correspond to one of the user's accounts.")},
{2004, I18S("Account closed"), I18S("The specified account number corresponds to an account that has been closed.")},
{2005, I18S("Account not authorized"), I18S("The user is not authorized to perform this action on the account, or the server does not allow this type of action to be performed on the account.")},
{2006, I18S("Source account not found"), I18S("The specified account number does not correspond to one of the user's accounts.")},
{2007, I18S("Source account closed"), I18S("The specified account number corresponds to an account that has been closed.")},
{2008, I18S("Source account not authorized"), I18S("The user is not authorized to perform this action on the account, or the server does not allow this type of action to be performed on the account.")},
{2009, I18S("Destination account not found"), I18S("The specified account number does not correspond to one of the user's accounts.")},
{2010, I18S("Destination account closed"), I18S("The specified account number corresponds to an account that has been closed.")},
{2011, I18S("Destination account not authorized"), I18S("The user is not authorized to perform this action on the account, or the server does not allow this type of action to be performed on the account.")},
{2012, I18S("Invalid amount"), I18S("The specified amount is not valid for this action; for example, the user specified a negative payment amount.")},
{2014, I18S("Date too soon"), I18S("The server cannot process the requested action by the date specified by the user.")},
{2015, I18S("Date too far in future"), I18S("The server cannot accept requests for an action that far in the future.")},
{2016, I18S("Transaction already committed"), I18S("Transaction has entered the processing loop and cannot be modified/cancelled using OFX. The transaction may still be cancelled or modified using other means (for example, a phone call to Customer Service).")},
{2017, I18S("Already canceled"), I18S("The transaction cannot be canceled or modified because it has already been canceled.")},
{2018, I18S("Unknown server ID"), I18S("The specified server ID does not exist or no longer exists.")},
{2019, I18S("Duplicate request"), I18S("A request with this <TRNUID> has already been received and processed.")},
{2020, I18S("Invalid date"), I18S("The specified datetime stamp cannot be parsed; for instance, the datetime stamp specifies 25:00 hours.")},
{2021, I18S("Unsupported version"), I18S("The server does not support the requested version. The version of the message set specified by the client is not supported by this server.")},
{2022, I18S("Invalid TAN"), I18S("The server was unable to validate the TAN sent in the request.")},
{2023, I18S("Unknown FITID"), I18S("The specified FITID/BILLID does not exist or no longer exists. [BILLID not found in the billing message sets]")},
{2025, I18S("Branch ID missing"), I18S("A <BRANCHID> value must be provided in the <BANKACCTFROM> aggregate for this country system, but this field is missing.")},
{2026, I18S("Bank name does not match bank ID"), I18S("The value of <BANKNAME> in the <EXTBANKACCTTO> aggregate is inconsistent with the value of <BANKID> in the <BANKACCTTO> aggregate.")},
{2027, I18S("Invalid date range"), I18S("Response for non-overlapping dates, date ranges in the future, et cetera.")},
{2028, I18S("Requested element unknown"), I18S("One or more elements of the request were not recognized by the server or the server (as noted in the FI Profile) does not support the elements. The server executed the element transactions it understood and supported. For example, the request file included private tags in a <PMTRQ> but the server was able to execute the rest of the request.")},
{6500, I18S("<REJECTIFMISSING>Y invalid without <TOKEN>"), I18S("This error code may appear <SYNCERROR> element of an <xxxSYNCRS> wrapper (in <PRESDLVMSGSRSV1> and V2 message set responses) or the <CODE> contained in any embedded transaction wrappers within a sync response. The corresponding sync request wrapper included <REJECTIFMISSING>Y with <REFRESH>Y or <TOKENONLY>Y, which is illegal.")},
{6501, I18S("Embedded transactions in request failed to process: Out of date"), I18S("<REJECTIFMISSING>Y and embedded transactions appeared in the request sync wrapper and the provided <TOKEN> was out of date. This code should be used in the <SYNCERROR> of the response sync wrapper.")},
{6502, I18S("Unable to process embedded transaction due to out-of-date <TOKEN>"), I18S("Used in response transaction wrapper for embedded transactions when <SYNCERROR>6501 appears in the surrounding sync wrapper.")},
{10000, I18S("Stop check in process"), I18S("Stop check is already in process.")},
{10500, I18S("Too many checks to process"), I18S("The stop-payment request <STPCHKRQ> specifies too many checks.")},
{10501, I18S("Invalid payee"), I18S("Payee error not specified by the remainingerror codes.")},
{10502, I18S("Invalid payee address"), I18S("Some portion of the payee's address is incorrect or unknown.")},
{10503, I18S("Invalid payee account number"), I18S("The account number <PAYACCT> of the requested payee is invalid.")},
{10504, I18S("Insufficient funds"), I18S("The server cannot process the request because the specified account does not have enough funds.")},
{10505, I18S("Cannot modify element"), I18S("The server does not allow modifications to one or more values in a modification request.")},
{10506, I18S("Cannot modify source account"), I18S("Reserved for future use.")},
{10507, I18S("Cannot modify destination account"), I18S("Reserved for future use.")},
{10508, I18S("Invalid frequency"), I18S("The specified frequency <FREQ> does not match one of the accepted frequencies for recurring transactions.")},
{10509, I18S("Model already canceled"), I18S("The server has already canceled the specified recurring model.")},
{10510, I18S("Invalid payee ID"), I18S("The specified payee ID does not exist or no longer exists.")},
{10511, I18S("Invalid payee city"), I18S("The specified city is incorrect or unknown.")},
{10512, I18S("Invalid payee state"), I18S("The specified state is incorrect or unknown.")},
{10513, I18S("Invalid payee postal code"), I18S("The specified postal code is incorrect or unknown.")},
{10514, I18S("Transaction already processed"), I18S("Transaction has already been sent or date due is past")},
{10515, I18S("Payee not modifiable by client"), I18S("The server does not allow clients to change payee information.")},
{10516, I18S("Wire beneficiary invalid"), I18S("The specified wire beneficiary does not exist or no longer exists.")},
{10517, I18S("Invalid payee name"), I18S("The server does not recognize the specified payee name.")},
{10518, I18S("Unknown model ID"), I18S("The specified model ID does not exist or no longer exists.")},
{10519, I18S("Invalid payee list ID"), I18S("The specified payee list ID does not exist or no longer exists.")},
{10600, I18S("Table type not found"), I18S("The specified table type is not recognized or does not exist.")},
{12250, I18S("Investment transaction download not supported (WARN)"), I18S("The server does not support investment transaction download.")},
{12251, I18S("Investment position download not supported (WARN)"), I18S("The server does not support investment position download.")},
{12252, I18S("Investment positions for specified date not available"), I18S("The server does not support investment positions for the specified date.")},
{12253, I18S("Investment open order download not supported (WARN)"), I18S("The server does not support open order download.")},
{12254, I18S("Investment balances download not supported (WARN)"), I18S("The server does not support investment balances download.")},
{12255, I18S("401(k) not available for this account"), I18S("401(k) information requested from a non-401(k) account.")},
{12500, I18S("One or more securities not found"), I18S("The server could not find the requested securities.")},
{13000, I18S("User ID & password will be sent out-of-band (INFO)"), I18S("The server will send the user ID and password via postal mail, e-mail, or another means. The accompanying message will provide details.")},
{13500, I18S("Unable to enroll user"), I18S("The server could not enroll the user.")},
{13501, I18S("User already enrolled"), I18S("The server has already enrolled the user.")},
{13502, I18S("Invalid service"), I18S("The server does not support the service <SVC> specified in the service-activation request.")},
{13503, I18S("Cannot change user information"), I18S("The server does not support the <CHGUSERINFORQ> request.")},
{13504, I18S("<FI> Missing or Invalid in <SONRQ>"), I18S("The FI requires the client to provide the <FI> aggregate in the <SONRQ> request, but either none was provided, or the one provided was invalid.")},
{14500, I18S("1099 forms not available"), I18S("1099 forms are not yet available for the tax year requested.")},
{14501, I18S("1099 forms not available for user ID"), I18S("This user does not have any 1099 forms available.")},
{14600, I18S("W2 forms not available"), I18S("W2 forms are not yet available for the tax year requested.")},
{14601, I18S("W2 forms not available for user ID"), I18S("The user does not have any W2 forms available.")},
{14700, I18S("1098 forms not available"), I18S("1098 forms are not yet available for the tax year requested.")},
{14701, I18S("1098 forms not available for user ID"), I18S("The user does not have any 1098 forms available.")},
{15000, I18S("Must change USERPASS"), I18S("The user must change his or her <USERPASS> number as part of the next OFX request.")},
{15500, I18S("Signon invalid"), I18S("The user cannot signon because he or she entered an invalid user ID or password.")},
{15501, I18S("Customer account already in use"), I18S("The server allows only one connection at a time, and another user is already signed on. Please try again later.")},
{15502, I18S("USERPASS lockout"), I18S("The server has received too many failed signon attempts for this user. Please call the FI's technical support number.")},
{15503, I18S("Could not change USERPASS"), I18S("The server does not support the <PINCHRQ> request.")},
{15504, I18S("Could not provide random data"), I18S("The server could not generate random data as requested by the <CHALLENGERQ>.")},
{15505, I18S("Country system not supported"), I18S("The server does not support the country specified in the <COUNTRY> field of the <SONRQ> aggregate.")},
{15506, I18S("Empty signon not supported"), I18S("The server does not support signons not accompanied by some other transaction.")},
{15507, I18S("Signon invalid without supporting pin change request"), I18S("The OFX block associated with the signon does not contain a pin change request and should.")},
{15508, I18S("Transaction not authorized"), I18S("Current user is not authorized to perform this action on behalf of the <USERID>.")},
{16500, I18S("HTML not allowed"), I18S("The server does not accept HTML formatting in the request.")},
{16501, I18S("Unknown mail To:"), I18S("The server was unable to send mail to the specified Internet address.")},
{16502, I18S("Invalid URL"), I18S("The server could not parse the URL.")},
{16503, I18S("Unable to get URL"), I18S("The server was unable to retrieve the information at this URL (e.g., an HTTP 400 or 500 series error).")},
{-1, I18S("Unknown code"), I18S("No description for this code")}
};



const AIO_OFX_GROUP_STATUS_ERROR*
AIO_OfxGroup_STATUS__getErrorStruct(int e) {
  int i;

  for (i=0; ; i++) {
    if (error_msg_list[i].code==e)
      return &error_msg_list[i];

    if (error_msg_list[i].code==-1)
      break;
  }
  return NULL;
}



AIO_OFX_GROUP *AIO_OfxGroup_STATUS_new(const char *groupName,
				       AIO_OFX_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx,
				       const char *description) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_STATUS *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_STATUS, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STATUS, g, xg,
                       AIO_OfxGroup_STATUS_FreeData);

  if (description)
    xg->description=strdup(description);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_STATUS_StartTag);
  xg->oldEndTagFn=AIO_OfxGroup_SetEndTagFn(g, AIO_OfxGroup_STATUS_EndTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_STATUS_AddData);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_STATUS_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_STATUS *xg;

  xg=(AIO_OFX_GROUP_STATUS*)p;
  assert(xg);
  free(xg->currentElement);
  free(xg->severity);
  free(xg->description);
  GWEN_FREE_OBJECT(xg);
}



int AIO_OfxGroup_STATUS_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_STATUS *xg;
  //GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STATUS, g);
  assert(xg);

  //ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "CODE")==0) {
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "SEVERITY")==0) {
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "MESSAGE")==0) {
    xg->currentElement=strdup(tagName);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
  }

  return 0;
}



int AIO_OfxGroup_STATUS_EndTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_STATUS *xg;
  //GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STATUS, g);
  assert(xg);

  //ctx=AIO_OfxGroup_GetXmlContext(g);

  assert(tagName);
  if (strcasecmp(tagName, AIO_OfxGroup_GetGroupName(g))!=0) {
    /* tag does not close this one */
    DBG_DEBUG(AQBANKING_LOGDOMAIN,
	      "Tag [%s] does not close [%s], ignoring",
	      tagName, AIO_OfxGroup_GetGroupName(g));
    return 0;
  }

  /* show status message */
  if (xg->description) {
    GWEN_BUFFER *buf;
    char numbuf[32];
    const AIO_OFX_GROUP_STATUS_ERROR *e;

    e=AIO_OfxGroup_STATUS__getErrorStruct(xg->code);
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(buf, xg->description);
    GWEN_Buffer_AppendString(buf, ": ");

    /* append error string if available */
    if (e && e->name) {
      GWEN_Buffer_AppendString(buf, I18N(e->name));
      GWEN_Buffer_AppendString(buf, " (");
    }
    GWEN_Buffer_AppendString(buf, I18N("Code"));
    GWEN_Buffer_AppendString(buf, " ");
    snprintf(numbuf, sizeof(numbuf)-1, "%d", xg->code);
    numbuf[sizeof(numbuf)-1]=0;
    GWEN_Buffer_AppendString(buf, numbuf);
    if (xg->severity) {
      GWEN_Buffer_AppendString(buf, ", ");
      GWEN_Buffer_AppendString(buf, I18N("severity"));
      GWEN_Buffer_AppendString(buf, " \"");
      GWEN_Buffer_AppendString(buf, xg->severity);
      GWEN_Buffer_AppendString(buf, "\"");
    }
    
    if (e && e->name) {
      GWEN_Buffer_AppendString(buf, ")");
    }

    /* append error description if available */
    if (e && e->description) {
      GWEN_Buffer_AppendString(buf, "\n");
      GWEN_Buffer_AppendString(buf, I18N(e->description));
    }
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "%s", GWEN_Buffer_GetStart(buf));
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }
  if (xg->oldEndTagFn)
    return xg->oldEndTagFn(g, tagName);
  else
    return 1;
}



int AIO_OfxGroup_STATUS_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_STATUS *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STATUS, g);
  assert(xg);

  if (xg->currentElement) {
    GWEN_BUFFER *buf;
    int rv;
    const char *s;

    buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
    rv=AIO_OfxXmlCtx_SanitizeData(AIO_OfxGroup_GetXmlContext(g), data, buf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    s=GWEN_Buffer_GetStart(buf);
    if (*s) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
	       "AddData: %s=[%s]", xg->currentElement, s);
      if (strcasecmp(xg->currentElement, "CODE")==0) {
	if (1!=sscanf(s, "%d", &xg->code)) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Bad data for element [%s]",
		    xg->currentElement);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
      }
      else if (strcasecmp(xg->currentElement, "SEVERITY")==0) {
	free(xg->severity);
	xg->severity=strdup(GWEN_Buffer_GetStart(buf));
      }
      else {
	DBG_INFO(AQBANKING_LOGDOMAIN,
		 "Ignoring data for unknown element [%s]",
		 xg->currentElement);
      }
    }
    GWEN_Buffer_free(buf);
  }

  return 0;
}





