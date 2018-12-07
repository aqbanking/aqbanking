

#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>



int EBC_Provider_XchgHkdRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u) {
  int rv;
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=EBC_Provider_XchgDownloadRequest(pro, sess, u, "HKD", buf, 0, NULL, NULL);
  if (rv<0 || rv>=300) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }
  else {
    xmlDocPtr orderDoc=NULL;
    xmlNodePtr root_node=NULL;
    xmlNodePtr node=NULL;
    xmlNodePtr nodeX=NULL;
    GWEN_DB_NODE *dbAll;
    GWEN_DB_NODE *db;

    /* parse XML document */
    rv=EB_Xml_DocFromBuffer(GWEN_Buffer_GetStart(buf),
			    GWEN_Buffer_GetUsedBytes(buf),
			    &orderDoc);
    GWEN_Buffer_free(buf);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* get keys */
    root_node=xmlDocGetRootElement(orderDoc);

    /* get auth key */
    node=EB_Xml_GetNode(root_node, "PartnerInfo",
			GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (node==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"No PartnerInfo found");
      xmlFreeDoc(orderDoc);
      return GWEN_ERROR_BAD_DATA;
    }

    dbAll=GWEN_DB_Group_new("HKDResponse");

    /* sample accounts */
    nodeX=node->children;
    while(nodeX) {
      if (nodeX->type==XML_ELEMENT_NODE) {
	if (nodeX->name && strcmp((const char*)nodeX->name, "AccountInfo")==0) {
	  xmlChar *xs;
	  xmlNodePtr nodeXX;

	  DBG_DEBUG(AQEBICS_LOGDOMAIN, "Reading AccountInfo node");

	  db=GWEN_DB_GetGroup(dbAll, GWEN_PATH_FLAGS_CREATE_GROUP, "Account");

	  xs=xmlGetProp(nodeX, BAD_CAST "ID");
	  if (xs) {
	    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "EbicsId", (const char*)xs);
            xmlFree(xs);
	  }

	  xs=xmlGetProp(nodeX, BAD_CAST "Currency");
	  if (xs) {
	    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "Currency", (const char*)xs);
            xmlFree(xs);
	  }

	  xs=xmlGetProp(nodeX, BAD_CAST "Description");
	  if (xs) {
	    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "AccountName", (const char*)xs);
	    xmlFree(xs);
	  }

	  nodeXX=nodeX->children;
	  while (nodeXX) {
	    if (nodeXX->type==XML_ELEMENT_NODE &&
		nodeXX->name) {
	      if (strcmp((const char*)nodeXX->name, "AccountNumber")==0) {
		xs=xmlGetProp(nodeXX, BAD_CAST "international");
		if (xs) {
		  xmlNodePtr nodeXXX=NULL;

		  if (strcasecmp((const char*)xs, "false")==0) {
		    nodeXXX=nodeXX->children;
		    if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
					   "AccountNumber", (const char*)nodeXXX->content);
		    }
		  }
		  else {
		    nodeXXX=nodeXX->children;
		    if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
					   "IBAN", (const char*)nodeXXX->content);
		    }
		  }
		  xmlFree(xs);
		}
	      }
	      else if (strcmp((const char*)nodeXX->name, "NationalAccountNumber")==0) {
		xmlNodePtr nodeXXX=NULL;
  
		nodeXXX=nodeXX->children;
		if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				       "AccountNumber", (const char*)nodeXXX->content);
		}
	      }
	      else if (strcmp((const char*)nodeXX->name, "BankCode")==0) {
		xs=xmlGetProp(nodeXX, BAD_CAST "international");
		if (xs) {
		  xmlNodePtr nodeXXX=NULL;

		  if (strcasecmp((const char*)xs, "false")==0) {
		    nodeXXX=nodeXX->children;
		    if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
					   "BankCode", (const char*)nodeXXX->content);
		    }
		  }
		  else {
		    nodeXXX=nodeXX->children;
		    if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
					   "BIC", (const char*)nodeXXX->content);
		    }
		  }
		  xmlFree(xs);
		}
	      }
	      else if (strcmp((const char*)nodeXX->name, "NationalBankCode")==0) {
		xmlNodePtr nodeXXX=NULL;

		nodeXXX=nodeXX->children;
		if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				       "BankCode", (const char*)nodeXXX->content);
		}
	      }
	      else if (strcmp((const char*)nodeXX->name, "AccountHolder")==0) {
		xmlNodePtr nodeXXX=NULL;

		nodeXXX=nodeXX->children;
		if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
		  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				       "owner", (const char*)nodeXXX->content);
		}
	      }
	    }
	    nodeXX=nodeXX->next;
	  }
	}
      }
      nodeX=nodeX->next;
    }

    /* add all accounts which are complete */
    db=GWEN_DB_FindFirstGroup(dbAll, "Account");
    while(db) {
      const char *ebicsId;
      const char *bankCode;
      const char *accountNumber;
      const char *owner;
      const char *descr;
      const char *currency;
      const char *bic;
      const char *iban;
      AB_ACCOUNT *a=NULL;

      ebicsId=GWEN_DB_GetCharValue(db, "ebicsId", 0, NULL);
      bankCode=GWEN_DB_GetCharValue(db, "bankCode", 0, NULL);
      accountNumber=GWEN_DB_GetCharValue(db, "accountNumber", 0, NULL);
      owner=GWEN_DB_GetCharValue(db, "owner", 0, NULL);
      if (owner==NULL)
        owner=AB_User_GetUserName(u);
      descr=GWEN_DB_GetCharValue(db, "accountName", 0, NULL);
      currency=GWEN_DB_GetCharValue(db, "currency", 0, NULL);
      if (currency==NULL)
        currency="EUR";
      iban=GWEN_DB_GetCharValue(db, "IBAN", 0, NULL);
      bic=GWEN_DB_GetCharValue(db, "BIC", 0, NULL);

      if (bankCode && accountNumber)
	a=AB_Banking_FindAccount(AB_Provider_GetBanking(pro),
				 "aqebics",
				 "de",
				 bankCode,
				 accountNumber,
                                 "*");
      else if (iban)
	a=AB_Banking_GetAccountByIban(AB_Provider_GetBanking(pro), iban);

      if (!a) {
	char lbuf[256];

	DBG_INFO(AQEBICS_LOGDOMAIN,
		 "Adding account %s / %s", bankCode, accountNumber);

	snprintf(lbuf, sizeof(lbuf)-1,
		 I18N("Adding account %s /%s"),
		 bankCode, accountNumber);
	lbuf[sizeof(lbuf)-1]=0;
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, lbuf);
	a=AB_Banking_CreateAccount(AB_Provider_GetBanking(pro), "aqebics");
	assert(a);
	AB_Account_SetAccountType(a, AB_AccountType_Bank);
	AB_Account_SetBankCode(a, bankCode);
	AB_Account_SetAccountNumber(a, accountNumber);
	if (descr)
	  AB_Account_SetAccountName(a, descr);
	if (owner)
	  AB_Account_SetOwnerName(a, owner);
	if (currency)
	  AB_Account_SetCurrency(a, currency);
	AB_Account_SetCountry(a, "de");

	if (iban)
	  AB_Account_SetIBAN(a, iban);
	if (bic)
	  AB_Account_SetBIC(a, bic);

	if (ebicsId)
	  EBC_Account_SetEbicsId(a, ebicsId);

	AB_Account_SetUser(a, u);
	AB_Account_SetSelectedUser(a, u);

	rv=AB_Banking_AddAccount(AB_Provider_GetBanking(pro), a);
	if (rv<0) {
	  DBG_WARN(AQEBICS_LOGDOMAIN, "Could not add account %s / %s (%d)",
		   bankCode, accountNumber, rv);
	}
      }
      else {
	char lbuf[256];

	DBG_INFO(AQEBICS_LOGDOMAIN, "Account %s / %s already exists",
		 bankCode, accountNumber);
	snprintf(lbuf, sizeof(lbuf)-1,
		 I18N("Account %s / %s already exists"),
		 bankCode, accountNumber);
	lbuf[sizeof(lbuf)-1]=0;
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, lbuf);
      }

      db=GWEN_DB_FindNextGroup(db, "Account");
    }

    xmlFreeDoc(orderDoc);
    return 0;
  }


}



