
#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"



static int EBC_Provider_MkUploadInitRequest_H002(AB_PROVIDER *pro,
						 GWEN_HTTP_SESSION *sess,
						 AB_USER *u,
						 const char *requestType,
						 GWEN_CRYPT_KEY *skey,
						 const char *pEu,
						 uint32_t dlen,
						 EB_MSG **pMsg) {
  EBC_PROVIDER *dp;
  int rv;
  xmlNsPtr ns;
  EB_MSG *msg;
  const char *userId;
  const char *partnerId;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr nodeXX = NULL;
  xmlNodePtr sigNode = NULL;
  GWEN_BUFFER *tbuf;
  const char *s;
  char numbuf[32];

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  userId=AB_User_GetUserId(u);
  partnerId=AB_User_GetCustomerId(u);
  if (partnerId==NULL)
    partnerId=userId;

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H002",
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
	       BAD_CAST "http://www.ebics.org/H002 "
	       "http://www.ebics.org/H002/ebics_request.xsd");

  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H002");
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
  GWEN_Buffer_Reset(tbuf);

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

  /* generate order id */
  rv=EBC_Provider_Generate_OrderId(pro, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  xmlNewTextChild(nodeXX, NULL,
		  BAD_CAST "OrderID",
		  BAD_CAST GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_NO_EU)
    xmlNewTextChild(nodeXX, NULL,
		    BAD_CAST "OrderAttribute",
		    BAD_CAST "DZHNN");
  else
    xmlNewTextChild(nodeXX, NULL,
		    BAD_CAST "OrderAttribute",
		    BAD_CAST "OZHNN");
  xmlNewChild(nodeXX, NULL, BAD_CAST "StandardOrderParams", NULL);

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

  snprintf(numbuf, sizeof(numbuf)-1, "%i", (dlen+(1024*1024)-1)/(1024*1024));
  numbuf[sizeof(numbuf)-1]=0;
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "NumSegments",
		  BAD_CAST numbuf);


  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "TransactionPhase",
		  BAD_CAST "Initialisation");


  /* prepare signature node */
  sigNode=xmlNewChild(root_node, NULL, BAD_CAST "AuthSignature", NULL);

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  /* data transfer */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "DataTransfer", NULL);

  /* add session key and info */
  nodeXX=xmlNewChild(nodeX, NULL, BAD_CAST "DataEncryptionInfo", NULL);
  xmlNewProp(nodeXX, BAD_CAST "authenticate", BAD_CAST "true");
  rv=EBC_Provider_FillDataEncryptionInfoNode(pro, u, skey, nodeXX);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }

  if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_NO_EU)) {
    /* add EU */
    nodeXX=xmlNewTextChild(nodeX, NULL,
			   BAD_CAST "SignatureData",
			   BAD_CAST pEu);
    xmlNewProp(nodeXX, BAD_CAST "authenticate", BAD_CAST "true");
  }

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



static int EBC_Provider_MkUploadTransferRequest_H002(AB_PROVIDER *pro,
						     GWEN_HTTP_SESSION *sess,
						     AB_USER *u,
						     const char *transactionId,
						     const char *pData,
						     uint32_t lData,
						     int segmentNumber,
						     int isLast,
						     EB_MSG **pMsg) {
  EBC_PROVIDER *dp;
  int rv;
  xmlNsPtr ns;
  EB_MSG *msg;
  const char *userId;
  const char *partnerId;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr nodeXX = NULL;
  xmlNodePtr sigNode = NULL;
  GWEN_BUFFER *tbuf;
  const char *s;
  char numbuf[32];

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  userId=AB_User_GetUserId(u);
  partnerId=AB_User_GetCustomerId(u);
  if (partnerId==NULL)
    partnerId=userId;

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H002",
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
	       BAD_CAST "http://www.ebics.org/H002 "
	       "http://www.ebics.org/H002/ebics_request.xsd");

  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H002");
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
  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "TransactionID",
			 BAD_CAST transactionId);

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL,
		  BAD_CAST "TransactionPhase",
		  BAD_CAST "Transfer");
  snprintf(numbuf, sizeof(numbuf)-1, "%d", segmentNumber);
  numbuf[sizeof(numbuf)-1]=0;
  nodeXX=xmlNewTextChild(nodeX, NULL,
			 BAD_CAST "SegmentNumber",
			 BAD_CAST numbuf);
  xmlNewProp(nodeXX, BAD_CAST "lastSegment",
	     BAD_CAST (isLast?"true":"false"));

  /* prepare signature node */
  sigNode=xmlNewChild(root_node, NULL, BAD_CAST "AuthSignature", NULL);

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  /* data transfer */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "DataTransfer", NULL);
  tbuf=GWEN_Buffer_new(0, lData, 0, 1);
  GWEN_Buffer_AppendBytes(tbuf, pData, lData);
  nodeXX=xmlNewChild(nodeX, NULL, BAD_CAST "OrderData",
		     BAD_CAST GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

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



int EBC_Provider_XchgUploadRequest_H002(AB_PROVIDER *pro,
					GWEN_HTTP_SESSION *sess,
					AB_USER *u,
					const char *requestType,
					const uint8_t *pData,
					uint32_t lData) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_KEY *skey;
  GWEN_BUFFER *euBuf=NULL;
  GWEN_BUFFER *dbuf;
  EB_MSG *msg=NULL;
  EB_MSG *mRsp;
  uint32_t numSegs;
  uint32_t i;
  EB_RC rc;
  GWEN_BUFFER *logbuf;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  logbuf=GWEN_Buffer_new(0, 128, 0, 1);

  /* generate session key */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Generating session key");
  skey=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
  if (skey==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unable to generate DES key");
    return GWEN_ERROR_GENERIC;
  }

  if (!(EBC_User_GetFlags(u) & EBC_USER_FLAGS_NO_EU)) {
    /* generate electronic signature */
    DBG_INFO(AQEBICS_LOGDOMAIN, "Generating electronic signature for user [%s]",
	     AB_User_GetUserId(u));
    euBuf=GWEN_Buffer_new(0, 1024, 0, 1);
    rv=EBC_Provider_MkEuCryptZipDoc(pro, u, requestType, pData, lData, skey, euBuf);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(euBuf);
      GWEN_Crypt_Key_free(skey);
      GWEN_Buffer_AppendString(logbuf, I18N("\tError signing upload document"));
      GWEN_Buffer_AppendString(logbuf, " (");
      GWEN_Buffer_AppendString(logbuf, AB_User_GetUserId(u));
      GWEN_Buffer_AppendString(logbuf, ")\n");
      Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
      GWEN_Buffer_free(logbuf);
      return rv;
    }
    GWEN_Buffer_AppendString(logbuf, I18N("\tUpload document signed"));
    GWEN_Buffer_AppendString(logbuf, " (");
    GWEN_Buffer_AppendString(logbuf, AB_User_GetUserId(u));
    GWEN_Buffer_AppendString(logbuf, ")\n");
  }

  /* encrypt and encode data */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Encrypting, zipping and encoding upload data");
  dbuf=GWEN_Buffer_new(0, (lData*4)/3, 0, 1);
  rv=EBC_Provider_EncryptData(pro, u, skey, pData, lData, dbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    GWEN_Buffer_free(euBuf);
    GWEN_Crypt_Key_free(skey);
    GWEN_Buffer_AppendString(logbuf, I18N("\tError encrypting upload document\n"));
    Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
    GWEN_Buffer_free(logbuf);
    return rv;
  }
  GWEN_Buffer_AppendString(logbuf, I18N("\tUpload document encrypted\n"));

  numSegs=(GWEN_Buffer_GetUsedBytes(dbuf)+(1024*1024)-1)/(1024*1024);

  /* create upload init request */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Generating upload init request");
  if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_NO_EU)
    rv=EBC_Provider_MkUploadInitRequest_H002(pro, sess, u, requestType,
					     skey,
					     NULL, /* no EU */
					     GWEN_Buffer_GetUsedBytes(dbuf),
					     &msg);
  else
    rv=EBC_Provider_MkUploadInitRequest_H002(pro, sess, u, requestType,
					     skey,
					     GWEN_Buffer_GetStart(euBuf),
					     GWEN_Buffer_GetUsedBytes(dbuf),
					     &msg);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    GWEN_Buffer_free(euBuf);
    GWEN_Crypt_Key_free(skey);
    Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
    GWEN_Buffer_free(logbuf);
    return rv;
  }

  /* exchange requests */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Exchanging upload init request");
  GWEN_Buffer_AppendString(logbuf, I18N("\tExchanging upload init request"));
  rv=EBC_Dialog_ExchangeMessages(sess, msg, &mRsp);
  if (rv<0 || rv>=300) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
    EB_Msg_free(msg);
    GWEN_Buffer_free(dbuf);
    GWEN_Buffer_free(euBuf);
    GWEN_Crypt_Key_free(skey);
    Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
    GWEN_Buffer_free(logbuf);
    return rv;
  }
  EB_Msg_free(msg);
  GWEN_Buffer_free(euBuf);
  GWEN_Crypt_Key_free(skey);

  /* check response */
  assert(mRsp);

  /* log results */
  EBC_Provider_LogRequestResults(pro, mRsp, logbuf);

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
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
      EB_Msg_free(mRsp);
      GWEN_Buffer_free(dbuf);
      Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
      GWEN_Buffer_free(logbuf);
      if ((rc & 0xfff00)==0x091300 ||
	  (rc & 0xfff00)==0x091200)
	return AB_ERROR_SECURITY;
      else
	return GWEN_ERROR_GENERIC;
    }
  }
  if (1) {
    const char *s;
    char transactionId[36];
    const char *p;
    uint32_t bytesLeft;

    /* extract transaction id */
    s=EB_Msg_GetCharValue(mRsp, "header/static/TransactionID", NULL);
    if (s==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      EB_Msg_free(mRsp);
      GWEN_Buffer_free(dbuf);
      Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
      GWEN_Buffer_free(logbuf);
      return rv;
    }
    strncpy(transactionId, s, sizeof(transactionId)-1);
    EB_Msg_free(mRsp);

    /* write data */
    p=GWEN_Buffer_GetStart(dbuf);
    bytesLeft=GWEN_Buffer_GetUsedBytes(dbuf);
    for (i=0; i<numSegs; i++) {
      uint32_t n;

      n=1024*1024;
      if (n>bytesLeft)
	n=bytesLeft;
      assert(n);
      DBG_INFO(AQEBICS_LOGDOMAIN, "Generating upload transfer request");
      rv=EBC_Provider_MkUploadTransferRequest_H002(pro, sess, u,
						   transactionId,
						   p,
						   n,
						   i+1,
						   (i==numSegs-1)?1:0,
						   &msg);
      if (rv<0) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(dbuf);
	Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
	GWEN_Buffer_free(logbuf);
	return rv;
      }

      /* exchange requests */
      DBG_INFO(AQEBICS_LOGDOMAIN, "Exchanging upload transfer request");
      GWEN_Buffer_AppendString(logbuf, I18N("\tExchanging upload transfer request"));
      rv=EBC_Dialog_ExchangeMessages(sess, msg, &mRsp);
      if (rv<0 || rv>=300) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
	EB_Msg_free(msg);
	GWEN_Buffer_free(dbuf);
	Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
	GWEN_Buffer_free(logbuf);
	return rv;
      }
      EB_Msg_free(msg);

      /* check response */
      assert(mRsp);

      /* log results */
      EBC_Provider_LogRequestResults(pro, mRsp, logbuf);

      rc=EB_Msg_GetResultCode(mRsp);
      if ((rc & 0xff0000)==0x090000 ||
	  (rc & 0xff0000)==0x060000) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
	EB_Msg_free(mRsp);
	GWEN_Buffer_free(dbuf);
	Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
	GWEN_Buffer_free(logbuf);
	return AB_ERROR_SECURITY;
      }

      /* prepare next round */
      EB_Msg_free(mRsp);
      p+=n;
      bytesLeft-=n;
    } /* for */
  }

  GWEN_Buffer_free(dbuf);
  DBG_INFO(AQEBICS_LOGDOMAIN, "Upload finished");
  GWEN_Buffer_AppendString(logbuf, I18N("\tUpload finished"));
  Ab_HttpSession_AddLog(sess, GWEN_Buffer_GetStart(logbuf));
  GWEN_Buffer_free(logbuf);

  return 0;
}






