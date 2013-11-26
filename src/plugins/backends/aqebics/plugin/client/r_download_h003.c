

#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>



static int EBC_Provider_MkDownloadInitRequest_H003(AB_PROVIDER *pro,
						   GWEN_HTTP_SESSION *sess,
						   AB_USER *u,
						   const char *requestType,
						   const GWEN_TIME *fromTime,
						   const GWEN_TIME *toTime,
						   EB_MSG **pMsg) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  xmlNsPtr ns;
  EB_MSG *msg;
  const char *userId;
  const char *partnerId;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr nodeXX = NULL;
  xmlNodePtr nodeXXX = NULL;
  xmlNodePtr sigNode = NULL;
  GWEN_BUFFER *tbuf;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  userId=AB_User_GetUserId(u);
  partnerId=AB_User_GetCustomerId(u);
  if (partnerId==NULL)
    partnerId=userId;

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H003",
	      NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
	      BAD_CAST "ds");
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
	      BAD_CAST "xsi");
  xmlNewNsProp(root_node,
	       ns,
	       BAD_CAST "schemaLocation", /* xsi:schemaLocation */
	       BAD_CAST "http://www.ebics.org/H003 "
	       "http://www.ebics.org/H003/ebics_request.xsd");

  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H003");
  xmlNewProp(root_node, BAD_CAST "Revision", BAD_CAST "1");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  s=EBC_User_GetPeerId(u);
  if (!s)
    s="EBICS";
  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "HostID",
			 BAD_CAST s);

  /* generate Nonce */
  tbuf=GWEN_Buffer_new(0, 128, 0, 1);
  rv=EBC_Provider_GenerateNonce(pro, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "Nonce",
			 BAD_CAST GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* generate timestamp */
  rv=EBC_Provider_GenerateTimeStamp(pro, u, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "Timestamp",
			 BAD_CAST GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "PartnerID",
			 BAD_CAST partnerId);

  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "UserID",
			 BAD_CAST userId);

  /* order details */
  nodeXX=xmlNewChild(nodeX, NULL, BAD_CAST "OrderDetails", NULL);
  xmlNewTextChild(nodeXX, NULL,
		  BAD_CAST "OrderType",
		  BAD_CAST requestType);
  xmlNewTextChild(nodeXX, NULL,
		  BAD_CAST "OrderAttribute",
		  BAD_CAST "DZHNN");
  nodeXXX=xmlNewChild(nodeXX, NULL, BAD_CAST "StandardOrderParams", NULL);
  if (fromTime || toTime) {
    xmlNodePtr nodeXXXX;
    GWEN_TIME *tempTime=NULL;
    const GWEN_TIME *t1;
    const GWEN_TIME *t2;

    t1=fromTime;
    t2=toTime;

    if (t1==NULL)
      /* no fromDate, use toDate for both */
      t1=t2;
    if (t2==NULL) {
      /* no toDate, use current date */
      tempTime=GWEN_CurrentTime();
      t2=tempTime;
    }

    nodeXXXX=xmlNewChild(nodeXXX, NULL, BAD_CAST "DateRange", NULL);
    if (t1) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Time_toString(t1, "YYYY-MM-DD", tbuf);
      xmlNewTextChild(nodeXXXX, NULL,
		      BAD_CAST "Start",
		      BAD_CAST GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    if (t2) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Time_toString(t2, "YYYY-MM-DD", tbuf);
      xmlNewTextChild(nodeXXXX, NULL,
		      BAD_CAST "End",
		      BAD_CAST GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    if (tempTime)
      GWEN_Time_free(tempTime);
  }

  /* bank pubkey digests */
  nodeXX=xmlNewChild(nodeX, NULL, BAD_CAST "BankPubKeyDigests", NULL);
  rv=EBC_Provider_AddBankPubKeyDigests(pro, u, nodeXX);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }

  /* security medium */
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "SecurityMedium",
		  BAD_CAST "0000");

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "TransactionPhase",
		  BAD_CAST "Initialisation");


  /* prepare signature node */
  sigNode=xmlNewChild(root_node, NULL, BAD_CAST "AuthSignature", NULL);

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  /* sign */
  rv=EBC_Provider_SignMessage(pro, msg, u, sigNode);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }

  *pMsg=msg;
  return 0;
}



static int EBC_Provider_MkDownloadTransferRequest_H003(AB_PROVIDER *pro,
						       GWEN_HTTP_SESSION *sess,
						       AB_USER *u,
						       const char *transactionId,
						       int segmentNumber,
						       EB_MSG **pMsg) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  xmlNsPtr ns;
  EB_MSG *msg;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr sigNode = NULL;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H003",
	      NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
	      BAD_CAST "ds");
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
	      BAD_CAST "xsi");
  xmlNewNsProp(root_node,
	       ns,
	       BAD_CAST "schemaLocation", /* xsi:schemaLocation */
	       BAD_CAST "http://www.ebics.org/H003 "
	       "http://www.ebics.org/H003/ebics_request.xsd");

  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H003");
  xmlNewProp(root_node, BAD_CAST "Revision", BAD_CAST "1");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  s=EBC_User_GetPeerId(u);
  if (!s)
    s="EBICS";
  xmlNewTextChild(nodeX, NULL,
                  BAD_CAST "HostID",
                  BAD_CAST s);

  xmlNewTextChild(nodeX, NULL,
                  BAD_CAST "TransactionID",
                  BAD_CAST transactionId);

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "TransactionPhase",
		  BAD_CAST "Transfer");
  EB_Msg_SetIntValue(msg, "header/mutable/SegmentNumber", segmentNumber);

  /* prepare signature node */
  sigNode=xmlNewChild(root_node, NULL, BAD_CAST "AuthSignature", NULL);

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  /* sign */
  rv=EBC_Provider_SignMessage(pro, msg, u, sigNode);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }

  *pMsg=msg;
  return 0;
}



static int EBC_Provider_MkDownloadReceiptRequest_H003(AB_PROVIDER *pro,
						      GWEN_HTTP_SESSION *sess,
						      AB_USER *u,
						      const char *transactionId,
						      int receiptCode,
						      EB_MSG **pMsg) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  xmlNsPtr ns;
  EB_MSG *msg;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr sigNode = NULL;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H003",
	      NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
	      BAD_CAST "ds");
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
	      BAD_CAST "xsi");
  xmlNewNsProp(root_node,
	       ns,
	       BAD_CAST "schemaLocation", /* xsi:schemaLocation */
	       BAD_CAST "http://www.ebics.org/H003 "
	       "http://www.ebics.org/H003/ebics_request.xsd");

  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H003");
  xmlNewProp(root_node, BAD_CAST "Revision", BAD_CAST "1");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  s=EBC_User_GetPeerId(u);
  if (!s)
    s="EBICS";
  xmlNewTextChild(nodeX, NULL,
                  BAD_CAST "HostID",
                  BAD_CAST s);

  xmlNewTextChild(nodeX, NULL,
                  BAD_CAST "TransactionID",
                  BAD_CAST transactionId);

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "TransactionPhase",
		  BAD_CAST "Receipt");

  /* prepare signature node */
  sigNode=xmlNewChild(root_node, NULL, BAD_CAST "AuthSignature", NULL);

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);
  nodeX=xmlNewChild(node, NULL, BAD_CAST "TransferReceipt", NULL);
  xmlNewProp(nodeX, BAD_CAST "authenticate", BAD_CAST "true");
  EB_Msg_SetIntValue(msg, "body/TransferReceipt/ReceiptCode", receiptCode);

  /* sign */
  rv=EBC_Provider_SignMessage(pro, msg, u, sigNode);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }

  *pMsg=msg;
  return 0;
}




int EBC_Provider_XchgDownloadRequest_H003(AB_PROVIDER *pro,
					  GWEN_HTTP_SESSION *sess,
					  AB_USER *u,
					  const char *requestType,
					  GWEN_BUFFER *targetBuffer,
					  int withReceipt,
					  const GWEN_TIME *fromTime,
					  const GWEN_TIME *toTime) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  EB_MSG *msg=NULL;
  EB_MSG *mRsp;
  EB_RC rc;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* create initialisation request */
  rv=EBC_Provider_MkDownloadInitRequest_H003(pro, sess, u, requestType,
					     fromTime, toTime,
					     &msg);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* exchange requests */
  rv=EBC_Dialog_ExchangeMessages(sess, msg, &mRsp);
  if (rv<0 || rv>=300) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_free(msg);

  /* check response */
  assert(mRsp);

  /* log results */
  EBC_Provider_LogRequestResults(pro, mRsp, NULL);

  rc=EB_Msg_GetResultCode(mRsp);
  if ((rc & 0xff0000)==0x090000 ||
      (rc & 0xff0000)==0x060000) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
    EB_Msg_free(mRsp);
    return AB_ERROR_SECURITY;
  }
  rc=EB_Msg_GetBodyResultCode(mRsp);
  if (rc) {
    if ((rc & 0xff0000)==0x090000 ||
	(rc & 0xff0000)==0x060000) {
      EB_Msg_free(mRsp);
      if (rc==0x090005) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "No download data");
	return GWEN_ERROR_NO_DATA;
      }
      else if ((rc & 0xfff00)==0x091300 ||
	       (rc & 0xfff00)==0x091200) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Security error (%06x)", rc);
	return AB_ERROR_SECURITY;
      }
      else {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Generic error (%06x)", rc);
	return GWEN_ERROR_GENERIC;
      }
    }
    else {
      DBG_NOTICE(AQEBICS_LOGDOMAIN, "Response: (%06x)", rc);
    }
  }
  if (1) {
    xmlNodePtr node=NULL;
    GWEN_CRYPT_KEY *skey=NULL;
    GWEN_BUFFER *buf1;
    GWEN_BUFFER *dbuffer;
    int segmentNumber;
    int segmentCount;
    const char *s;
    char transactionId[36];

    /* extract keys and store them */
    node=EB_Xml_GetNode(EB_Msg_GetRootNode(mRsp),
			"body/DataTransfer/DataEncryptionInfo",
			GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (node==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Bad message from server: Missing session key");
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }
    rv=EBC_Provider_ExtractSessionKey(pro, u, node, &skey);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      EB_Msg_free(mRsp);
      return rv;
    }
    DBG_INFO(AQEBICS_LOGDOMAIN, "Got session key");

    s=EB_Msg_GetCharValue(mRsp, "header/static/TransactionID", NULL);
    if (s==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Crypt_Key_free(skey);
      EB_Msg_free(mRsp);
      return rv;
    }
    strncpy(transactionId, s, sizeof(transactionId)-1);

    segmentCount=EB_Msg_GetIntValue(mRsp, "header/static/NumSegments", 0);
    if (segmentCount==0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Crypt_Key_free(skey);
      EB_Msg_free(mRsp);
      return rv;
    }

    dbuffer=GWEN_Buffer_new(0, 1024, 0, 1);
    segmentNumber=1;
    for (;;) {
      int i;

      i=EB_Msg_GetIntValue(mRsp, "header/mutable/SegmentNumber", 0);
      if (i!=segmentNumber) {
	DBG_ERROR(AQEBICS_LOGDOMAIN,
		  "Unexpected segment number (%d, expected %d)", i, segmentNumber);
	GWEN_Buffer_free(dbuffer);
	GWEN_Crypt_Key_free(skey);
	EB_Msg_free(mRsp);
	return GWEN_ERROR_BAD_DATA;
      }

      /* read next chunk of data */
      s=EB_Msg_GetCharValue(mRsp, "body/DataTransfer/OrderData", NULL);
      if (!s) {
	DBG_ERROR(AQEBICS_LOGDOMAIN,
		  "Bad message from server: Missing OrderData");
	GWEN_Buffer_free(dbuffer);
	GWEN_Crypt_Key_free(skey);
	EB_Msg_free(mRsp);
	return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(dbuffer, s);

      if (segmentNumber>=segmentCount) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "Transfer finished");
	break;
      }

      /* exchange next message */
      segmentNumber++;

      rv=EBC_Provider_MkDownloadTransferRequest_H003(pro, sess, u, transactionId, segmentNumber, &msg);
      if (rv<0) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(dbuffer);
	GWEN_Crypt_Key_free(skey);
	return rv;
      }

      /* exchange requests */
      rv=EBC_Dialog_ExchangeMessages(sess, msg, &mRsp);
      if (rv<0 || rv>=300) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
	EB_Msg_free(msg);
	GWEN_Buffer_free(dbuffer);
	GWEN_Crypt_Key_free(skey);
	return rv;
      }
      EB_Msg_free(msg);
    
      /* check response */
      assert(mRsp);
    
      /* log results */
      EBC_Provider_LogRequestResults(pro, mRsp, NULL);

      rc=EB_Msg_GetResultCode(mRsp);
      if ((rc & 0xff0000)==0x090000 ||
	  (rc & 0xff0000)==0x060000) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
	EB_Msg_free(mRsp);
	GWEN_Buffer_free(dbuffer);
	GWEN_Crypt_Key_free(skey);
	return AB_ERROR_SECURITY;
      }
    }
    EB_Msg_free(mRsp);

    /* BASE64-decode receiced data */
    s=GWEN_Buffer_GetStart(dbuffer);
    if (*s==0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Bad message from server: Missing OrderData");
      GWEN_Buffer_free(dbuffer);
      GWEN_Crypt_Key_free(skey);
      return GWEN_ERROR_BAD_DATA;
    }
    buf1=GWEN_Buffer_new(0, strlen(s), 0, 1);
    rv=GWEN_Base64_Decode((const uint8_t*)s, 0, buf1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not decode OrderData (%d)", rv);
      GWEN_Buffer_free(buf1);
      GWEN_Buffer_free(dbuffer);
      GWEN_Crypt_Key_free(skey);
      return rv;
    }
    GWEN_Buffer_free(dbuffer);

    /* decrypt/unzip data */
    rv=EBC_Provider_DecryptData(pro, u, skey,
				(const uint8_t*)GWEN_Buffer_GetStart(buf1),
				GWEN_Buffer_GetUsedBytes(buf1),
				targetBuffer);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not decrypt OrderData (%d)", rv);
      GWEN_Buffer_free(buf1);
      GWEN_Crypt_Key_free(skey);
      return rv;
    }
    GWEN_Crypt_Key_free(skey);

    /*DBG_ERROR(0, "Got this data:");
    GWEN_Buffer_Dump(targetBuffer, stderr, 2);*/

    /* send receipt message */
    rv=EBC_Provider_MkDownloadReceiptRequest_H003(pro, sess, u, transactionId,
						  withReceipt?0:1,
						  &msg);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* exchange requests */
    rv=EBC_Dialog_ExchangeMessages(sess, msg, &mRsp);
    if (rv<0 || rv>=300) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
      EB_Msg_free(msg);
      return rv;
    }
    EB_Msg_free(msg);

    /* check response */
    assert(mRsp);

    /* log results */
    EBC_Provider_LogRequestResults(pro, mRsp, NULL);

    rc=EB_Msg_GetResultCode(mRsp);
    if ((rc & 0xff0000)==0x090000 ||
	(rc & 0xff0000)==0x060000) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
      EB_Msg_free(mRsp);
      return AB_ERROR_SECURITY;
    }

    EB_Msg_free(mRsp);

    return 0;
  }
}






