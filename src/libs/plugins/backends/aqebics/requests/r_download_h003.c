/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "r_download_l.h"

#include "aqebics/aqebics_l.h"
#include "aqebics/msg/msg.h"
#include "aqebics/msg/keys.h"
#include "aqebics/msg/zip.h"
#include "aqebics/msg/xml.h"
#include "aqebics/client/user_l.h"
#include "aqebics/client/provider_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/httpsession.h>


/* -------------------------------------------------------------------------------------------------------------------------
 * forward declarations
 * -------------------------------------------------------------------------------------------------------------------------
 */

static int _mkDownloadInitRequest(AB_PROVIDER *pro,
                                  AB_USER *u,
                                  const char *requestType,
                                  const GWEN_DATE *fromDate,
                                  const GWEN_DATE *toDate,
                                  EB_MSG **pMsg);

static int _mkDownloadTransferRequest(AB_PROVIDER *pro,
                                      AB_USER *u,
                                      const char *transactionId,
                                      int segmentNumber,
                                      EB_MSG **pMsg);

static int _mkDownloadReceiptRequest(AB_PROVIDER *pro,
                                     AB_USER *u,
                                     const char *transactionId,
                                     int receiptCode,
                                     EB_MSG **pMsg);

static int _xchgDownloadInitRequest(AB_PROVIDER *pro,
                                    GWEN_HTTP_SESSION *sess,
                                    AB_USER *u,
                                    const char *requestType,
                                    const GWEN_DATE *fromDate,
                                    const GWEN_DATE *toDate,
                                    EB_MSG **pMsg);

static int _downloadRemainingSegments(AB_PROVIDER *pro,
                                      GWEN_HTTP_SESSION *sess,
                                      AB_USER *u,
                                      const char *transactionId,
                                      int segmentCount,
                                      GWEN_BUFFER *dbuffer);


static int _sendReceipt(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u, const char *transactionId,
                        int withReceipt);





/* -------------------------------------------------------------------------------------------------------------------------
 * code
 * --------------------------------------------------------------------------------------------------------------------------
 */


int EBC_Provider_XchgDownloadRequest_H003(AB_PROVIDER *pro,
                                          GWEN_HTTP_SESSION *sess,
                                          AB_USER *u,
                                          const char *requestType,
                                          GWEN_BUFFER *targetBuffer,
                                          int withReceipt,
                                          const GWEN_DATE *fromDate,
                                          const GWEN_DATE *toDate)
{
  int rv;
  EB_MSG *mRsp=NULL;
  GWEN_CRYPT_KEY *skey=NULL;
  GWEN_BUFFER *dbuffer;
  int segmentCount;
  const char *s;
  char transactionId[36];

  /* exchange initial request */
  rv=_xchgDownloadInitRequest(pro, sess, u, requestType, fromDate, toDate, &mRsp);
  if (rv<0 || rv>=300) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* extract key from response */
  skey=EB_Msg_ExtractAndDecodeSessionKey(mRsp, pro, u);
  if (skey==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  /* extract transaction id */
  s=EB_Msg_GetCharValue(mRsp, "header/static/TransactionID", NULL);
  if (s==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_Key_free(skey);
    EB_Msg_free(mRsp);
    return rv;
  }
  strncpy(transactionId, s, sizeof(transactionId)-1);
  transactionId[sizeof(transactionId)-1]=0;

  /* extract number of segments */
  segmentCount=EB_Msg_GetIntValue(mRsp, "header/static/NumSegments", 0);
  if (segmentCount==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid segment count zero");
    GWEN_Crypt_Key_free(skey);
    EB_Msg_free(mRsp);
    return rv;
  }

  /* create data buffer */
  dbuffer=GWEN_Buffer_new(0, 1024, 0, 1);

  /* read first chunk of data */
  s=EB_Msg_GetCharValue(mRsp, "body/DataTransfer/OrderData", NULL);
  if (!s) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad message from server: Missing OrderData");
    GWEN_Buffer_free(dbuffer);
    GWEN_Crypt_Key_free(skey);
    EB_Msg_free(mRsp);
    return GWEN_ERROR_BAD_DATA;
  }
  GWEN_Buffer_AppendString(dbuffer, s);

  EB_Msg_free(mRsp);

  /* read remaining segments if any */
  if (segmentCount>1) {
    rv=_downloadRemainingSegments(pro, sess, u, transactionId, segmentCount, dbuffer);
    if (rv<0 || rv>=300) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuffer);
      GWEN_Crypt_Key_free(skey);
      return rv;
    }
  }

  /* decode and decrypt received data */
  rv=EBC_Provider_DecodeAndDecryptData(pro, u, skey, GWEN_Buffer_GetStart(dbuffer), targetBuffer);
  if (rv<0 || rv>=300) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuffer);
    GWEN_Crypt_Key_free(skey);
    return rv;
  }

  GWEN_Buffer_free(dbuffer);
  GWEN_Crypt_Key_free(skey);

  /* send receipt */
  rv=_sendReceipt(pro, sess, u, transactionId, withReceipt);
  if (rv<0 || rv>=300) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _xchgDownloadInitRequest(AB_PROVIDER *pro,
                             GWEN_HTTP_SESSION *sess,
                             AB_USER *u,
                             const char *requestType,
                             const GWEN_DATE *fromDate,
                             const GWEN_DATE *toDate,
                             EB_MSG **pMsg)
{
  int rv;
  EB_MSG *msg=NULL;
  EB_MSG *mRsp;
  EB_RC rc;

  /* create initialisation request */
  rv=_mkDownloadInitRequest(pro, u, requestType, fromDate, toDate, &msg);
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

  *pMsg=mRsp;

  return rv;
}



int _downloadRemainingSegments(AB_PROVIDER *pro,
                               GWEN_HTTP_SESSION *sess,
                               AB_USER *u,
                               const char *transactionId,
                               int segmentCount,
                               GWEN_BUFFER *dbuffer)
{
  int segmentNumber;

  segmentNumber=2;
  for (;;) {
    EB_MSG *msg=NULL;
    EB_MSG *mRsp=NULL;
    int i;
    EB_RC rc;
    int rv;
    const char *s;

    rv=_mkDownloadTransferRequest(pro, u, transactionId, segmentNumber, &msg);
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

    i=EB_Msg_GetIntValue(mRsp, "header/mutable/SegmentNumber", 0);
    if (i!=segmentNumber) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Unexpected segment number (%d, expected %d)", i, segmentNumber);
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }

    /* read next chunk of data */
    s=EB_Msg_GetCharValue(mRsp, "body/DataTransfer/OrderData", NULL);
    if (!s) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad message from server: Missing OrderData");
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_Buffer_AppendString(dbuffer, s);
    EB_Msg_free(mRsp);

    segmentNumber++;
    if (segmentNumber>=segmentCount) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Transfer finished");
      break;
    }
  } /* for */

  return 0;
}



int _sendReceipt(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u, const char *transactionId, int withReceipt)
{
  EB_MSG *msg=NULL;
  EB_MSG *mRsp=NULL;
  int rv;

  /* send receipt message */
  rv=_mkDownloadReceiptRequest(pro, u, transactionId, withReceipt?0:1, &msg);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* exchange requests */
  rv=EBC_Dialog_ExchangeMessagesAndCheckResponse(sess, msg, &mRsp);
  if (rv<0 || rv>=300) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_free(msg);
  EB_Msg_free(mRsp);

  return rv;
}




int _mkDownloadInitRequest(AB_PROVIDER *pro,
                           AB_USER *u,
                           const char *requestType,
                           const GWEN_DATE *fromDate,
                           const GWEN_DATE *toDate,
                           EB_MSG **pMsg)
{
  int rv;
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

  userId=AB_User_GetUserId(u);
  partnerId=AB_User_GetCustomerId(u);
  if (partnerId==NULL)
    partnerId=userId;

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  EB_Xml_Ebicsify(root_node, "H003");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  s=EBC_User_GetPeerId(u);
  nodeXX=xmlNewTextChild(nodeX, NULL, BAD_CAST "HostID", BAD_CAST(s?s:"EBICS"));

  /* generate Nonce */
  tbuf=GWEN_Buffer_new(0, 128, 0, 1);
  rv=EBC_Provider_GenerateNonce(pro, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  nodeXX=xmlNewTextChild(nodeX, NULL, BAD_CAST "Nonce", BAD_CAST GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* generate timestamp */
  rv=EBC_Provider_GenerateTimeStamp(pro, u, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  nodeXX=xmlNewTextChild(nodeX, NULL, BAD_CAST "Timestamp", BAD_CAST GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  nodeXX=xmlNewTextChild(nodeX, NULL, BAD_CAST "PartnerID", BAD_CAST partnerId);
  nodeXX=xmlNewTextChild(nodeX, NULL, BAD_CAST "UserID", BAD_CAST userId);

  /* order details */
  nodeXX=xmlNewChild(nodeX, NULL, BAD_CAST "OrderDetails", NULL);
  xmlNewTextChild(nodeXX, NULL, BAD_CAST "OrderType", BAD_CAST requestType);
  xmlNewTextChild(nodeXX, NULL, BAD_CAST "OrderAttribute", BAD_CAST "DZHNN");

  /* order params */
  nodeXXX=xmlNewChild(nodeXX, NULL, BAD_CAST "StandardOrderParams", NULL);
  if (fromDate || toDate) {
    xmlNodePtr nodeXXXX;
    GWEN_DATE *tempTime=NULL;
    const GWEN_DATE *t1;
    const GWEN_DATE *t2;

    t1=fromDate;
    t2=toDate;

    if (t1==NULL)
      /* no fromDate, use toDate for both */
      t1=t2;
    if (t2==NULL) {
      /* no toDate, use current date */
      tempTime=GWEN_Date_CurrentDate();
      t2=tempTime;
    }

    nodeXXXX=xmlNewChild(nodeXXX, NULL, BAD_CAST "DateRange", NULL);
    if (t1) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Date_toStringWithTemplate(t1, "YYYY-MM-DD", tbuf);
      xmlNewTextChild(nodeXXXX, NULL, BAD_CAST "Start", BAD_CAST GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    if (t2) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Date_toStringWithTemplate(t2, "YYYY-MM-DD", tbuf);
      xmlNewTextChild(nodeXXXX, NULL, BAD_CAST "End", BAD_CAST GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    if (tempTime)
      GWEN_Date_free(tempTime);
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
  xmlNewTextChild(nodeX, NULL, BAD_CAST "SecurityMedium", BAD_CAST "0000");

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL, BAD_CAST "TransactionPhase", BAD_CAST "Initialisation");


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



int _mkDownloadTransferRequest(AB_PROVIDER *pro, AB_USER *u, const char *transactionId, int segmentNumber,
                               EB_MSG **pMsg)
{
  int rv;
  EB_MSG *msg;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr sigNode = NULL;
  const char *s;

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  EB_Xml_Ebicsify(root_node, "H003");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  s=EBC_User_GetPeerId(u);
  xmlNewTextChild(nodeX, NULL, BAD_CAST "HostID", BAD_CAST(s?s:"EBICS"));

  xmlNewTextChild(nodeX, NULL, BAD_CAST "TransactionID", BAD_CAST transactionId);

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL, BAD_CAST "TransactionPhase", BAD_CAST "Transfer");
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



int _mkDownloadReceiptRequest(AB_PROVIDER *pro, AB_USER *u, const char *transactionId, int receiptCode, EB_MSG **pMsg)
{
  int rv;
  EB_MSG *msg;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr sigNode = NULL;
  const char *s;

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsRequest");
  xmlDocSetRootElement(doc, root_node);
  EB_Xml_Ebicsify(root_node, "H003");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  s=EBC_User_GetPeerId(u);
  xmlNewTextChild(nodeX, NULL, BAD_CAST "HostID", BAD_CAST(s?s:"EBICS"));
  xmlNewTextChild(nodeX, NULL, BAD_CAST "TransactionID", BAD_CAST transactionId);

  /* mutable */
  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewTextChild(nodeX, NULL, BAD_CAST "TransactionPhase", BAD_CAST "Receipt");

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










